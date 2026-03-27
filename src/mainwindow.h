#pragma once
#include <QMainWindow>
#include <QToolButton>
#include <QLineEdit>
#include <QCompleter>
#include <QMap>
#include <QPair>
#include "widgets/mode_sidebar.h"

class AnimatedStackedWidget;
class QSplitter;

class MathEngine;
class HistoryManager;
class HistoryPanel;
class FormulaPanel;
class QActionGroup;

enum class ThemeMode {
    System = 0, Light, Dark,
    Midnight, Dracula, Nord, Monokai, Solarized,
    HighContrast,
    Custom,
    Gradient
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
    void loadCommunityThemes();
    void loadGradientTheme();
    void syncGraphTheme(bool dark);
    void buildSearchIndex();
    void onSearchActivated(const QString& text = {});
    void applyAccentColor();

    ModeSidebar*          m_sidebar;
    AnimatedStackedWidget* m_stack;
    HistoryPanel*   m_historyPanel;
    FormulaPanel*   m_formulaPanel = nullptr;
    QToolButton*    m_historyBtn   = nullptr;
    QToolButton*    m_formulaBtn   = nullptr;
    QLineEdit*      m_searchBar    = nullptr;
    // keyword → (stack index, tab index within that mode's QTabWidget)
    QMap<QString, QPair<int,int>> m_searchIndex;
    QWidget*        m_central    = nullptr;
    QLayout*        m_rootLayout = nullptr;
    QSplitter*      m_splitter   = nullptr; // for resizable sidebar

    MathEngine*     m_engine;
    HistoryManager* m_history;

    ThemeMode       m_themeMode = ThemeMode::System;
    QString         m_customThemePath;
    QActionGroup*   m_themeGroup = nullptr;
    int             m_fontSize   = 12;
    QString         m_accentColor; // empty = default (no override)

    // Gradient theme colors
    QColor          m_gradientStart;
    QColor          m_gradientEnd;
    bool            m_gradientDarkBase = true;
    int             m_gradientAngle    = 45;
};
