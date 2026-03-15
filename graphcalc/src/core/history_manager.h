#pragma once
#include <QString>
#include <QStringList>

// Stores calculation history for the session.
// Entries are stored as "expression = result" strings.
class HistoryManager {
public:
    explicit HistoryManager(int maxEntries = 100);

    void        add(const QString& expression, const QString& result);
    void        clear();
    QStringList entries() const;   // newest first
    int         count()   const;

private:
    QStringList m_entries;
    int         m_maxEntries;
};
