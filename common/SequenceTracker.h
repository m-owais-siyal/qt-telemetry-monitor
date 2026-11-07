#pragma once

#include <QtGlobal>

// Classifies packets by sequence number and counts the ones that never arrived.
//
// Sequence numbers are quint32 and wrap from 0xFFFFFFFF back to 0, so distances
// are computed with unsigned arithmetic (serial-number arithmetic, as in RFC 1982):
// a forward distance below 2^31 means "newer", anything else means "older".
class SequenceTracker
{
public:
    enum class Kind {
        First,     // first packet of the session
        InOrder,   // exactly the expected next sequence number
        Gap,       // newer than expected: `missing` packets were lost before it
        Duplicate, // same sequence number as the last accepted packet
        Late,      // a little older than the last accepted packet (reordered): ignored
        Restart,   // far older than the last packet: the sender restarted, so resync
    };

    struct Observation
    {
        Kind kind = Kind::First;
        quint32 missing = 0; // non-zero only for Kind::Gap

        bool accepted() const { return kind != Kind::Duplicate && kind != Kind::Late; }
    };

    // A packet this many steps or fewer behind the last one is a late packet;
    // anything further behind means the sender restarted its sequence.
    static constexpr quint32 RestartThreshold = 1000;

    Observation observe(quint32 sequence);

    quint64 received() const { return m_received; }
    quint64 lost() const { return m_lost; }
    quint64 restarts() const { return m_restarts; }
    double lossPercent() const; // lost / (received + lost) * 100, or 0 when empty
    void reset();

private:
    bool m_hasLast = false;
    quint32 m_last = 0;
    quint64 m_received = 0;
    quint64 m_lost = 0;
    quint64 m_restarts = 0;
};
