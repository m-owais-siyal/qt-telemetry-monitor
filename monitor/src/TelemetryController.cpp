#include "TelemetryController.h"

#include "UdpReceiver.h"

#include <QDateTime>
#include <QHostAddress>

#include <algorithm>

TelemetryController::TelemetryController(QObject *parent)
    : QObject(parent)
{
    m_receiver = new UdpReceiver; // no parent: it is moved to the worker thread
    m_receiver->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, m_receiver, &QObject::deleteLater);

    // Both connections are queued, because the receiver lives on another thread.
    connect(m_receiver, &UdpReceiver::batchReady, this, &TelemetryController::onBatch);
    connect(m_receiver, &UdpReceiver::errorOccurred, this, &TelemetryController::onReceiverError);

    m_staleTimer.setSingleShot(true);
    m_staleTimer.setInterval(StaleTimeoutMs);
    connect(&m_staleTimer, &QTimer::timeout, this, &TelemetryController::onStaleTimeout);

    m_thread.setObjectName(QStringLiteral("udp-receiver"));
    m_thread.start();
}

TelemetryController::~TelemetryController()
{
    // The receiver is deleted on its own thread once the event loop exits.
    m_thread.quit();
    m_thread.wait();
}

void TelemetryController::setAddress(const QString &address)
{
    const QString trimmed = address.trimmed();
    if (m_address == trimmed)
        return;
    m_address = trimmed;
    emit addressChanged();
}

void TelemetryController::setPort(int port)
{
    if (m_port == port)
        return;
    m_port = port;
    emit portChanged();
}

void TelemetryController::setWindowSeconds(int seconds)
{
    seconds = std::clamp(seconds, 1, MaxWindowSeconds);
    if (m_windowSeconds == seconds)
        return;
    m_windowSeconds = seconds;
    emit windowSecondsChanged();
    updateCharts();
}

void TelemetryController::start()
{
    if (m_running)
        return;

    const QHostAddress address(m_address);
    // QHostAddress also accepts legacy shorthand such as "239.255.42" (= 239.255.0.42),
    // which is almost always a typo, so IPv4 input must round-trip exactly.
    const bool shorthandIPv4 = address.protocol() == QAbstractSocket::IPv4Protocol && address.toString() != m_address;
    if (address.isNull() || shorthandIPv4) {
        setErrorMessage(tr("\"%1\" is not a valid IP address. Use the full form, e.g. 239.255.42.1.").arg(m_address));
        setStatus(Error);
        return;
    }
    if (m_port < 1 || m_port > 65535) {
        setErrorMessage(tr("Port must be between 1 and 65535."));
        setStatus(Error);
        return;
    }

    setErrorMessage({});
    resetSession();

    const quint64 session = ++m_session;
    const quint16 port = quint16(m_port);
    UdpReceiver *receiver = m_receiver;
    QMetaObject::invokeMethod(receiver, [receiver, address, port, session] {
        receiver->start(address, port, session);
    });

    setRunning(true);
    setStatus(Waiting);
    log(EventLogModel::Info,
        address.isMulticast() ? tr("Joined group %1:%2").arg(m_address).arg(m_port)
                              : tr("Listening on %1:%2").arg(m_address).arg(m_port));
}

void TelemetryController::stop()
{
    if (!m_running)
        return;

    UdpReceiver *receiver = m_receiver;
    QMetaObject::invokeMethod(receiver, [receiver] { receiver->stop(); });

    m_staleTimer.stop();
    setRunning(false);
    setStatus(Idle);
    log(EventLogModel::Info, tr("Stopped"));
}

void TelemetryController::bindSeries(int channel, QXYSeries *series)
{
    if (channel < 0 || channel >= ChannelCount)
        return;
    m_series[size_t(channel)] = series;
    updateCharts();
}

void TelemetryController::onBatch(const ReceiverBatch &batch)
{
    // Batches from a previous session can still be queued after stop() / start().
    if (!m_running || batch.session != m_session)
        return;

    m_stats = batch.stats;
    emit statsChanged();

    m_events.addReceiverEvents(batch.events);

    if (batch.samples.isEmpty())
        return;

    m_channels.addSamples(batch.samples);
    appendToHistory(batch.samples);
    updateCharts();

    if (m_status == Waiting)
        log(EventLogModel::Success, tr("Receiving data"));
    else if (m_status == Stale)
        log(EventLogModel::Success, tr("Data resumed"));
    setStatus(Receiving);
    m_staleTimer.start();
}

void TelemetryController::onReceiverError(quint64 session, const QString &message)
{
    if (session != m_session)
        return;

    m_staleTimer.stop();
    setRunning(false);
    setErrorMessage(message);
    setStatus(Error);
    log(EventLogModel::Error, message);
}

void TelemetryController::onStaleTimeout()
{
    if (!m_running || m_status != Receiving)
        return;
    setStatus(Stale);
    log(EventLogModel::Warning, tr("No data for %1 seconds").arg(StaleTimeoutMs / 1000));
}

void TelemetryController::appendToHistory(const QList<TelemetrySample> &samples)
{
    for (const TelemetrySample &sample : samples) {
        if (m_firstTimestampMs < 0)
            m_firstTimestampMs = sample.timestampMs;

        const double t = double(sample.timestampMs - m_firstTimestampMs) / 1000.0;
        m_latestTime = std::max(m_latestTime, t);
        for (size_t c = 0; c < m_history.size(); ++c)
            m_history[c].append(QPointF(t, double(sample.channels[c])));
    }

    // Keep enough history for the widest window, so switching windows is instant.
    const double keepFrom = m_latestTime - MaxWindowSeconds - 1.0;
    for (QList<QPointF> &history : m_history) {
        const auto firstKept = std::find_if(history.cbegin(), history.cend(),
                                            [keepFrom](const QPointF &p) { return p.x() >= keepFrom; });
        history.erase(history.cbegin(), firstKept);
    }
}

void TelemetryController::updateCharts()
{
    const double end = std::max(m_latestTime, double(m_windowSeconds));
    const double start = end - m_windowSeconds;
    if (!qFuzzyCompare(m_viewStart, start) || !qFuzzyCompare(m_viewEnd, end)) {
        m_viewStart = start;
        m_viewEnd = end;
        emit viewChanged();
    }

    for (size_t c = 0; c < m_history.size(); ++c) {
        const QList<QPointF> &history = m_history[c];

        // First point inside the window, plus one before it so the line reaches the left edge.
        auto first = std::lower_bound(history.cbegin(), history.cend(), start,
                                      [](const QPointF &p, double x) { return p.x() < x; });
        if (first != history.cbegin())
            --first;

        const QList<QPointF> visible(first, history.cend());
        if (QXYSeries *series = m_series[c])
            series->replace(visible);

        if (!visible.isEmpty()) {
            const auto [lowest, highest] = std::minmax_element(
                visible.cbegin(), visible.cend(), [](const QPointF &a, const QPointF &b) { return a.y() < b.y(); });
            m_channels.fitAxisRange(int(c), lowest->y(), highest->y());
        }
    }
}

void TelemetryController::resetSession()
{
    m_stats = {};
    emit statsChanged();

    m_channels.reset();
    for (QList<QPointF> &history : m_history)
        history.clear();
    m_firstTimestampMs = -1;
    m_latestTime = 0.0;
    updateCharts();
}

void TelemetryController::setRunning(bool running)
{
    if (m_running == running)
        return;
    m_running = running;
    emit runningChanged();
}

void TelemetryController::setStatus(Status status)
{
    if (m_status == status)
        return;
    m_status = status;
    emit statusChanged();
}

void TelemetryController::setErrorMessage(const QString &message)
{
    if (m_errorMessage == message)
        return;
    m_errorMessage = message;
    emit errorMessageChanged();
}

void TelemetryController::log(EventLogModel::Severity severity, const QString &message)
{
    m_events.addMessage(severity, message, QDateTime::currentMSecsSinceEpoch());
}
