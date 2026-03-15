#include "history_manager.h"

HistoryManager::HistoryManager(int maxEntries)
    : m_maxEntries(maxEntries) {}

void HistoryManager::add(const QString& expression, const QString& result) {
    if (expression.trimmed().isEmpty()) return;
    m_entries.prepend(QString("%1 = %2").arg(expression, result));
    if (m_entries.size() > m_maxEntries)
        m_entries.removeLast();
}

void HistoryManager::clear() {
    m_entries.clear();
}

QStringList HistoryManager::entries() const {
    return m_entries;
}

int HistoryManager::count() const {
    return m_entries.size();
}
