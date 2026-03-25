#include "numbertheory_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QIntValidator>
#include <cmath>
#include <vector>

static QPushButton* mkBtn(const QString& t, QWidget* p) {
    auto* b = new QPushButton(t, p);
    b->setProperty("class", "actionButton");
    b->setMinimumHeight(36); b->setFocusPolicy(Qt::NoFocus);
    return b;
}
static QLineEdit* intEdit(const QString& def, QWidget* p) {
    auto* e = new QLineEdit(def, p); e->setFixedWidth(120);
    return e;
}
static QLabel* resultLbl(QWidget* p) {
    auto* l = new QLabel("—", p); l->setWordWrap(true);
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

void NumberTheoryWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

long long NumberTheoryWidget::gcd(long long a, long long b) {
    return b == 0 ? a : gcd(b, a % b);
}
long long NumberTheoryWidget::modpow(long long base, long long exp, long long mod) {
    long long result = 1; base %= mod;
    while (exp > 0) {
        if (exp & 1) result = result * base % mod;
        base = base * base % mod; exp >>= 1;
    }
    return result;
}
long long NumberTheoryWidget::modInverse(long long a, long long m) {
    // Extended Euclidean
    long long g=m, x=0, y=1, ta=a;
    while (ta != 0) { long long q=g/ta; g-=q*ta; std::swap(g,ta); x-=q*y; std::swap(x,y); }
    if (g != 1) return -1;
    return (x%m+m)%m;
}

NumberTheoryWidget::NumberTheoryWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Number Theory", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildPrimeTab(), "Prime & Factors");
    tabs->addTab(buildGcdTab(),   "GCD / LCM");
    tabs->addTab(buildModTab(),   "Modular Arithmetic");
    tabs->addTab(buildBaseTab(),  "Base Converter");
    tabs->addTab(buildSeqTab(),   "Sequences");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Prime & Factorization ─────────────────────────────────────────────────────
QWidget* NumberTheoryWidget::buildPrimeTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Checks if a number is prime and finds its prime factorization using trial division. Also lists all prime factors with multiplicity.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_primeN = intEdit("360", w);
    g->addWidget(new QLabel("n =", w), 0, 0); g->addWidget(m_primeN, 0, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_primeResult = resultLbl(w); m_primeShow = new QCheckBox("Show Steps", w); m_primeSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_primeResult);
    v->addWidget(m_primeShow); v->addWidget(m_primeSteps); v->addStretch();
    connect(m_primeShow, &QCheckBox::toggled, this, [this](bool on){ m_primeSteps->setVisible(on && !m_primeSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &NumberTheoryWidget::computePrime);
    return w;
}

void NumberTheoryWidget::computePrime() {
    bool ok; long long n = m_primeN->text().toLongLong(&ok);
    if (!ok || n < 1) { m_primeResult->setText("Enter a positive integer."); return; }
    QStringList steps;
    steps << QString("n = %1").arg(n);

    // Primality
    bool isPrime = n > 1;
    if (n > 1) {
        for (long long i = 2; i * i <= n; i++) {
            if (n % i == 0) { isPrime = false; break; }
        }
    }
    steps << (isPrime ? QString("%1 is prime").arg(n) : QString("%1 is not prime").arg(n));

    // Factorization
    QStringList factors;
    long long temp = n;
    for (long long i = 2; i * i <= temp; i++) {
        int exp = 0;
        while (temp % i == 0) { temp /= i; exp++; }
        if (exp > 0) {
            factors << (exp > 1 ? QString("%1^%2").arg(i).arg(exp) : QString::number(i));
            steps << QString("Divide by %1 (%2 times)").arg(i).arg(exp);
        }
    }
    if (temp > 1) { factors << QString::number(temp); steps << QString("Remaining prime factor: %1").arg(temp); }

    QString factStr = factors.isEmpty() ? "1" : factors.join(" × ");
    steps << QString("Factorization: %1 = %2").arg(n).arg(factStr);

    // Number of divisors
    long long divCount = 1, tmp2 = n;
    for (long long i = 2; i * i <= tmp2; i++) {
        int e = 0; while (tmp2 % i == 0) { tmp2 /= i; e++; } divCount *= (e+1);
    }
    if (tmp2 > 1) divCount *= 2;

    m_primeResult->setText(QString("%1 is %2\nFactorization: %3\nNumber of divisors: %4")
        .arg(n).arg(isPrime ? "PRIME" : "COMPOSITE").arg(factStr).arg(divCount));
    showSteps(m_primeSteps, m_primeShow, steps);
}

// ── GCD / LCM ─────────────────────────────────────────────────────────────────
QWidget* NumberTheoryWidget::buildGcdTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes GCD (Greatest Common Divisor) using the Euclidean algorithm, and LCM = |a×b| / GCD(a,b).", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_gcdA = intEdit("48", w); m_gcdB = intEdit("18", w);
    g->addWidget(new QLabel("a =", w), 0, 0); g->addWidget(m_gcdA, 0, 1);
    g->addWidget(new QLabel("b =", w), 1, 0); g->addWidget(m_gcdB, 1, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_gcdResult = resultLbl(w); m_gcdShow = new QCheckBox("Show Steps", w); m_gcdSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_gcdResult);
    v->addWidget(m_gcdShow); v->addWidget(m_gcdSteps); v->addStretch();
    connect(m_gcdShow, &QCheckBox::toggled, this, [this](bool on){ m_gcdSteps->setVisible(on && !m_gcdSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &NumberTheoryWidget::computeGcd);
    return w;
}

void NumberTheoryWidget::computeGcd() {
    long long a = m_gcdA->text().toLongLong();
    long long b = m_gcdB->text().toLongLong();
    QStringList steps;
    steps << QString("Euclidean algorithm: GCD(%1, %2)").arg(a).arg(b);
    long long ta = std::abs(a), tb = std::abs(b);
    while (tb != 0) {
        long long r = ta % tb;
        steps << QString("%1 = %2 × %3 + %4").arg(ta).arg(ta/tb).arg(tb).arg(r);
        ta = tb; tb = r;
    }
    long long g = ta;
    long long lcm = (a != 0 && b != 0) ? std::abs(a / g * b) : 0;
    steps << QString("GCD = %1").arg(g);
    steps << QString("LCM = |a×b| / GCD = |%1×%2| / %3 = %4").arg(a).arg(b).arg(g).arg(lcm);
    m_gcdResult->setText(QString("GCD(%1, %2) = %3\nLCM(%1, %2) = %4").arg(a).arg(b).arg(g).arg(lcm));
    showSteps(m_gcdSteps, m_gcdShow, steps);
}

// ── Modular Arithmetic ────────────────────────────────────────────────────────
QWidget* NumberTheoryWidget::buildModTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Modular arithmetic: computes a mod m, modular inverse (a⁻¹ mod m), and fast modular exponentiation a^exp mod m.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_modA   = intEdit("7",  w); m_modM = intEdit("13", w); m_modExp = intEdit("10", w);
    g->addWidget(new QLabel("a =",   w), 0, 0); g->addWidget(m_modA,   0, 1);
    g->addWidget(new QLabel("m =",   w), 1, 0); g->addWidget(m_modM,   1, 1);
    g->addWidget(new QLabel("exp =", w), 2, 0); g->addWidget(m_modExp, 2, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_modResult = resultLbl(w); m_modShow = new QCheckBox("Show Steps", w); m_modSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_modResult);
    v->addWidget(m_modShow); v->addWidget(m_modSteps); v->addStretch();
    connect(m_modShow, &QCheckBox::toggled, this, [this](bool on){ m_modSteps->setVisible(on && !m_modSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &NumberTheoryWidget::computeMod);
    return w;
}

void NumberTheoryWidget::computeMod() {
    long long a = m_modA->text().toLongLong();
    long long m = m_modM->text().toLongLong();
    long long e = m_modExp->text().toLongLong();
    if (m <= 0) { m_modResult->setText("m must be > 0"); return; }
    QStringList steps;
    long long amod = ((a % m) + m) % m;
    steps << QString("a mod m = %1 mod %2 = %3").arg(a).arg(m).arg(amod);
    long long inv = modInverse(a, m);
    if (inv >= 0)
        steps << QString("a⁻¹ mod m = %1 (since %2 × %3 ≡ 1 mod %4)").arg(inv).arg(a).arg(inv).arg(m);
    else
        steps << QString("a⁻¹ mod m does not exist (GCD(%1,%2) ≠ 1)").arg(a).arg(m);
    long long pw = modpow(a, e, m);
    steps << QString("a^exp mod m = %1^%2 mod %3 = %4 (fast exponentiation)").arg(a).arg(e).arg(m).arg(pw);
    m_modResult->setText(QString(
        "a mod m = %1\na⁻¹ mod m = %2\na^%3 mod m = %4")
        .arg(amod).arg(inv >= 0 ? QString::number(inv) : "does not exist").arg(e).arg(pw));
    showSteps(m_modSteps, m_modShow, steps);
}

// ── Base Converter ────────────────────────────────────────────────────────────
QWidget* NumberTheoryWidget::buildBaseTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Converts a number from any base (2–36) to any other base. Enter the number as a string (e.g. 'FF' for hex, '1010' for binary).", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_baseValue = new QLineEdit("255", w);
    m_baseFrom  = new QSpinBox(w); m_baseFrom->setRange(2,36); m_baseFrom->setValue(10);
    m_baseTo    = new QSpinBox(w); m_baseTo->setRange(2,36);   m_baseTo->setValue(2);
    g->addWidget(new QLabel("Value:",      w), 0, 0); g->addWidget(m_baseValue, 0, 1);
    g->addWidget(new QLabel("From base:", w), 1, 0); g->addWidget(m_baseFrom,  1, 1);
    g->addWidget(new QLabel("To base:",   w), 2, 0); g->addWidget(m_baseTo,    2, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_baseResult = resultLbl(w);
    v->addWidget(mkBtn("Convert", w)); v->addWidget(m_baseResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &NumberTheoryWidget::computeBase);
    return w;
}

void NumberTheoryWidget::computeBase() {
    bool ok;
    long long val = m_baseValue->text().toUpper().toLongLong(&ok, m_baseFrom->value());
    if (!ok) { m_baseResult->setText("Invalid number for the given base."); return; }
    QString result = QString::number(val, m_baseTo->value()).toUpper();
    m_baseResult->setText(QString("%1 (base %2) = %3 (base %4)\nDecimal value = %5")
        .arg(m_baseValue->text().toUpper()).arg(m_baseFrom->value())
        .arg(result).arg(m_baseTo->value()).arg(val));
}

// ── Sequences ─────────────────────────────────────────────────────────────────
QWidget* NumberTheoryWidget::buildSeqTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Generates the first n terms of Fibonacci, prime numbers, triangular numbers, and perfect squares.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_seqN = intEdit("10", w);
    g->addWidget(new QLabel("n terms:", w), 0, 0); g->addWidget(m_seqN, 0, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_seqResult = resultLbl(w);
    v->addWidget(mkBtn("Generate", w)); v->addWidget(m_seqResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &NumberTheoryWidget::computeSeq);
    return w;
}

void NumberTheoryWidget::computeSeq() {
    int n = qBound(1, m_seqN->text().toInt(), 50);
    // Fibonacci
    QStringList fib; long long a=0,b=1;
    for(int i=0;i<n;i++){ fib<<QString::number(a); long long c=a+b; a=b; b=c; }
    // Primes
    QStringList primes; int num=2;
    while((int)primes.size()<n){
        bool p=true; for(int i=2;i*i<=num;i++) if(num%i==0){p=false;break;}
        if(p) primes<<QString::number(num); num++;
    }
    // Triangular
    QStringList tri;
    for(int i=1;i<=n;i++) tri<<QString::number(i*(i+1)/2);
    // Perfect squares
    QStringList sq;
    for(int i=1;i<=n;i++) sq<<QString::number(i*i);

    m_seqResult->setText(QString(
        "Fibonacci:  %1\nPrimes:     %2\nTriangular: %3\nSquares:    %4")
        .arg(fib.join(", ")).arg(primes.join(", "))
        .arg(tri.join(", ")).arg(sq.join(", ")));
}
