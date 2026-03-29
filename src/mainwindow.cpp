#include "mainwindow.h"
#include "core/math_engine.h"
#include "core/history_manager.h"
#include "widgets/history_panel.h"
#include "widgets/formula_panel.h"
#include "widgets/animated_stacked_widget.h"
#include "widgets/gradient_theme_dialog.h"
#include "widgets/mode_pager.h"
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
#include <QGuiApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QToolButton>
#include <QToolBar>
#include <QShortcut>
#include <QKeySequence>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QColorDialog>
#include <QCompleter>
#include <QTabWidget>
#include <QSplitter>
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
    m_stack = new AnimatedStackedWidget(this);

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

#if defined(Q_OS_ANDROID)
    // Hide desktop toolbar
    auto* tb = addToolBar("Main");
    tb->setObjectName("mainToolbar");
    tb->setVisible(false);

    // Use ModePager: header + content + full-width bottom mode bar
    auto* pager = new ModePager(m_central);

    // Add all mode widgets to the pager's stack
    pager->stack()->addWidget(new BasicWidget       (m_engine, m_history, m_historyPanel, this));
    pager->stack()->addWidget(new ScientificWidget   (m_engine, m_history, m_historyPanel, this));
    pager->stack()->addWidget(new ProgrammingWidget  (m_history, m_historyPanel, this));
    pager->stack()->addWidget(new DateWidget         (this));
    pager->stack()->addWidget(new ConversionWidget   (this));
    pager->stack()->addWidget(new EquationsWidget    (this));
    pager->stack()->addWidget(new GraphingWidget     (this));
    pager->stack()->addWidget(new StatisticsWidget   (this));
    pager->stack()->addWidget(new CalculusWidget     (this));
    pager->stack()->addWidget(new FinancialWidget    (this));
    pager->stack()->addWidget(new NumberTheoryWidget (this));
    pager->stack()->addWidget(new ElectricalWidget   (this));
    pager->stack()->addWidget(new DigitalLogicWidget (this));
    pager->stack()->addWidget(new VectorsWidget      (this));
    pager->stack()->addWidget(new PhysicsWidget      (this));
    pager->stack()->addWidget(new ChemistryWidget    (this));
    pager->stack()->addWidget(new CivilMechWidget    (this));
    pager->stack()->addWidget(new AdvancedMathWidget (this));
    pager->stack()->addWidget(new DiscreteMathWidget (this));
    pager->stack()->addWidget(new McsWidget          (this));
    pager->stack()->addWidget(new SignalProcessingWidget(this));
    pager->stack()->addWidget(new ControlSystemsWidget  (this));

    // Replace m_stack reference for search/navigation
    m_stack = pager->stack();

    // Also add history and formula buttons to the pager header area
    // (ModePager header already has Caliber title + prev/next arrows)

    auto* mobileLayout = new QVBoxLayout(m_central);
    mobileLayout->setContentsMargins(0, 0, 0, 0);
    mobileLayout->setSpacing(0);
    mobileLayout->addWidget(pager, 1);
    m_central->setLayout(mobileLayout);

    connect(pager, &ModePager::modeChanged, this, &MainWindow::onModeChanged);
    connect(pager, &ModePager::settingsClicked, this, [this]{
        QMenu menu(this);
        auto* themeMenu = menu.addMenu("Theme");
        QStringList names = {"Light","Dark","Midnight","Dracula","Nord","Monokai","Solarized","High Contrast"};
        for (int i = 0; i < names.size(); ++i) {
            auto* a = themeMenu->addAction(names[i]);
            a->setCheckable(true);
            a->setChecked(static_cast<int>(m_themeMode) == i + 1);
            connect(a, &QAction::triggered, this, [this, i]{
                m_themeMode = static_cast<ThemeMode>(i + 1);
                applyTheme();
                applyMobileOverrides();
                saveSettings();
            });
        }
        menu.addSeparator();
        menu.addAction("History", [this]{ m_historyPanel->toggleDrawer(); });
        menu.addAction("Formulas", [this]{ m_formulaPanel->toggleDrawer(); });
        menu.exec(QCursor::pos());
    });

    applyMobileOverrides();

    // History and formula toggles via long-press on the mode bar
    // or add them to the pager header — for now keep the toolbar buttons available
    m_historyBtn = new QToolButton(this);
    m_historyBtn->setVisible(false); // hidden, accessible via menu
    m_formulaBtn = new QToolButton(this);
    m_formulaBtn->setVisible(false);
#else
    auto* animatedStack = qobject_cast<AnimatedStackedWidget*>(m_stack);
    if (animatedStack) animatedStack->setAnimationDuration(200);
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
#endif

    // When history drawer toggles, shrink 3D container so it doesn't overlap
    connect(m_historyPanel, &HistoryPanel::drawerToggled, this, [this](bool open) {
        auto* gw = qobject_cast<GraphingWidget*>(m_stack->widget(6));
#ifdef HAVE_DATAVISUALIZATION
        if (gw) gw->adjustFor3DOverlap(open, 240);
#endif
    });
}

void MainWindow::applyLayout(bool portrait) {
    if (m_rootLayout) {
        while (m_rootLayout->count())
            m_rootLayout->takeAt(0);
        delete m_rootLayout;
        m_rootLayout = nullptr;
    }
    if (m_splitter) {
        // Reparent children back before deleting splitter
        m_sidebar->setParent(m_central);
        m_stack->setParent(m_central);
        delete m_splitter;
        m_splitter = nullptr;
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
        m_splitter = new QSplitter(Qt::Horizontal, m_central);
        m_splitter->addWidget(m_sidebar);
        m_splitter->addWidget(m_stack);
        m_splitter->setStretchFactor(0, 0);
        m_splitter->setStretchFactor(1, 1);
        m_splitter->setCollapsible(0, false);
        m_splitter->setCollapsible(1, false);
        m_splitter->setHandleWidth(3);
        auto* hl = new QHBoxLayout(m_central);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(0);
        hl->addWidget(m_splitter);
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

    auto* gradientAction = themeMenu->addAction("Custom Gradient...");
    connect(gradientAction, &QAction::triggered, this, &MainWindow::loadGradientTheme);

    // Load community themes from ~/.config/Caliber/themes/
    loadCommunityThemes();

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

    if (m_themeMode == ThemeMode::Gradient) {
        // Generate QSS from gradient colors
        bool dark = m_gradientDarkBase;
        QColor c1 = m_gradientStart;
        QColor c2 = m_gradientEnd;
        QColor mid = QColor((c1.red()+c2.red())/2, (c1.green()+c2.green())/2, (c1.blue()+c2.blue())/2);

        // Derive UI colors from the gradient
        QColor bg        = dark ? mid.darker(180) : mid.lighter(200);
        QColor sidebarBg = dark ? c1.darker(200)  : c1.lighter(220);
        QColor cardBg    = dark ? mid.darker(140) : mid.lighter(180);
        QColor border    = dark ? mid.lighter(130) : mid.darker(130);
        QColor text      = dark ? QColor(0xe8,0xe8,0xe8) : QColor(0x1a,0x1a,0x1a);
        QColor textDim   = dark ? QColor(0x88,0x88,0x88) : QColor(0x66,0x66,0x66);
        QColor textBright= dark ? QColor(0xff,0xff,0xff) : QColor(0x00,0x00,0x00);
        QColor accent    = c2.lighter(dark ? 130 : 90);
        QColor accentDark= c1.darker(dark ? 110 : 120);

        // Compute gradient coordinates from angle
        double a = m_gradientAngle * M_PI / 180.0;
        double x1 = 0.5 - std::cos(a)*0.5, y1 = 0.5 - std::sin(a)*0.5;
        double x2 = 0.5 + std::cos(a)*0.5, y2 = 0.5 + std::sin(a)*0.5;

        QString gradMain = QString("qlineargradient(x1:%1,y1:%2,x2:%3,y2:%4, stop:0 %5, stop:1 %6)")
            .arg(x1,0,'f',2).arg(y1,0,'f',2).arg(x2,0,'f',2).arg(y2,0,'f',2)
            .arg(c1.name()).arg(c2.name());
        QString gradSidebar = QString("qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 %1, stop:1 %2)")
            .arg(c1.darker(dark?160:140).name()).arg(c1.name());
        QString gradBtn = QString("qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 %1, stop:1 %2)")
            .arg(cardBg.lighter(110).name()).arg(cardBg.name());
        QString gradAction = QString("qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 %1, stop:1 %2)")
            .arg(c1.lighter(120).name()).arg(c2.lighter(110).name());

        QString qss = QString(R"(
* { font-family: "Segoe UI", "Helvetica Neue", Arial, sans-serif; font-size: 14px; outline: none; }
QMainWindow { background: %1; }
QWidget { background-color: %11; color: %2; }
#modeSidebar { background-color: %3; border-right: 1px solid %4; min-width: 120px; max-width: 300px; }
#modeSidebar QLabel { color: %5; font-size: 15px; font-weight: bold; padding: 4px 0 12px 0; }
#modeSidebar QPushButton { background: transparent; border: none; border-radius: 8px; padding: 10px 12px; text-align: left; color: %6; font-size: 13px; }
#modeSidebar QPushButton:hover { background: rgba(%7,%8,%9,0.15); color: %2; }
#modeSidebar QPushButton:checked { background: rgba(%7,%8,%9,0.25); color: %5; font-weight: bold; border-left: 3px solid %10; }
#displayWidget { background-color: %11; border: 1px solid %4; border-radius: 12px; padding: 4px; }
#expressionLabel { color: %6; font-size: 13px; padding: 2px 8px 0 8px; }
#resultLabel { color: %2; font-size: 30px; font-weight: bold; padding: 0 8px 4px 8px; }
QPushButton[class="calcButton"] { background-color: %12; border: 1px solid %4; border-radius: 8px; font-size: 16px; color: %2; min-height: 52px; }
QPushButton[class="calcButton"]:hover { background-color: %13; border-color: %10; }
QPushButton[class="calcButton"]:pressed { background-color: %4; }
QPushButton[class="operatorButton"] { background-color: rgba(%7,%8,%9,0.2); border: 1px solid %10; border-radius: 8px; font-size: 16px; color: %10; min-height: 52px; font-weight: 600; }
QPushButton[class="operatorButton"]:hover { background-color: rgba(%7,%8,%9,0.35); color: %2; }
QPushButton[class="operatorButton"]:pressed { background-color: %10; color: %11; }
QPushButton[class="actionButton"] { background-color: %14; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; color: %15; min-height: 52px; }
QPushButton[class="actionButton"]:hover { background-color: %10; }
QPushButton[class="actionButton"]:pressed { background-color: %16; }
QPushButton[class="clearButton"] { background-color: rgba(255,60,60,0.15); border: 1px solid rgba(255,60,60,0.5); border-radius: 8px; font-size: 16px; color: #ff5555; min-height: 52px; }
QPushButton[class="clearButton"]:hover { background-color: rgba(255,60,60,0.3); }
#historyPanel { background-color: %11; }
#historyPanel QLabel { background: transparent; color: %5; font-weight: bold; font-size: 13px; }
#historyPanel QListWidget { background-color: %11; border: none; font-size: 12px; }
#historyPanel QListWidget::item { padding: 6px 8px; border-bottom: 1px solid %4; color: %6; border-radius: 4px; }
#historyPanel QListWidget::item:hover { background: rgba(%7,%8,%9,0.15); }
QTabWidget { background: transparent; }
QTabWidget::pane { border: 1px solid %4; border-radius: 8px; background-color: %11; }
QTabWidget QWidget { background-color: %11; }
QTabBar::tab { background-color: %17; color: %6; border: 1px solid %4; border-bottom: none; border-radius: 6px 6px 0 0; padding: 6px 14px; margin-right: 2px; font-size: 13px; }
QTabBar::tab:selected { background-color: %14; color: %15; font-weight: bold; }
QTabBar::tab:hover:!selected { background-color: rgba(%7,%8,%9,0.15); color: %2; }
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateEdit { background-color: %11; border: 1px solid %4; border-radius: 6px; padding: 5px 8px; color: %2; selection-background-color: %10; }
QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QDateEdit:focus { border: 2px solid %10; }
QComboBox QAbstractItemView { background-color: %11; border: 1px solid %4; color: %2; selection-background-color: rgba(%7,%8,%9,0.3); }
QScrollBar:vertical { background-color: %17; width: 8px; border-radius: 4px; }
QScrollBar::handle:vertical { background-color: %10; border-radius: 4px; min-height: 20px; }
QScrollBar::handle:vertical:hover { background-color: %5; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QMenuBar { background-color: %3; color: %6; padding: 2px; }
QMenuBar::item:selected { background: rgba(%7,%8,%9,0.2); border-radius: 4px; }
QMenu { background-color: %11; border: 1px solid %4; border-radius: 6px; }
QMenu::item { padding: 6px 24px; color: %2; }
QMenu::item:selected { background: rgba(%7,%8,%9,0.25); }
QTableWidget { background-color: %11; border: 1px solid %4; border-radius: 6px; gridline-color: %4; color: %2; }
QTableWidget QWidget { background-color: %11; }
QHeaderView::section { background-color: %17; color: %6; border: 1px solid %4; padding: 4px; }
QTextEdit { background-color: %11; border: 1px solid %4; border-radius: 6px; color: %2; }
QFrame[frameShape="4"] { color: %4; }
#graphBottomPanel { background-color: rgba(%18,%19,%20,0.95); border-top: 1px solid %4; }
#graphBottomPanel QWidget { background-color: transparent; }
QToolButton { background-color: transparent; border: 1px solid %4; border-radius: 6px; padding: 4px 8px; color: %2; }
QToolButton:hover { background-color: rgba(%7,%8,%9,0.2); }
QToolButton:checked { background-color: %10; color: %15; border-color: %10; }
QPushButton { min-height: 24px; }
QGroupBox { background-color: %11; border: 1px solid %4; border-radius: 8px; margin-top: 12px; padding-top: 16px; }
QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; color: %5; }
QLabel { background: transparent; }
QCheckBox { background: transparent; }
QRadioButton { background: transparent; }
QSplitter { background-color: %11; }
QSplitter::handle { background-color: %4; width: 3px; }
)").arg(gradMain).arg(text.name())                                   // %1, %2
   .arg(gradSidebar).arg(border.name())                              // %3, %4
   .arg(accent.name()).arg(textDim.name())                           // %5, %6
   .arg(accent.red()).arg(accent.green()).arg(accent.blue())         // %7, %8, %9
   .arg(accent.name())                                               // %10
   .arg(cardBg.name())                                               // %11
   .arg(gradBtn)                                                     // %12
   .arg(cardBg.lighter(120).name())                                  // %13
   .arg(gradAction).arg(dark ? textBright.name() : text.name())      // %14, %15
   .arg(accentDark.name())                                           // %16
   .arg(bg.name())                                                   // %17
   .arg(mid.red()).arg(mid.green()).arg(mid.blue());                 // %18, %19, %20

        qApp->setStyleSheet(qss);
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

void MainWindow::loadGradientTheme() {
    GradientThemeDialog dlg(this);
    dlg.loadSettings();
    if (dlg.exec() != QDialog::Accepted) return;

    m_gradientStart    = dlg.gradientStart();
    m_gradientEnd      = dlg.gradientEnd();
    m_gradientDarkBase = dlg.darkBase();
    m_gradientAngle    = dlg.angle();
    dlg.saveSettings();

    m_themeMode = ThemeMode::Gradient;
    applyTheme();
    saveSettings();

    // Uncheck all theme group actions
    if (m_themeGroup && m_themeGroup->checkedAction())
        m_themeGroup->checkedAction()->setChecked(false);
}

void MainWindow::loadCommunityThemes() {
    // Scan ~/.config/Caliber/themes/ for .qss files
    QString themesDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/Caliber/themes";
    QDir dir(themesDir);
    if (!dir.exists()) {
        dir.mkpath(".");
        return; // No themes yet
    }

    QStringList filters; filters << "*.qss";
    auto files = dir.entryList(filters, QDir::Files);
    if (files.isEmpty()) return;

    // Find the theme menu from the menu bar
    auto* themeMenu = m_themeGroup ? qobject_cast<QMenu*>(m_themeGroup->parent()) : nullptr;
    if (!themeMenu) return;

    themeMenu->addSeparator();
    auto* communityHeader = themeMenu->addAction("── Community Themes ──");
    communityHeader->setEnabled(false);

    for (const QString& file : files) {
        QString fullPath = dir.absoluteFilePath(file);
        QString name = file;
        name.chop(4); // remove .qss
        auto* a = themeMenu->addAction(name);
        a->setCheckable(true);
        m_themeGroup->addAction(a);
        connect(a, &QAction::triggered, this, [this, fullPath]{
            m_customThemePath = fullPath;
            m_themeMode = ThemeMode::Custom;
            applyTheme();
            saveSettings();
        });
    }
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

void MainWindow::applyMobileOverrides() {
#if defined(Q_OS_ANDROID)
    // Compact overrides — smaller elements, tighter spacing
    static const QString mobile = R"(
        QPushButton[class="calcButton"] {
            min-height: 44px; font-size: 15px; padding: 4px;
            border-radius: 6px; margin: 1px;
        }
        QPushButton[class="operatorButton"] {
            min-height: 44px; font-size: 15px; padding: 4px;
            border-radius: 6px; margin: 1px;
        }
        QPushButton[class="actionButton"] {
            min-height: 44px; font-size: 15px; padding: 4px;
            border-radius: 6px; margin: 1px;
        }
        QPushButton[class="clearButton"] {
            min-height: 44px; font-size: 15px; padding: 4px;
            border-radius: 6px; margin: 1px;
        }
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateEdit {
            min-height: 36px; font-size: 14px; padding: 4px 6px;
        }
        QTabBar::tab {
            font-size: 12px; padding: 4px 10px; min-height: 32px;
        }
        QLabel { font-size: 13px; }
        QToolButton {
            min-height: 36px; font-size: 14px; padding: 4px;
        }
        QTextEdit { font-size: 13px; }
        QGroupBox { font-size: 13px; }
        QTableWidget { font-size: 12px; }
        QHeaderView::section { font-size: 12px; padding: 2px; }
        #displayWidget { border-radius: 8px; }
        #expressionLabel { font-size: 12px; }
        #resultLabel { font-size: 24px; }
    )";
    qApp->setStyleSheet(qApp->styleSheet() + mobile);
#endif
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
    // Gradient theme settings are saved by GradientThemeDialog
}

void MainWindow::restoreSettings() {
    QSettings s("Caliber", "Caliber");

    if (s.contains("window/geometry"))
        restoreGeometry(s.value("window/geometry").toByteArray());
    else {
#if defined(Q_OS_ANDROID)
        // On Android, use screen size
        resize(QGuiApplication::primaryScreen()->availableSize());
#else
        resize(1100, 680);
#endif
    }

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

    // Load gradient theme settings
    m_gradientStart    = QColor(s.value("gradient/start", "#1a1a2e").toString());
    m_gradientEnd      = QColor(s.value("gradient/end",   "#16213e").toString());
    m_gradientDarkBase = s.value("gradient/darkBase", true).toBool();
    m_gradientAngle    = s.value("gradient/angle", 45).toInt();
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

    // Update formula panel mode filter
    static const QStringList modeNames = {
        "Basic", "Scientific", "Programming", "Date", "Conversion",
        "Equations", "Graphing", "Statistics", "Calculus", "Financial",
        "Number Theory", "Electrical", "Digital Logic", "Vectors",
        "Physics", "Chemistry", "Civil/Mech", "Advanced Math", "Discrete",
        "MCS", "Signals", "Control"
    };
    if (idx >= 0 && idx < modeNames.size())
        m_formulaPanel->setFilterMode(modeNames[idx]);
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
