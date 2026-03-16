#include "mode_sidebar.h"
#include <QLabel>

ModeSidebar::ModeSidebar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("modeSidebar");

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 16, 8, 16);
    layout->setSpacing(4);

    // App title label
    auto* title = new QLabel("GraphCalc", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-weight: bold; font-size: 15px; margin-bottom: 12px;");
    layout->addWidget(title);

    addModeButton("Basic",       CalcMode::Basic);
    addModeButton("Scientific",  CalcMode::Scientific);
    addModeButton("Programming", CalcMode::Programming);
    addModeButton("Date",        CalcMode::Date);
    addModeButton("Conversion",  CalcMode::Conversion);
    addModeButton("Equations",   CalcMode::Equations);
    addModeButton("Graphing",    CalcMode::Graphing);

    layout->addStretch();
    setLayout(layout);

    // Default: Basic selected
    if (!m_buttons.isEmpty())
        m_buttons.first()->setChecked(true);

    connect(m_group, &QButtonGroup::idClicked, this, [this](int id) {
        emit modeChanged(static_cast<CalcMode>(id));
    });
}

QPushButton* ModeSidebar::addModeButton(const QString& label, CalcMode mode) {
    auto* btn = new QPushButton(label, this);
    btn->setCheckable(true);
    btn->setFlat(true);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_group->addButton(btn, static_cast<int>(mode));
    m_buttons.append(btn);
    static_cast<QVBoxLayout*>(layout())->addWidget(btn);
    return btn;
}

void ModeSidebar::setCurrentMode(CalcMode mode) {
    auto* btn = m_group->button(static_cast<int>(mode));
    if (btn) btn->setChecked(true);
}
