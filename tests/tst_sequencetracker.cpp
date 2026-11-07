#include "SequenceTracker.h"

#include <QtTest>

using Kind = SequenceTracker::Kind;

class TestSequenceTracker : public QObject
{
    Q_OBJECT

private slots:
    void firstPacket();
    void inOrder();
    void gap();
    void wrapsAround();
    void gapAcrossWrap();
    void ignoresDuplicates();
    void ignoresLatePackets();
    void resyncsAfterSenderRestart();
    void reset();
};

void TestSequenceTracker::firstPacket()
{
    SequenceTracker tracker;
    const auto observation = tracker.observe(1000);
    QCOMPARE(observation.kind, Kind::First);
    QVERIFY(observation.accepted());
    QCOMPARE(tracker.received(), 1u);
    QCOMPARE(tracker.lost(), 0u);
}

void TestSequenceTracker::inOrder()
{
    SequenceTracker tracker;
    tracker.observe(0);
    for (quint32 seq = 1; seq < 4; ++seq) {
        const auto observation = tracker.observe(seq);
        QCOMPARE(observation.kind, Kind::InOrder);
        QCOMPARE(observation.missing, 0u);
    }
    QCOMPARE(tracker.received(), 4u);
    QCOMPARE(tracker.lost(), 0u);
    QCOMPARE(tracker.lossPercent(), 0.0);
}

void TestSequenceTracker::gap()
{
    SequenceTracker tracker;
    tracker.observe(0);
    tracker.observe(1);
    const auto observation = tracker.observe(5); // 2, 3 and 4 are missing
    QCOMPARE(observation.kind, Kind::Gap);
    QCOMPARE(observation.missing, 3u);
    QCOMPARE(tracker.received(), 3u);
    QCOMPARE(tracker.lost(), 3u);
    QCOMPARE(tracker.lossPercent(), 50.0);
}

void TestSequenceTracker::wrapsAround()
{
    SequenceTracker tracker;
    tracker.observe(0xFFFFFFFE);
    QCOMPARE(tracker.observe(0xFFFFFFFF).kind, Kind::InOrder);
    QCOMPARE(tracker.observe(0).kind, Kind::InOrder);
    QCOMPARE(tracker.observe(1).kind, Kind::InOrder);
    QCOMPARE(tracker.received(), 4u);
    QCOMPARE(tracker.lost(), 0u);
}

void TestSequenceTracker::gapAcrossWrap()
{
    SequenceTracker tracker;
    tracker.observe(0xFFFFFFFE);
    const auto observation = tracker.observe(1); // 0xFFFFFFFF and 0 are missing
    QCOMPARE(observation.kind, Kind::Gap);
    QCOMPARE(observation.missing, 2u);
    QCOMPARE(tracker.lost(), 2u);
}

void TestSequenceTracker::ignoresDuplicates()
{
    SequenceTracker tracker;
    tracker.observe(0);
    tracker.observe(1);
    const auto observation = tracker.observe(1);
    QCOMPARE(observation.kind, Kind::Duplicate);
    QVERIFY(!observation.accepted());
    QCOMPARE(tracker.received(), 2u);
    QCOMPARE(tracker.lost(), 0u);
}

void TestSequenceTracker::ignoresLatePackets()
{
    SequenceTracker tracker;
    tracker.observe(0);
    tracker.observe(1);
    tracker.observe(5);
    const auto observation = tracker.observe(3); // arrived after 5
    QCOMPARE(observation.kind, Kind::Late);
    QVERIFY(!observation.accepted());
    QCOMPARE(tracker.received(), 3u);
    QCOMPARE(tracker.lost(), 3u);

    // Still tracking from 5, not from the late packet.
    QCOMPARE(tracker.observe(6).kind, Kind::InOrder);
}

void TestSequenceTracker::resyncsAfterSenderRestart()
{
    SequenceTracker tracker;
    tracker.observe(5000);
    tracker.observe(5001);

    const auto observation = tracker.observe(0); // sender restarted from zero
    QCOMPARE(observation.kind, Kind::Restart);
    QVERIFY(observation.accepted());
    QCOMPARE(tracker.restarts(), 1u);
    QCOMPARE(tracker.lost(), 0u);
    QCOMPARE(tracker.received(), 3u);

    QCOMPARE(tracker.observe(1).kind, Kind::InOrder);
}

void TestSequenceTracker::reset()
{
    SequenceTracker tracker;
    tracker.observe(0);
    tracker.observe(10);
    tracker.reset();
    QCOMPARE(tracker.received(), 0u);
    QCOMPARE(tracker.lost(), 0u);
    QCOMPARE(tracker.restarts(), 0u);
    QCOMPARE(tracker.observe(42).kind, Kind::First);
    QCOMPARE(tracker.received(), 1u);
}

QTEST_APPLESS_MAIN(TestSequenceTracker)
#include "tst_sequencetracker.moc"
