#include "TelemetryPacket.h"

#include <QtTest>

class TestTelemetryPacket : public QObject
{
    Q_OBJECT

private slots:
    void roundTrip();
    void sizeAndMagic();
    void rejectsWrongSize();
    void rejectsWrongMagic();
};

void TestTelemetryPacket::roundTrip()
{
    TelemetrySample in;
    in.sequence = 123456;
    in.timestampMs = 1731000000000;
    in.channels = {21.5f, 101.325f, 3.3f};

    const auto out = TelemetryPacket::decode(TelemetryPacket::encode(in));
    QVERIFY(out.has_value());
    QCOMPARE(out->sequence, in.sequence);
    QCOMPARE(out->timestampMs, in.timestampMs);
    for (size_t i = 0; i < in.channels.size(); ++i)
        QCOMPARE(out->channels[i], in.channels[i]);
}

void TestTelemetryPacket::sizeAndMagic()
{
    const QByteArray bytes = TelemetryPacket::encode(TelemetrySample{});
    QCOMPARE(bytes.size(), TelemetryPacket::Size);
    QCOMPARE(bytes.left(4), QByteArray("TLM1"));
}

void TestTelemetryPacket::rejectsWrongSize()
{
    QVERIFY(!TelemetryPacket::decode(QByteArray(10, '\0')).has_value());

    QByteArray tooLong = TelemetryPacket::encode(TelemetrySample{});
    tooLong.append('\0');
    QVERIFY(!TelemetryPacket::decode(tooLong).has_value());
}

void TestTelemetryPacket::rejectsWrongMagic()
{
    QByteArray bytes = TelemetryPacket::encode(TelemetrySample{});
    QCOMPARE(bytes.size(), TelemetryPacket::Size);
    bytes[0] = 'X';
    QVERIFY(!TelemetryPacket::decode(bytes).has_value());
}

QTEST_APPLESS_MAIN(TestTelemetryPacket)
#include "tst_telemetrypacket.moc"
