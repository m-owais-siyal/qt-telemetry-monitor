#include "TelemetryPacket.h"

#include <QtEndian>

#include <cstring>
#include <limits>

static_assert(sizeof(float) == sizeof(quint32), "the wire format needs 32-bit floats");
static_assert(std::numeric_limits<float>::is_iec559, "the wire format needs IEEE-754 floats");

namespace
{
constexpr qsizetype MagicOffset = 0;
constexpr qsizetype SequenceOffset = 4;
constexpr qsizetype TimestampOffset = 8;
constexpr qsizetype ChannelsOffset = 16;

quint32 floatBits(float value)
{
    quint32 bits = 0;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float floatFromBits(quint32 bits)
{
    float value = 0.0f;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}
} // namespace

namespace TelemetryPacket
{
QByteArray encode(const TelemetrySample &sample)
{
    QByteArray bytes(Size, Qt::Uninitialized);
    char *data = bytes.data();

    qToBigEndian<quint32>(Magic, data + MagicOffset);
    qToBigEndian<quint32>(sample.sequence, data + SequenceOffset);
    qToBigEndian<qint64>(sample.timestampMs, data + TimestampOffset);
    for (size_t i = 0; i < sample.channels.size(); ++i)
        qToBigEndian<quint32>(floatBits(sample.channels[i]), data + ChannelsOffset + qsizetype(4 * i));

    return bytes;
}

std::optional<TelemetrySample> decode(QByteArrayView datagram)
{
    if (datagram.size() != Size)
        return std::nullopt;

    const char *data = datagram.data();
    if (qFromBigEndian<quint32>(data + MagicOffset) != Magic)
        return std::nullopt;

    TelemetrySample sample;
    sample.sequence = qFromBigEndian<quint32>(data + SequenceOffset);
    sample.timestampMs = qFromBigEndian<qint64>(data + TimestampOffset);
    for (size_t i = 0; i < sample.channels.size(); ++i)
        sample.channels[i] = floatFromBits(qFromBigEndian<quint32>(data + ChannelsOffset + qsizetype(4 * i)));

    return sample;
}
} // namespace TelemetryPacket
