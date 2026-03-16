#include "programming_widget.h"
#include "core/history_manager.h"
#include "widgets/history_panel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QFrame>
#include <QKeyEvent>
#include <QFont>
#include <cmath>
#include <climits>
#include <stdexcept>

// ── helpers ──────────────────────────────────────────────────────────────────

static QPushButton* makeBtn(const QString& text, const QString& cls,
                             QWidget* parent = nullptr, int minH = 44)
{
    auto* b = new QPushButton(text, parent);
    b->setProperty("class", cls);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    b->setMinimumHeight(minH);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}

static QPushButton* makeToggle(const QString& text, QWidget* parent = nullptr) {
    auto* b = new QPushButton(text, parent);
    b->setCheckable(true);
    b->setFocusPolicy(Qt::NoFocus);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    b->setFixedHeight(30);
    return b;
}

// ── constructor ───────────────────────────────────────────────────────────────

ProgrammingWidget::ProgrammingWidget(HistoryManager* history,
                                     HistoryPanel* historyPanel,
                                     QWidget* parent)
    : QWidget(parent), m_history(history), m_historyPanel(historyPanel)
{
    buildUI();
    setFocusPolicy(Qt::StrongFocus);
    updateDisplays();
    updateButtonStates();
}

// ── UI construction ───────────────────────────────────────────────────────────

void ProgrammingWidget::buildUI() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    // ── Base selector ─────────────────────────────────────────────────────────
    m_baseGroup = new QButtonGroup(this);
    m_baseGroup->setExclusive(true);
    auto* baseRow = new QHBoxLayout();
    for (auto [label, base] : {std::pair{"HEX",16},{"DEC",10},{"OCT",8},{"BIN",2}}) {
        auto* b = makeToggle(label, this);
        m_baseGroup->addButton(b, base);
        baseRow->addWidget(b);
        if (base == 10) b->setChecked(true);
    }
    root->addLayout(baseRow);

    // ── Bit-width selector ────────────────────────────────────────────────────
    m_bitWidthGroup = new QButtonGroup(this);
    m_bitWidthGroup->setExclusive(true);
    auto* bwRow = new QHBoxLayout();
    for (auto [label, bits] : {std::pair{"8-bit",8},{"16-bit",16},{"32-bit",32},{"64-bit",64}}) {
        auto* b = makeToggle(label, this);
        m_bitWidthGroup->addButton(b, bits);
        bwRow->addWidget(b);
        if (bits == 32) b->setChecked(true);
    }
    root->addLayout(bwRow);

    // ── Multi-base display ────────────────────────────────────────────────────
    auto* dispFrame = new QFrame(this);
    dispFrame->setObjectName("displayWidget");
    dispFrame->setFrameShape(QFrame::StyledPanel);
    auto* dispGrid = new QGridLayout(dispFrame);
    dispGrid->setContentsMargins(10, 8, 10, 8);
    dispGrid->setSpacing(4);

    auto makeDispRow = [&](const QString& lbl, QLabel*& valLabel, int row) {
        auto* tag = new QLabel(lbl, dispFrame);
        tag->setStyleSheet("color:#888; font-size:11px; font-weight:bold;");
        tag->setFixedWidth(32);
        valLabel = new QLabel("0", dispFrame);
        valLabel->setObjectName("resultLabel");
        valLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        valLabel->setStyleSheet("font-size:15px; font-family: monospace;");
        valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        dispGrid->addWidget(tag,      row, 0);
        dispGrid->addWidget(valLabel, row, 1);
    };

    makeDispRow("HEX", m_hexLabel, 0);
    makeDispRow("DEC", m_decLabel, 1);
    makeDispRow("OCT", m_octLabel, 2);
    makeDispRow("BIN", m_binLabel, 3);
    root->addWidget(dispFrame);

    // ── Bit visualizer ────────────────────────────────────────────────────────
    auto* bitFrame = new QFrame(this);
    bitFrame->setObjectName("displayWidget");
    auto* bitLayout = new QHBoxLayout(bitFrame);
    bitLayout->setContentsMargins(6, 4, 6, 4);
    bitLayout->setSpacing(2);

    for (int i = 63; i >= 0; --i) {
        auto* lbl = new QLabel("0", bitFrame);
        lbl->setFixedSize(14, 18);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("font-size:9px; font-family:monospace; "
                           "background:#2d2d2d; color:#888; border-radius:2px;");
        m_bitLabels.prepend(lbl);   // index 0 = bit 0 (LSB)
        bitLayout->addWidget(lbl);
        // separator every 8 bits
        if (i > 0 && i % 8 == 0) {
            auto* sep = new QLabel(bitFrame);
            sep->setFixedWidth(4);
            bitLayout->addWidget(sep);
        }
    }
    root->addWidget(bitFrame);

    // ── Button grid ───────────────────────────────────────────────────────────
    auto* grid = new QGridLayout();
    grid->setSpacing(5);

    // Hex digit buttons A-F (row 0)
    const QStringList hexDigits = {"A","B","C","D","E","F"};
    for (int i = 0; i < 6; ++i) {
        auto* b = makeBtn(hexDigits[i], "operatorButton", this);
        grid->addWidget(b, 0, i);
        m_digitButtons.append(b);
        connect(b, &QPushButton::clicked, this, [this, d=hexDigits[i]]{ onDigitClicked(d); });
    }

    // Bitwise ops (row 1)
    const QVector<std::pair<QString,QString>> bitwiseOps = {
        {"AND","AND"},{"OR","OR"},{"XOR","XOR"},{"NOT","NOT"},{"LSH","<<"},{"RSH",">>"}
    };
    for (int i = 0; i < 6; ++i) {
        auto* b = makeBtn(bitwiseOps[i].first, "operatorButton", this);
        grid->addWidget(b, 1, i);
        QString op = bitwiseOps[i].second;
        connect(b, &QPushButton::clicked, this, [this, op]{ onOperatorClicked(op); });
    }

    // Number pad rows 2-5
    struct NBtn { QString lbl; int r,c,rs,cs; QString cls; };
    const QVector<NBtn> numPad = {
        {"C",  2,0,1,1,"clearButton"}, {"⌫", 2,1,1,1,"clearButton"},
        {"MOD",2,2,1,1,"operatorButton"}, {"÷",2,3,1,1,"operatorButton"},
        {"×",  2,4,1,1,"operatorButton"}, {"(",2,5,1,1,"calcButton"},

        {"7",  3,0,1,1,"calcButton"}, {"8",3,1,1,1,"calcButton"},
        {"9",  3,2,1,1,"calcButton"}, {"-",3,3,1,1,"operatorButton"},
        {"+",  3,4,1,1,"operatorButton"}, {")",3,5,1,1,"calcButton"},

        {"4",  4,0,1,1,"calcButton"}, {"5",4,1,1,1,"calcButton"},
        {"6",  4,2,1,1,"calcButton"}, {"=",4,3,2,3,"actionButton"},

        {"1",  5,0,1,1,"calcButton"}, {"2",5,1,1,1,"calcButton"},
        {"3",  5,2,1,1,"calcButton"},

        {"0",  6,0,1,2,"calcButton"}, {"±",6,2,1,1,"calcButton"},
    };

    for (const auto& b : numPad) {
        auto* btn = makeBtn(b.lbl, b.cls, this);
        grid->addWidget(btn, b.r, b.c, b.rs, b.cs);

        // Track 0-9 digit buttons for enable/disable
        if (b.lbl.length() == 1 && b.lbl[0].isDigit())
            m_digitButtons.append(btn);

        connect(btn, &QPushButton::clicked, this, [this, lbl=b.lbl] {
            if (lbl == "=")        onEquals();
            else if (lbl == "C")   onClear();
            else if (lbl == "⌫")  onBackspace();
            else if (lbl == "÷")   onOperatorClicked("/");
            else if (lbl == "×")   onOperatorClicked("*");
            else if (lbl == "+")   onOperatorClicked("+");
            else if (lbl == "-")   onOperatorClicked("-");
            else if (lbl == "MOD") onOperatorClicked("%");
            else if (lbl == "±")   onOperatorClicked("NEG");
            else                   onDigitClicked(lbl);
        });
    }

    root->addLayout(grid);
    setLayout(root);

    // Connect selectors
    connect(m_baseGroup, &QButtonGroup::idClicked, this, &ProgrammingWidget::onBaseChanged);
    connect(m_bitWidthGroup, &QButtonGroup::idClicked, this, &ProgrammingWidget::onBitWidthChanged);
}

// ── slot handlers ─────────────────────────────────────────────────────────────

void ProgrammingWidget::onDigitClicked(const QString& digit) {
    if (m_resultShown) { m_inputBuffer.clear(); m_resultShown = false; }
    m_inputBuffer += digit;
    try {
        m_currentValue = parseInput(m_inputBuffer, static_cast<int>(m_base));
        applyBitWidth();
        updateDisplays();
    } catch (...) {
        m_inputBuffer.chop(1); // revert bad input
    }
}

void ProgrammingWidget::onOperatorClicked(const QString& op) {
    if (op == "NEG") {
        m_currentValue = -m_currentValue;
        applyBitWidth();
        m_inputBuffer = formatInBase(m_currentValue, static_cast<int>(m_base));
        updateDisplays();
        return;
    }
    if (op == "NOT") {
        m_currentValue = ~m_currentValue;
        applyBitWidth();
        m_inputBuffer = formatInBase(m_currentValue, static_cast<int>(m_base));
        updateDisplays();
        return;
    }
    // Binary op: store current and wait for second operand
    if (!m_pendingOp.isEmpty()) {
        // Chain: evaluate pending first
        try {
            m_currentValue = applyOperation(m_storedValue, m_pendingOp, m_currentValue);
            applyBitWidth();
        } catch (...) {}
    }
    m_storedValue  = m_currentValue;
    m_pendingOp    = op;
    m_inputBuffer.clear();
    m_resultShown  = false;
    updateDisplays();
}

void ProgrammingWidget::onEquals() {
    if (m_pendingOp.isEmpty()) return;
    try {
        long long result = applyOperation(m_storedValue, m_pendingOp, m_currentValue);
        applyBitWidth();
        m_currentValue = result;
        applyBitWidth();

        QString expr = formatInBase(m_storedValue, static_cast<int>(m_base))
                     + " " + m_pendingOp + " "
                     + formatInBase(m_currentValue, static_cast<int>(m_base));  // before overwrite
        // re-format after result
        m_inputBuffer = formatInBase(m_currentValue, static_cast<int>(m_base));
        m_history->add(expr, m_inputBuffer);
        m_historyPanel->addEntry(expr + " = " + m_inputBuffer);

        m_pendingOp.clear();
        m_resultShown = true;
        updateDisplays();
    } catch (const std::exception& e) {
        m_decLabel->setText(QString("Error: %1").arg(e.what()));
    }
}

void ProgrammingWidget::onClear() {
    m_currentValue = 0;
    m_storedValue  = 0;
    m_pendingOp.clear();
    m_inputBuffer.clear();
    m_resultShown = false;
    updateDisplays();
}

void ProgrammingWidget::onBackspace() {
    if (!m_inputBuffer.isEmpty()) {
        m_inputBuffer.chop(1);
        m_currentValue = m_inputBuffer.isEmpty()
            ? 0 : parseInput(m_inputBuffer, static_cast<int>(m_base));
        applyBitWidth();
        updateDisplays();
    }
}

void ProgrammingWidget::onBaseChanged(int base) {
    m_base = static_cast<BaseMode>(base);
    m_inputBuffer = formatInBase(m_currentValue, base);
    updateDisplays();
    updateButtonStates();
}

void ProgrammingWidget::onBitWidthChanged(int bits) {
    m_bitWidth = static_cast<BitWidth>(bits);
    applyBitWidth();
    m_inputBuffer = formatInBase(m_currentValue, static_cast<int>(m_base));
    updateDisplays();
}

// ── helpers ───────────────────────────────────────────────────────────────────

void ProgrammingWidget::applyBitWidth() {
    int bits = static_cast<int>(m_bitWidth);
    if (bits < 64) {
        long long mask = (1LL << bits) - 1LL;
        m_currentValue &= mask;
        // Sign-extend
        long long signBit = 1LL << (bits - 1);
        if (m_currentValue & signBit)
            m_currentValue |= ~mask;
    }
}

void ProgrammingWidget::updateDisplays() {
    long long v = m_currentValue;

    // Highlight active base label
    auto highlight = [](QLabel* lbl, bool active) {
        lbl->setStyleSheet(active
            ? "font-size:15px; font-family:monospace; color:#90caf9; font-weight:bold;"
            : "font-size:15px; font-family:monospace;");
    };

    m_hexLabel->setText(formatInBase(v, 16).toUpper());
    m_decLabel->setText(formatInBase(v, 10));
    m_octLabel->setText(formatInBase(v, 8));
    m_binLabel->setText(formatInBase(v, 2));

    highlight(m_hexLabel, m_base == BaseMode::Hex);
    highlight(m_decLabel, m_base == BaseMode::Dec);
    highlight(m_octLabel, m_base == BaseMode::Oct);
    highlight(m_binLabel, m_base == BaseMode::Bin);

    // Bit visualizer
    int activeBits = static_cast<int>(m_bitWidth);
    for (int i = 0; i < 64; ++i) {
        bool active = (i < activeBits);
        bool set    = active && ((v >> i) & 1);
        m_bitLabels[i]->setText(set ? "1" : "0");
        m_bitLabels[i]->setStyleSheet(
            active
            ? (set ? "font-size:9px;font-family:monospace;background:#1565c0;color:#fff;border-radius:2px;"
                   : "font-size:9px;font-family:monospace;background:#2d2d2d;color:#888;border-radius:2px;")
            : "font-size:9px;font-family:monospace;background:#1a1a1a;color:#333;border-radius:2px;"
        );
    }
}

void ProgrammingWidget::updateButtonStates() {
    // Enable only digits valid for current base
    int base = static_cast<int>(m_base);
    for (auto* btn : m_digitButtons) {
        QString lbl = btn->text();
        bool valid = false;
        if (lbl.length() == 1) {
            QChar c = lbl[0].toUpper();
            if (c >= '0' && c <= '9') valid = (c.digitValue() < base);
            else if (c >= 'A' && c <= 'F') valid = (base == 16);
        }
        btn->setEnabled(valid);
        btn->setStyleSheet(valid ? "" : "color:#555;");
    }
}

long long ProgrammingWidget::applyOperation(long long a, const QString& op, long long b) {
    if (op == "+")   return a + b;
    if (op == "-")   return a - b;
    if (op == "*")   return a * b;
    if (op == "/") {
        if (b == 0) throw std::runtime_error("Division by zero");
        return a / b;
    }
    if (op == "%") {
        if (b == 0) throw std::runtime_error("Division by zero");
        return a % b;
    }
    if (op == "AND") return a & b;
    if (op == "OR")  return a | b;
    if (op == "XOR") return a ^ b;
    if (op == "<<")  return a << (b & 63);
    if (op == ">>")  return a >> (b & 63);
    throw std::runtime_error("Unknown operator");
}

long long ProgrammingWidget::parseInput(const QString& text, int base) {
    if (text.isEmpty()) return 0;
    bool ok = false;
    long long val = text.toLongLong(&ok, base);
    if (!ok) throw std::runtime_error("Invalid input");
    return val;
}

QString ProgrammingWidget::formatInBase(long long value, int base) const {
    if (base == 10) return QString::number(value);

    // For non-decimal, treat as unsigned within bit width
    int bits = static_cast<int>(m_bitWidth);
    unsigned long long mask = (bits < 64) ? ((1ULL << bits) - 1ULL) : ~0ULL;
    unsigned long long uval = static_cast<unsigned long long>(value) & mask;

    return QString::number(uval, base).toUpper();
}

// ── keyboard ──────────────────────────────────────────────────────────────────

void ProgrammingWidget::keyPressEvent(QKeyEvent* event) {
    int base = static_cast<int>(m_base);
    QString key = event->text().toUpper();

    if (!key.isEmpty()) {
        QChar c = key[0];
        bool isHexDigit = (c >= 'A' && c <= 'F' && base == 16);
        bool isDigit    = c.isDigit() && (c.digitValue() < base);
        if (isDigit || isHexDigit) { onDigitClicked(key); return; }
    }

    switch (event->key()) {
    case Qt::Key_Plus:     onOperatorClicked("+"); break;
    case Qt::Key_Minus:    onOperatorClicked("-"); break;
    case Qt::Key_Asterisk: onOperatorClicked("*"); break;
    case Qt::Key_Slash:    onOperatorClicked("/"); break;
    case Qt::Key_Percent:  onOperatorClicked("%"); break;
    case Qt::Key_Return:
    case Qt::Key_Enter:    onEquals();    break;
    case Qt::Key_Backspace: onBackspace(); break;
    case Qt::Key_Escape:   onClear();     break;
    default: QWidget::keyPressEvent(event);
    }
}
