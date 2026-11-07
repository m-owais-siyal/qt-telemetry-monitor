#include "TelemetrySimulator.h"

#include <QDateTime>
#include <QTextStream>

#include <algorithm>
#include <cmath>

namespace
{
constexpr double TwoPi = 6.283185307179586;
}

TelemetrySimulator::TelemetrySimulator(const SimulatorConfig &config, QObject *parent)
    : QObject(parent)
    , m_config(config)
{
    // Tick at least twice per packet period so packets leave close to their due time.
    m_tickTimer.setTimerType(Qt::PreciseTimer);
    m_tickTimer.setInterval(std::clamp(500 / m_config.rateHz, 1, 10));
    connect(&m_tickTimer, &QTimer::timeout, this, &TelemetrySimulator::tick);

    // Precise, because a coarse timer may fire up to 5% early and make the
    // per-second counts look like the rate is off.
    m_reportTimer.setTimerType(Qt::PreciseTimer);
    m_reportTimer.setInterval(1000);
    connect(&m_reportTimer, &QTimer::timeout, this, &TelemetrySimulator::reportProgress);
}

bool TelemetrySimulator::start(QString *errorMessage)
{
    const bool ipv6 = m_config.address.protocol() == QAbstractSocket::IPv6Protocol;
    if (!m_socket.bind(QHostAddress(ipv6 ? QHostAddress::AnyIPv6 : QHostAddress::AnyIPv4), 0)) {
        if (errorMessage)
            *errorMessage = m_socket.errorString();
        return false;
    }

    if (m_config.address.isMulticast()) {
        // TTL 1 keeps the traffic on the local network; loopback lets a receiver
        // on this same machine see the packets.
        m_socket.setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
        m_socket.setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
    }

    m_clock.start();
    m_tickTimer.start();
    m_reportTimer.start();
    tick(); // send the first packet straight away
    return true;
}

void TelemetrySimulator::tick()
{
    const double elapsedSeconds = double(m_clock.nsecsElapsed()) / 1e9;
    const quint64 due = quint64(elapsedSeconds * m_config.rateHz) + 1;

    // After a long stall (the machine slept, a debugger paused us) don't fire a
    // huge burst to catch up: skip ahead and keep at most one second's backlog.
    const quint64 maxBacklog = quint64(m_config.rateHz);
    if (due > m_scheduled + maxBacklog)
        m_scheduled = due - maxBacklog;

    while (m_scheduled < due)
        sendNext();
}

void TelemetrySimulator::sendNext()
{
    // Use the ideal sample time for the waveform so the signal is perfectly regular,
    // and the wall clock for the timestamp the receiver sees.
    const double sampleTime = double(m_scheduled) / m_config.rateHz;
    ++m_scheduled;

    TelemetrySample sample = makeSample(sampleTime);
    sample.sequence = m_nextSequence++;
    sample.timestampMs = QDateTime::currentMSecsSinceEpoch();

    // A dropped packet still consumes a sequence number, so the receiver sees a gap.
    if (m_config.dropPercent > 0.0 && m_percent(m_random) < m_config.dropPercent) {
        ++m_droppedSinceReport;
        return;
    }

    const QByteArray datagram = TelemetryPacket::encode(sample);
    if (m_socket.writeDatagram(datagram, m_config.address, m_config.port) == datagram.size()) {
        ++m_sentSinceReport;
    } else {
        ++m_failedSinceReport;
        m_lastError = m_socket.errorString();
    }
}

TelemetrySample TelemetrySimulator::makeSample(double t)
{
    TelemetrySample sample;

    // Temperature (degrees C): slow drift plus a faster ripple and sensor noise.
    sample.channels[0] = float(24.0 + 3.0 * std::sin(TwoPi * t / 20.0) + 0.6 * std::sin(TwoPi * t / 3.1)
                               + 0.12 * m_noise(m_random));

    // Pressure (kPa): close to sea level, slow weather-like drift.
    sample.channels[1] = float(101.3 + 0.6 * std::sin(TwoPi * t / 45.0 + 1.0) + 0.05 * m_noise(m_random));

    // Voltage (V): a 3.3 V rail with ripple and a short brown-out dip every 12 seconds.
    const bool dip = std::fmod(t, 12.0) < 0.6;
    sample.channels[2] = float(3.30 + 0.04 * std::sin(TwoPi * t / 7.0) + (dip ? -0.25 : 0.0)
                               + 0.008 * m_noise(m_random));

    return sample;
}

void TelemetrySimulator::reportProgress()
{
    QTextStream out(stdout);
    out << "seq " << m_nextSequence << "  sent " << m_sentSinceReport << "  dropped "
        << m_droppedSinceReport;
    if (m_failedSinceReport > 0)
        out << "  failed " << m_failedSinceReport << " (" << m_lastError << ')';
    out << '\n';

    m_sentSinceReport = m_droppedSinceReport = m_failedSinceReport = 0;
}
