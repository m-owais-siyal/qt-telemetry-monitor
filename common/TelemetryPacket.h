#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <QMetaType>

#include <array>
#include <optional>

struct TelemetrySample
{
    quint32 sequence = 0;
    qint64 timestampMs = 0;          // sender clock, milliseconds since the Unix epoch
    std::array<float, 3> channels{}; // temperature (degrees C), pressure (kPa), voltage (V)
};
Q_DECLARE_METATYPE(TelemetrySample)

namespace TelemetryPacket
{
// Wire format, big-endian, 28 bytes:
//   quint32 magic        'TLM1'
//   quint32 sequence
//   qint64  timestampMs
//   float   channels[3]  IEEE-754 single precision
inline constexpr quint32 Magic = 0x544C4D31; // "TLM1"
inline constexpr qsizetype Size = 4 + 4 + 8 + 3 * 4;

QByteArray encode(const TelemetrySample &sample);

// Returns std::nullopt for datagrams with the wrong size or magic.
std::optional<TelemetrySample> decode(QByteArrayView datagram);
} // namespace TelemetryPacket
