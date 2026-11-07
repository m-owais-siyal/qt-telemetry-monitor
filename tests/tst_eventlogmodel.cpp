#include "EventLogModel.h"

#include <QtTest>

class TestEventLogModel : public QObject
{
    Q_OBJECT

private slots:
    void newestFirst();
    void describesEvents();
    void capsEntries();
    void clear();
};

void TestEventLogModel::newestFirst()
{
    EventLogModel model;
    model.addMessage(EventLogModel::Info, QStringLiteral("first"), 1000);
    model.addReceiverEvents({
        {ReceiverEvent::Type::Gap, 2000, 10, 1},
        {ReceiverEvent::Type::Gap, 3000, 20, 1},
    });

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.index(0).data(EventLogModel::MessageRole).toString(), QStringLiteral("Lost packet #20"));
    QCOMPARE(model.index(1).data(EventLogModel::MessageRole).toString(), QStringLiteral("Lost packet #10"));
    QCOMPARE(model.index(2).data(EventLogModel::MessageRole).toString(), QStringLiteral("first"));
    QCOMPARE(model.index(0).data(EventLogModel::SeverityRole).toInt(), int(EventLogModel::Warning));
}

void TestEventLogModel::describesEvents()
{
    const QString range = EventLogModel::describe({ReceiverEvent::Type::Gap, 0, 100, 3});
    QVERIFY(range.startsWith(QStringLiteral("Lost 3 packets (#100")));
    QVERIFY(range.endsWith(QStringLiteral("#102)")));

    QCOMPARE(EventLogModel::describe({ReceiverEvent::Type::Restart, 0, 0, 0}),
             QStringLiteral("Sender restarted at #0"));
    QCOMPARE(EventLogModel::describe({ReceiverEvent::Type::InvalidDatagrams, 0, 0, 4}),
             QStringLiteral("Ignored 4 invalid datagrams"));
}

void TestEventLogModel::capsEntries()
{
    EventLogModel model;
    QList<ReceiverEvent> events;
    for (int i = 0; i < EventLogModel::MaxEntries + 50; ++i)
        events.append({ReceiverEvent::Type::Gap, 0, quint32(i), 1});

    model.addReceiverEvents(events);
    QCOMPARE(model.rowCount(), EventLogModel::MaxEntries);
    QCOMPARE(model.index(0).data(EventLogModel::MessageRole).toString(),
             QStringLiteral("Lost packet #%1").arg(EventLogModel::MaxEntries + 49));

    model.addMessage(EventLogModel::Info, QStringLiteral("newest"), 0);
    QCOMPARE(model.rowCount(), EventLogModel::MaxEntries);
    QCOMPARE(model.index(0).data(EventLogModel::MessageRole).toString(), QStringLiteral("newest"));
}

void TestEventLogModel::clear()
{
    EventLogModel model;
    model.addMessage(EventLogModel::Info, QStringLiteral("hello"), 0);
    model.clear();
    QCOMPARE(model.rowCount(), 0);
}

QTEST_APPLESS_MAIN(TestEventLogModel)
#include "tst_eventlogmodel.moc"
