#include "mainwindow.h"
#include "core/math_engine.h"
#include "core/history_manager.h"
#include "widgets/history_panel.h"
#include "widgets/formula_panel.h"
#include "modes/basic/basic_widget.h"
#include "modes/scientific/scientific_widget.h"
#include "modes/programming/programming_widget.h"
#include "modes/date/date_widget.h"
#include "modes/conversion/conversion_widget.h"
#include "modes/equations/equations_widget.h"
#include "modes/graphing/graphing_widget.h"
#include "modes/statistics/statistics_widget.h"
#include "modes/calculus/calculus_widget.h"
#include "modes/financial/financial_widget.h"
#include "modes/numbertheory/numbertheory_widget.h"
#include "modes/electrical/electrical_widget.h"
#include "modes/digitallogic/digitallogic_widget.h"
#include "modes/vectors/vectors_widget.h"
#include "modes/physics/physics_widget.h"
#include "modes/chemistry/chemistry_widget.h"
#include "modes/civilmech/civilmech_widget.h"
#include "modes/advancedmath/advancedmath_widget.h"
#include "modes/discretemath/discretemath_widget.h"
#include "modes/mcs/mcs_widget.h"
#include "modes/signalprocessing/signalprocessing_widget.h"
#include "modes/controlsystems/controlsystems_widget.h"

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
#include <QColorDialog>
#include <QCompleter>
#include <QTabWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>

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
    m_formulaPanel = new FormulaPanel(m_central);

    m_stack->addWidget(new BasicWidget      (m_engine, m_history, m_historyPanel, this)); // 0
    m_stack->addWidget(new ScientificWidget (m_engine, m_history, m_historyPanel, this)); // 1
    m_stack->addWidget(new ProgrammingWidget(m_history, m_historyPanel, this));           // 2
    m_stack->addWidget(new DateWidget       (this));                                       // 3
    m_stack->addWidget(new ConversionWidget (this));                                       // 4
    m_stack->addWidget(new EquationsWidget  (this));                                       // 5
    m_stack->addWidget(new GraphingWidget   (this));                                       // 6
    m_stack->addWidget(new StatisticsWidget (this));                                       // 7
    m_stack->addWidget(new CalculusWidget     (this));                                       // 8
    m_stack->addWidget(new FinancialWidget    (this));                                       // 9
    m_stack->addWidget(new NumberTheoryWidget (this));                                       // 10
    m_stack->addWidget(new ElectricalWidget    (this));                                       // 11
    m_stack->addWidget(new DigitalLogicWidget  (this));                                       // 12
    m_stack->addWidget(new VectorsWidget       (this));                                       // 13
    m_stack->addWidget(new PhysicsWidget       (this));                                       // 14
    m_stack->addWidget(new ChemistryWidget     (this));                                       // 15
    m_stack->addWidget(new CivilMechWidget     (this));                                       // 16
    m_stack->addWidget(new AdvancedMathWidget  (this));                                       // 17
    m_stack->addWidget(new DiscreteMathWidget  (this));                                       // 18
    m_stack->addWidget(new McsWidget           (this));                                       // 19
    m_stack->addWidget(new SignalProcessingWidget(this));                                     // 20
    m_stack->addWidget(new ControlSystemsWidget (this));                                      // 21

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

    // ── Global solver search bar ──────────────────────────────────────────────
    m_searchBar = new QLineEdit(toolbar);
    m_searchBar->setPlaceholderText("🔍  Search solver... (e.g. pH, eigenvalue, Ohm)");
    m_searchBar->setFixedWidth(280);
    m_searchBar->setClearButtonEnabled(true);
    m_searchBar->setObjectName("solverSearchBar");
    toolbar->addWidget(m_searchBar);

    // Build search index: keyword → (stack index, tab index)
    buildSearchIndex();

    // Completer from all keywords
    QStringList allKeywords;
    for (auto it = m_searchIndex.begin(); it != m_searchIndex.end(); ++it)
        allKeywords << it.key();
    allKeywords.sort(Qt::CaseInsensitive);
    auto* completer = new QCompleter(allKeywords, m_searchBar);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    m_searchBar->setCompleter(completer);

    connect(m_searchBar, &QLineEdit::returnPressed, this, [this]{ onSearchActivated(); });
    connect(completer, QOverload<const QString&>::of(&QCompleter::activated),
            this, &MainWindow::onSearchActivated);

    // Small spacer between search and buttons
    auto* spacer2 = new QWidget(toolbar);
    spacer2->setFixedWidth(8);
    toolbar->addWidget(spacer2);

    m_historyBtn = new QToolButton(toolbar);
    m_historyBtn->setText("History  ⏱");
    m_historyBtn->setCheckable(true);
    m_historyBtn->setChecked(false);
    m_historyBtn->setToolTip("Toggle history panel  (Ctrl+H)");
    m_historyBtn->setObjectName("historyToggleBtn");
    m_historyBtn->setFocusPolicy(Qt::NoFocus);
    toolbar->addWidget(m_historyBtn);

    m_formulaBtn = new QToolButton(toolbar);
    m_formulaBtn->setText("Formulas  📖");
    m_formulaBtn->setCheckable(true);
    m_formulaBtn->setChecked(false);
    m_formulaBtn->setToolTip("Toggle formula book  (Ctrl+F)");
    m_formulaBtn->setObjectName("historyToggleBtn");
    m_formulaBtn->setFocusPolicy(Qt::NoFocus);
    toolbar->addWidget(m_formulaBtn);

    setCentralWidget(m_central);
    applyLayout(false);

    connect(m_sidebar, &ModeSidebar::modeChanged, this, &MainWindow::onModeChanged);
    connect(m_historyBtn, &QToolButton::toggled, this, [this](bool checked) {
        if (checked != m_historyPanel->isDrawerOpen())
            m_historyPanel->toggleDrawer();
    });

    connect(m_formulaBtn, &QToolButton::toggled, this, [this](bool checked) {
        if (checked != m_formulaPanel->isDrawerOpen())
            m_formulaPanel->toggleDrawer();
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
    if (m_formulaPanel)
        m_formulaPanel->repositionToParent();
}

// ── Menu bar ──────────────────────────────────────────────────────────────────

void MainWindow::setupMenuBar() {
    auto* viewMenu  = menuBar()->addMenu("&View");
    auto* themeMenu = viewMenu->addMenu("Theme");
    m_themeGroup = new QActionGroup(this);
    m_themeGroup->setExclusive(true);

    struct ThemeDef { QString label; ThemeMode mode; QString shortcut; };
    const QList<ThemeDef> themes = {
        { "Follow System",  ThemeMode::System,       "Ctrl+Shift+S" },
        { "Light",          ThemeMode::Light,        "Ctrl+Shift+L" },
        { "Dark",           ThemeMode::Dark,         "Ctrl+Shift+D" },
        { "Midnight Blue",  ThemeMode::Midnight,     ""             },
        { "Dracula",        ThemeMode::Dracula,      ""             },
        { "Nord",           ThemeMode::Nord,         ""             },
        { "Monokai",        ThemeMode::Monokai,      ""             },
        { "Solarized Dark", ThemeMode::Solarized,    ""             },
        { "High Contrast",  ThemeMode::HighContrast, ""             },
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

    auto* formulaAction = viewMenu->addAction("Toggle Formula Book");
    formulaAction->setShortcut(QKeySequence("Ctrl+F"));
    connect(formulaAction, &QAction::triggered, this, [this]{
        m_formulaBtn->toggle();
    });

    // ── Font size ─────────────────────────────────────────────────────────────
    viewMenu->addSeparator();
    auto* fontMenu = viewMenu->addMenu("Font Size");    const QList<int> sizes = {10, 11, 12, 13, 14, 16, 18};
    auto* fontGroup = new QActionGroup(this);
    fontGroup->setExclusive(true);
    int currentSize = QApplication::font().pointSize();
    for (int sz : sizes) {
        auto* a = fontMenu->addAction(QString("%1pt").arg(sz));
        a->setCheckable(true);
        a->setChecked(sz == currentSize);
        fontGroup->addAction(a);
        connect(a, &QAction::triggered, this, [this, sz, fontGroup]{
            QFont f = QApplication::font();
            f.setPointSize(sz);
            QApplication::setFont(f);
            // Force all widgets to pick up the new font
            for (QWidget* w : QApplication::allWidgets()) {
                w->setFont(f);
                w->update();
            }
            for (auto* act : fontGroup->actions())
                act->setChecked(act->text() == QString("%1pt").arg(sz));
            m_fontSize = sz;
            saveSettings();
        });
    }

    // ── Accent color ──────────────────────────────────────────────────────────
    auto* accentAction = viewMenu->addAction("Accent Color...");
    connect(accentAction, &QAction::triggered, this, [this]{
        QColor initial = m_accentColor.isEmpty() ? QColor(0x42, 0x9e, 0xf5) : QColor(m_accentColor);
        QColor chosen = QColorDialog::getColor(initial, this, "Choose Accent Color");
        if (!chosen.isValid()) return;
        m_accentColor = chosen.name();
        applyAccentColor();
        saveSettings();
    });
    auto* resetAccent = viewMenu->addAction("Reset Accent Color");
    connect(resetAccent, &QAction::triggered, this, [this]{
        m_accentColor.clear();
        applyTheme(); // reapply base theme without override
        saveSettings();
    });

    auto* modeMenu = menuBar()->addMenu("&Mode");
    const QStringList modeNames = {
        "Basic", "Scientific", "Programming",
        "Date", "Conversion", "Equations", "Graphing",
        "Statistics", "Calculus", "Financial", "Number Theory",
        "Electrical", "Digital Logic", "Vectors", "Physics", "Chemistry",
        "Civil/Mech", "Adv. Math", "Discrete Math",
        "MCS", "Signal Proc.", "Control Systems"
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
        { ThemeMode::Light,         ":/themes/light.qss"         },
        { ThemeMode::Dark,          ":/themes/dark.qss"          },
        { ThemeMode::Midnight,      ":/themes/midnight.qss"      },
        { ThemeMode::Dracula,       ":/themes/dracula.qss"       },
        { ThemeMode::Nord,          ":/themes/nord.qss"          },
        { ThemeMode::Monokai,       ":/themes/monokai.qss"       },
        { ThemeMode::Solarized,     ":/themes/solarized.qss"     },
        { ThemeMode::HighContrast,  ":/themes/highcontrast.qss"  },
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

    // Apply accent color override on top of base theme
    if (!m_accentColor.isEmpty())
        applyAccentColor();
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

void MainWindow::applyAccentColor() {
    if (m_accentColor.isEmpty()) return;
    QColor c(m_accentColor);
    QColor cDark = c.darker(130);
    QColor cLight = c.lighter(130);
    // Append override on top of current stylesheet
    QString override = QString(R"(
QPushButton[class="actionButton"] {
    background: %1; border-color: %1; color: #ffffff;
}
QPushButton[class="actionButton"]:hover { background: %3; }
QPushButton[class="actionButton"]:pressed { background: %2; }
#modeSidebar QPushButton:checked {
    background: rgba(%4,%5,%6,0.15);
    border-left: 3px solid %1;
    color: %1;
}
QTabBar::tab:selected { color: %1; border-bottom: 2px solid %1; }
QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus,
QComboBox:focus, QDateEdit:focus { border-color: %1; }
#historyToggleBtn:checked { background: %1; color: #ffffff; border-color: %1; }
QScrollBar::handle:vertical { background: %1; }
QScrollBar::handle:vertical:hover { background: %3; }
)").arg(c.name()).arg(cDark.name()).arg(cLight.name())
   .arg(c.red()).arg(c.green()).arg(c.blue());

    qApp->setStyleSheet(qApp->styleSheet() + override);
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
    s.setValue("ui/fontSize",     m_fontSize);
    s.setValue("ui/accentColor",  m_accentColor);
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

    // Restore font size
    m_fontSize = s.value("ui/fontSize", 12).toInt();
    QFont appFont = QApplication::font();
    appFont.setPointSize(m_fontSize);
    QApplication::setFont(appFont);

    m_accentColor = s.value("ui/accentColor", "").toString();
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
    int idx = static_cast<int>(mode);
    if (idx == m_stack->currentIndex()) return;
    m_stack->setCurrentIndex(idx);
    saveSettings();
}

void MainWindow::buildSearchIndex() {
    // Format: keyword → (stack index, tab index)
    // Stack indices match the order widgets were added in buildUI()
    // Tab indices match the order tabs were added in each widget's constructor

    auto add = [&](const QString& kw, int stack, int tab) {
        m_searchIndex[kw.toLower()] = {stack, tab};
    };

    // ── Basic (0) ─────────────────────────────────────────────────────────────
    // No tabs

    // ── Scientific (1) ────────────────────────────────────────────────────────
    // No tabs

    // ── Programming (2) ──────────────────────────────────────────────────────
    // No tabs

    // ── Equations (5) ────────────────────────────────────────────────────────
    add("linear equation",      5, 0); add("linear solver",       5, 0);
    add("quadratic",            5, 1); add("quadratic formula",   5, 1);
    add("system of equations",  5, 2); add("gaussian elimination",5, 2);
    add("matrix",               5, 3); add("determinant",         5, 3);
    add("inverse matrix",       5, 3); add("eigenvalue",          5, 3);

    // ── Statistics (7) ───────────────────────────────────────────────────────
    add("mean",                 7, 1); add("median",              7, 1);
    add("standard deviation",   7, 1); add("variance",            7, 1);
    add("descriptive",          7, 1);
    add("normal distribution",  7, 2); add("binomial",            7, 2);
    add("poisson",              7, 2); add("pdf",                 7, 2);
    add("z-test",               7, 3); add("t-test",              7, 3);
    add("hypothesis",           7, 3);
    add("confidence interval",  7, 4);
    add("chi-square",           7, 5); add("chi square",          7, 5);
    add("anova",                7, 6);
    add("histogram",            7, 7);
    add("regression",           7, 8); add("correlation",         7, 8);
    add("pearson",              7, 8); add("spearman",            7, 8);
    add("frequency table",      7, 9);

    // ── Calculus (8) ─────────────────────────────────────────────────────────
    add("differentiation",      8, 0); add("derivative",          8, 0);
    add("integration",          8, 1); add("integral",            8, 1);
    add("double integral",      8, 2); add("double integration",  8, 2);
    add("triple integral",      8, 3); add("triple integration",  8, 3);
    add("partial derivative",   8, 4); add("gradient",            8, 4);
    add("limit",                8, 5);
    add("taylor series",        8, 6); add("taylor",              8, 6);
    add("root finder",          8, 7); add("newton raphson",      8, 7);
    add("bisection",            8, 7);

    // ── Financial (9) ────────────────────────────────────────────────────────
    add("compound interest",    9, 0); add("future value",        9, 0);
    add("loan",                 9, 1); add("mortgage",            9, 1);
    add("npv",                  9, 2); add("irr",                 9, 2);
    add("net present value",    9, 2);
    add("percentage",           9, 3); add("discount",            9, 3);
    add("markup",               9, 3);

    // ── Number Theory (10) ───────────────────────────────────────────────────
    add("prime",               10, 0); add("factorization",      10, 0);
    add("gcd",                 10, 1); add("lcm",                10, 1);
    add("modular arithmetic",  10, 2); add("modular inverse",    10, 2);
    add("base converter",      10, 3); add("binary",             10, 3);
    add("fibonacci",           10, 4); add("sequence",           10, 4);

    // ── Electrical (11) ──────────────────────────────────────────────────────
    add("ohm's law",           11, 0); add("ohm",                11, 0);
    add("resistor",            11, 1); add("series parallel",    11, 1);
    add("rc circuit",          11, 2); add("rlc",                11, 2);
    add("voltage divider",     11, 3);
    add("op-amp",              11, 4); add("amplifier",          11, 4);
    add("decibel",             11, 5); add("db",                 11, 5);
    add("phasor",              11, 6);
    add("power factor",        11, 7);

    // ── Digital Logic (12) ───────────────────────────────────────────────────
    add("truth table",         12, 0); add("boolean",            12, 0);
    add("k-map",               12, 1); add("karnaugh",           12, 1);
    add("number system",       12, 2); add("bcd",                12, 2);
    add("gray code",           12, 2); add("two's complement",   12, 2);
    add("flip flop",           12, 3); add("sr flip flop",       12, 3);
    add("adder",               12, 4); add("subtractor",         12, 4);
    add("mux",                 12, 5); add("multiplexer",        12, 5);
    add("demux",               12, 5); add("demultiplexer",      12, 5);
    add("ieee 754",            12, 6); add("floating point",     12, 6);

    // ── Vectors (13) ─────────────────────────────────────────────────────────
    add("vector arithmetic",   13, 0); add("vector addition",    13, 0);
    add("dot product",         13, 1); add("cross product",      13, 1);
    add("magnitude",           13, 2); add("unit vector",        13, 2);
    add("angle between vectors",13,3); add("projection",         13, 3);
    add("line equation",       13, 4); add("plane equation",     13, 4);
    add("distance",            13, 5); add("point to line",      13, 5);

    // ── Physics (14) ─────────────────────────────────────────────────────────
    add("kinematics",          14, 0); add("suvat",              14, 0);
    add("newton's law",        14, 1); add("friction",           14, 1);
    add("kinetic energy",      14, 2); add("potential energy",   14, 2);
    add("work energy",         14, 2);
    add("projectile",          14, 3); add("projectile motion",  14, 3);
    add("circular motion",     14, 4); add("centripetal",        14, 4);
    add("waves",               14, 5); add("snell's law",        14, 5);
    add("thermodynamics",      14, 6); add("ideal gas",          14, 6);
    add("electrostatics",      14, 7); add("coulomb",            14, 7);

    // ── Chemistry (15) ───────────────────────────────────────────────────────
    add("molar mass",          15, 0); add("molecular weight",   15, 0);
    add("ideal gas law",       15, 1);
    add("ph",                  15, 2); add("poh",                15, 2);
    add("acid base",           15, 2);
    add("dilution",            15, 3);
    add("gibbs free energy",   15, 4); add("enthalpy",           15, 4);
    add("hess's law",          15, 4);

    // ── Civil/Mech (16) ──────────────────────────────────────────────────────
    add("beam",                16, 0); add("bending moment",     16, 0);
    add("stress strain",       16, 1); add("young's modulus",    16, 1);
    add("fluid mechanics",     16, 2); add("bernoulli",          16, 2);
    add("reynolds number",     16, 2);
    add("heat transfer",       16, 3); add("fourier",            16, 3);
    add("gear ratio",          16, 4); add("pulley",             16, 4);

    // ── Advanced Math (17) ───────────────────────────────────────────────────
    add("complex numbers",     17, 0); add("complex arithmetic",  17, 0);
    add("eigenvalue",          17, 1); add("eigenvector",         17, 1);
    add("linear algebra",      17, 1);
    add("arithmetic series",   17, 2); add("geometric series",    17, 2);
    add("triangle solver",     17, 3); add("law of sines",        17, 3);
    add("law of cosines",      17, 3);
    add("polynomial roots",    17, 4); add("cubic equation",      17, 4);
    add("set theory",          17, 5); add("union intersection",  17, 5);

    // ── Discrete Math (18) ───────────────────────────────────────────────────
    add("graph theory",        18, 0); add("dijkstra",            18, 0);
    add("bfs",                 18, 0); add("dfs",                 18, 0);
    add("combinatorics",       18, 1); add("permutation",         18, 1);
    add("combination",         18, 1);
    add("relations",           18, 2); add("reflexive",           18, 2);
    add("recurrence",          18, 3); add("recurrence relation", 18, 3);
    add("boolean algebra",     18, 4); add("de morgan",           18, 4);

    // ── MCS (19) ─────────────────────────────────────────────────────────────
    add("big-o",               19, 0); add("asymptotic",          19, 0);
    add("master theorem",      19, 1); add("time complexity",     19, 1);
    add("floating point",      19, 2); add("machine epsilon",     19, 2);
    add("formal logic",        19, 3); add("tautology",           19, 3);
    add("hashing",             19, 4); add("hash function",       19, 4);

    // ── Signal Processing (20) ───────────────────────────────────────────────
    add("dft",                 20, 0); add("fourier transform",   20, 0);
    add("filter design",       20, 1); add("low pass",            20, 1);
    add("high pass",           20, 1); add("band pass",           20, 1);
    add("nyquist",             20, 2); add("sampling",            20, 2);
    add("aliasing",            20, 2);
    add("convolution",         20, 3);
    add("transfer function",   20, 4); add("poles zeros",         20, 4);

    // ── Control Systems (21) ─────────────────────────────────────────────────
    add("transfer function",   21, 0); add("control system",      21, 0);
    add("routh hurwitz",       21, 1); add("stability",           21, 1);
    add("pid",                 21, 2); add("pid tuning",          21, 2);
    add("steady state error",  21, 3);
    add("state space",         21, 4); add("controllability",     21, 4);
}

void MainWindow::onSearchActivated(const QString& text) {
    QString query = text.isEmpty() ? m_searchBar->text().trimmed().toLower() : text.trimmed().toLower();
    if (query.isEmpty()) return;

    // Find best match — exact first, then contains
    QPair<int,int> result = {-1, -1};
    if (m_searchIndex.contains(query)) {
        result = m_searchIndex[query];
    } else {
        for (auto it = m_searchIndex.begin(); it != m_searchIndex.end(); ++it) {
            if (it.key().contains(query)) {
                result = it.value();
                break;
            }
        }
    }

    if (result.first < 0) return; // no match

    // Switch to the mode
    int stackIdx = result.first;
    int tabIdx   = result.second;
    m_stack->setCurrentIndex(stackIdx);
    m_sidebar->setCurrentMode(static_cast<CalcMode>(stackIdx));

    // Switch to the correct tab within the mode widget
    if (tabIdx >= 0) {
        auto* modeWidget = m_stack->widget(stackIdx);
        if (modeWidget) {
            auto* tabWidget = modeWidget->findChild<QTabWidget*>();
            if (tabWidget && tabIdx < tabWidget->count())
                tabWidget->setCurrentIndex(tabIdx);
        }
    }

    m_searchBar->clear();
}
