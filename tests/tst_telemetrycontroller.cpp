#include "TelemetryController.h"

#include <QSignalSpy>
#include <QUdpSocket>
#include <QtTest>

class TestTelemetryController : public QObject
{
    Q_OBJECT

private slots:
    void rejectsInvalidAddresses_data();
    void rejectsInvalidAddresses();
    void startLogsAndWaits();
    void receivesDataAndGoesStale();
    void stopReturnsToIdle();

private:
    static QString firstMessage(TelemetryController &controller);
};

QString TestTelemetryController::firstMessage(TelemetryController &controller)
{
    return controller.events()->index(0).data(EventLogModel::MessageRole).toString();
}

void TestTelemetryController::rejectsInvalidAddresses_data()
{
    QTest::addColumn<QString>("address");
    QTest::newRow("empty") << QString();
    QTest::newRow("text") << QStringLiteral("not-an-address");
    QTest::newRow("ipv4 shorthand") << QStringLiteral("239.255.42");
}

void TestTelemetryController::rejectsInvalidAddresses()
{
    QFETCH(QString, address);

    TelemetryController controller;
    controller.setAddress(address);
    controller.start();

    QCOMPARE(controller.status(), TelemetryController::Error);
    QVERIFY(!controller.isRunning());
    QVERIFY(!controller.errorMessage().isEmpty());
}

void TestTelemetryController::startLogsAndWaits()
{
    TelemetryController controller;
    controller.setAddress(QStringLiteral("127.0.0.1"));
    controller.setPort(47231);
    controller.start();

    QVERIFY(controller.isRunning());
    QCOMPARE(controller.status(), TelemetryController::Waiting);
    QCOMPARE(controller.events()->rowCount(), 1);
    QCOMPARE(firstMessage(controller), QStringLiteral("Listening on 127.0.0.1:47231"));
}

void TestTelemetryController::receivesDataAndGoesStale()
{
    TelemetryController controller;
    controller.setAddress(QStringLiteral("127.0.0.1"));
    controller.setPort(47232);
    controller.start();
    QTest::qWait(100); // let the worker thread bind the socket

    QUdpSocket sender;
    for (quint32 seq : {0u, 1u, 3u}) {
        TelemetrySample sample;
        sample.sequence = seq;
        sample.timestampMs = 1000 + 20 * seq;
        sample.channels = {25.0f, 101.3f, 3.3f};
        sender.writeDatagram(TelemetryPacket::encode(sample), QHostAddress::LocalHost, 47232);
    }

    QTRY_COMPARE(controller.status(), TelemetryController::Receiving);
    QTRY_COMPARE(controller.receivedPackets(), 3.0);
    QCOMPARE(controller.lostPackets(), 1.0);
    QCOMPARE(controller.channels()->index(0).data(ChannelModel::ValueRole).toDouble(), 25.0);

    QTRY_COMPARE_WITH_TIMEOUT(controller.status(), TelemetryController::Stale,
                              TelemetryController::StaleTimeoutMs + 1000);
}

void TestTelemetryController::stopReturnsToIdle()
{
    TelemetryController controller;
    controller.setAddress(QStringLiteral("127.0.0.1"));
    controller.setPort(47233);
    controller.start();
    controller.stop();

    QVERIFY(!controller.isRunning());
    QCOMPARE(controller.status(), TelemetryController::Idle);
    QCOMPARE(firstMessage(controller), QStringLiteral("Stopped"));
}

QTEST_GUILESS_MAIN(TestTelemetryController)
#include "tst_telemetrycontroller.moc"
