#include "mainwindow.h"
#include "core/math_engine.h"
#include "core/history_manager.h"
#include "widgets/history_panel.h"
#include "modes/basic/basic_widget.h"
#include "modes/scientific/scientific_widget.h"
#include "modes/programming/programming_widget.h"
#include "modes/date/date_widget.h"
#include "modes/conversion/conversion_widget.h"
#include "modes/equations/equations_widget.h"
#include "modes/graphing/graphing_widget.h"

#include <QHBoxLayout>
#include <QWidget>
#include <QFile>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QSettings>
#include <QStyleHints>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QShortcut>
#include <QKeySequence>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_engine  = new MathEngine();
    m_history = new HistoryManager();

    buildUI();
    setupMenuBar();
    restoreSettings();

    setWindowTitle("Caliber");
    setMinimumSize(720, 540);

    // System theme change signal (Qt6.5+)
    connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged,
            this, &MainWindow::onSystemThemeChanged);
}

MainWindow::~MainWindow() {
    delete m_engine;
    delete m_history;
}

void MainWindow::buildUI() {
    auto* central = new QWidget(this);
    auto* root    = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_sidebar      = new ModeSidebar(this);
    m_stack        = new QStackedWidget(this);
    m_historyPanel = new HistoryPanel(this);

    m_stack->addWidget(new BasicWidget      (m_engine, m_history, m_historyPanel, this)); // 0
    m_stack->addWidget(new ScientificWidget (m_engine, m_history, m_historyPanel, this)); // 1
    m_stack->addWidget(new ProgrammingWidget(m_history, m_historyPanel, this));           // 2
    m_stack->addWidget(new DateWidget       (this));                                       // 3
    m_stack->addWidget(new ConversionWidget (this));                                       // 4
    m_stack->addWidget(new EquationsWidget  (this));                                       // 5
    m_stack->addWidget(new GraphingWidget   (this));                                       // 6

    root->addWidget(m_sidebar);
    root->addWidget(m_stack, 1);
    root->addWidget(m_historyPanel);
    setCentralWidget(central);

    connect(m_sidebar, &ModeSidebar::modeChanged, this, &MainWindow::onModeChanged);
}

void MainWindow::setupMenuBar() {
    // ── View menu ─────────────────────────────────────────────────────────────
    auto* viewMenu = menuBar()->addMenu("&View");

    // Theme submenu
    auto* themeMenu = viewMenu->addMenu("Theme");
    m_themeGroup = new QActionGroup(this);
    m_themeGroup->setExclusive(true);

    auto addTheme = [&](const QString& label, ThemeMode mode, const QString& shortcut) {
        auto* a = themeMenu->addAction(label);
        a->setCheckable(true);
        a->setShortcut(QKeySequence(shortcut));
        m_themeGroup->addAction(a);
        connect(a, &QAction::triggered, this, [this, mode]{ setThemeMode(mode); });
        return a;
    };

    addTheme("Follow System", ThemeMode::System, "Ctrl+Shift+S")->setChecked(true);
    addTheme("Light",         ThemeMode::Light,  "Ctrl+Shift+L");
    addTheme("Dark",          ThemeMode::Dark,   "Ctrl+Shift+D");

    viewMenu->addSeparator();

    // History panel toggle
    auto* histAction = viewMenu->addAction("Toggle History Panel");
    histAction->setShortcut(QKeySequence("Ctrl+H"));
    histAction->setCheckable(true);
    histAction->setChecked(true);
    connect(histAction, &QAction::triggered, this, [this](bool checked){
        m_historyPanel->setVisible(checked);
    });

    viewMenu->addSeparator();

    // Mode shortcuts
    auto* modeMenu = menuBar()->addMenu("&Mode");
    const QStringList modeNames = {
        "Basic", "Scientific", "Programming",
        "Date", "Conversion", "Equations", "Graphing"
    };
    for (int i = 0; i < modeNames.size(); ++i) {
        auto* a = modeMenu->addAction(modeNames[i]);
        a->setShortcut(QKeySequence(QString("Ctrl+%1").arg(i + 1)));
        connect(a, &QAction::triggered, this, [this, i]{
            m_sidebar->setCurrentMode(static_cast<CalcMode>(i));
            m_stack->setCurrentIndex(i);
        });
    }
}

// ── Theme ─────────────────────────────────────────────────────────────────────

void MainWindow::setThemeMode(ThemeMode mode) {
    m_themeMode = mode;
    applyTheme();
    saveSettings();
}

void MainWindow::onSystemThemeChanged() {
    if (m_themeMode == ThemeMode::System)
        applyTheme();
}

void MainWindow::applyTheme() {
    bool dark = false;
    switch (m_themeMode) {
    case ThemeMode::Dark:   dark = true;  break;
    case ThemeMode::Light:  dark = false; break;
    case ThemeMode::System:
        dark = (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark);
        break;
    }
    loadTheme(dark ? ":/themes/dark.qss" : ":/themes/light.qss");
}

void MainWindow::loadTheme(const QString& qrcPath) {
    QFile f(qrcPath);
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
        f.close();
    }
}

// ── Settings persistence ──────────────────────────────────────────────────────

void MainWindow::saveSettings() {
    QSettings s("GraphCalc", "GraphCalc");
    s.setValue("window/geometry", saveGeometry());
    s.setValue("window/state",    saveState());
    s.setValue("ui/mode",         m_stack->currentIndex());
    s.setValue("ui/theme",        static_cast<int>(m_themeMode));
    s.setValue("ui/historyVisible", m_historyPanel->isVisible());
}

void MainWindow::restoreSettings() {
    QSettings s("GraphCalc", "GraphCalc");

    if (s.contains("window/geometry"))
        restoreGeometry(s.value("window/geometry").toByteArray());
    else
        resize(860, 600);

    if (s.contains("window/state"))
        restoreState(s.value("window/state").toByteArray());

    int mode = s.value("ui/mode", 0).toInt();
    m_stack->setCurrentIndex(mode);
    m_sidebar->setCurrentMode(static_cast<CalcMode>(mode));

    m_themeMode = static_cast<ThemeMode>(s.value("ui/theme", 0).toInt());

    // Sync theme menu checkmarks
    if (m_themeGroup) {
        auto actions = m_themeGroup->actions();
        int idx = static_cast<int>(m_themeMode);
        if (idx < actions.size()) actions[idx]->setChecked(true);
    }

    bool histVisible = s.value("ui/historyVisible", true).toBool();
    m_historyPanel->setVisible(histVisible);

    applyTheme();
}

// ── Events ────────────────────────────────────────────────────────────────────

void MainWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    // Ctrl+1..7 — switch modes
    if (event->modifiers() == Qt::ControlModifier) {
        int key = event->key() - Qt::Key_1;
        if (key >= 0 && key <= 6) {
            m_stack->setCurrentIndex(key);
            m_sidebar->setCurrentMode(static_cast<CalcMode>(key));
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::onModeChanged(CalcMode mode) {
    m_stack->setCurrentIndex(static_cast<int>(mode));
    saveSettings();
}
