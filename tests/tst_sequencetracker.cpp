#include "SequenceTracker.h"

#include <QtTest>

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
    void reset();
};

void TestSequenceTracker::firstPacket()
{
    SequenceTracker tracker;
    QCOMPARE(tracker.observe(1000), 0u);
    QCOMPARE(tracker.received(), 1u);
    QCOMPARE(tracker.lost(), 0u);
}

void TestSequenceTracker::inOrder()
{
    SequenceTracker tracker;
    for (quint32 seq = 0; seq < 4; ++seq)
        QCOMPARE(tracker.observe(seq), 0u);
    QCOMPARE(tracker.received(), 4u);
    QCOMPARE(tracker.lost(), 0u);
    QCOMPARE(tracker.lossPercent(), 0.0);
}

void TestSequenceTracker::gap()
{
    SequenceTracker tracker;
    tracker.observe(0);
    tracker.observe(1);
    QCOMPARE(tracker.observe(5), 3u); // 2, 3 and 4 are missing
    QCOMPARE(tracker.received(), 3u);
    QCOMPARE(tracker.lost(), 3u);
    QCOMPARE(tracker.lossPercent(), 50.0);
}

void TestSequenceTracker::wrapsAround()
{
    SequenceTracker tracker;
    QCOMPARE(tracker.observe(0xFFFFFFFE), 0u);
    QCOMPARE(tracker.observe(0xFFFFFFFF), 0u);
    QCOMPARE(tracker.observe(0), 0u);
    QCOMPARE(tracker.observe(1), 0u);
    QCOMPARE(tracker.received(), 4u);
    QCOMPARE(tracker.lost(), 0u);
}

void TestSequenceTracker::gapAcrossWrap()
{
    SequenceTracker tracker;
    tracker.observe(0xFFFFFFFE);
    QCOMPARE(tracker.observe(1), 2u); // 0xFFFFFFFF and 0 are missing
    QCOMPARE(tracker.lost(), 2u);
}

void TestSequenceTracker::ignoresDuplicates()
{
    SequenceTracker tracker;
    tracker.observe(0);
    tracker.observe(1);
    QCOMPARE(tracker.observe(1), 0u);
    QCOMPARE(tracker.received(), 2u);
    QCOMPARE(tracker.lost(), 0u);
}

void TestSequenceTracker::ignoresLatePackets()
{
    SequenceTracker tracker;
    tracker.observe(0);
    tracker.observe(1);
    tracker.observe(5);
    QCOMPARE(tracker.observe(3), 0u); // arrived after 5: ignored
    QCOMPARE(tracker.received(), 3u);
    QCOMPARE(tracker.lost(), 3u);
}

void TestSequenceTracker::reset()
{
    SequenceTracker tracker;
    tracker.observe(0);
    tracker.observe(10);
    tracker.reset();
    QCOMPARE(tracker.received(), 0u);
    QCOMPARE(tracker.lost(), 0u);
    QCOMPARE(tracker.observe(42), 0u);
    QCOMPARE(tracker.received(), 1u);
}

QTEST_APPLESS_MAIN(TestSequenceTracker)
#include "tst_sequencetracker.moc"
