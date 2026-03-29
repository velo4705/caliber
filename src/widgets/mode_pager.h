#pragma once
#include <QWidget>
#include <QVector>
#include "mode_sidebar.h" // for CalcMode enum

class QStackedWidget;
class QLabel;
class QToolButton;
class QPushButton;

class ModePager : public QWidget {
    Q_OBJECT
public:
    explicit ModePager(QWidget* parent = nullptr);

    QStackedWidget* stack() const { return m_stack; }
    CalcMode currentMode() const;

signals:
    void modeChanged(CalcMode mode);
    void settingsClicked();

private:
    void showModeMenu();

    QStackedWidget* m_stack;
    QPushButton*    m_modeButton;
    QToolButton*    m_prevBtn;
    QToolButton*    m_nextBtn;
    QToolButton*    m_settingsBtn;

    static const QVector<QPair<QString, CalcMode>> s_modes;
};
