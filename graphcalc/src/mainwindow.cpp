#include "mainwindow.h"
#include "core/math_engine.h"
#include "core/history_manager.h"
#include "widgets/history_panel.h"
#include "modes/basic/basic_widget.h"

#include <QHBoxLayout>
#include <QWidget>
#include <QFile>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_engine  = new MathEngine();
    m_history = new HistoryManager();

    buildUI();
    setupMenuBar();
    loadTheme(":/themes/light.qss");

    setWindowTitle("GraphCalc");
    setMinimumSize(700, 520);
    resize(820, 580);
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

    // Sidebar
    m_sidebar = new ModeSidebar(this);
    root->addWidget(m_sidebar);

    // Stacked content area
    m_stack = new QStackedWidget(this);

    // History panel
    m_historyPanel = new HistoryPanel(this);

    // --- Milestone 1: Basic mode ---
    auto* basicWidget = new BasicWidget(m_engine, m_history, m_historyPanel, this);
    m_stack->addWidget(basicWidget); // index 0 = Basic

    // Placeholder widgets for future modes (Milestones 2-7)
    const QStringList placeholders = {
        "Scientific — coming in Milestone 2",
        "Programming — coming in Milestone 3",
        "Date Calculation — coming in Milestone 4",
        "Conversion — coming in Milestone 5",
        "Equations & Matrices — coming in Milestone 6",
        "Graphing — coming in Milestone 7",
    };
    for (const auto& text : placeholders) {
        auto* ph = new QLabel(text, this);
        ph->setAlignment(Qt::AlignCenter);
        ph->setStyleSheet("color: #888; font-size: 16px;");
        m_stack->addWidget(ph);
    }

    root->addWidget(m_stack, 1);
    root->addWidget(m_historyPanel);

    setCentralWidget(central);

    connect(m_sidebar, &ModeSidebar::modeChanged, this, &MainWindow::onModeChanged);
}

void MainWindow::setupMenuBar() {
    auto* viewMenu = menuBar()->addMenu("View");

    auto* themeAction = new QAction("Toggle Dark Mode", this);
    themeAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(themeAction, &QAction::triggered, this, &MainWindow::toggleTheme);
    viewMenu->addAction(themeAction);
}

void MainWindow::onModeChanged(CalcMode mode) {
    m_stack->setCurrentIndex(static_cast<int>(mode));
}

void MainWindow::toggleTheme() {
    m_darkMode = !m_darkMode;
    loadTheme(m_darkMode ? ":/themes/dark.qss" : ":/themes/light.qss");
}

void MainWindow::loadTheme(const QString& qrcPath) {
    QFile f(qrcPath);
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
        f.close();
    }
}
