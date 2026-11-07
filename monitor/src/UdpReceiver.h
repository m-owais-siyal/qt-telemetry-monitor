#pragma once

#include "SequenceTracker.h"
#include "TelemetryTypes.h"

#include <QElapsedTimer>
#include <QHostAddress>
#include <QObject>

class QTimer;
class QUdpSocket;

// Receives telemetry datagrams on a worker thread.
//
// Each datagram is decoded and checked against the sequence tracker as soon as it
// arrives, but results are handed to the GUI thread in batches every
// FlushIntervalMs. At 1000 packets/s that is 20 signals per second instead of 1000.
class UdpReceiver : public QObject
{
    Q_OBJECT

public:
    static constexpr int FlushIntervalMs = 50;

    explicit UdpReceiver(QObject *parent = nullptr);

    // The port the socket is bound to; useful when start() was given port 0.
    quint16 boundPort() const;

public slots:
    // `address` is a multicast group to join, or a local address to listen on
    // (0.0.0.0 for every interface). `session` is echoed back in every batch so
    // the controller can discard batches from a previous run.
    void start(const QHostAddress &address, quint16 port, quint64 session);
    void stop();

signals:
    void batchReady(const ReceiverBatch &batch);
    void errorOccurred(quint64 session, const QString &message);

private:
    void readPendingDatagrams();
    void flush();

    QUdpSocket *m_socket = nullptr; // children of this object, so they move to the
    QTimer *m_flushTimer = nullptr; // worker thread together with it

    QHostAddress m_group;
    quint64 m_session = 0;
    SequenceTracker m_tracker;

    QElapsedTimer m_rateClock;
    quint64 m_packetsInRateWindow = 0;
    double m_packetsPerSecond = 0.0;

    quint64 m_invalid = 0;
    quint32 m_invalidSinceFlush = 0;

    QList<TelemetrySample> m_pendingSamples;
    QList<ReceiverEvent> m_pendingEvents;
};
