#include "UdpReceiver.h"

#include <QSignalSpy>
#include <QUdpSocket>
#include <QtTest>

// Drives a real UdpReceiver over the loopback interface.
class TestUdpReceiver : public QObject
{
    Q_OBJECT

private slots:
    void receivesBatchesAndDetectsLoss();
    void ignoresBatchesFromNoSession();
    void reportsBindErrors();

private:
    static void send(QUdpSocket &socket, quint16 port, quint32 sequence);
};

void TestUdpReceiver::send(QUdpSocket &socket, quint16 port, quint32 sequence)
{
    TelemetrySample sample;
    sample.sequence = sequence;
    sample.timestampMs = 1000 + sequence;
    sample.channels = {float(sequence), 101.0f, 3.3f};
    socket.writeDatagram(TelemetryPacket::encode(sample), QHostAddress::LocalHost, port);
}

void TestUdpReceiver::receivesBatchesAndDetectsLoss()
{
    UdpReceiver receiver;
    QList<ReceiverBatch> batches;
    connect(&receiver, &UdpReceiver::batchReady, this, [&](const ReceiverBatch &batch) { batches.append(batch); });

    receiver.start(QHostAddress::LocalHost, 0, 7);
    const quint16 port = receiver.boundPort();
    QVERIFY(port != 0);

    QUdpSocket sender;
    for (quint32 seq : {0u, 1u, 2u, 5u, 5u, 3u}) // gap of 2, a duplicate, a late packet
        send(sender, port, seq);
    sender.writeDatagram(QByteArray("not a telemetry packet"), QHostAddress::LocalHost, port);

    auto received = [&] {
        qsizetype count = 0;
        for (const ReceiverBatch &batch : batches)
            count += batch.samples.size();
        return count;
    };
    QTRY_COMPARE(received(), qsizetype(4));
    QTRY_VERIFY(!batches.isEmpty() && batches.last().stats.invalid == 1);

    QList<ReceiverEvent> events;
    for (const ReceiverBatch &batch : batches) {
        QCOMPARE(batch.session, quint64(7));
        events.append(batch.events);
    }

    const ReceiverStats &stats = batches.last().stats;
    QCOMPARE(stats.received, quint64(4));
    QCOMPARE(stats.lost, quint64(2));

    const auto gap = std::find_if(events.cbegin(), events.cend(),
                                  [](const ReceiverEvent &e) { return e.type == ReceiverEvent::Type::Gap; });
    QVERIFY(gap != events.cend());
    QCOMPARE(gap->sequence, 3u);
    QCOMPARE(gap->count, 2u);

    receiver.stop();
}

void TestUdpReceiver::ignoresBatchesFromNoSession()
{
    // Before start() nothing is listening, so no batches are produced.
    UdpReceiver receiver;
    QSignalSpy spy(&receiver, &UdpReceiver::batchReady);
    QTest::qWait(3 * UdpReceiver::FlushIntervalMs);
    QCOMPARE(spy.count(), 0);
}

void TestUdpReceiver::reportsBindErrors()
{
    UdpReceiver receiver;
    QSignalSpy errors(&receiver, &UdpReceiver::errorOccurred);

    // 192.0.2.1 is reserved for documentation (TEST-NET-1) and never a local address.
    receiver.start(QHostAddress(QStringLiteral("192.0.2.1")), 0, 3);

    QCOMPARE(errors.count(), 1);
    QCOMPARE(errors.first().at(0).value<quint64>(), quint64(3));
}

QTEST_GUILESS_MAIN(TestUdpReceiver)
#include "tst_udpreceiver.moc"
