#pragma once

#include "TelemetryPacket.h"

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

// One row per telemetry channel: latest value plus running min, max and average.
class ChannelModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("ChannelModel is owned by TelemetryController")

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        UnitRole,
        ValueRole,
        MinimumRole,
        MaximumRole,
        AverageRole,
    };

    explicit ChannelModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Updates every channel from a batch of samples, then emits dataChanged once.
    void addSamples(const QList<TelemetrySample> &samples);
    void reset();

private:
    struct Channel
    {
        QString name;
        QString unit;
        double value = 0.0;
        double minimum = 0.0;
        double maximum = 0.0;
        double sum = 0.0;
        quint64 count = 0;
    };

    QList<Channel> m_channels;
};
