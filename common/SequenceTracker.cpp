#include "SequenceTracker.h"

SequenceTracker::Observation SequenceTracker::observe(quint32 sequence)
{
    if (!m_hasLast) {
        m_hasLast = true;
        m_last = sequence;
        ++m_received;
        return {Kind::First, 0};
    }

    // Unsigned subtraction wraps, so (0 - 0xFFFFFFFF) == 1: one step forward.
    const quint32 forward = sequence - m_last;

    if (forward == 0)
        return {Kind::Duplicate, 0};

    if (forward < 0x80000000u) {
        const quint32 missing = forward - 1;
        m_lost += missing;
        ++m_received;
        m_last = sequence;
        return {missing == 0 ? Kind::InOrder : Kind::Gap, missing};
    }

    // A "forward" distance in the upper half of the range means the packet is behind us.
    const quint32 backward = m_last - sequence;
    if (backward <= RestartThreshold)
        return {Kind::Late, 0};

    ++m_restarts;
    ++m_received;
    m_last = sequence;
    return {Kind::Restart, 0};
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
