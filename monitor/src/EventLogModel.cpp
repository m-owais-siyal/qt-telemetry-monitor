#include "EventLogModel.h"

#include <QDateTime>

#include <algorithm>

EventLogModel::EventLogModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int EventLogModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_entries.size());
}

QVariant EventLogModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return {};

    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case TimeRole:
        return QDateTime::fromMSecsSinceEpoch(entry.timeMs).toString(QStringLiteral("HH:mm:ss"));
    case SeverityRole:
        return entry.severity;
    case MessageRole:
        return entry.message;
    default:
        return {};
    }
}

QHash<int, QByteArray> EventLogModel::roleNames() const
{
    return {
        {TimeRole, "time"},
        {SeverityRole, "severity"},
        {MessageRole, "message"},
    };
}

void EventLogModel::addMessage(Severity severity, const QString &message, qint64 timeMs)
{
    prepend({Entry{timeMs, severity, message}});
}

void EventLogModel::addReceiverEvents(const QList<ReceiverEvent> &events)
{
    if (events.isEmpty())
        return;

    QList<Entry> entries;
    entries.reserve(events.size());
    for (const ReceiverEvent &event : events) {
        const Severity severity = event.type == ReceiverEvent::Type::Restart ? Info : Warning;
        entries.append({event.timeMs, severity, describe(event)});
    }
    prepend(entries);
}

void EventLogModel::clear()
{
    if (m_entries.isEmpty())
        return;
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

QString EventLogModel::describe(const ReceiverEvent &event)
{
    switch (event.type) {
    case ReceiverEvent::Type::Gap:
        if (event.count == 1)
            return tr("Lost packet #%1").arg(event.sequence);
        return tr("Lost %1 packets (#%2%3#%4)")
            .arg(event.count)
            .arg(event.sequence)
            .arg(QChar(0x2013)) // en dash
            .arg(event.sequence + event.count - 1);
    case ReceiverEvent::Type::Restart:
        return tr("Sender restarted at #%1").arg(event.sequence);
    case ReceiverEvent::Type::InvalidDatagrams:
        if (event.count == 1)
            return tr("Ignored 1 invalid datagram");
        return tr("Ignored %1 invalid datagrams").arg(event.count);
    }
    return {};
}

void EventLogModel::prepend(const QList<Entry> &entries)
{
    if (entries.isEmpty())
        return;

    // Only the newest MaxEntries of this batch can ever be shown.
    const qsizetype count = std::min<qsizetype>(entries.size(), MaxEntries);
    const auto newest = entries.cend() - count;

    beginInsertRows({}, 0, int(count - 1));
    QList<Entry> merged;
    merged.reserve(count + m_entries.size());
    std::reverse_copy(newest, entries.cend(), std::back_inserter(merged));
    merged.append(m_entries);
    m_entries = std::move(merged);
    endInsertRows();

    if (m_entries.size() > MaxEntries) {
        beginRemoveRows({}, MaxEntries, int(m_entries.size() - 1));
        m_entries.resize(MaxEntries);
        endRemoveRows();
    }
}
