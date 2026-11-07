#include "ChannelModel.h"

#include <QSignalSpy>
#include <cmath>
#include <QtTest>

namespace
{
TelemetrySample sample(float temperature, float pressure, float voltage)
{
    TelemetrySample s;
    s.channels = {temperature, pressure, voltage};
    return s;
}
} // namespace

class TestChannelModel : public QObject
{
    Q_OBJECT

private slots:
    void startsEmpty();
    void tracksLatestMinMaxAverage();
    void emitsOneDataChangedPerBatch();
    void axisRangeCoversData();
    void axisRangeIgnoresSmallChanges();
    void axisRangeTightensWhenFarTooWide();
    void resetClearsStatistics();
    void niceTickIntervals();
};

void TestChannelModel::startsEmpty()
{
    ChannelModel model;
    QCOMPARE(model.rowCount(), 3);

    const QModelIndex first = model.index(0);
    QCOMPARE(first.data(ChannelModel::NameRole).toString(), QStringLiteral("Temperature"));
    QCOMPARE(first.data(ChannelModel::HasDataRole).toBool(), false);
    QVERIFY(!first.data(ChannelModel::ValueRole).isValid());
}

void TestChannelModel::tracksLatestMinMaxAverage()
{
    ChannelModel model;
    model.addSamples({sample(20.0f, 101.0f, 3.0f), sample(30.0f, 100.0f, 3.5f), sample(25.0f, 102.0f, 3.25f)});

    const QModelIndex temperature = model.index(0);
    QCOMPARE(temperature.data(ChannelModel::HasDataRole).toBool(), true);
    QCOMPARE(temperature.data(ChannelModel::ValueRole).toDouble(), 25.0);
    QCOMPARE(temperature.data(ChannelModel::MinimumRole).toDouble(), 20.0);
    QCOMPARE(temperature.data(ChannelModel::MaximumRole).toDouble(), 30.0);
    QCOMPARE(temperature.data(ChannelModel::AverageRole).toDouble(), 25.0);

    const QModelIndex pressure = model.index(1);
    QCOMPARE(pressure.data(ChannelModel::MinimumRole).toDouble(), 100.0);
    QCOMPARE(pressure.data(ChannelModel::MaximumRole).toDouble(), 102.0);
}

void TestChannelModel::emitsOneDataChangedPerBatch()
{
    ChannelModel model;
    QSignalSpy spy(&model, &QAbstractItemModel::dataChanged);
    model.addSamples({sample(1, 2, 3), sample(4, 5, 6), sample(7, 8, 9)});
    QCOMPARE(spy.count(), 1);
}

void TestChannelModel::axisRangeCoversData()
{
    ChannelModel model;
    model.fitAxisRange(0, 20.0, 30.0);
    const double lo = model.index(0).data(ChannelModel::RangeMinRole).toDouble();
    const double hi = model.index(0).data(ChannelModel::RangeMaxRole).toDouble();
    QVERIFY(lo < 20.0);
    QVERIFY(hi > 30.0);
}

void TestChannelModel::axisRangeIgnoresSmallChanges()
{
    ChannelModel model;
    model.fitAxisRange(0, 20.0, 30.0);

    QSignalSpy spy(&model, &QAbstractItemModel::dataChanged);
    model.fitAxisRange(0, 21.0, 29.0); // still inside, not much narrower
    QCOMPARE(spy.count(), 0);

    model.fitAxisRange(0, 21.0, 40.0); // leaves the range
    QCOMPARE(spy.count(), 1);
    QVERIFY(model.index(0).data(ChannelModel::RangeMaxRole).toDouble() > 40.0);
}

void TestChannelModel::axisRangeTightensWhenFarTooWide()
{
    ChannelModel model;
    model.fitAxisRange(0, 0.0, 100.0);
    model.fitAxisRange(0, 50.0, 51.0); // data now fills 1% of the range
    const double lo = model.index(0).data(ChannelModel::RangeMinRole).toDouble();
    const double hi = model.index(0).data(ChannelModel::RangeMaxRole).toDouble();
    QVERIFY(hi - lo < 5.0);
}

void TestChannelModel::resetClearsStatistics()
{
    ChannelModel model;
    model.addSamples({sample(20.0f, 101.0f, 3.0f)});
    model.reset();
    QCOMPARE(model.index(0).data(ChannelModel::HasDataRole).toBool(), false);
    QVERIFY(!model.index(0).data(ChannelModel::AverageRole).isValid());
}

void TestChannelModel::niceTickIntervals()
{
    QCOMPARE(ChannelModel::niceTickInterval(10.0), 2.0);
    QCOMPARE(ChannelModel::niceTickInterval(15.0), 5.0);
    QCOMPARE(ChannelModel::niceTickInterval(0.5), 0.1);
    QCOMPARE(ChannelModel::niceTickInterval(40.0), 10.0);

    ChannelModel model;
    model.fitAxisRange(2, 3.21, 3.39);
    const double step = model.index(2).data(ChannelModel::TickIntervalRole).toDouble();
    const double lo = model.index(2).data(ChannelModel::RangeMinRole).toDouble();
    QCOMPARE(step, 0.05);
    QVERIFY(qAbs(std::remainder(lo, step)) < 1e-9); // the range starts on a tick
    QCOMPARE(model.index(2).data(ChannelModel::LabelDecimalsRole).toInt(), 2);
}

QTEST_APPLESS_MAIN(TestChannelModel)
#include "tst_channelmodel.moc"
