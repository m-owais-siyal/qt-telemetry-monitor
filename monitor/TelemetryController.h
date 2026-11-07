#pragma once

#include "ChannelModel.h"
#include "UdpReceiver.h"

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QtGraphs/QXYSeries>
#include <QtQml/qqmlregistration.h>

#include <array>

// Owns the receiver's worker thread and exposes its state to QML.
// QML only displays what this class exposes; no business logic lives in QML.
class TelemetryController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(double packetsPerSecond READ packetsPerSecond NOTIFY statsChanged)
    Q_PROPERTY(qint64 receivedPackets READ receivedPackets NOTIFY statsChanged)
    Q_PROPERTY(qint64 lostPackets READ lostPackets NOTIFY statsChanged)
    Q_PROPERTY(double lossPercent READ lossPercent NOTIFY statsChanged)
    Q_PROPERTY(ChannelModel *channels READ channels CONSTANT)

public:
    enum class Status { Idle, Receiving, Stale, Error };
    Q_ENUM(Status)

    explicit TelemetryController(QObject *parent = nullptr);
    ~TelemetryController() override;

    QString address() const { return m_address; }
    void setAddress(const QString &address);
    int port() const { return m_port; }
    void setPort(int port);
    bool isRunning() const { return m_running; }
    Status status() const { return m_status; }
    double packetsPerSecond() const { return m_stats.packetsPerSecond; }
    qint64 receivedPackets() const { return qint64(m_stats.received); }
    qint64 lostPackets() const { return qint64(m_stats.lost); }
    double lossPercent() const { return m_stats.lossPercent; }
    ChannelModel *channels() { return &m_channels; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    // Called from QML once per chart line, e.g. bindSeries(0, temperatureSeries).
    // New points are pushed into the series from C++, never appended point by point in QML.
    Q_INVOKABLE void bindSeries(int channel, QXYSeries *series);

signals:
    void addressChanged();
    void portChanged();
    void runningChanged();
    void statusChanged();
    void statsChanged();
    void errorOccurred(const QString &message);

private:
    void onSamples(const QList<TelemetrySample> &samples);
    void onStats(const ReceiverStats &stats);
    void setRunning(bool running);
    void setStatus(Status status);

    QThread m_thread;
    UdpReceiver *m_receiver = nullptr; // lives on m_thread
    ChannelModel m_channels;
    QTimer m_staleTimer;               // fires when no data arrives for 2 s
    std::array<QXYSeries *, 3> m_series{};

    QString m_address = QStringLiteral("239.255.42.1");
    int m_port = 45454;
    bool m_running = false;
    Status m_status = Status::Idle;
    ReceiverStats m_stats;
};
