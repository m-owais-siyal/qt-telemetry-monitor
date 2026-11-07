#pragma once

#include "TelemetryTypes.h"

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

// Recent receiver events, newest first, capped at MaxEntries.
class EventLogModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("EventLogModel is owned by TelemetryController")

public:
    enum Severity { Info, Success, Warning, Error };
    Q_ENUM(Severity)

    enum Role {
        TimeRole = Qt::UserRole + 1,
        SeverityRole,
        MessageRole,
    };

    static constexpr int MaxEntries = 200;

    explicit EventLogModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addMessage(Severity severity, const QString &message, qint64 timeMs);
    void addReceiverEvents(const QList<ReceiverEvent> &events);
    Q_INVOKABLE void clear();

    static QString describe(const ReceiverEvent &event);

private:
    struct Entry
    {
        qint64 timeMs = 0;
        Severity severity = Info;
        QString message;
    };

    void prepend(const QList<Entry> &entries); // entries in chronological order

    QList<Entry> m_entries; // newest first
};
