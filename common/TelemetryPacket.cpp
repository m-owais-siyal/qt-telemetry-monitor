#include "TelemetryPacket.h"

namespace TelemetryPacket
{
QByteArray encode(const TelemetrySample &sample)
{
    // TODO: write magic, sequence, timestampMs and the three channels in big-endian order.
    // QDataStream works (remember setFloatingPointPrecision(QDataStream::SinglePrecision)),
    // or qToBigEndian into a fixed-size buffer.
    Q_UNUSED(sample);
    return {};
}

std::optional<TelemetrySample> decode(QByteArrayView datagram)
{
    // TODO: reject datagrams whose size != Size or whose magic != Magic,
    // otherwise read the fields back in the same order.
    Q_UNUSED(datagram);
    return std::nullopt;
}
} // namespace TelemetryPacket
