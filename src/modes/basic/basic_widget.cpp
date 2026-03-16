#include "basic_widget.h"
#include "widgets/display_widget.h"
#include "core/math_engine.h"
#include "core/history_manager.h"
#include "widgets/history_panel.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QKeyEvent>

// Helper to create styled buttons
static QPushButton* makeButton(const QString& text,
                                const QString& cssClass,
                                QWidget* parent = nullptr)
{
    auto* btn = new QPushButton(text, parent);
    btn->setProperty("class", cssClass);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn->setMinimumHeight(52);
    btn->setFocusPolicy(Qt::NoFocus);
    return btn;
}

BasicWidget::BasicWidget(MathEngine* engine,
                         HistoryManager* history,
                         HistoryPanel* historyPanel,
                         QWidget* parent)
    : QWidget(parent)
    , m_engine(engine)
    , m_history(history)
    , m_historyPanel(historyPanel)
{
    buildUI();
    setFocusPolicy(Qt::StrongFocus);
}

void BasicWidget::buildUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // Display
    m_display = new DisplayWidget(this);
    mainLayout->addWidget(m_display);

    // Button grid
    auto* grid = new QGridLayout();
    grid->setSpacing(6);

    // Row 0: Clear, +/-, %, /
    struct BtnDef { QString label; int row; int col; int rowSpan; int colSpan; QString cls; };
    const QVector<BtnDef> buttons = {
        {"C",   0, 0, 1, 1, "clearButton"},
        {"+/-", 0, 1, 1, 1, "calcButton"},
        {"%",   0, 2, 1, 1, "calcButton"},
        {"÷",   0, 3, 1, 1, "operatorButton"},
        {"7",   1, 0, 1, 1, "calcButton"},
        {"8",   1, 1, 1, 1, "calcButton"},
        {"9",   1, 2, 1, 1, "calcButton"},
        {"×",   1, 3, 1, 1, "operatorButton"},
        {"4",   2, 0, 1, 1, "calcButton"},
        {"5",   2, 1, 1, 1, "calcButton"},
        {"6",   2, 2, 1, 1, "calcButton"},
        {"-",   2, 3, 1, 1, "operatorButton"},
        {"1",   3, 0, 1, 1, "calcButton"},
        {"2",   3, 1, 1, 1, "calcButton"},
        {"3",   3, 2, 1, 1, "calcButton"},
        {"+",   3, 3, 1, 1, "operatorButton"},
        {"0",   4, 0, 1, 2, "calcButton"},
        {".",   4, 2, 1, 1, "calcButton"},
        {"=",   4, 3, 1, 1, "actionButton"},
    };

    for (const auto& b : buttons) {
        auto* btn = makeButton(b.label, b.cls, this);
        grid->addWidget(btn, b.row, b.col, b.rowSpan, b.colSpan);

        connect(btn, &QPushButton::clicked, this, [this, label = b.label]() {
            if (label == "=")        onEquals();
            else if (label == "C")   onClear();
            else                     onButtonClicked(label);
        });
    }

    mainLayout->addLayout(grid);
    setLayout(mainLayout);
}

void BasicWidget::onButtonClicked(const QString& value) {
    if (value == "+/-") {
        // Toggle sign of current expression
        if (!m_expression.isEmpty()) {
            if (m_expression.startsWith('-'))
                m_expression.remove(0, 1);
            else
                m_expression.prepend('-');
            m_display->setExpression(m_expression);
            m_display->setResult(m_expression);
        }
        return;
    }

    // Map display symbols to actual operators
    QString actual = value;
    if (value == "÷") actual = "/";
    if (value == "×") actual = "*";

    // If result was just shown and user types a digit, start fresh
    if (m_resultShown && (actual[0].isDigit() || actual == ".")) {
        m_expression.clear();
        m_resultShown = false;
    } else if (m_resultShown) {
        // Operator after result: continue with result
        m_resultShown = false;
    }

    appendToExpression(actual);
}

void BasicWidget::appendToExpression(const QString& text) {
    m_expression += text;
    m_display->setExpression(m_expression);

    // Live evaluation preview
    QString result = m_engine->evaluate(m_expression);
    if (!m_engine->hasError())
        m_display->setResult(result);
}

void BasicWidget::onEquals() {
    if (m_expression.isEmpty()) return;
    calculate();
}

void BasicWidget::calculate() {
    QString result = m_engine->evaluate(m_expression);
    bool error = m_engine->hasError();

    m_display->setExpression(m_expression);
    m_display->setResult(result, error);

    if (!error) {
        m_history->add(m_expression, result);
        m_historyPanel->addEntry(QString("%1 = %2").arg(m_expression, result));
        m_expression = result; // allow chaining
        m_resultShown = true;
    }
}

void BasicWidget::onClear() {
    m_expression.clear();
    m_resultShown = false;
    m_display->clear();
}

void BasicWidget::onBackspace() {
    if (!m_expression.isEmpty()) {
        m_expression.chop(1);
        m_display->setExpression(m_expression);
        if (m_expression.isEmpty())
            m_display->setResult("0");
        else {
            QString result = m_engine->evaluate(m_expression);
            if (!m_engine->hasError())
                m_display->setResult(result);
        }
    }
}

void BasicWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_0: case Qt::Key_1: case Qt::Key_2: case Qt::Key_3:
    case Qt::Key_4: case Qt::Key_5: case Qt::Key_6: case Qt::Key_7:
    case Qt::Key_8: case Qt::Key_9:
        onButtonClicked(event->text()); break;
    case Qt::Key_Plus:        onButtonClicked("+"); break;
    case Qt::Key_Minus:       onButtonClicked("-"); break;
    case Qt::Key_Asterisk:    onButtonClicked("×"); break;
    case Qt::Key_Slash:       onButtonClicked("÷"); break;
    case Qt::Key_Percent:     onButtonClicked("%"); break;
    case Qt::Key_Period:      onButtonClicked("."); break;
    case Qt::Key_Return:
    case Qt::Key_Enter:       onEquals(); break;
    case Qt::Key_Backspace:   onBackspace(); break;
    case Qt::Key_Escape:      onClear(); break;
    default: QWidget::keyPressEvent(event);
    }
}
