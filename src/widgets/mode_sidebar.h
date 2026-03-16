#pragma once
#include <QWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPushButton>
#include <QVector>

enum class CalcMode {
    Basic = 0,
    Scientific,
    Programming,
    Date,
    Conversion,
    Equations,
    Graphing
};

// Vertical sidebar with mode selection buttons
class ModeSidebar : public QWidget {
    Q_OBJECT
public:
    explicit ModeSidebar(QWidget* parent = nullptr);

    void setCurrentMode(CalcMode mode);

signals:
    void modeChanged(CalcMode mode);

private:
    QButtonGroup*        m_group;
    QVector<QPushButton*> m_buttons;

    QPushButton* addModeButton(const QString& label, CalcMode mode);
};
