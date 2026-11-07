#pragma once

#include "TelemetryPacket.h"

#include <QElapsedTimer>
#include <QHostAddress>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

#include <random>

struct SimulatorConfig
{
    QHostAddress address;
    quint16 port = 45454;
    int rateHz = 50;
    double dropPercent = 0.0;
};

// Sends TelemetrySample packets at a fixed rate.
//
// The send schedule is driven by elapsed time rather than by counting timer ticks,
// so the average rate stays exact even when 1000 / rate is not a whole number of
// milliseconds (60 Hz, say) or the OS delivers a tick late.
class TelemetrySimulator : public QObject
{
    Q_OBJECT

public:
    explicit TelemetrySimulator(const SimulatorConfig &config, QObject *parent = nullptr);

    bool start(QString *errorMessage);

private:
    void tick();
    void sendNext();
    TelemetrySample makeSample(double seconds);
    void reportProgress();

    SimulatorConfig m_config;
    QUdpSocket m_socket;
    QTimer m_tickTimer;
    QTimer m_reportTimer;
    QElapsedTimer m_clock;

    quint64 m_scheduled = 0; // packets due so far, sent or deliberately dropped
    quint32 m_nextSequence = 0;
    quint64 m_sentSinceReport = 0;
    quint64 m_droppedSinceReport = 0;
    quint64 m_failedSinceReport = 0;
    QString m_lastError;

    std::mt19937 m_random{std::random_device{}()};
    std::normal_distribution<double> m_noise{0.0, 1.0};
    std::uniform_real_distribution<double> m_percent{0.0, 100.0};
};
