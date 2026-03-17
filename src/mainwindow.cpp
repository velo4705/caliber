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
#include <QVBoxLayout>
#include <QWidget>
#include <QFile>
#include <QIcon>
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
#include <QResizeEvent>
#include <QToolButton>
#include <QToolBar>
#include <QShortcut>
#include <QKeySequence>
#include <QFileDialog>
#include <QDir>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_engine  = new MathEngine();
    m_history = new HistoryManager();

    buildUI();
    setupMenuBar();
    restoreSettings();

    setWindowTitle("Caliber");
    setMinimumSize(800, 560);
    setWindowIcon(QIcon(":/icons/caliber.svg"));

    connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged,
            this, &MainWindow::onSystemThemeChanged);
}

MainWindow::~MainWindow() {
    delete m_engine;
    delete m_history;
}

void MainWindow::buildUI() {
    m_central      = new QWidget(this);
    m_sidebar      = new ModeSidebar(this);
    m_stack        = new QStackedWidget(this);

    // History panel is parented to m_central so it overlays the content area
    m_historyPanel = new HistoryPanel(m_central);

    m_stack->addWidget(new BasicWidget      (m_engine, m_history, m_historyPanel, this)); // 0
    m_stack->addWidget(new ScientificWidget (m_engine, m_history, m_historyPanel, this)); // 1
    m_stack->addWidget(new ProgrammingWidget(m_history, m_historyPanel, this));           // 2
    m_stack->addWidget(new DateWidget       (this));                                       // 3
    m_stack->addWidget(new ConversionWidget (this));                                       // 4
    m_stack->addWidget(new EquationsWidget  (this));                                       // 5
    m_stack->addWidget(new GraphingWidget   (this));                                       // 6

    // ── Toolbar with history toggle button ────────────────────────────────────
    auto* toolbar = addToolBar("Main");
    toolbar->setObjectName("mainToolbar");
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setIconSize(QSize(18, 18));

    // Spacer to push button to the right
    auto* spacer = new QWidget(toolbar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    m_historyBtn = new QToolButton(toolbar);
    m_historyBtn->setText("History  ⏱");
    m_historyBtn->setCheckable(true);
    m_historyBtn->setChecked(false);
    m_historyBtn->setToolTip("Toggle history panel  (Ctrl+H)");
    m_historyBtn->setObjectName("historyToggleBtn");
    m_historyBtn->setFocusPolicy(Qt::NoFocus);
    toolbar->addWidget(m_historyBtn);

    setCentralWidget(m_central);
    applyLayout(false);

    connect(m_sidebar, &ModeSidebar::modeChanged, this, &MainWindow::onModeChanged);
    connect(m_historyBtn, &QToolButton::toggled, this, [this](bool checked) {
        if (checked != m_historyPanel->isDrawerOpen())
            m_historyPanel->toggleDrawer();
    });

    // When history drawer toggles, shrink 3D container so it doesn't overlap
    connect(m_historyPanel, &HistoryPanel::drawerToggled, this, [this](bool open) {
        auto* gw = qobject_cast<GraphingWidget*>(m_stack->widget(6));
        if (gw) gw->adjustFor3DOverlap(open, 240);
    });
}

void MainWindow::applyLayout(bool portrait) {
    if (m_rootLayout) {
        while (m_rootLayout->count())
            m_rootLayout->takeAt(0);
        delete m_rootLayout;
        m_rootLayout = nullptr;
    }

    if (portrait) {
        m_sidebar->setOrientation(SidebarOrientation::Horizontal);
        auto* vl = new QVBoxLayout(m_central);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(0);
        vl->addWidget(m_stack, 1);
        vl->addWidget(m_sidebar);
        m_rootLayout = vl;
    } else {
        m_sidebar->setOrientation(SidebarOrientation::Vertical);
        auto* hl = new QHBoxLayout(m_central);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(0);
        hl->addWidget(m_sidebar);
        hl->addWidget(m_stack, 1);
        m_rootLayout = hl;
    }

    m_central->setLayout(m_rootLayout);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    bool portrait    = (event->size().height() > event->size().width());
    bool wasPortrait = (m_sidebar->orientation() == SidebarOrientation::Horizontal);
    if (portrait != wasPortrait)
        applyLayout(portrait);

    // Keep overlay panel correctly positioned after any resize
    if (m_historyPanel)
        m_historyPanel->repositionToParent();
}

// ── Menu bar ──────────────────────────────────────────────────────────────────

void MainWindow::setupMenuBar() {
    auto* viewMenu  = menuBar()->addMenu("&View");
    auto* themeMenu = viewMenu->addMenu("Theme");
    m_themeGroup = new QActionGroup(this);
    m_themeGroup->setExclusive(true);

    struct ThemeDef { QString label; ThemeMode mode; QString shortcut; };
    const QList<ThemeDef> themes = {
        { "Follow System",  ThemeMode::System,    "Ctrl+Shift+S" },
        { "Light",          ThemeMode::Light,     "Ctrl+Shift+L" },
        { "Dark",           ThemeMode::Dark,      "Ctrl+Shift+D" },
        { "Midnight Blue",  ThemeMode::Midnight,  ""             },
        { "Dracula",        ThemeMode::Dracula,   ""             },
        { "Nord",           ThemeMode::Nord,      ""             },
        { "Monokai",        ThemeMode::Monokai,   ""             },
        { "Solarized Dark", ThemeMode::Solarized, ""             },
    };

    for (const auto& t : themes) {
        auto* a = themeMenu->addAction(t.label);
        a->setCheckable(true);
        if (!t.shortcut.isEmpty()) a->setShortcut(QKeySequence(t.shortcut));
        m_themeGroup->addAction(a);
        connect(a, &QAction::triggered, this, [this, mode=t.mode]{ setThemeMode(mode); });
    }

    themeMenu->addSeparator();
    auto* customAction = themeMenu->addAction("Load Custom Theme (.qss)...");
    connect(customAction, &QAction::triggered, this, &MainWindow::loadCustomTheme);

    viewMenu->addSeparator();
    auto* histAction = viewMenu->addAction("Toggle History");
    histAction->setShortcut(QKeySequence("Ctrl+H"));
    connect(histAction, &QAction::triggered, this, [this]{
        m_historyBtn->toggle();
    });

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
    static const QMap<ThemeMode, QString> builtins = {
        { ThemeMode::Light,     ":/themes/light.qss"     },
        { ThemeMode::Dark,      ":/themes/dark.qss"      },
        { ThemeMode::Midnight,  ":/themes/midnight.qss"  },
        { ThemeMode::Dracula,   ":/themes/dracula.qss"   },
        { ThemeMode::Nord,      ":/themes/nord.qss"      },
        { ThemeMode::Monokai,   ":/themes/monokai.qss"   },
        { ThemeMode::Solarized, ":/themes/solarized.qss" },
    };

    if (m_themeMode == ThemeMode::Custom && !m_customThemePath.isEmpty()) {
        QFile f(m_customThemePath);
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
            syncGraphTheme(true);  // assume dark for custom
            return;
        }
    }

    if (m_themeMode == ThemeMode::System) {
        bool dark = (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark);
        loadTheme(dark ? ":/themes/dark.qss" : ":/themes/light.qss");
        syncGraphTheme(dark);
        return;
    }

    if (builtins.contains(m_themeMode))
        loadTheme(builtins[m_themeMode]);

    // Light is the only non-dark built-in theme
    syncGraphTheme(m_themeMode != ThemeMode::Light);
}

void MainWindow::loadCustomTheme() {
    QString path = QFileDialog::getOpenFileName(
        this, "Load Custom Theme", QDir::homePath(), "Qt Stylesheets (*.qss)");
    if (path.isEmpty()) return;
    m_customThemePath = path;
    m_themeMode = ThemeMode::Custom;
    applyTheme();
    saveSettings();
    if (m_themeGroup && m_themeGroup->checkedAction())
        m_themeGroup->checkedAction()->setChecked(false);
}

void MainWindow::loadTheme(const QString& qrcPath) {
    QFile f(qrcPath);
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
        f.close();
    }
}

void MainWindow::syncGraphTheme(bool dark) {
    auto* gw = qobject_cast<GraphingWidget*>(m_stack->widget(6));
    if (gw) gw->syncToAppTheme(dark);
}

// ── Settings ──────────────────────────────────────────────────────────────────

void MainWindow::saveSettings() {
    QSettings s("Caliber", "Caliber");
    s.setValue("window/geometry", saveGeometry());
    s.setValue("window/state",    saveState());
    s.setValue("ui/mode",         m_stack->currentIndex());
    s.setValue("ui/theme",        static_cast<int>(m_themeMode));
    s.setValue("ui/customTheme",  m_customThemePath);
}

void MainWindow::restoreSettings() {
    QSettings s("Caliber", "Caliber");

    if (s.contains("window/geometry"))
        restoreGeometry(s.value("window/geometry").toByteArray());
    else
        resize(1100, 680);

    if (s.contains("window/state"))
        restoreState(s.value("window/state").toByteArray());

    int mode = s.value("ui/mode", 0).toInt();
    m_stack->setCurrentIndex(mode);
    m_sidebar->setCurrentMode(static_cast<CalcMode>(mode));

    m_themeMode = static_cast<ThemeMode>(s.value("ui/theme", 0).toInt());
    m_customThemePath = s.value("ui/customTheme", "").toString();

    if (m_themeGroup) {
        auto actions = m_themeGroup->actions();
        int idx = static_cast<int>(m_themeMode);
        if (idx < actions.size()) actions[idx]->setChecked(true);
    }

    applyTheme();
}

// ── Events ────────────────────────────────────────────────────────────────────

void MainWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
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
