#include "ChannelModel.h"

#include <algorithm>
#include <cmath>

ChannelModel::ChannelModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const QString degreesC = QString(QChar(0x00B0)) + QLatin1Char('C');

    m_channels = {
        {QStringLiteral("Temperature"), degreesC, 1, 18.0, 30.0},
        {QStringLiteral("Pressure"), QStringLiteral("kPa"), 2, 100.0, 103.0},
        {QStringLiteral("Voltage"), QStringLiteral("V"), 3, 2.9, 3.5},
    };
    reset();
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
    case DecimalsRole:
        return channel.decimals;
    case HasDataRole:
        return hasData;
    case ValueRole:
        return hasData ? QVariant(channel.value) : QVariant();
    case MinimumRole:
        return hasData ? QVariant(channel.minimum) : QVariant();
    case MaximumRole:
        return hasData ? QVariant(channel.maximum) : QVariant();
    case AverageRole:
        return hasData ? QVariant(channel.sum / double(channel.count)) : QVariant();
    case RangeMinRole:
        return channel.rangeMin;
    case RangeMaxRole:
        return channel.rangeMax;
    case TickIntervalRole:
        return channel.tickInterval;
    case LabelDecimalsRole:
        return channel.labelDecimals;
    default:
        return {};
    }
}

QHash<int, QByteArray> ChannelModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {UnitRole, "unit"},
        {DecimalsRole, "decimals"},
        {HasDataRole, "hasData"},
        {ValueRole, "value"},
        {MinimumRole, "minimum"},
        {MaximumRole, "maximum"},
        {AverageRole, "average"},
        {RangeMinRole, "rangeMin"},
        {RangeMaxRole, "rangeMax"},
        {TickIntervalRole, "tickInterval"},
        {LabelDecimalsRole, "labelDecimals"},
    };
}

void ChannelModel::addSamples(const QList<TelemetrySample> &samples)
{
    if (samples.isEmpty() || m_channels.isEmpty())
        return;

    const qsizetype channelCount = std::min<qsizetype>(m_channels.size(), qsizetype(TelemetrySample{}.channels.size()));
    for (const TelemetrySample &sample : samples) {
        for (qsizetype row = 0; row < channelCount; ++row) {
            Channel &channel = m_channels[row];
            const double value = sample.channels[size_t(row)];
            channel.value = value;
            channel.minimum = channel.count == 0 ? value : std::min(channel.minimum, value);
            channel.maximum = channel.count == 0 ? value : std::max(channel.maximum, value);
            channel.sum += value;
            ++channel.count;
        }
    }

    emit dataChanged(index(0), index(rowCount() - 1),
                     {HasDataRole, ValueRole, MinimumRole, MaximumRole, AverageRole});
}

void ChannelModel::fitAxisRange(int row, double dataMin, double dataMax)
{
    if (row < 0 || row >= m_channels.size() || dataMin > dataMax)
        return;

    Channel &channel = m_channels[row];

    // Never let the span collapse to zero, or a flat signal would have no axis.
    const double magnitude = std::max({std::abs(dataMin), std::abs(dataMax), 1.0});
    const double span = std::max(dataMax - dataMin, magnitude * 0.01);

    const bool dataOutside = !channel.hasRange || dataMin < channel.rangeMin || dataMax > channel.rangeMax;
    const bool rangeTooLoose = channel.hasRange && (channel.rangeMax - channel.rangeMin) > 3.0 * span;
    if (!dataOutside && !rangeTooLoose)
        return;

    const double padding = span * 0.25;
    setRange(channel, dataMin - padding, dataMax + padding);
    channel.hasRange = true;

    emit dataChanged(index(row), index(row), {RangeMinRole, RangeMaxRole, TickIntervalRole, LabelDecimalsRole});
}

double ChannelModel::niceTickInterval(double span, int targetTicks)
{
    if (span <= 0.0 || targetTicks < 1)
        return 1.0;

    const double rough = span / targetTicks;
    const double magnitude = std::pow(10.0, std::floor(std::log10(rough)));
    const double fraction = rough / magnitude;
    const double nice = fraction < 1.5 ? 1.0 : fraction < 3.0 ? 2.0 : fraction < 7.0 ? 5.0 : 10.0;
    return nice * magnitude;
}

void ChannelModel::setRange(Channel &channel, double lo, double hi)
{
    const double step = niceTickInterval(hi - lo);
    channel.tickInterval = step;
    channel.rangeMin = std::floor(lo / step) * step;
    channel.rangeMax = std::ceil(hi / step) * step;
    channel.labelDecimals = std::max(0, -int(std::floor(std::log10(step))));
}

void ChannelModel::reset()
{
    for (Channel &channel : m_channels) {
        channel.value = channel.minimum = channel.maximum = channel.sum = 0.0;
        channel.count = 0;
        channel.hasRange = false;
        setRange(channel, channel.defaultMin, channel.defaultMax);
    }

    if (!m_channels.isEmpty())
        emit dataChanged(index(0), index(rowCount() - 1));
}
