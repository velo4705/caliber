#pragma once
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QStringList>

class HistoryPanel : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int drawerX READ drawerX WRITE setDrawerX)
public:
    explicit HistoryPanel(QWidget* parent = nullptr);

    void addEntry(const QString& entry);
    void clearEntries();
    void repositionToParent();
    void toggleDrawer();
    bool isDrawerOpen() const { return m_open; }

signals:
    void entryClicked(const QString& expression);
    void drawerToggled(bool open);

private:
    int  drawerX() const;
    void setDrawerX(int x);
    void filterEntries(const QString& query);

    QLineEdit*          m_search;
    QListWidget*        m_list;
    QPushButton*        m_clearBtn;
    QPropertyAnimation* m_anim;
    bool                m_open = false;
    QStringList         m_allEntries; // full unfiltered list

    static constexpr int PANEL_WIDTH = 260;
};
