#include "display_widget.h"
#include <QStyle>

DisplayWidget::DisplayWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("displayWidget");

    m_expressionLabel = new QLabel(this);
    m_expressionLabel->setObjectName("expressionLabel");
    m_expressionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_expressionLabel->setWordWrap(false);
    m_expressionLabel->setText("");

    m_resultLabel = new QLabel("0", this);
    m_resultLabel->setObjectName("resultLabel");
    m_resultLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_resultLabel->setWordWrap(false);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);
    layout->addStretch();
    layout->addWidget(m_expressionLabel);
    layout->addWidget(m_resultLabel);
    setLayout(layout);

    setMinimumHeight(100);
}

void DisplayWidget::setExpression(const QString& expr) {
    m_expressionLabel->setText(expr);
}

void DisplayWidget::setResult(const QString& result, bool isError) {
    m_resultLabel->setText(result);
    // Red tint for errors via dynamic property
    m_resultLabel->setProperty("error", isError);
    m_resultLabel->style()->unpolish(m_resultLabel);
    m_resultLabel->style()->polish(m_resultLabel);
}

void DisplayWidget::clear() {
    m_expressionLabel->setText("");
    m_resultLabel->setText("0");
    m_resultLabel->setProperty("error", false);
}
