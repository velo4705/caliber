#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QToolButton>
#include "widgets/mode_sidebar.h"

class MathEngine;
class HistoryManager;
class HistoryPanel;
class QActionGroup;

enum class ThemeMode {
    System = 0, Light, Dark,
    Midnight, Dracula, Nord, Monokai, Solarized,
    Custom
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

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
    void applyLayout(bool portrait);
    void loadCustomTheme();

    ModeSidebar*    m_sidebar;
    QStackedWidget* m_stack;
    HistoryPanel*   m_historyPanel;
    QToolButton*    m_historyBtn = nullptr;  // top-right toggle button
    QWidget*        m_central    = nullptr;
    QLayout*        m_rootLayout = nullptr;

    MathEngine*     m_engine;
    HistoryManager* m_history;

    ThemeMode       m_themeMode = ThemeMode::System;
    QString         m_customThemePath;
    QActionGroup*   m_themeGroup = nullptr;
};
