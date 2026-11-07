#pragma once

#include "TelemetryPacket.h"

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

// One row per telemetry channel: latest value, running min / max / average,
// and a y-axis range for its chart.
class ChannelModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("ChannelModel is owned by TelemetryController")

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        UnitRole,
        DecimalsRole,
        HasDataRole,
        ValueRole,
        MinimumRole,
        MaximumRole,
        AverageRole,
        RangeMinRole,
        RangeMaxRole,
        TickIntervalRole,
        LabelDecimalsRole,
    };

    explicit ChannelModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Updates every channel from a batch of samples and emits dataChanged once.
    void addSamples(const QList<TelemetrySample> &samples);

    // Fits the chart's y-axis to the visible data, with hysteresis: the range only
    // changes when data leaves it or it becomes much wider than the data, so the
    // axis does not twitch on every update. The range is snapped to a "nice" tick
    // interval (1, 2 or 5 times a power of ten) so the labels are round numbers.
    void fitAxisRange(int row, double dataMin, double dataMax);

    // A tick interval that splits `span` into roughly `targetTicks` round steps.
    static double niceTickInterval(double span, int targetTicks = 4);

    void reset();

private:
    struct Channel
    {
        QString name;
        QString unit;
        int decimals = 2;
        double defaultMin = 0.0;
        double defaultMax = 1.0;

        double value = 0.0;
        double minimum = 0.0;
        double maximum = 0.0;
        double sum = 0.0;
        quint64 count = 0;

        bool hasRange = false;
        double rangeMin = 0.0;
        double rangeMax = 1.0;
        double tickInterval = 1.0;
        int labelDecimals = 0;
    };

    void setRange(Channel &channel, double lo, double hi);

    QList<Channel> m_channels;
};
