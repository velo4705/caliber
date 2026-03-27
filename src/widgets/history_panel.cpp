#include "history_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QMenu>
#include <QContextMenuEvent>
#include <QSettings>

HistoryPanel::HistoryPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("historyPanel");
    setFixedWidth(PANEL_WIDTH);
    setAttribute(Qt::WA_StyledBackground, true);
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
    m_clearBtn->setStyleSheet("QPushButton { font-size: 12px; padding: 0; }");
    headerRow->addWidget(m_clearBtn);

    auto* exportBtn = new QPushButton("Export", this);
    exportBtn->setObjectName("historyClearBtn");
    exportBtn->setFixedSize(60, 26);
    exportBtn->setStyleSheet("QPushButton { font-size: 12px; padding: 0; }");
    headerRow->addWidget(exportBtn);
    layout->addLayout(headerRow);

    // Search bar
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("Filter history...");
    m_search->setClearButtonEnabled(true);
    layout->addWidget(m_search);

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

    m_anim = new QPropertyAnimation(this, "drawerX", this);
    m_anim->setDuration(220);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
    hide();

    connect(m_clearBtn, &QPushButton::clicked, this, &HistoryPanel::clearEntries);
    connect(exportBtn, &QPushButton::clicked, this, [this] {
        if (m_allEntries.isEmpty()) {
            QMessageBox::information(this, "Export", "No history to export.");
            return;
        }
        QString path = QFileDialog::getSaveFileName(this, "Export History",
            "history.txt", "Text file (*.txt);;CSV file (*.csv)");
        if (path.isEmpty()) return;
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;
        QTextStream out(&f);
        bool csv = path.endsWith(".csv", Qt::CaseInsensitive);
        if (csv) out << "Expression,Result\n";
        for (const QString& entry : m_allEntries) {
            if (csv) {
                int sep = entry.lastIndexOf(" = ");
                if (sep != -1)
                    out << "\"" << entry.left(sep) << "\",\"" << entry.mid(sep+3) << "\"\n";
                else
                    out << "\"" << entry << "\",\"\"\n";
            } else {
                out << entry << "\n";
            }
        }
        f.close();
    });
    connect(m_search, &QLineEdit::textChanged, this, &HistoryPanel::filterEntries);
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        QString text = item->data(Qt::UserRole).toString();
        int sep = text.lastIndexOf(" = ");
        emit entryClicked(sep != -1 ? text.left(sep) : text);
    });

    loadPins();
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
    m_allEntries.prepend(entry);
    filterEntries(m_search->text());
}

void HistoryPanel::clearEntries() {
    m_allEntries.clear();
    m_list->clear();
    m_search->clear();
}

void HistoryPanel::filterEntries(const QString& query) {
    m_list->clear();
    // Show pinned entries first
    for (const QString& entry : m_allEntries) {
        if (!m_pinnedEntries.contains(entry)) continue;
        if (!query.isEmpty() && !entry.contains(query, Qt::CaseInsensitive)) continue;
        auto* item = new QListWidgetItem("★ " + entry);
        item->setData(Qt::UserRole, entry);
        item->setForeground(QColor(0xff, 0xd7, 0x00)); // gold for pinned
        m_list->addItem(item);
    }
    // Then unpinned entries
    for (const QString& entry : m_allEntries) {
        if (m_pinnedEntries.contains(entry)) continue;
        if (!query.isEmpty() && !entry.contains(query, Qt::CaseInsensitive)) continue;
        auto* item = new QListWidgetItem(entry);
        item->setData(Qt::UserRole, entry);
        m_list->addItem(item);
    }
}

void HistoryPanel::contextMenuEvent(QContextMenuEvent* event) {
    auto* item = m_list->itemAt(m_list->mapFromGlobal(event->globalPos()));
    if (!item) return;

    QString entry = item->data(Qt::UserRole).toString();
    bool pinned = m_pinnedEntries.contains(entry);

    QMenu menu(this);
    QAction* pinAction = menu.addAction(pinned ? "★ Unpin" : "☆ Pin");
    QAction* selected = menu.exec(event->globalPos());
    if (selected == pinAction) {
        if (pinned) {
            m_pinnedEntries.remove(entry);
        } else {
            m_pinnedEntries.insert(entry);
        }
        savePins();
        filterEntries(m_search->text());
    }
}

void HistoryPanel::refreshList() {
    filterEntries(m_search->text());
}

void HistoryPanel::loadPins() {
    QSettings s("Caliber", "Caliber");
    QStringList pins = s.value("history/pins").toStringList();
    for (const QString& p : pins) m_pinnedEntries.insert(p);
}

void HistoryPanel::savePins() {
    QSettings s("Caliber", "Caliber");
    s.setValue("history/pins", QStringList(m_pinnedEntries.values()));
}
