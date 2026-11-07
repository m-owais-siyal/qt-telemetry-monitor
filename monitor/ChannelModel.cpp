#include "ChannelModel.h"

ChannelModel::ChannelModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_channels = {
        {QStringLiteral("Temperature"), QStringLiteral("°C")},
        {QStringLiteral("Pressure"), QStringLiteral("kPa")},
        {QStringLiteral("Voltage"), QStringLiteral("V")},
    };
}

int ChannelModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_channels.size());
}

QVariant ChannelModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return {};

    const Channel &channel = m_channels.at(index.row());
    const bool hasData = channel.count > 0;

    switch (role) {
    case NameRole:
        return channel.name;
    case UnitRole:
        return channel.unit;
    case ValueRole:
        return hasData ? QVariant(channel.value) : QVariant();
    case MinimumRole:
        return hasData ? QVariant(channel.minimum) : QVariant();
    case MaximumRole:
        return hasData ? QVariant(channel.maximum) : QVariant();
    case AverageRole:
        return hasData ? QVariant(channel.sum / double(channel.count)) : QVariant();
    default:
        return {};
    }
}

QHash<int, QByteArray> ChannelModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {UnitRole, "unit"},
        {ValueRole, "value"},
        {MinimumRole, "minimum"},
        {MaximumRole, "maximum"},
        {AverageRole, "average"},
    };
}

void ChannelModel::addSamples(const QList<TelemetrySample> &samples)
{
    // TODO: for each sample and each channel, update value/min/max/sum/count.
    // Then emit dataChanged(index(0), index(rowCount() - 1)) once for the whole batch.
    Q_UNUSED(samples);
}

void ChannelModel::reset()
{
    for (Channel &channel : m_channels) {
        channel.value = channel.minimum = channel.maximum = channel.sum = 0.0;
        channel.count = 0;
    }
    if (!m_channels.isEmpty())
        emit dataChanged(index(0), index(rowCount() - 1));
}
