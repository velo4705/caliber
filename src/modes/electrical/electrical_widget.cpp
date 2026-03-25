#include "electrical_widget.h"
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

static const double PI = M_PI;

static QPushButton* mkBtn(const QString& t, QWidget* p) {
    auto* b = new QPushButton(t, p);
    b->setProperty("class", "actionButton");
    b->setMinimumHeight(36); b->setFocusPolicy(Qt::NoFocus);
    return b;
}
static QLineEdit* numEdit(const QString& def, QWidget* p) {
    auto* e = new QLineEdit(def, p);
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,p));
    e->setFixedWidth(100);
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

void ElectricalWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

ElectricalWidget::ElectricalWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Electrical Engineering", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildOhmTab(),      "Ohm's Law");
    tabs->addTab(buildResistorTab(), "Resistors");
    tabs->addTab(buildRlcTab(),      "RC/RL/RLC");
    tabs->addTab(buildVdivTab(),     "Voltage Divider");
    tabs->addTab(buildOpAmpTab(),    "Op-Amp");
    tabs->addTab(buildDbTab(),       "dB Converter");
    tabs->addTab(buildPhasorTab(),   "Phasors");
    tabs->addTab(buildPowerTab(),    "Power Factor");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Ohm's Law ─────────────────────────────────────────────────────────────────
QWidget* ElectricalWidget::buildOhmTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Solves V = IR and P = VI for any two known values. Leave the unknowns empty (or 0) and fill in any two — Caliber solves for the rest.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_ohmV = numEdit("", w); m_ohmV->setPlaceholderText("V (volts)");
    m_ohmI = numEdit("", w); m_ohmI->setPlaceholderText("I (amps)");
    m_ohmR = numEdit("", w); m_ohmR->setPlaceholderText("R (ohms)");
    m_ohmP = numEdit("", w); m_ohmP->setPlaceholderText("P (watts)");
    g->addWidget(new QLabel("Voltage V:", w),   0, 0); g->addWidget(m_ohmV, 0, 1);
    g->addWidget(new QLabel("Current I:", w),   1, 0); g->addWidget(m_ohmI, 1, 1);
    g->addWidget(new QLabel("Resistance R:", w),2, 0); g->addWidget(m_ohmR, 2, 1);
    g->addWidget(new QLabel("Power P:", w),     3, 0); g->addWidget(m_ohmP, 3, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_ohmResult = resultLbl(w); m_ohmShow = new QCheckBox("Show Steps", w); m_ohmSteps = stepsBox(w);
    v->addWidget(mkBtn("Solve", w)); v->addWidget(m_ohmResult);
    v->addWidget(m_ohmShow); v->addWidget(m_ohmSteps); v->addStretch();
    connect(m_ohmShow, &QCheckBox::toggled, this, [this](bool on){ m_ohmSteps->setVisible(on && !m_ohmSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &ElectricalWidget::computeOhm);
    return w;
}

void ElectricalWidget::computeOhm() {
    double V = m_ohmV->text().toDouble();
    double I = m_ohmI->text().toDouble();
    double R = m_ohmR->text().toDouble();
    double P = m_ohmP->text().toDouble();
    bool hV = !m_ohmV->text().isEmpty() && V != 0;
    bool hI = !m_ohmI->text().isEmpty() && I != 0;
    bool hR = !m_ohmR->text().isEmpty() && R != 0;
    bool hP = !m_ohmP->text().isEmpty() && P != 0;
    QStringList steps;

    if (hV && hI) { R = V/I; P = V*I; steps << "V and I known"; steps << QString("R = V/I = %1/%2 = %3 Ω").arg(V).arg(I).arg(R,0,'g',6); steps << QString("P = V×I = %1×%2 = %3 W").arg(V).arg(I).arg(P,0,'g',6); }
    else if (hV && hR) { I = V/R; P = V*V/R; steps << "V and R known"; steps << QString("I = V/R = %1/%2 = %3 A").arg(V).arg(R).arg(I,0,'g',6); steps << QString("P = V²/R = %1").arg(P,0,'g',6); }
    else if (hI && hR) { V = I*R; P = I*I*R; steps << "I and R known"; steps << QString("V = I×R = %1×%2 = %3 V").arg(I).arg(R).arg(V,0,'g',6); steps << QString("P = I²×R = %1").arg(P,0,'g',6); }
    else if (hV && hP) { I = P/V; R = V*V/P; steps << "V and P known"; steps << QString("I = P/V = %1/%2 = %3 A").arg(P).arg(V).arg(I,0,'g',6); steps << QString("R = V²/P = %1").arg(R,0,'g',6); }
    else if (hI && hP) { V = P/I; R = P/(I*I); steps << "I and P known"; steps << QString("V = P/I = %1/%2 = %3 V").arg(P).arg(I).arg(V,0,'g',6); steps << QString("R = P/I² = %1").arg(R,0,'g',6); }
    else if (hR && hP) { V = std::sqrt(P*R); I = std::sqrt(P/R); steps << "R and P known"; steps << QString("V = √(P×R) = %1 V").arg(V,0,'g',6); steps << QString("I = √(P/R) = %1 A").arg(I,0,'g',6); }
    else { m_ohmResult->setText("Enter any two values."); return; }

    m_ohmResult->setText(QString("V = %1 V\nI = %2 A\nR = %3 Ω\nP = %4 W").arg(V,0,'g',6).arg(I,0,'g',6).arg(R,0,'g',6).arg(P,0,'g',6));
    showSteps(m_ohmSteps, m_ohmShow, steps);
}

// ── Resistors ─────────────────────────────────────────────────────────────────
QWidget* ElectricalWidget::buildResistorTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes equivalent resistance for resistors in series (R = R1+R2+...) or parallel (1/R = 1/R1+1/R2+...). Enter values comma-separated.", w));
    auto* row = new QHBoxLayout();
    m_resMode = new QComboBox(w); m_resMode->addItems({"Series", "Parallel"});
    row->addWidget(new QLabel("Mode:", w)); row->addWidget(m_resMode); row->addStretch();
    v->addLayout(row);
    v->addWidget(new QLabel("Resistor values (Ω), comma-separated:", w));
    m_resValues = new QTextEdit(w); m_resValues->setPlaceholderText("e.g. 100, 220, 470");
    m_resValues->setMaximumHeight(60);
    v->addWidget(m_resValues);
    m_resResult = resultLbl(w); m_resShow = new QCheckBox("Show Steps", w); m_resSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_resResult);
    v->addWidget(m_resShow); v->addWidget(m_resSteps); v->addStretch();
    connect(m_resShow, &QCheckBox::toggled, this, [this](bool on){ m_resSteps->setVisible(on && !m_resSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &ElectricalWidget::computeResistor);
    return w;
}

void ElectricalWidget::computeResistor() {
    QVector<double> rs;
    for (const QString& p : m_resValues->toPlainText().split(',', Qt::SkipEmptyParts)) {
        bool ok; double v = p.trimmed().toDouble(&ok); if (ok && v > 0) rs.append(v);
    }
    if (rs.isEmpty()) { m_resResult->setText("Enter resistor values."); return; }
    QStringList steps;
    double req;
    if (m_resMode->currentIndex() == 0) {
        steps << "Series: R_eq = R1 + R2 + ...";
        req = 0; for (double r : rs) { req += r; steps << QString("+ %1 Ω → running total = %2 Ω").arg(r).arg(req,0,'g',6); }
    } else {
        steps << "Parallel: 1/R_eq = 1/R1 + 1/R2 + ...";
        double inv = 0; for (double r : rs) { inv += 1.0/r; steps << QString("+ 1/%1 → running 1/R = %2").arg(r).arg(inv,0,'g',6); }
        req = 1.0/inv;
        steps << QString("R_eq = 1/%1 = %2 Ω").arg(inv,0,'g',6).arg(req,0,'g',6);
    }
    m_resResult->setText(QString("R_eq = %1 Ω").arg(req,0,'g',8));
    showSteps(m_resSteps, m_resShow, steps);
}

// ── RC / RL / RLC ─────────────────────────────────────────────────────────────
QWidget* ElectricalWidget::buildRlcTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes RC/RL time constants, RLC resonant frequency, impedance, and Q factor. Leave unused components as 0.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_rlcR = numEdit("100",  w); m_rlcL = numEdit("0.01", w);
    m_rlcC = numEdit("1e-6", w); m_rlcF = numEdit("1000", w);
    g->addWidget(new QLabel("R (Ω):",  w), 0, 0); g->addWidget(m_rlcR, 0, 1);
    g->addWidget(new QLabel("L (H):",  w), 1, 0); g->addWidget(m_rlcL, 1, 1);
    g->addWidget(new QLabel("C (F):",  w), 2, 0); g->addWidget(m_rlcC, 2, 1);
    g->addWidget(new QLabel("f (Hz):", w), 3, 0); g->addWidget(m_rlcF, 3, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_rlcResult = resultLbl(w); m_rlcShow = new QCheckBox("Show Steps", w); m_rlcSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_rlcResult);
    v->addWidget(m_rlcShow); v->addWidget(m_rlcSteps); v->addStretch();
    connect(m_rlcShow, &QCheckBox::toggled, this, [this](bool on){ m_rlcSteps->setVisible(on && !m_rlcSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &ElectricalWidget::computeRlc);
    return w;
}

void ElectricalWidget::computeRlc() {
    double R = m_rlcR->text().toDouble();
    double L = m_rlcL->text().toDouble();
    double C = m_rlcC->text().toDouble();
    double f = m_rlcF->text().toDouble();
    double w = 2 * PI * f;
    QStringList steps;
    steps << QString("R=%1Ω, L=%2H, C=%3F, f=%4Hz, ω=2πf=%5 rad/s").arg(R).arg(L).arg(C).arg(f).arg(w,0,'g',6);
    QString out;
    if (R > 0 && C > 0) {
        double tau_rc = R * C;
        double fc_rc  = 1.0 / (2*PI*R*C);
        steps << QString("RC time constant τ = R×C = %1 s").arg(tau_rc,0,'g',6);
        steps << QString("RC cutoff frequency fc = 1/(2πRC) = %1 Hz").arg(fc_rc,0,'g',6);
        out += QString("RC τ = %1 s,  fc = %2 Hz\n").arg(tau_rc,0,'g',6).arg(fc_rc,0,'g',6);
    }
    if (R > 0 && L > 0) {
        double tau_rl = L / R;
        double fc_rl  = R / (2*PI*L);
        steps << QString("RL time constant τ = L/R = %1 s").arg(tau_rl,0,'g',6);
        steps << QString("RL cutoff frequency fc = R/(2πL) = %1 Hz").arg(fc_rl,0,'g',6);
        out += QString("RL τ = %1 s,  fc = %2 Hz\n").arg(tau_rl,0,'g',6).arg(fc_rl,0,'g',6);
    }
    if (L > 0 && C > 0) {
        double f0 = 1.0 / (2*PI*std::sqrt(L*C));
        double Q  = R > 0 ? (1.0/R)*std::sqrt(L/C) : 0;
        double XL = w * L, XC = C > 0 ? 1.0/(w*C) : 0;
        double Z  = std::sqrt(R*R + (XL-XC)*(XL-XC));
        steps << QString("Resonant frequency f₀ = 1/(2π√LC) = %1 Hz").arg(f0,0,'g',6);
        steps << QString("Q factor = (1/R)√(L/C) = %1").arg(Q,0,'g',6);
        steps << QString("XL = ωL = %1 Ω,  XC = 1/ωC = %2 Ω").arg(XL,0,'g',6).arg(XC,0,'g',6);
        steps << QString("Impedance Z = √(R²+(XL−XC)²) = %1 Ω").arg(Z,0,'g',6);
        out += QString("f₀ = %1 Hz,  Q = %2\nXL = %3 Ω,  XC = %4 Ω\nZ = %5 Ω")
            .arg(f0,0,'g',6).arg(Q,0,'g',4).arg(XL,0,'g',6).arg(XC,0,'g',6).arg(Z,0,'g',6);
    }
    if (out.isEmpty()) out = "Enter R, L, C values.";
    m_rlcResult->setText(out.trimmed());
    showSteps(m_rlcSteps, m_rlcShow, steps);
}

// ── Voltage Divider ───────────────────────────────────────────────────────────
QWidget* ElectricalWidget::buildVdivTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes output voltage of a resistive voltage divider: Vout = Vin × R2/(R1+R2). Also shows current and power dissipation.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_vdivVin = numEdit("12",  w); m_vdivR1 = numEdit("1000", w); m_vdivR2 = numEdit("2200", w);
    g->addWidget(new QLabel("Vin (V):", w), 0, 0); g->addWidget(m_vdivVin, 0, 1);
    g->addWidget(new QLabel("R1 (Ω):", w), 1, 0); g->addWidget(m_vdivR1,  1, 1);
    g->addWidget(new QLabel("R2 (Ω):", w), 2, 0); g->addWidget(m_vdivR2,  2, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_vdivResult = resultLbl(w); m_vdivShow = new QCheckBox("Show Steps", w); m_vdivSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_vdivResult);
    v->addWidget(m_vdivShow); v->addWidget(m_vdivSteps); v->addStretch();
    connect(m_vdivShow, &QCheckBox::toggled, this, [this](bool on){ m_vdivSteps->setVisible(on && !m_vdivSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &ElectricalWidget::computeVdiv);
    return w;
}

void ElectricalWidget::computeVdiv() {
    double Vin = m_vdivVin->text().toDouble();
    double R1  = m_vdivR1->text().toDouble();
    double R2  = m_vdivR2->text().toDouble();
    if (R1+R2 == 0) { m_vdivResult->setText("R1+R2 must be > 0"); return; }
    QStringList steps;
    steps << QString("Vin=%1V, R1=%2Ω, R2=%3Ω").arg(Vin).arg(R1).arg(R2);
    double Vout = Vin * R2 / (R1+R2);
    double I    = Vin / (R1+R2);
    double P    = Vin * I;
    steps << QString("Vout = Vin × R2/(R1+R2) = %1 × %2/%3 = %4 V").arg(Vin).arg(R2).arg(R1+R2).arg(Vout,0,'g',6);
    steps << QString("Current I = Vin/(R1+R2) = %1 A").arg(I,0,'g',6);
    steps << QString("Total power P = Vin×I = %1 W").arg(P,0,'g',6);
    m_vdivResult->setText(QString("Vout = %1 V\nI = %2 A\nP = %3 W").arg(Vout,0,'g',6).arg(I,0,'g',6).arg(P,0,'g',6));
    showSteps(m_vdivSteps, m_vdivShow, steps);
}

// ── Op-Amp ────────────────────────────────────────────────────────────────────
QWidget* ElectricalWidget::buildOpAmpTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes gain and output voltage for common op-amp configurations. Inverting: Vout = −(Rf/Rin)×Vin. Non-inverting: Vout = (1+Rf/R1)×Vin.", w));
    auto* row = new QHBoxLayout();
    m_opAmpType = new QComboBox(w);
    m_opAmpType->addItems({"Inverting", "Non-Inverting", "Voltage Follower", "Summing (2 inputs)"});
    row->addWidget(new QLabel("Config:", w)); row->addWidget(m_opAmpType); row->addStretch();
    v->addLayout(row);
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_opAmpR1  = numEdit("1000", w); m_opAmpR2 = numEdit("10000", w); m_opAmpVin = numEdit("1", w);
    g->addWidget(new QLabel("Rin / R1 (Ω):", w), 0, 0); g->addWidget(m_opAmpR1,  0, 1);
    g->addWidget(new QLabel("Rf / R2 (Ω):",  w), 1, 0); g->addWidget(m_opAmpR2,  1, 1);
    g->addWidget(new QLabel("Vin (V):",       w), 2, 0); g->addWidget(m_opAmpVin, 2, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_opAmpResult = resultLbl(w); m_opAmpShow = new QCheckBox("Show Steps", w); m_opAmpSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_opAmpResult);
    v->addWidget(m_opAmpShow); v->addWidget(m_opAmpSteps); v->addStretch();
    connect(m_opAmpShow, &QCheckBox::toggled, this, [this](bool on){ m_opAmpSteps->setVisible(on && !m_opAmpSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &ElectricalWidget::computeOpAmp);
    return w;
}

void ElectricalWidget::computeOpAmp() {
    double R1 = m_opAmpR1->text().toDouble();
    double R2 = m_opAmpR2->text().toDouble();
    double Vin= m_opAmpVin->text().toDouble();
    QStringList steps;
    int idx = m_opAmpType->currentIndex();
    double gain, Vout;
    if (idx == 0) { // Inverting
        gain = -(R2/R1); Vout = gain*Vin;
        steps << "Inverting amplifier: Gain = −Rf/Rin";
        steps << QString("Gain = −%1/%2 = %3").arg(R2).arg(R1).arg(gain,0,'g',6);
    } else if (idx == 1) { // Non-inverting
        gain = 1 + R2/R1; Vout = gain*Vin;
        steps << "Non-inverting amplifier: Gain = 1 + Rf/R1";
        steps << QString("Gain = 1 + %1/%2 = %3").arg(R2).arg(R1).arg(gain,0,'g',6);
    } else if (idx == 2) { // Voltage follower
        gain = 1; Vout = Vin;
        steps << "Voltage follower: Gain = 1, Vout = Vin";
    } else { // Summing (2 inputs, equal Rin)
        gain = -(R2/R1); Vout = gain * 2 * Vin;
        steps << "Summing amplifier (2 equal inputs): Vout = −(Rf/Rin)×(V1+V2)";
        steps << QString("Gain per input = −%1/%2 = %3").arg(R2).arg(R1).arg(gain,0,'g',6);
        steps << QString("Vout = %1 × 2×%2 = %3 V").arg(gain,0,'g',4).arg(Vin).arg(Vout,0,'g',6);
    }
    steps << QString("Vout = Gain × Vin = %1 × %2 = %3 V").arg(gain,0,'g',6).arg(Vin).arg(Vout,0,'g',6);
    m_opAmpResult->setText(QString("Gain = %1\nVout = %2 V").arg(gain,0,'g',6).arg(Vout,0,'g',6));
    showSteps(m_opAmpSteps, m_opAmpShow, steps);
}

// ── dB Converter ──────────────────────────────────────────────────────────────
QWidget* ElectricalWidget::buildDbTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Converts between dB and linear ratios. Voltage gain: dB = 20×log10(Vout/Vin). Power gain: dB = 10×log10(Pout/Pin). Also converts dBm ↔ watts.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_dbValue = numEdit("6", w);
    m_dbMode  = new QComboBox(w);
    m_dbMode->addItems({"dB → Voltage ratio", "Voltage ratio → dB", "dB → Power ratio", "Power ratio → dB", "dBm → Watts", "Watts → dBm"});
    g->addWidget(new QLabel("Value:", w), 0, 0); g->addWidget(m_dbValue, 0, 1);
    g->addWidget(new QLabel("Mode:",  w), 1, 0); g->addWidget(m_dbMode,  1, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_dbResult = resultLbl(w);
    v->addWidget(mkBtn("Convert", w)); v->addWidget(m_dbResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &ElectricalWidget::computeDb);
    return w;
}

void ElectricalWidget::computeDb() {
    double val = m_dbValue->text().toDouble();
    int idx = m_dbMode->currentIndex();
    double result;
    QString label;
    switch (idx) {
        case 0: result = std::pow(10, val/20.0); label = QString("%1 dB → voltage ratio = %2").arg(val).arg(result,0,'g',6); break;
        case 1: result = 20*std::log10(val);     label = QString("voltage ratio %1 → %2 dB").arg(val).arg(result,0,'g',6); break;
        case 2: result = std::pow(10, val/10.0); label = QString("%1 dB → power ratio = %2").arg(val).arg(result,0,'g',6); break;
        case 3: result = 10*std::log10(val);     label = QString("power ratio %1 → %2 dB").arg(val).arg(result,0,'g',6); break;
        case 4: result = std::pow(10,(val-30)/10.0); label = QString("%1 dBm → %2 W").arg(val).arg(result,0,'g',6); break;
        case 5: result = 10*std::log10(val)+30;  label = QString("%1 W → %2 dBm").arg(val).arg(result,0,'g',6); break;
        default: return;
    }
    m_dbResult->setText(label);
}

// ── Phasor Calculator ─────────────────────────────────────────────────────────
QWidget* ElectricalWidget::buildPhasorTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Adds or subtracts two phasors in polar form (magnitude ∠ angle°). Converts to rectangular, performs the operation, then converts back to polar.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_ph1Mag = numEdit("5",  w); m_ph1Ang = numEdit("30",  w);
    m_ph2Mag = numEdit("3",  w); m_ph2Ang = numEdit("60",  w);
    m_phOp   = new QComboBox(w); m_phOp->addItems({"Add", "Subtract", "Multiply", "Divide"});
    g->addWidget(new QLabel("Phasor 1 — Mag:", w), 0, 0); g->addWidget(m_ph1Mag, 0, 1);
    g->addWidget(new QLabel("Angle (°):",       w), 0, 2); g->addWidget(m_ph1Ang, 0, 3);
    g->addWidget(new QLabel("Phasor 2 — Mag:", w), 1, 0); g->addWidget(m_ph2Mag, 1, 1);
    g->addWidget(new QLabel("Angle (°):",       w), 1, 2); g->addWidget(m_ph2Ang, 1, 3);
    g->addWidget(new QLabel("Operation:",       w), 2, 0); g->addWidget(m_phOp,   2, 1);
    g->setColumnStretch(4, 1);
    v->addLayout(g);
    m_phResult = resultLbl(w); m_phShow = new QCheckBox("Show Steps", w); m_phSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_phResult);
    v->addWidget(m_phShow); v->addWidget(m_phSteps); v->addStretch();
    connect(m_phShow, &QCheckBox::toggled, this, [this](bool on){ m_phSteps->setVisible(on && !m_phSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &ElectricalWidget::computePhasor);
    return w;
}

void ElectricalWidget::computePhasor() {
    double m1 = m_ph1Mag->text().toDouble(), a1 = m_ph1Ang->text().toDouble() * PI/180;
    double m2 = m_ph2Mag->text().toDouble(), a2 = m_ph2Ang->text().toDouble() * PI/180;
    double re1=m1*std::cos(a1), im1=m1*std::sin(a1);
    double re2=m2*std::cos(a2), im2=m2*std::sin(a2);
    QStringList steps;
    steps << QString("P1 = %1∠%2° = %3 + %4j").arg(m1).arg(m_ph1Ang->text()).arg(re1,0,'g',6).arg(im1,0,'g',6);
    steps << QString("P2 = %1∠%2° = %3 + %4j").arg(m2).arg(m_ph2Ang->text()).arg(re2,0,'g',6).arg(im2,0,'g',6);
    double re, im;
    int idx = m_phOp->currentIndex();
    if (idx == 0) { re=re1+re2; im=im1+im2; steps << "Add rectangular components"; }
    else if (idx == 1) { re=re1-re2; im=im1-im2; steps << "Subtract rectangular components"; }
    else if (idx == 2) { // multiply in polar
        double rm=m1*m2, ra=(a1+a2)*180/PI;
        steps << QString("Multiply: mag=%1×%2=%3, angle=%4°+%5°=%6°").arg(m1).arg(m2).arg(rm,0,'g',6).arg(m_ph1Ang->text()).arg(m_ph2Ang->text()).arg(ra,0,'g',6);
        m_phResult->setText(QString("Result = %1 ∠ %2°").arg(rm,0,'g',6).arg(ra,0,'g',6));
        showSteps(m_phSteps, m_phShow, steps); return;
    } else { // divide in polar
        if (m2 == 0) { m_phResult->setText("Cannot divide by zero phasor."); return; }
        double rm=m1/m2, ra=(a1-a2)*180/PI;
        steps << QString("Divide: mag=%1/%2=%3, angle=%4°−%5°=%6°").arg(m1).arg(m2).arg(rm,0,'g',6).arg(m_ph1Ang->text()).arg(m_ph2Ang->text()).arg(ra,0,'g',6);
        m_phResult->setText(QString("Result = %1 ∠ %2°").arg(rm,0,'g',6).arg(ra,0,'g',6));
        showSteps(m_phSteps, m_phShow, steps); return;
    }
    double mag = std::sqrt(re*re+im*im);
    double ang = std::atan2(im,re)*180/PI;
    steps << QString("Result rectangular: %1 + %2j").arg(re,0,'g',6).arg(im,0,'g',6);
    steps << QString("Convert to polar: mag=%1, angle=%2°").arg(mag,0,'g',6).arg(ang,0,'g',6);
    m_phResult->setText(QString("Result = %1 ∠ %2°\n= %3 + %4j").arg(mag,0,'g',6).arg(ang,0,'g',6).arg(re,0,'g',6).arg(im,0,'g',6));
    showSteps(m_phSteps, m_phShow, steps);
}

// ── Power Factor ──────────────────────────────────────────────────────────────
QWidget* ElectricalWidget::buildPowerTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Solves the power triangle: Real power P (W), Reactive power Q (VAR), Apparent power S (VA), and Power Factor PF = P/S = cos(φ). Enter any two known values.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_pwrP  = numEdit("", w); m_pwrP->setPlaceholderText("P (W)");
    m_pwrQ  = numEdit("", w); m_pwrQ->setPlaceholderText("Q (VAR)");
    m_pwrS  = numEdit("", w); m_pwrS->setPlaceholderText("S (VA)");
    m_pwrPf = numEdit("", w); m_pwrPf->setPlaceholderText("PF (0–1)");
    g->addWidget(new QLabel("Real P (W):",      w), 0, 0); g->addWidget(m_pwrP,  0, 1);
    g->addWidget(new QLabel("Reactive Q (VAR):",w), 1, 0); g->addWidget(m_pwrQ,  1, 1);
    g->addWidget(new QLabel("Apparent S (VA):", w), 2, 0); g->addWidget(m_pwrS,  2, 1);
    g->addWidget(new QLabel("Power Factor PF:", w), 3, 0); g->addWidget(m_pwrPf, 3, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_pwrResult = resultLbl(w); m_pwrShow = new QCheckBox("Show Steps", w); m_pwrSteps = stepsBox(w);
    v->addWidget(mkBtn("Solve", w)); v->addWidget(m_pwrResult);
    v->addWidget(m_pwrShow); v->addWidget(m_pwrSteps); v->addStretch();
    connect(m_pwrShow, &QCheckBox::toggled, this, [this](bool on){ m_pwrSteps->setVisible(on && !m_pwrSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &ElectricalWidget::computePower);
    return w;
}

void ElectricalWidget::computePower() {
    bool hP  = !m_pwrP->text().isEmpty();
    bool hQ  = !m_pwrQ->text().isEmpty();
    bool hS  = !m_pwrS->text().isEmpty();
    bool hPf = !m_pwrPf->text().isEmpty();
    double P  = m_pwrP->text().toDouble();
    double Q  = m_pwrQ->text().toDouble();
    double S  = m_pwrS->text().toDouble();
    double PF = m_pwrPf->text().toDouble();
    QStringList steps;
    steps << "Power triangle: S² = P² + Q²,  PF = P/S = cos(φ)";

    // Solve for all four from any two
    if (hP && hQ) {
        S  = std::sqrt(P*P + Q*Q);
        PF = S > 0 ? P/S : 0;
        steps << QString("S = √(P²+Q²) = √(%1²+%2²) = %3 VA").arg(P).arg(Q).arg(S,0,'g',6);
        steps << QString("PF = P/S = %1/%2 = %3").arg(P).arg(S,0,'g',6).arg(PF,0,'g',6);
    } else if (hP && hS) {
        Q  = std::sqrt(qMax(0.0, S*S - P*P));
        PF = S > 0 ? P/S : 0;
        steps << QString("Q = √(S²−P²) = %1 VAR").arg(Q,0,'g',6);
        steps << QString("PF = P/S = %1").arg(PF,0,'g',6);
    } else if (hQ && hS) {
        P  = std::sqrt(qMax(0.0, S*S - Q*Q));
        PF = S > 0 ? P/S : 0;
        steps << QString("P = √(S²−Q²) = %1 W").arg(P,0,'g',6);
        steps << QString("PF = P/S = %1").arg(PF,0,'g',6);
    } else if (hP && hPf && PF > 0) {
        S  = P / PF;
        Q  = std::sqrt(qMax(0.0, S*S - P*P));
        steps << QString("S = P/PF = %1/%2 = %3 VA").arg(P).arg(PF).arg(S,0,'g',6);
        steps << QString("Q = √(S²−P²) = %1 VAR").arg(Q,0,'g',6);
    } else if (hS && hPf) {
        P  = S * PF;
        Q  = std::sqrt(qMax(0.0, S*S - P*P));
        steps << QString("P = S×PF = %1×%2 = %3 W").arg(S).arg(PF).arg(P,0,'g',6);
        steps << QString("Q = √(S²−P²) = %1 VAR").arg(Q,0,'g',6);
    } else {
        m_pwrResult->setText("Enter any two values.");
        return;
    }

    double phi = std::acos(qBound(-1.0, PF, 1.0)) * 180.0 / PI;
    steps << QString("Phase angle φ = arccos(PF) = %1°").arg(phi,0,'f',2);
    steps << (PF >= 0.95 ? "PF ≥ 0.95 — good power factor" :
              PF >= 0.80 ? "PF 0.80–0.95 — acceptable" : "PF < 0.80 — poor, consider correction");

    m_pwrResult->setText(QString(
        "P = %1 W\nQ = %2 VAR\nS = %3 VA\nPF = %4\nφ = %5°\n%6")
        .arg(P,0,'g',6).arg(Q,0,'g',6).arg(S,0,'g',6).arg(PF,0,'g',4).arg(phi,0,'f',2)
        .arg(PF >= 0.95 ? "Good power factor" : PF >= 0.80 ? "Acceptable power factor" : "Poor power factor"));
    showSteps(m_pwrSteps, m_pwrShow, steps);
}
