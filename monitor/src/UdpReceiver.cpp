#include "UdpReceiver.h"

#include <QDateTime>
#include <QNetworkDatagram>
#include <QTimer>
#include <QUdpSocket>

#include <utility>

UdpReceiver::UdpReceiver(QObject *parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))
    , m_flushTimer(new QTimer(this))
{
    m_flushTimer->setInterval(FlushIntervalMs);
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpReceiver::readPendingDatagrams);
    connect(m_flushTimer, &QTimer::timeout, this, &UdpReceiver::flush);
}

quint16 UdpReceiver::boundPort() const
{
    return m_socket->localPort();
}

void UdpReceiver::start(const QHostAddress &address, quint16 port, quint64 session)
{
    stop();

    m_session = session;
    m_tracker.reset();
    m_pendingSamples.clear();
    m_pendingEvents.clear();
    m_packetsInRateWindow = 0;
    m_packetsPerSecond = 0.0;
    m_invalid = 0;
    m_invalidSinceFlush = 0;

    const bool multicast = address.isMulticast();
    const bool ipv6 = address.protocol() == QAbstractSocket::IPv6Protocol;
    const QHostAddress bindAddress = multicast
        ? QHostAddress(ipv6 ? QHostAddress::AnyIPv6 : QHostAddress::AnyIPv4)
        : address;

    // ShareAddress | ReuseAddressHint lets several receivers on one machine
    // listen to the same multicast group and port.
    if (!m_socket->bind(bindAddress, port, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)) {
        emit errorOccurred(session, tr("Could not listen on %1:%2 (%3)")
                                        .arg(bindAddress.toString())
                                        .arg(port)
                                        .arg(m_socket->errorString()));
        return;
    }

    if (multicast && !m_socket->joinMulticastGroup(address)) {
        const QString message = tr("Could not join multicast group %1 (%2)")
                                    .arg(address.toString(), m_socket->errorString());
        m_socket->close();
        emit errorOccurred(session, message);
        return;
    }

    m_group = multicast ? address : QHostAddress();
    m_rateClock.start();
    m_flushTimer->start();
}

void UdpReceiver::stop()
{
    if (m_socket->state() == QAbstractSocket::UnconnectedState)
        return;

    m_flushTimer->stop();
    flush();

    if (!m_group.isNull())
        m_socket->leaveMulticastGroup(m_group);
    m_socket->close();
    m_group.clear();
}

void UdpReceiver::readPendingDatagrams()
{
    while (m_socket->hasPendingDatagrams()) {
        const QNetworkDatagram datagram = m_socket->receiveDatagram();
        const auto sample = TelemetryPacket::decode(datagram.data());
        if (!sample) {
            ++m_invalid;
            ++m_invalidSinceFlush;
            continue;
        }

        const auto observation = m_tracker.observe(sample->sequence);
        if (!observation.accepted())
            continue; // duplicate or late: already accounted for

        if (observation.kind == SequenceTracker::Kind::Gap) {
            m_pendingEvents.append({ReceiverEvent::Type::Gap, QDateTime::currentMSecsSinceEpoch(),
                                    sample->sequence - observation.missing, observation.missing});
        } else if (observation.kind == SequenceTracker::Kind::Restart) {
            m_pendingEvents.append({ReceiverEvent::Type::Restart, QDateTime::currentMSecsSinceEpoch(),
                                    sample->sequence, 0});
        }

        ++m_packetsInRateWindow;
        m_pendingSamples.append(*sample);
    }
}

void UdpReceiver::flush()
{
    // Packets per second over a one-second window, so the number on screen is steady.
    const qint64 elapsedMs = m_rateClock.elapsed();
    if (elapsedMs >= 1000) {
        m_packetsPerSecond = double(m_packetsInRateWindow) * 1000.0 / double(elapsedMs);
        m_packetsInRateWindow = 0;
        m_rateClock.restart();
    }

    if (m_invalidSinceFlush > 0) {
        m_pendingEvents.append({ReceiverEvent::Type::InvalidDatagrams, QDateTime::currentMSecsSinceEpoch(), 0,
                                m_invalidSinceFlush});
        m_invalidSinceFlush = 0;
    }

    ReceiverBatch batch;
    batch.session = m_session;
    batch.samples = std::exchange(m_pendingSamples, {});
    batch.events = std::exchange(m_pendingEvents, {});
    batch.stats.packetsPerSecond = m_packetsPerSecond;
    batch.stats.received = m_tracker.received();
    batch.stats.lost = m_tracker.lost();
    batch.stats.lossPercent = m_tracker.lossPercent();
    batch.stats.invalid = m_invalid;
    batch.stats.restarts = m_tracker.restarts();

    emit batchReady(batch);
}
