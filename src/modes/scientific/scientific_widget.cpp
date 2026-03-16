#include "scientific_widget.h"
#include "widgets/display_widget.h"
#include "core/math_engine.h"
#include "core/history_manager.h"
#include "widgets/history_panel.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QKeyEvent>

static QPushButton* makeBtn(const QString& text, const QString& cls, QWidget* parent = nullptr) {
    auto* btn = new QPushButton(text, parent);
    btn->setProperty("class", cls);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn->setMinimumHeight(44);
    btn->setFocusPolicy(Qt::NoFocus);
    return btn;
}

ScientificWidget::ScientificWidget(MathEngine* engine,
                                   HistoryManager* history,
                                   HistoryPanel* historyPanel,
                                   QWidget* parent)
    : QWidget(parent), m_engine(engine), m_history(history), m_historyPanel(historyPanel)
{
    buildUI();
    setFocusPolicy(Qt::StrongFocus);
}

void ScientificWidget::buildUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // Display
    m_display = new DisplayWidget(this);
    mainLayout->addWidget(m_display);

    // Angle mode + INV toggle row
    auto* topRow = new QHBoxLayout();
    auto* angleLabel = new QLabel("Angle:", this);
    auto* angleCombo = new QComboBox(this);
    angleCombo->addItems({"DEG", "RAD", "GRAD"});
    angleCombo->setFixedWidth(80);

    auto* invBtn = makeBtn("INV", "operatorButton", this);
    invBtn->setCheckable(true);
    invBtn->setFixedWidth(60);

    topRow->addWidget(angleLabel);
    topRow->addWidget(angleCombo);
    topRow->addStretch();
    topRow->addWidget(invBtn);
    mainLayout->addLayout(topRow);

    // Button grid — 5 columns
    auto* grid = new QGridLayout();
    grid->setSpacing(5);

    // Row 0: scientific functions
    struct Btn { QString label; QString inv; int row; int col; QString cls; };
    const QVector<Btn> sciButtons = {
        {"sin",  "asin", 0, 0, "operatorButton"},
        {"cos",  "acos", 0, 1, "operatorButton"},
        {"tan",  "atan", 0, 2, "operatorButton"},
        {"log",  "10^", 0, 3, "operatorButton"},
        {"ln",   "e^",  0, 4, "operatorButton"},

        {"x²",   "√",   1, 0, "operatorButton"},
        {"x³",   "∛",   1, 1, "operatorButton"},
        {"xʸ",   "ʸ√x", 1, 2, "operatorButton"},
        {"√",    "x²",  1, 3, "operatorButton"},
        {"1/x",  "x",   1, 4, "operatorButton"},

        {"π",    "π",   2, 0, "calcButton"},
        {"e",    "e",   2, 1, "calcButton"},
        {"(",    "(",   2, 2, "calcButton"},
        {")",    ")",   2, 3, "calcButton"},
        {"%",    "%",   2, 4, "calcButton"},
    };

    // Store sci buttons for INV toggle
    QVector<QPushButton*> sciBtns;
    for (const auto& b : sciButtons) {
        auto* btn = makeBtn(b.label, b.cls, this);
        grid->addWidget(btn, b.row, b.col);
        sciBtns.append(btn);

        QString primary = b.label;
        QString inverse = b.inv;
        connect(btn, &QPushButton::clicked, this, [this, btn, primary, inverse]() {
            onButtonClicked(m_invertMode ? inverse : primary);
        });
    }

    // INV toggle updates button labels
    connect(invBtn, &QPushButton::toggled, this, [this, sciBtns, sciButtons](bool checked) {
        m_invertMode = checked;
        for (int i = 0; i < sciBtns.size(); ++i)
            sciBtns[i]->setText(checked ? sciButtons[i].inv : sciButtons[i].label);
    });

    // Row 3-6: number pad + operators (same as basic)
    const QVector<std::tuple<QString,int,int,int,int,QString>> numPad = {
        {"C",  3, 0, 1, 1, "clearButton"},
        {"⌫",  3, 1, 1, 1, "clearButton"},
        {"n!", 3, 2, 1, 1, "operatorButton"},
        {"÷",  3, 3, 1, 1, "operatorButton"},
        {"^",  3, 4, 1, 1, "operatorButton"},

        {"7",  4, 0, 1, 1, "calcButton"},
        {"8",  4, 1, 1, 1, "calcButton"},
        {"9",  4, 2, 1, 1, "calcButton"},
        {"×",  4, 3, 1, 1, "operatorButton"},
        {"+/-",4, 4, 1, 1, "calcButton"},

        {"4",  5, 0, 1, 1, "calcButton"},
        {"5",  5, 1, 1, 1, "calcButton"},
        {"6",  5, 2, 1, 1, "calcButton"},
        {"-",  5, 3, 1, 1, "operatorButton"},
        {"EE", 5, 4, 1, 1, "calcButton"},

        {"1",  6, 0, 1, 1, "calcButton"},
        {"2",  6, 1, 1, 1, "calcButton"},
        {"3",  6, 2, 1, 1, "calcButton"},
        {"+",  6, 3, 1, 1, "operatorButton"},
        {"=",  6, 4, 2, 1, "actionButton"},

        {"0",  7, 0, 1, 2, "calcButton"},
        {".",  7, 2, 1, 1, "calcButton"},
        {"ANS",7, 3, 1, 1, "calcButton"},
    };

    for (const auto& [label, row, col, rs, cs, cls] : numPad) {
        auto* btn = makeBtn(label, cls, this);
        grid->addWidget(btn, row, col, rs, cs);
        connect(btn, &QPushButton::clicked, this, [this, label]() {
            if (label == "=")  onEquals();
            else if (label == "C")  onClear();
            else if (label == "⌫") onBackspace();
            else onButtonClicked(label);
        });
    }

    mainLayout->addLayout(grid);
    setLayout(mainLayout);

    connect(angleCombo, &QComboBox::currentIndexChanged, this, &ScientificWidget::onAngleModeChanged);
}

void ScientificWidget::onAngleModeChanged(int index) {
    m_angleMode = static_cast<AngleMode>(index);
    ParserAngleMode pm = (index == 0) ? ParserAngleMode::Degrees
                       : (index == 1) ? ParserAngleMode::Radians
                                      : ParserAngleMode::Gradians;
    m_engine->setAngleMode(pm);
}

void ScientificWidget::onButtonClicked(const QString& value) {
    // Map display symbols to expression tokens
    QString token = value;

    if (value == "÷")   token = "/";
    else if (value == "×") token = "*";
    else if (value == "x²") token = "^2";
    else if (value == "x³") token = "^3";
    else if (value == "xʸ") token = "^";
    else if (value == "√")  token = "sqrt(";
    else if (value == "∛")  token = "cbrt(";
    else if (value == "10^") token = "10^";
    else if (value == "e^")  token = "e^";
    else if (value == "1/x") token = "1/";
    else if (value == "n!")  token = "!";
    else if (value == "EE")  token = "e";
    else if (value == "ANS") token = QString::number(m_engine->lastResult(), 'g', 10);
    else if (value == "sin" || value == "cos" || value == "tan" ||
             value == "asin"|| value == "acos"|| value == "atan"||
             value == "log" || value == "ln"  || value == "log2")
        token = value + "(";
    else if (value == "+/-") {
        if (!m_expression.isEmpty()) {
            m_expression = m_expression.startsWith('-')
                ? m_expression.mid(1) : "-" + m_expression;
            m_display->setExpression(m_expression);
        }
        return;
    }

    if (m_resultShown && !token.isEmpty() && (token[0].isDigit() || token == ".")) {
        m_expression.clear();
        m_resultShown = false;
    } else {
        m_resultShown = false;
    }

    appendToExpression(token);
}

void ScientificWidget::appendToExpression(const QString& text) {
    m_expression += text;
    m_display->setExpression(m_expression);
    QString result = m_engine->evaluate(m_expression);
    if (!m_engine->hasError())
        m_display->setResult(result);
}

void ScientificWidget::onEquals() {
    if (m_expression.isEmpty()) return;
    calculate();
}

void ScientificWidget::calculate() {
    QString result = m_engine->evaluate(m_expression);
    bool error = m_engine->hasError();
    m_display->setExpression(m_expression);
    m_display->setResult(result, error);
    if (!error) {
        m_history->add(m_expression, result);
        m_historyPanel->addEntry(QString("%1 = %2").arg(m_expression, result));
        m_expression  = result;
        m_resultShown = true;
    }
}

void ScientificWidget::onClear() {
    m_expression.clear();
    m_resultShown = false;
    m_display->clear();
}

void ScientificWidget::onBackspace() {
    if (!m_expression.isEmpty()) {
        m_expression.chop(1);
        m_display->setExpression(m_expression);
        if (m_expression.isEmpty()) m_display->setResult("0");
        else {
            QString r = m_engine->evaluate(m_expression);
            if (!m_engine->hasError()) m_display->setResult(r);
        }
    }
}

void ScientificWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_0: case Qt::Key_1: case Qt::Key_2: case Qt::Key_3:
    case Qt::Key_4: case Qt::Key_5: case Qt::Key_6: case Qt::Key_7:
    case Qt::Key_8: case Qt::Key_9:
        onButtonClicked(event->text()); break;
    case Qt::Key_Plus:     onButtonClicked("+"); break;
    case Qt::Key_Minus:    onButtonClicked("-"); break;
    case Qt::Key_Asterisk: onButtonClicked("×"); break;
    case Qt::Key_Slash:    onButtonClicked("÷"); break;
    case Qt::Key_Percent:  onButtonClicked("%"); break;
    case Qt::Key_Period:   onButtonClicked("."); break;
    case Qt::Key_AsciiCircum: onButtonClicked("^"); break;
    case Qt::Key_Return:
    case Qt::Key_Enter:    onEquals(); break;
    case Qt::Key_Backspace: onBackspace(); break;
    case Qt::Key_Escape:   onClear(); break;
    default: QWidget::keyPressEvent(event);
    }
}
