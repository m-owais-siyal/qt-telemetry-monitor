#pragma once

#include "TelemetryPacket.h"

#include <QList>
#include <QMetaType>

struct ReceiverStats
{
    double packetsPerSecond = 0.0;
    quint64 received = 0;
    quint64 lost = 0;
    double lossPercent = 0.0;
    quint64 invalid = 0;  // datagrams that failed to decode
    quint64 restarts = 0; // times the sender restarted its sequence numbers
};

struct ReceiverEvent
{
    enum class Type { Gap, Restart, InvalidDatagrams };

    Type type = Type::Gap;
    qint64 timeMs = 0;    // receive time, milliseconds since the Unix epoch
    quint32 sequence = 0; // Gap: first missing sequence number. Restart: the new sequence number
    quint32 count = 0;    // Gap: how many packets are missing. InvalidDatagrams: how many were ignored
};

// Everything the receiver produced during one flush interval. It reaches the
// GUI thread as a single queued signal, so the UI updates once per batch
// instead of once per packet.
struct ReceiverBatch
{
    quint64 session = 0;
    QList<TelemetrySample> samples;
    QList<ReceiverEvent> events;
    ReceiverStats stats;
};
Q_DECLARE_METATYPE(ReceiverBatch)
