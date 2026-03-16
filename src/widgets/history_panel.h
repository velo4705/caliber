#pragma once
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QPropertyAnimation>

// Overlay drawer — floats over content, toggled by toolbar button
class HistoryPanel : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int drawerX READ drawerX WRITE setDrawerX)
public:
    explicit HistoryPanel(QWidget* parent = nullptr);

    void addEntry(const QString& entry);
    void clearEntries();

    // Call after parent resizes to reposition the panel
    void repositionToParent();

    // Animated open/close
    void toggleDrawer();
    bool isDrawerOpen() const { return m_open; }

signals:
    void entryClicked(const QString& expression);
    void drawerToggled(bool open);  // emitted when drawer finishes opening or starts closing

private:
    int  drawerX() const;
    void setDrawerX(int x);

    QListWidget*        m_list;
    QPushButton*        m_clearBtn;
    QPropertyAnimation* m_anim;
    bool                m_open = false;

    static constexpr int PANEL_WIDTH = 240;
};
