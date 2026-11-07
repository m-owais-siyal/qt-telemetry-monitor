#pragma once

#include <QtGlobal>

// Counts received and lost packets from their sequence numbers.
// Sequence numbers are quint32 and wrap from 0xFFFFFFFF back to 0.
class SequenceTracker
{
public:
    // Records a sequence number and returns how many packets were missed
    // immediately before it (0 when it arrived in order).
    // Duplicates and late (reordered) packets are ignored: they return 0
    // and leave every counter unchanged.
    quint32 observe(quint32 sequence);

    quint64 received() const { return m_received; }
    quint64 lost() const { return m_lost; }
    double lossPercent() const; // lost / (received + lost) * 100, or 0 when empty
    void reset();

private:
    bool m_hasLast = false;
    quint32 m_last = 0;
    quint64 m_received = 0;
    quint64 m_lost = 0;
};
