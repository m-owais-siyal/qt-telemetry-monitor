#pragma once

#include "ChannelModel.h"
#include "EventLogModel.h"
#include "TelemetryTypes.h"

#include <QList>
#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QtGraphs/QXYSeries>
#include <QtQml/qqmlregistration.h>

#include <array>

class UdpReceiver;

// Owns the receiver's worker thread and turns its batches into state for QML.
// QML only displays what this class exposes; no logic lives in QML.
class TelemetryController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(int windowSeconds READ windowSeconds WRITE setWindowSeconds NOTIFY windowSecondsChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(double packetsPerSecond READ packetsPerSecond NOTIFY statsChanged)
    Q_PROPERTY(double receivedPackets READ receivedPackets NOTIFY statsChanged)
    Q_PROPERTY(double lostPackets READ lostPackets NOTIFY statsChanged)
    Q_PROPERTY(double lossPercent READ lossPercent NOTIFY statsChanged)
    Q_PROPERTY(double viewStart READ viewStart NOTIFY viewChanged)
    Q_PROPERTY(double viewEnd READ viewEnd NOTIFY viewChanged)
    Q_PROPERTY(ChannelModel *channels READ channels CONSTANT)
    Q_PROPERTY(EventLogModel *events READ events CONSTANT)

public:
    enum Status {
        Idle,      // not listening
        Waiting,   // listening, nothing received yet
        Receiving, // data is arriving
        Stale,     // data stopped arriving
        Error,     // could not start
    };
    Q_ENUM(Status)

    static constexpr int ChannelCount = 3;
    static constexpr int MaxWindowSeconds = 60;
    static constexpr int StaleTimeoutMs = 2000;

    explicit TelemetryController(QObject *parent = nullptr);
    ~TelemetryController() override;

    QString address() const { return m_address; }
    void setAddress(const QString &address);
    int port() const { return m_port; }
    void setPort(int port);
    int windowSeconds() const { return m_windowSeconds; }
    void setWindowSeconds(int seconds);

    bool isRunning() const { return m_running; }
    Status status() const { return m_status; }
    QString errorMessage() const { return m_errorMessage; }

    double packetsPerSecond() const { return m_stats.packetsPerSecond; }
    double receivedPackets() const { return double(m_stats.received); }
    double lostPackets() const { return double(m_stats.lost); }
    double lossPercent() const { return m_stats.lossPercent; }

    double viewStart() const { return m_viewStart; }
    double viewEnd() const { return m_viewEnd; }

    ChannelModel *channels() { return &m_channels; }
    EventLogModel *events() { return &m_events; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    // Called from QML once per chart. Points are pushed into the series from C++
    // in one replace() per batch, never appended point by point from QML.
    Q_INVOKABLE void bindSeries(int channel, QXYSeries *series);

signals:
    void addressChanged();
    void portChanged();
    void windowSecondsChanged();
    void runningChanged();
    void statusChanged();
    void errorMessageChanged();
    void statsChanged();
    void viewChanged();

private:
    void onBatch(const ReceiverBatch &batch);
    void onReceiverError(quint64 session, const QString &message);
    void onStaleTimeout();

    void appendToHistory(const QList<TelemetrySample> &samples);
    void updateCharts();
    void resetSession();
    void setRunning(bool running);
    void setStatus(Status status);
    void setErrorMessage(const QString &message);
    void log(EventLogModel::Severity severity, const QString &message);

    QThread m_thread;
    UdpReceiver *m_receiver = nullptr; // lives on m_thread
    ChannelModel m_channels;
    EventLogModel m_events;
    QTimer m_staleTimer;

    QString m_address = QStringLiteral("239.255.42.1");
    int m_port = 45454;
    int m_windowSeconds = 10;

    quint64 m_session = 0;
    bool m_running = false;
    Status m_status = Idle;
    QString m_errorMessage;
    ReceiverStats m_stats;

    // Chart history: up to MaxWindowSeconds of points per channel, x in seconds
    // since the first sample of the session.
    std::array<QList<QPointF>, ChannelCount> m_history;
    std::array<QPointer<QXYSeries>, ChannelCount> m_series;
    qint64 m_firstTimestampMs = -1;
    double m_latestTime = 0.0;
    double m_viewStart = 0.0;
    double m_viewEnd = 10.0;
};
