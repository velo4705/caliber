#include "financial_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QDoubleValidator>
#include <cmath>

static QPushButton* mkBtn(const QString& t, QWidget* p) {
    auto* b = new QPushButton(t, p);
    b->setProperty("class", "actionButton");
    b->setMinimumHeight(36); b->setFocusPolicy(Qt::NoFocus);
    return b;
}
static QLineEdit* numEdit(const QString& def, QWidget* p) {
    auto* e = new QLineEdit(def);
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,e));
    e->setFixedWidth(100);
    return e;
}
static QLabel* resultLbl(QWidget* p) {
    auto* l = new QLabel("—", p);
    l->setWordWrap(true);
    l->setStyleSheet("font-size:13px; font-weight:bold; padding:8px;");
    return l;
}
static QTextEdit* stepsBox(QWidget* p) {
    auto* t = new QTextEdit(p); t->setReadOnly(true);
    t->setMinimumHeight(100); t->setMaximumHeight(180);
    t->setStyleSheet("font-family:monospace; font-size:12px;");
    t->hide(); return t;
}
static QLabel* descLbl(const QString& text, QWidget* p) {
    auto* l = new QLabel(text, p); l->setWordWrap(true);
    l->setStyleSheet("color:gray; font-size:12px; padding:4px 0;");
    return l;
}

void FinancialWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

FinancialWidget::FinancialWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Financial", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildCompoundTab(), "Compound Interest");
    tabs->addTab(buildLoanTab(),     "Loan / Mortgage");
    tabs->addTab(buildNpvTab(),      "NPV / IRR");
    tabs->addTab(buildPercentTab(),  "Percentage");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Compound Interest ─────────────────────────────────────────────────────────
QWidget* FinancialWidget::buildCompoundTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes future value A = P(1 + r/n)^(nt). Also solves for present value, rate, or time. Enter the known values.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_ciP = numEdit("1000", w); m_ciR = numEdit("0.05", w);
    m_ciN = numEdit("12",   w); m_ciT = numEdit("5",    w);
    m_ciComp = new QComboBox(w);
    m_ciComp->addItems({"Monthly (12)", "Quarterly (4)", "Semi-annual (2)", "Annual (1)", "Daily (365)", "Continuous"});
    g->addWidget(new QLabel("Principal P:",      w), 0, 0); g->addWidget(m_ciP,    0, 1);
    g->addWidget(new QLabel("Annual rate r:",    w), 1, 0); g->addWidget(m_ciR,    1, 1);
    g->addWidget(new QLabel("Time t (years):",   w), 2, 0); g->addWidget(m_ciT,    2, 1);
    g->addWidget(new QLabel("Compounding:",      w), 3, 0); g->addWidget(m_ciComp, 3, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_ciResult = resultLbl(w); m_ciShow = new QCheckBox("Show Steps", w); m_ciSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_ciResult);
    v->addWidget(m_ciShow); v->addWidget(m_ciSteps); v->addStretch();
    connect(m_ciShow, &QCheckBox::toggled, this, [this](bool on){ m_ciSteps->setVisible(on && !m_ciSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &FinancialWidget::computeCompound);
    return w;
}

void FinancialWidget::computeCompound() {
    double P = m_ciP->text().toDouble();
    double r = m_ciR->text().toDouble();
    double t = m_ciT->text().toDouble();
    QStringList steps;
    steps << QString("P = %1,  r = %2 (%3%),  t = %4 years").arg(P).arg(r).arg(r*100,0,'f',2).arg(t);

    int idx = m_ciComp->currentIndex();
    double A;
    if (idx == 5) { // Continuous
        A = P * std::exp(r * t);
        steps << "Continuous compounding: A = P × e^(rt)";
        steps << QString("A = %1 × e^(%2 × %3) = %4").arg(P).arg(r).arg(t).arg(A,0,'f',2);
    } else {
        int n[] = {12, 4, 2, 1, 365};
        int ni = n[idx];
        steps << QString("Compounding %1 times/year: A = P(1 + r/n)^(nt)").arg(ni);
        steps << QString("A = %1 × (1 + %2/%3)^(%3 × %4)").arg(P).arg(r).arg(ni).arg(t);
        A = P * std::pow(1 + r/ni, ni * t);
        steps << QString("A = %1").arg(A,0,'f',2);
    }
    double interest = A - P;
    steps << QString("Interest earned = A − P = %1").arg(interest,0,'f',2);
    m_ciResult->setText(QString("Future Value A = %1\nInterest Earned = %2\nTotal Return = %3%")
        .arg(A,0,'f',2).arg(interest,0,'f',2).arg(interest/P*100,0,'f',2));
    showSteps(m_ciSteps, m_ciShow, steps);
}

// ── Loan / Mortgage ───────────────────────────────────────────────────────────
QWidget* FinancialWidget::buildLoanTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes monthly payment, total payment, and total interest for a loan. Formula: M = P[r(1+r)^n] / [(1+r)^n − 1] where r is monthly rate.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_loanP = numEdit("200000", w); m_loanR = numEdit("0.05", w); m_loanN = numEdit("360", w);
    g->addWidget(new QLabel("Principal P:",         w), 0, 0); g->addWidget(m_loanP, 0, 1);
    g->addWidget(new QLabel("Annual rate r:",        w), 1, 0); g->addWidget(m_loanR, 1, 1);
    g->addWidget(new QLabel("Months n:",             w), 2, 0); g->addWidget(m_loanN, 2, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_loanResult = resultLbl(w); m_loanShow = new QCheckBox("Show Steps", w); m_loanSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_loanResult);
    v->addWidget(m_loanShow); v->addWidget(m_loanSteps); v->addStretch();
    connect(m_loanShow, &QCheckBox::toggled, this, [this](bool on){ m_loanSteps->setVisible(on && !m_loanSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &FinancialWidget::computeLoan);
    return w;
}

void FinancialWidget::computeLoan() {
    double P = m_loanP->text().toDouble();
    double r = m_loanR->text().toDouble() / 12.0; // monthly rate
    int    n = qMax(1, (int)m_loanN->text().toDouble());
    QStringList steps;
    steps << QString("P = %1,  annual rate = %2%,  monthly rate r = %3,  n = %4 months")
             .arg(P).arg(m_loanR->text().toDouble()*100,0,'f',2).arg(r,0,'g',6).arg(n);
    steps << "Monthly payment M = P × r(1+r)^n / [(1+r)^n − 1]";
    double M, totalPaid, totalInterest;
    if (r == 0) {
        M = P / n;
        steps << "r = 0 (interest-free): M = P/n";
    } else {
        double factor = std::pow(1+r, n);
        M = P * r * factor / (factor - 1);
        steps << QString("(1+r)^n = %1").arg(factor,0,'g',8);
        steps << QString("M = %1 × %2 × %3 / (%3 − 1) = %4").arg(P).arg(r,0,'g',6).arg(factor,0,'g',8).arg(M,0,'f',2);
    }
    totalPaid     = M * n;
    totalInterest = totalPaid - P;
    steps << QString("Total paid = M × n = %1").arg(totalPaid,0,'f',2);
    steps << QString("Total interest = Total − Principal = %1").arg(totalInterest,0,'f',2);
    m_loanResult->setText(QString("Monthly Payment = %1\nTotal Paid = %2\nTotal Interest = %3")
        .arg(M,0,'f',2).arg(totalPaid,0,'f',2).arg(totalInterest,0,'f',2));
    showSteps(m_loanSteps, m_loanShow, steps);
}

// ── NPV / IRR ─────────────────────────────────────────────────────────────────
QWidget* FinancialWidget::buildNpvTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("NPV: Net Present Value — sum of discounted future cash flows minus initial investment. IRR: the discount rate that makes NPV = 0. Enter cash flows one per line.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_npvRate    = numEdit("0.10", w);
    m_npvInitial = numEdit("1000", w);
    g->addWidget(new QLabel("Discount rate r:", w), 0, 0); g->addWidget(m_npvRate,    0, 1);
    g->addWidget(new QLabel("Initial invest.:", w), 1, 0); g->addWidget(m_npvInitial, 1, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    v->addWidget(new QLabel("Cash flows (one per line, year 1 onwards):", w));
    m_npvCashflows = new QTextEdit(w);
    m_npvCashflows->setPlaceholderText("300\n400\n500\n200");
    m_npvCashflows->setMaximumHeight(100);
    v->addWidget(m_npvCashflows);
    m_npvResult = resultLbl(w); m_npvShow = new QCheckBox("Show Steps", w); m_npvSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute NPV & IRR", w)); v->addWidget(m_npvResult);
    v->addWidget(m_npvShow); v->addWidget(m_npvSteps); v->addStretch();
    connect(m_npvShow, &QCheckBox::toggled, this, [this](bool on){ m_npvSteps->setVisible(on && !m_npvSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &FinancialWidget::computeNpv);
    return w;
}

void FinancialWidget::computeNpv() {
    double r  = m_npvRate->text().toDouble();
    double C0 = m_npvInitial->text().toDouble();
    QVector<double> cf;
    for (const QString& line : m_npvCashflows->toPlainText().split('\n', Qt::SkipEmptyParts)) {
        bool ok; double v = line.trimmed().toDouble(&ok); if (ok) cf.append(v);
    }
    if (cf.isEmpty()) { m_npvResult->setText("Enter cash flows."); return; }
    QStringList steps;
    steps << QString("Initial investment C₀ = %1,  discount rate r = %2%").arg(C0).arg(r*100,0,'f',1);
    steps << "NPV = −C₀ + Σ CFₜ/(1+r)^t";
    double npv = -C0;
    for (int t = 0; t < cf.size(); t++) {
        double pv = cf[t] / std::pow(1+r, t+1);
        npv += pv;
        steps << QString("Year %1: CF=%2, PV=%3").arg(t+1).arg(cf[t],0,'f',2).arg(pv,0,'f',2);
    }
    steps << QString("NPV = %1").arg(npv,0,'f',2);

    // IRR via bisection
    auto npvAt = [&](double rate) {
        double n = -C0;
        for (int t = 0; t < cf.size(); t++) n += cf[t] / std::pow(1+rate, t+1);
        return n;
    };
    double lo = -0.99, hi = 10.0, irr = 0;
    bool irrFound = false;
    if (npvAt(lo) * npvAt(hi) < 0) {
        for (int i = 0; i < 100; i++) {
            irr = (lo+hi)/2;
            if (npvAt(irr) > 0) lo = irr; else hi = irr;
        }
        irrFound = true;
        steps << QString("IRR ≈ %1% (rate where NPV = 0)").arg(irr*100,0,'f',4);
    } else {
        steps << "IRR: could not find a root in [-99%, 1000%]";
    }
    m_npvResult->setText(QString("NPV = %1\n%2")
        .arg(npv,0,'f',2)
        .arg(irrFound ? QString("IRR = %1%").arg(irr*100,0,'f',4) : "IRR: not found"));
    showSteps(m_npvSteps, m_npvShow, steps);
}

// ── Percentage Tools ──────────────────────────────────────────────────────────
QWidget* FinancialWidget::buildPercentTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Quick percentage calculations: X% of Y, markup, discount, percentage change, tip, and reverse percentage.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_pctValue = numEdit("200", w);
    m_pctPct   = numEdit("15",  w);
    g->addWidget(new QLabel("Value:",      w), 0, 0); g->addWidget(m_pctValue, 0, 1);
    g->addWidget(new QLabel("Percent %:",  w), 1, 0); g->addWidget(m_pctPct,   1, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_pctResult = resultLbl(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_pctResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &FinancialWidget::computePercent);
    return w;
}

void FinancialWidget::computePercent() {
    double val = m_pctValue->text().toDouble();
    double pct = m_pctPct->text().toDouble();
    double p   = pct / 100.0;
    m_pctResult->setText(QString(
        "%1% of %2 = %3\n"
        "%2 + %1% (markup) = %4\n"
        "%2 − %1% (discount) = %5\n"
        "Tip (%1%) on %2 = %3  →  Total = %4\n"
        "If %2 is after %1% increase, original = %6\n"
        "If %2 is after %1% decrease, original = %7")
        .arg(pct,0,'f',2).arg(val,0,'f',2)
        .arg(val*p,0,'f',2)
        .arg(val*(1+p),0,'f',2)
        .arg(val*(1-p),0,'f',2)
        .arg(val/(1+p),0,'f',2)
        .arg(val/(1-p),0,'f',2));
}
