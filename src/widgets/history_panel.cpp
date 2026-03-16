#include "history_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QEasingCurve>

HistoryPanel::HistoryPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("historyPanel");
    setFixedWidth(PANEL_WIDTH);
    setAttribute(Qt::WA_StyledBackground, true);
    // Ensure solid background — no bleed-through
    setAutoFillBackground(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 14, 10, 10);
    layout->setSpacing(6);

    // Header row
    auto* headerRow = new QHBoxLayout();
    auto* title = new QLabel("History", this);
    title->setObjectName("historyTitle");
    headerRow->addWidget(title);
    headerRow->addStretch();
    m_clearBtn = new QPushButton("Clear", this);
    m_clearBtn->setObjectName("historyClearBtn");
    m_clearBtn->setFixedSize(60, 26);
    m_clearBtn->setStyleSheet(
        "QPushButton { font-size: 12px; padding: 0; }"
    );
    headerRow->addWidget(m_clearBtn);
    layout->addLayout(headerRow);

    // Divider
    auto* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setFrameShape(QFrame::NoFrame);
    layout->addWidget(m_list, 1);

    setLayout(layout);

    // Animation
    m_anim = new QPropertyAnimation(this, "drawerX", this);
    m_anim->setDuration(220);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);

    // Start hidden (off-screen to the right)
    hide();

    connect(m_clearBtn, &QPushButton::clicked, this, &HistoryPanel::clearEntries);
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        QString text = item->text();
        int sep = text.lastIndexOf(" = ");
        emit entryClicked(sep != -1 ? text.left(sep) : text);
    });
}

int HistoryPanel::drawerX() const {
    return x();
}

void HistoryPanel::setDrawerX(int x) {
    move(x, 0);
}

void HistoryPanel::repositionToParent() {
    if (!parentWidget()) return;
    int ph = parentWidget()->height();
    int pw = parentWidget()->width();
    setFixedHeight(ph);
    setFixedWidth(qMin(PANEL_WIDTH, pw)); // never wider than parent

    int openX   = pw - qMin(PANEL_WIDTH, pw);
    int closedX = pw;

    if (m_open)
        move(openX, 0);
    else
        move(closedX, 0);
}

void HistoryPanel::toggleDrawer() {
    if (!parentWidget()) return;

    int ph = parentWidget()->height();
    int pw = parentWidget()->width();
    int panelW = qMin(PANEL_WIDTH, pw);
    setFixedHeight(ph);
    setFixedWidth(panelW);

    int openX   = pw - panelW;
    int closedX = pw;

    m_anim->stop();

    if (!m_open) {
        move(closedX, 0);
        show();
        raise();
        m_anim->setStartValue(closedX);
        m_anim->setEndValue(openX);
        m_open = true;
        emit drawerToggled(true);
    } else {
        m_anim->setStartValue(openX);
        m_anim->setEndValue(closedX);
        m_open = false;
        emit drawerToggled(false);
        connect(m_anim, &QPropertyAnimation::finished, this, [this]{
            if (!m_open) hide();
            disconnect(m_anim, &QPropertyAnimation::finished, this, nullptr);
        });
    }

    m_anim->start();
}

void HistoryPanel::addEntry(const QString& entry) {
    m_list->insertItem(0, entry);
}

void HistoryPanel::clearEntries() {
    m_list->clear();
}
