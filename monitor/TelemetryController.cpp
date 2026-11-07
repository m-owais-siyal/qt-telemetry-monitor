#include "TelemetryController.h"

#include <QHostAddress>

TelemetryController::TelemetryController(QObject *parent)
    : QObject(parent)
{
    m_receiver = new UdpReceiver; // no parent: it is moved to the worker thread
    m_receiver->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, m_receiver, &QObject::deleteLater);

    connect(m_receiver, &UdpReceiver::samplesReady, this, &TelemetryController::onSamples);
    connect(m_receiver, &UdpReceiver::statsUpdated, this, &TelemetryController::onStats);
    connect(m_receiver, &UdpReceiver::errorOccurred, this, [this](const QString &message) {
        setRunning(false);
        setStatus(Status::Error);
        emit errorOccurred(message);
    });

    m_staleTimer.setSingleShot(true);
    m_staleTimer.setInterval(2000);
    connect(&m_staleTimer, &QTimer::timeout, this, [this] { setStatus(Status::Stale); });

    m_thread.setObjectName(QStringLiteral("udp-receiver"));
    m_thread.start();
}

TelemetryController::~TelemetryController()
{
    m_thread.quit();
    m_thread.wait();
}

void TelemetryController::setAddress(const QString &address)
{
    if (m_address == address)
        return;
    m_address = address;
    emit addressChanged();
}

void TelemetryController::setPort(int port)
{
    if (m_port == port)
        return;
    m_port = port;
    emit portChanged();
}

void TelemetryController::start()
{
    // TODO: validate m_address and m_port (emit errorOccurred on bad input),
    // reset the channel model and chart series, then call the receiver on its own
    // thread with QMetaObject::invokeMethod(m_receiver, [=] { ... }, Qt::QueuedConnection).
    // Finally setRunning(true) and start m_staleTimer.
}

void TelemetryController::stop()
{
    // TODO: invoke m_receiver->stop() on its thread, stop m_staleTimer,
    // setRunning(false) and setStatus(Status::Idle).
}

void TelemetryController::bindSeries(int channel, QXYSeries *series)
{
    // TODO: bounds-check `channel` and store the series in m_series.
    Q_UNUSED(channel);
    Q_UNUSED(series);
}

void TelemetryController::onSamples(const QList<TelemetrySample> &samples)
{
    // TODO:
    //  - m_channels.addSamples(samples)
    //  - for each bound series: add the new points, drop points older than the
    //    visible window (e.g. 10 s), and call series->replace(points) once per batch
    //  - restart m_staleTimer and setStatus(Status::Receiving)
    Q_UNUSED(samples);
}

void TelemetryController::onStats(const ReceiverStats &stats)
{
    m_stats = stats;
    emit statsChanged();
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
