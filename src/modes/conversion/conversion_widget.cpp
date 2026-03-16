#include "conversion_widget.h"
#include "unit_converter.h"
#include "currency_api.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QDoubleValidator>

// ── helpers ───────────────────────────────────────────────────────────────────

static QFrame* card(QWidget* parent) {
    auto* f = new QFrame(parent);
    f->setObjectName("displayWidget");
    f->setFrameShape(QFrame::StyledPanel);
    return f;
}

static QPushButton* actionBtn(const QString& text, QWidget* parent) {
    auto* b = new QPushButton(text, parent);
    b->setProperty("class", "actionButton");
    b->setMinimumHeight(40);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}

static QPushButton* swapBtn(QWidget* parent) {
    auto* b = new QPushButton("⇅", parent);
    b->setProperty("class", "operatorButton");
    b->setFixedSize(36, 36);
    b->setToolTip("Swap");
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}

// ── constructor ───────────────────────────────────────────────────────────────

ConversionWidget::ConversionWidget(QWidget* parent)
    : QWidget(parent)
    , m_converter(new UnitConverter())
    , m_currencyApi(new CurrencyApi(this))
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    auto* title = new QLabel("Conversion", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    root->addWidget(title);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(buildUnitTab(),     "Unit Conversion");
    tabs->addTab(buildCurrencyTab(), "Currency");
    root->addWidget(tabs);
    setLayout(root);

    connect(m_currencyApi, &CurrencyApi::ratesUpdated, this, &ConversionWidget::onRatesUpdated);
    connect(m_currencyApi, &CurrencyApi::fetchError,   this, [this](const QString& err) {
        m_currStatus->setText("⚠ " + err + (m_currencyApi->hasRates() ? " (using cached rates)" : ""));
    });

    // Load cached rates immediately, then refresh in background
    if (m_currencyApi->hasRates()) populateCurrencies();
    m_currencyApi->fetchRates("USD");
}

// ── Unit Conversion Tab ───────────────────────────────────────────────────────

QWidget* ConversionWidget::buildUnitTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);

    // Category selector
    auto* catCard = card(w);
    auto* catForm = new QFormLayout(catCard);
    catForm->setContentsMargins(12, 10, 12, 10);
    m_categoryCombo = new QComboBox(w);
    m_categoryCombo->addItems(m_converter->categoryNames());
    catForm->addRow("Category:", m_categoryCombo);
    v->addWidget(catCard);

    // From / To row
    auto* convCard = card(w);
    auto* convGrid = new QGridLayout(convCard);
    convGrid->setContentsMargins(12, 10, 12, 10);
    convGrid->setSpacing(8);

    m_unitInput   = new QLineEdit("1", w);
    m_fromUnitCombo = new QComboBox(w);
    m_toUnitCombo   = new QComboBox(w);
    m_unitInput->setValidator(new QDoubleValidator(-1e15, 1e15, 10, w));
    m_unitInput->setMinimumHeight(36);

    auto* swap = swapBtn(w);

    convGrid->addWidget(new QLabel("From:", w), 0, 0);
    convGrid->addWidget(m_fromUnitCombo,         0, 1);
    convGrid->addWidget(m_unitInput,             1, 0, 1, 2);
    convGrid->addWidget(swap,                    2, 0, 1, 2, Qt::AlignCenter);
    convGrid->addWidget(new QLabel("To:", w),    3, 0);
    convGrid->addWidget(m_toUnitCombo,           3, 1);
    v->addWidget(convCard);

    // Result
    m_unitResult = new QLabel("—", w);
    m_unitResult->setObjectName("resultLabel");
    m_unitResult->setAlignment(Qt::AlignCenter);
    m_unitResult->setStyleSheet("font-size:22px; font-weight:bold; padding:12px;");
    m_unitResult->setWordWrap(true);
    v->addWidget(m_unitResult);
    v->addStretch();

    // Populate units for first category
    onCategoryChanged(m_converter->categoryNames().first());

    connect(m_categoryCombo, &QComboBox::currentTextChanged,
            this, &ConversionWidget::onCategoryChanged);
    connect(m_fromUnitCombo, &QComboBox::currentTextChanged,
            this, &ConversionWidget::convertUnits);
    connect(m_toUnitCombo,   &QComboBox::currentTextChanged,
            this, &ConversionWidget::convertUnits);
    connect(m_unitInput,     &QLineEdit::textChanged,
            this, &ConversionWidget::convertUnits);
    connect(swap, &QPushButton::clicked, this, &ConversionWidget::swapUnits);

    return w;
}

void ConversionWidget::onCategoryChanged(const QString& category) {
    QStringList units = m_converter->unitNames(category);
    m_fromUnitCombo->blockSignals(true);
    m_toUnitCombo->blockSignals(true);
    m_fromUnitCombo->clear();
    m_toUnitCombo->clear();
    m_fromUnitCombo->addItems(units);
    m_toUnitCombo->addItems(units);
    if (units.size() > 1) m_toUnitCombo->setCurrentIndex(1);
    m_fromUnitCombo->blockSignals(false);
    m_toUnitCombo->blockSignals(false);
    convertUnits();
}

void ConversionWidget::convertUnits() {
    bool ok;
    double val = m_unitInput->text().toDouble(&ok);
    if (!ok) { m_unitResult->setText("—"); return; }

    try {
        double result = m_converter->convert(
            m_categoryCombo->currentText(),
            m_fromUnitCombo->currentText(),
            m_toUnitCombo->currentText(),
            val
        );
        // Format nicely
        QString formatted = (result == (long long)result && qAbs(result) < 1e12)
            ? QString::number((long long)result)
            : QString::number(result, 'g', 8);

        m_unitResult->setText(
            QString("%1 %2  =  %3 %4")
                .arg(m_unitInput->text())
                .arg(m_fromUnitCombo->currentText())
                .arg(formatted)
                .arg(m_toUnitCombo->currentText())
        );
    } catch (const std::exception& e) {
        m_unitResult->setText(QString("Error: %1").arg(e.what()));
    }
}

void ConversionWidget::swapUnits() {
    int from = m_fromUnitCombo->currentIndex();
    int to   = m_toUnitCombo->currentIndex();
    m_fromUnitCombo->blockSignals(true);
    m_toUnitCombo->blockSignals(true);
    m_fromUnitCombo->setCurrentIndex(to);
    m_toUnitCombo->setCurrentIndex(from);
    m_fromUnitCombo->blockSignals(false);
    m_toUnitCombo->blockSignals(false);
    convertUnits();
}

// ── Currency Tab ──────────────────────────────────────────────────────────────

QWidget* ConversionWidget::buildCurrencyTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);

    // Status bar
    m_currStatus = new QLabel("Fetching live rates...", w);
    m_currStatus->setStyleSheet("color:#888; font-size:11px;");
    m_currStatus->setAlignment(Qt::AlignRight);
    v->addWidget(m_currStatus);

    // From / To
    auto* convCard = card(w);
    auto* convGrid = new QGridLayout(convCard);
    convGrid->setContentsMargins(12, 10, 12, 10);
    convGrid->setSpacing(8);

    m_fromCurrCombo = new QComboBox(w);
    m_toCurrCombo   = new QComboBox(w);
    m_currInput     = new QLineEdit("1", w);
    m_currInput->setValidator(new QDoubleValidator(0, 1e15, 6, w));
    m_currInput->setMinimumHeight(36);

    auto* swap = swapBtn(w);
    auto* refreshBtn = actionBtn("↻ Refresh Rates", w);

    convGrid->addWidget(new QLabel("Amount:", w),  0, 0);
    convGrid->addWidget(m_currInput,               0, 1);
    convGrid->addWidget(new QLabel("From:", w),    1, 0);
    convGrid->addWidget(m_fromCurrCombo,           1, 1);
    convGrid->addWidget(swap,                      2, 0, 1, 2, Qt::AlignCenter);
    convGrid->addWidget(new QLabel("To:", w),      3, 0);
    convGrid->addWidget(m_toCurrCombo,             3, 1);
    v->addWidget(convCard);

    v->addWidget(refreshBtn);

    m_currResult = new QLabel("—", w);
    m_currResult->setObjectName("resultLabel");
    m_currResult->setAlignment(Qt::AlignCenter);
    m_currResult->setStyleSheet("font-size:22px; font-weight:bold; padding:12px;");
    m_currResult->setWordWrap(true);
    v->addWidget(m_currResult);
    v->addStretch();

    connect(m_fromCurrCombo, &QComboBox::currentTextChanged,
            this, &ConversionWidget::convertCurrency);
    connect(m_toCurrCombo,   &QComboBox::currentTextChanged,
            this, &ConversionWidget::convertCurrency);
    connect(m_currInput,     &QLineEdit::textChanged,
            this, &ConversionWidget::convertCurrency);
    connect(swap,        &QPushButton::clicked, this, &ConversionWidget::swapCurrencies);
    connect(refreshBtn,  &QPushButton::clicked, this, [this] {
        m_currStatus->setText("Refreshing...");
        m_currencyApi->fetchRates("USD");
    });

    return w;
}

void ConversionWidget::onRatesUpdated() {
    populateCurrencies();
    m_currStatus->setText(
        QString("Rates updated: %1  (base: %2)")
            .arg(m_currencyApi->lastUpdated().toString("dd MMM yyyy hh:mm"))
            .arg(m_currencyApi->baseCurrency())
    );
    convertCurrency();
}

void ConversionWidget::populateCurrencies() {
    QStringList currencies = m_currencyApi->availableCurrencies();
    QString prevFrom = m_fromCurrCombo->currentText();
    QString prevTo   = m_toCurrCombo->currentText();

    m_fromCurrCombo->blockSignals(true);
    m_toCurrCombo->blockSignals(true);
    m_fromCurrCombo->clear();
    m_toCurrCombo->clear();
    m_fromCurrCombo->addItems(currencies);
    m_toCurrCombo->addItems(currencies);

    // Restore previous selection or sensible defaults
    int fromIdx = currencies.indexOf(prevFrom.isEmpty() ? "USD" : prevFrom);
    int toIdx   = currencies.indexOf(prevTo.isEmpty()   ? "EUR" : prevTo);
    if (fromIdx >= 0) m_fromCurrCombo->setCurrentIndex(fromIdx);
    if (toIdx   >= 0) m_toCurrCombo->setCurrentIndex(toIdx);

    m_fromCurrCombo->blockSignals(false);
    m_toCurrCombo->blockSignals(false);
}

void ConversionWidget::convertCurrency() {
    if (!m_currencyApi->hasRates()) {
        m_currResult->setText("Waiting for rates...");
        return;
    }
    bool ok;
    double amount = m_currInput->text().toDouble(&ok);
    if (!ok) { m_currResult->setText("—"); return; }

    double result = m_currencyApi->convert(
        m_fromCurrCombo->currentText(),
        m_toCurrCombo->currentText(),
        amount
    );

    if (result < 0) {
        m_currResult->setText("Currency not available");
        return;
    }

    m_currResult->setText(
        QString("%1 %2  =  %3 %4")
            .arg(m_currInput->text())
            .arg(m_fromCurrCombo->currentText())
            .arg(QString::number(result, 'f', 4))
            .arg(m_toCurrCombo->currentText())
    );
}

void ConversionWidget::swapCurrencies() {
    int from = m_fromCurrCombo->currentIndex();
    int to   = m_toCurrCombo->currentIndex();
    m_fromCurrCombo->blockSignals(true);
    m_toCurrCombo->blockSignals(true);
    m_fromCurrCombo->setCurrentIndex(to);
    m_toCurrCombo->setCurrentIndex(from);
    m_fromCurrCombo->blockSignals(false);
    m_toCurrCombo->blockSignals(false);
    convertCurrency();
}
