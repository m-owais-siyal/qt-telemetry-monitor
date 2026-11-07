#pragma once

#include "SequenceTracker.h"
#include "TelemetryPacket.h"

#include <QHostAddress>
#include <QList>
#include <QObject>

class QTimer;
class QUdpSocket;

struct ReceiverStats
{
    double packetsPerSecond = 0.0;
    quint64 received = 0;
    quint64 lost = 0;
    double lossPercent = 0.0;
};
Q_DECLARE_METATYPE(ReceiverStats)

// Lives on a worker thread. Receives datagrams, decodes them, tracks gaps,
// and hands samples to the GUI thread in batches every 50 ms, so the UI
// never handles one signal per packet.
class UdpReceiver : public QObject
{
    Q_OBJECT

public:
    explicit UdpReceiver(QObject *parent = nullptr);

public slots:
    void start(const QHostAddress &address, quint16 port);
    void stop();

signals:
    void samplesReady(const QList<TelemetrySample> &samples);
    void statsUpdated(const ReceiverStats &stats);
    void errorOccurred(const QString &message);

private:
    void readPendingDatagrams();
    void flush();

    QUdpSocket *m_socket = nullptr; // child of this, so it moves to the worker thread with us
    QTimer *m_flushTimer = nullptr; // child of this, same reason
    QHostAddress m_group;
    QList<TelemetrySample> m_pending;
    SequenceTracker m_tracker;
};
