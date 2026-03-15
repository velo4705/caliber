#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include "widgets/mode_sidebar.h"

class MathEngine;
class HistoryManager;
class HistoryPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onModeChanged(CalcMode mode);
    void toggleTheme();

private:
    void buildUI();
    void loadTheme(const QString& qrcPath);
    void setupMenuBar();

    ModeSidebar*    m_sidebar;
    QStackedWidget* m_stack;
    HistoryPanel*   m_historyPanel;

    MathEngine*     m_engine;
    HistoryManager* m_history;

    bool m_darkMode = false;
};
