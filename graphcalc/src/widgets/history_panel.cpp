#include "history_panel.h"
#include <QVBoxLayout>
#include <QLabel>

HistoryPanel::HistoryPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("historyPanel");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 12, 8, 8);
    layout->setSpacing(6);

    auto* title = new QLabel("History", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-weight: bold; font-size: 13px;");
    layout->addWidget(title);

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_list);

    m_clearBtn = new QPushButton("Clear", this);
    layout->addWidget(m_clearBtn);

    setLayout(layout);

    connect(m_clearBtn, &QPushButton::clicked, this, &HistoryPanel::clearEntries);

    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        // Entry format: "expression = result" — emit just the expression
        QString text = item->text();
        int sep = text.lastIndexOf(" = ");
        if (sep != -1)
            emit entryClicked(text.left(sep));
        else
            emit entryClicked(text);
    });
}

void HistoryPanel::addEntry(const QString& entry) {
    m_list->insertItem(0, entry); // newest at top
}

void HistoryPanel::clearEntries() {
    m_list->clear();
}
