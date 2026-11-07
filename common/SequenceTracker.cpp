#include "SequenceTracker.h"

quint32 SequenceTracker::observe(quint32 sequence)
{
    // TODO: the first packet only initialises m_last.
    // After that, compute the forward distance with unsigned wraparound
    // (sequence - m_last) and decide: duplicate, in order, gap, or late.
    Q_UNUSED(sequence);
    return 0;
}

double SequenceTracker::lossPercent() const
{
    const quint64 total = m_received + m_lost;
    return total == 0 ? 0.0 : 100.0 * double(m_lost) / double(total);
}

void SequenceTracker::reset()
{
    *this = SequenceTracker{};
}
