#include "mode_pager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QGestureEvent>
#include <QSwipeGesture>

const QVector<QPair<QString, CalcMode>> ModePager::s_modes = {
    {"Basic",        CalcMode::Basic},
    {"Scientific",   CalcMode::Scientific},
    {"Programming",  CalcMode::Programming},
    {"Date",         CalcMode::Date},
    {"Conversion",   CalcMode::Conversion},
    {"Equations",    CalcMode::Equations},
    {"Graphing",     CalcMode::Graphing},
    {"Statistics",   CalcMode::Statistics},
    {"Calculus",     CalcMode::Calculus},
    {"Financial",    CalcMode::Financial},
    {"Number Theory",CalcMode::NumberTheory},
    {"Electrical",   CalcMode::Electrical},
    {"Digital Logic",CalcMode::DigitalLogic},
    {"Vectors",      CalcMode::Vectors},
    {"Physics",      CalcMode::Physics},
    {"Chemistry",    CalcMode::Chemistry},
    {"Civil/Mech",   CalcMode::CivilMech},
    {"Advanced Math",CalcMode::AdvancedMath},
    {"Discrete Math",CalcMode::DiscreteMath},
    {"MCS",          CalcMode::MCS},
    {"Signals",      CalcMode::SignalProc},
    {"Control",      CalcMode::ControlSys},
};

ModePager::ModePager(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Header: "Caliber" + prev/next arrows
    auto* header = new QWidget(this);
    header->setObjectName("mobileHeader");
    header->setFixedHeight(48);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 0, 8, 0);
    headerLayout->setSpacing(4);

    auto* titleLabel = new QLabel("Caliber", header);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; background: transparent;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    m_prevBtn = new QToolButton(header);
    m_prevBtn->setText("◀");
    m_prevBtn->setFixedSize(44, 44);
    m_prevBtn->setObjectName("historyToggleBtn");
    m_prevBtn->setFocusPolicy(Qt::NoFocus);
    headerLayout->addWidget(m_prevBtn);

    m_nextBtn = new QToolButton(header);
    m_nextBtn->setText("▶");
    m_nextBtn->setFixedSize(44, 44);
    m_nextBtn->setObjectName("historyToggleBtn");
    m_nextBtn->setFocusPolicy(Qt::NoFocus);
    headerLayout->addWidget(m_nextBtn);

    // Settings button at top right
    m_settingsBtn = new QToolButton(header);
    m_settingsBtn->setText("⚙");
    m_settingsBtn->setFixedSize(44, 44);
    m_settingsBtn->setObjectName("historyToggleBtn");
    m_settingsBtn->setFocusPolicy(Qt::NoFocus);
    headerLayout->addWidget(m_settingsBtn);

    connect(m_settingsBtn, &QToolButton::clicked, this, &ModePager::settingsClicked);

    layout->addWidget(header);

    // Stacked content area
    m_stack = new QStackedWidget(this);
    layout->addWidget(m_stack, 1);

    // Bottom bar: full-width mode button
    m_modeButton = new QPushButton("Basic  ▾", this);
    m_modeButton->setObjectName("modeBarButton");
    m_modeButton->setFixedHeight(56);
    m_modeButton->setFocusPolicy(Qt::NoFocus);
    m_modeButton->setStyleSheet(
        "#modeBarButton {"
        "  font-size: 16px; font-weight: bold; text-align: center;"
        "  padding: 8px; border: none; border-top: 1px solid rgba(255,255,255,0.1);"
        "}"
    );
    layout->addWidget(m_modeButton);

    // Connect prev/next
    connect(m_prevBtn, &QToolButton::clicked, this, [this]{
        int idx = m_stack->currentIndex();
        if (idx > 0) {
            m_stack->setCurrentIndex(idx - 1);
            m_modeButton->setText(s_modes[idx - 1].first + "  ▾");
            emit modeChanged(s_modes[idx - 1].second);
        }
    });
    connect(m_nextBtn, &QToolButton::clicked, this, [this]{
        int idx = m_stack->currentIndex();
        if (idx < m_stack->count() - 1) {
            m_stack->setCurrentIndex(idx + 1);
            m_modeButton->setText(s_modes[idx + 1].first + "  ▾");
            emit modeChanged(s_modes[idx + 1].second);
        }
    });

    // Connect mode button → show menu
    connect(m_modeButton, &QPushButton::clicked, this, &ModePager::showModeMenu);

    // Install swipe gesture on the stacked widget for content swiping
    m_stack->grabGesture(Qt::SwipeGesture);
    installEventFilter(this);
}

CalcMode ModePager::currentMode() const {
    int idx = m_stack->currentIndex();
    if (idx >= 0 && idx < s_modes.size())
        return s_modes[idx].second;
    return CalcMode::Basic;
}

void ModePager::showModeMenu() {
    QMenu menu(this);
    for (int i = 0; i < s_modes.size(); ++i) {
        auto* a = menu.addAction(s_modes[i].first);
        a->setCheckable(true);
        a->setChecked(m_stack->currentIndex() == i);
        connect(a, &QAction::triggered, this, [this, i]{
            m_stack->setCurrentIndex(i);
            m_modeButton->setText(s_modes[i].first + "  ▾");
            emit modeChanged(s_modes[i].second);
        });
    }
    menu.exec(m_modeButton->mapToGlobal(QPoint(0, -menu.sizeHint().height())));
}
