#include "mode_sidebar.h"
#include <QLabel>
#include <QScrollArea>
#include <QWidget>
#include <QFrame>

const QVector<ModeSidebar::ModeEntry> ModeSidebar::s_modes = {
    { "Basic",       "⊞", CalcMode::Basic       },
    { "Scientific",  "∑", CalcMode::Scientific  },
    { "Programming", "</>",CalcMode::Programming },
    { "Date",        "📅", CalcMode::Date        },
    { "Conversion",  "⇄",  CalcMode::Conversion  },
    { "Equations",   "∫",  CalcMode::Equations   },
    { "Graphing",    "〜", CalcMode::Graphing    },
    { "Statistics",  "σ",  CalcMode::Statistics  },
    { "Calculus",    "∂",  CalcMode::Calculus     },
    { "Financial",   "$",  CalcMode::Financial    },
    { "Number Th.",  "ℕ",  CalcMode::NumberTheory },
    { "Electrical",  "⚡", CalcMode::Electrical   },
    { "Dig. Logic",  "⊕", CalcMode::DigitalLogic },
    { "Vectors",     "→", CalcMode::Vectors      },
    { "Physics",     "⚛", CalcMode::Physics      },
    { "Chemistry",   "⚗", CalcMode::Chemistry    },
    { "Civil/Mech",  "🔧", CalcMode::CivilMech   },
    { "Adv. Math",   "∞",  CalcMode::AdvancedMath },
    { "Discrete",    "⊆",  CalcMode::DiscreteMath },
    { "MCS",         "λ",  CalcMode::MCS          },
    { "Signals",     "∿",  CalcMode::SignalProc   },
    { "Control",     "⊙",  CalcMode::ControlSys  },
};

ModeSidebar::ModeSidebar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("modeSidebar");
    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    // Create all buttons once
    for (const auto& entry : s_modes) {
        auto* btn = new QPushButton(this);
        btn->setCheckable(true);
        btn->setFlat(true);
        btn->setFocusPolicy(Qt::NoFocus);
        m_group->addButton(btn, static_cast<int>(entry.mode));
        m_buttons.append(btn);
    }

    if (!m_buttons.isEmpty())
        m_buttons.first()->setChecked(true);

    buildLayout();

    connect(m_group, &QButtonGroup::idClicked, this, [this](int id) {
        emit modeChanged(static_cast<CalcMode>(id));
    });
}

void ModeSidebar::buildLayout() {
    // Remove old layout safely
    if (m_layout) {
        // Detach all widgets from old layout
        while (m_layout->count())
            m_layout->takeAt(0);
        delete m_layout;
        m_layout = nullptr;
    }

    bool vertical = (m_orientation == SidebarOrientation::Vertical);

    if (vertical) {
        setMinimumWidth(120); setMaximumWidth(300);
        setMinimumHeight(0);  setMaximumHeight(QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        // Container widget inside scroll area
        auto* container = new QWidget();
        container->setObjectName("modeSidebar");
        auto* innerLayout = new QBoxLayout(QBoxLayout::TopToBottom, container);
        innerLayout->setContentsMargins(8, 16, 8, 16);
        innerLayout->setSpacing(2);

        // Title
        auto* title = new QLabel("Caliber", container);
        title->setObjectName("sidebarTitle");
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-weight:bold; font-size:15px; margin-bottom:8px;");
        innerLayout->addWidget(title);

        for (int i = 0; i < m_buttons.size(); ++i) {
            auto* btn = m_buttons[i];
            btn->setParent(container);
            btn->setText(s_modes[i].icon + " " + s_modes[i].label);
            btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            btn->setMinimumHeight(44);
            btn->setStyleSheet("text-align: left; padding-left: 12px; font-size: 14px;");
            innerLayout->addWidget(btn);
        }
        innerLayout->addStretch();
        container->setLayout(innerLayout);

        // Scroll area
        auto* scroll = new QScrollArea(this);
        scroll->setWidget(container);
        scroll->setWidgetResizable(true);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setObjectName("sidebarScroll");

        m_layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->addWidget(scroll);

    } else {
        // Horizontal bottom bar — scrollable grid for mobile
        setMinimumHeight(80); setMaximumHeight(100);
        setMinimumWidth(0);   setMaximumWidth(QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // Container widget inside scroll area
        auto* container = new QWidget();
        container->setObjectName("modeSidebar");
        auto* innerLayout = new QBoxLayout(QBoxLayout::LeftToRight, container);
        innerLayout->setContentsMargins(6, 4, 6, 4);
        innerLayout->setSpacing(4);

        for (int i = 0; i < m_buttons.size(); ++i) {
            auto* btn = m_buttons[i];
            btn->setParent(container);
            // Icon on top, label below — compact for horizontal
            btn->setText(s_modes[i].icon + "\n" + s_modes[i].label);
            btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
            btn->setFixedWidth(56);
            btn->setMinimumHeight(68);
            btn->setStyleSheet("text-align: center; padding: 4px 2px; font-size: 10px; line-height: 1.1;");
            innerLayout->addWidget(btn);
        }
        container->setLayout(innerLayout);

        auto* scroll = new QScrollArea(this);
        scroll->setWidget(container);
        scroll->setWidgetResizable(true);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setObjectName("sidebarScroll");

        m_layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->addWidget(scroll);
    }

    setLayout(m_layout);
    update();
}

void ModeSidebar::setOrientation(SidebarOrientation orientation) {
    if (m_orientation == orientation) return;
    m_orientation = orientation;
    buildLayout();
}

void ModeSidebar::setCurrentMode(CalcMode mode) {
    auto* btn = m_group->button(static_cast<int>(mode));
    if (btn) btn->setChecked(true);
}
