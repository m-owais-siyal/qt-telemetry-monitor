#include "UdpReceiver.h"

#include <QTimer>
#include <QUdpSocket>

UdpReceiver::UdpReceiver(QObject *parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))
    , m_flushTimer(new QTimer(this))
{
    m_flushTimer->setInterval(50);
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpReceiver::readPendingDatagrams);
    connect(m_flushTimer, &QTimer::timeout, this, &UdpReceiver::flush);
}

void UdpReceiver::start(const QHostAddress &address, quint16 port)
{
    // TODO:
    //  1. Bind m_socket to QHostAddress::AnyIPv4 on `port`
    //     (ShareAddress | ReuseAddressHint so the simulator and monitor can share the machine).
    //  2. If address.isMulticast(), joinMulticastGroup(address) and remember it in m_group.
    //  3. On failure, emit errorOccurred(m_socket->errorString()) and return.
    //  4. Reset m_tracker and the packets/sec bookkeeping, then start m_flushTimer.
    Q_UNUSED(address);
    Q_UNUSED(port);
}

void UdpReceiver::stop()
{
    // TODO: stop the flush timer, flush what is pending, leave the multicast
    // group if one was joined, and close the socket.
}

void UdpReceiver::readPendingDatagrams()
{
    // TODO: while (m_socket->hasPendingDatagrams()), receiveDatagram(), decode it,
    // skip invalid packets, run the sequence through m_tracker, append to m_pending.
}

void UdpReceiver::flush()
{
    // TODO: if m_pending is not empty, emit samplesReady(m_pending) and clear it.
    // Work out packets/sec (a QElapsedTimer plus a counter is enough) and
    // emit statsUpdated() with the tracker's received/lost/lossPercent.
}
