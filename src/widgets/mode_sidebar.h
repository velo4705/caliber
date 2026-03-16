#pragma once
#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QVector>
#include <QBoxLayout>

enum class CalcMode {
    Basic = 0,
    Scientific,
    Programming,
    Date,
    Conversion,
    Equations,
    Graphing
};

enum class SidebarOrientation { Vertical, Horizontal };

// Mode selector — switches between vertical (landscape) and horizontal (portrait)
class ModeSidebar : public QWidget {
    Q_OBJECT
public:
    explicit ModeSidebar(QWidget* parent = nullptr);

    void setCurrentMode(CalcMode mode);
    void setOrientation(SidebarOrientation orientation);
    SidebarOrientation orientation() const { return m_orientation; }

signals:
    void modeChanged(CalcMode mode);

private:
    void buildLayout();

    QButtonGroup*          m_group;
    QVector<QPushButton*>  m_buttons;
    QBoxLayout*            m_layout = nullptr;
    SidebarOrientation     m_orientation = SidebarOrientation::Vertical;

    // Mode definitions
    struct ModeEntry { QString label; QString icon; CalcMode mode; };
    static const QVector<ModeEntry> s_modes;
};
