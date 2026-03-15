#pragma once
#include <QWidget>
#include <QListWidget>
#include <QPushButton>

// Side panel showing calculation history
class HistoryPanel : public QWidget {
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget* parent = nullptr);

    void addEntry(const QString& entry);
    void clearEntries();

signals:
    void entryClicked(const QString& expression); // emits the expression part

private:
    QListWidget* m_list;
    QPushButton* m_clearBtn;
};
