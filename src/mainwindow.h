#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include "widgets/mode_sidebar.h"

class MathEngine;
class HistoryManager;
class HistoryPanel;
class QActionGroup;

enum class ThemeMode { System, Light, Dark };

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onModeChanged(CalcMode mode);
    void setThemeMode(ThemeMode mode);
    void onSystemThemeChanged();

private:
    void buildUI();
    void setupMenuBar();
    void applyTheme();
    void loadTheme(const QString& qrcPath);
    void saveSettings();
    void restoreSettings();

    ModeSidebar*    m_sidebar;
    QStackedWidget* m_stack;
    HistoryPanel*   m_historyPanel;

    MathEngine*     m_engine;
    HistoryManager* m_history;

    ThemeMode       m_themeMode = ThemeMode::System;
    QActionGroup*   m_themeGroup = nullptr;
};
