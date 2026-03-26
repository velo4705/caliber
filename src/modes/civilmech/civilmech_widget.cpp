#include "civilmech_widget.h"
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
    auto* b = new QPushButton(t, p); b->setProperty("class","actionButton");
    b->setMinimumHeight(36); b->setFocusPolicy(Qt::NoFocus); return b;
}
static QLineEdit* numEdit(const QString& def, const QString& ph, QWidget* p) {
    auto* e = new QLineEdit(def); e->setPlaceholderText(ph);
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,e));
    e->setFixedWidth(110); return e;
}
static QLabel* resultLbl(QWidget* p) {
    auto* l = new QLabel("—", p); l->setWordWrap(true);
    l->setStyleSheet("font-size:13px;font-weight:bold;padding:8px;"); return l;
}
static QTextEdit* stepsBox(QWidget* p) {
    auto* t = new QTextEdit(p); t->setReadOnly(true);
    t->setMinimumHeight(100); t->setMaximumHeight(180);
    t->setStyleSheet("font-family:monospace;font-size:12px;"); t->hide(); return t;
}
static QLabel* descLbl(const QString& text, QWidget* p) {
    auto* l = new QLabel(text, p); l->setWordWrap(true);
    l->setStyleSheet("color:gray;font-size:12px;padding:4px 0;"); return l;
}

void CivilMechWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

CivilMechWidget::CivilMechWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Civil / Mechanical Engineering", this);
    title->setStyleSheet("font-size:18px;font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildBeamTab(),   "Beam Calculator");
    tabs->addTab(buildStressTab(), "Stress & Strain");
    tabs->addTab(buildFluidTab(),  "Fluid Mechanics");
    tabs->addTab(buildHeatTab(),   "Heat Transfer");
    tabs->addTab(buildGearTab(),   "Gear & Pulley");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Beam Calculator ───────────────────────────────────────────────────────────
QWidget* CivilMechWidget::buildBeamTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Simply supported beam analysis. Computes reactions at supports, maximum bending moment, and maximum shear force for UDL (uniform distributed load) or point load at centre.", w));
    auto* row = new QHBoxLayout();
    m_bLoad = new QComboBox(w);
    m_bLoad->addItems({"UDL (uniform distributed load)", "Point load at centre", "Point load at distance a"});
    row->addWidget(new QLabel("Load type:", w)); row->addWidget(m_bLoad); row->addStretch();
    v->addLayout(row);
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_bL = numEdit("5","L (m)",w); m_bW = numEdit("10","w (kN/m) or P (kN)",w);
    m_bA = numEdit("2","a (m) — point load pos",w);
    g->addWidget(new QLabel("Span L (m):",w),0,0); g->addWidget(m_bL,0,1);
    g->addWidget(new QLabel("Load w/P:",w),1,0); g->addWidget(m_bW,1,1);
    g->addWidget(new QLabel("Position a (m):",w),2,0); g->addWidget(m_bA,2,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_bResult=resultLbl(w); m_bShow=new QCheckBox("Show Steps",w); m_bSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_bResult);
    v->addWidget(m_bShow); v->addWidget(m_bSteps); v->addStretch();
    connect(m_bShow,&QCheckBox::toggled,this,[this](bool on){m_bSteps->setVisible(on&&!m_bSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&CivilMechWidget::computeBeam);
    return w;
}

void CivilMechWidget::computeBeam() {
    double L=m_bL->text().toDouble(), w=m_bW->text().toDouble(), a=m_bA->text().toDouble();
    int idx=m_bLoad->currentIndex();
    QStringList steps;
    double Ra,Rb,Mmax,Vmax;
    if (idx==0) { // UDL
        Ra=Rb=w*L/2; Mmax=w*L*L/8; Vmax=Ra;
        steps << QString("UDL: w=%1 kN/m, L=%2 m").arg(w).arg(L);
        steps << QString("Ra = Rb = wL/2 = %1 kN").arg(Ra,0,'g',6);
        steps << QString("Mmax = wL²/8 = %1 kN·m (at midspan)").arg(Mmax,0,'g',6);
        steps << QString("Vmax = Ra = %1 kN").arg(Vmax,0,'g',6);
    } else if (idx==1) { // Point load at centre
        Ra=Rb=w/2; Mmax=w*L/4; Vmax=Ra;
        steps << QString("Point load P=%1 kN at centre, L=%2 m").arg(w).arg(L);
        steps << QString("Ra = Rb = P/2 = %1 kN").arg(Ra,0,'g',6);
        steps << QString("Mmax = PL/4 = %1 kN·m").arg(Mmax,0,'g',6);
    } else { // Point load at a
        double b=L-a;
        Ra=w*b/L; Rb=w*a/L; Mmax=Ra*a; Vmax=qMax(Ra,Rb);
        steps << QString("Point load P=%1 kN at a=%2 m, b=%3 m").arg(w).arg(a).arg(b);
        steps << QString("Ra = Pb/L = %1 kN").arg(Ra,0,'g',6);
        steps << QString("Rb = Pa/L = %1 kN").arg(Rb,0,'g',6);
        steps << QString("Mmax = Ra×a = %1 kN·m").arg(Mmax,0,'g',6);
    }
    m_bResult->setText(QString("Ra = %1 kN\nRb = %2 kN\nMmax = %3 kN·m\nVmax = %4 kN")
        .arg(Ra,0,'g',8).arg(Rb,0,'g',8).arg(Mmax,0,'g',8).arg(Vmax,0,'g',8));
    showSteps(m_bSteps,m_bShow,steps);
}

// ── Stress & Strain ───────────────────────────────────────────────────────────
QWidget* CivilMechWidget::buildStressTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Normal stress σ=F/A, strain ε=ΔL/L, Young's modulus E=σ/ε, deformation ΔL=FL/AE. Factor of safety FOS=σ_yield/σ_applied.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_ssF=numEdit("10000","F (N)",w); m_ssA=numEdit("0.001","A (m²)",w);
    m_ssE=numEdit("200e9","E (Pa)",w); m_ssDL=numEdit("","ΔL (m)",w);
    m_ssL=numEdit("1","L (m)",w); m_ssFOS=numEdit("250e6","σ_yield (Pa)",w);
    g->addWidget(new QLabel("Force F (N):",w),0,0); g->addWidget(m_ssF,0,1);
    g->addWidget(new QLabel("Area A (m²):",w),1,0); g->addWidget(m_ssA,1,1);
    g->addWidget(new QLabel("E (Pa):",w),2,0); g->addWidget(m_ssE,2,1);
    g->addWidget(new QLabel("Length L (m):",w),3,0); g->addWidget(m_ssL,3,1);
    g->addWidget(new QLabel("σ_yield (Pa):",w),4,0); g->addWidget(m_ssFOS,4,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_ssResult=resultLbl(w); m_ssShow=new QCheckBox("Show Steps",w); m_ssSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_ssResult);
    v->addWidget(m_ssShow); v->addWidget(m_ssSteps); v->addStretch();
    connect(m_ssShow,&QCheckBox::toggled,this,[this](bool on){m_ssSteps->setVisible(on&&!m_ssSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&CivilMechWidget::computeStress);
    return w;
}

void CivilMechWidget::computeStress() {
    double F=m_ssF->text().toDouble(), A=m_ssA->text().toDouble();
    double E=m_ssE->text().toDouble(), L=m_ssL->text().toDouble();
    double syield=m_ssFOS->text().toDouble();
    QStringList steps;
    double sigma=F/A, dL=F*L/(A*E), eps=dL/L, FOS=syield/sigma;
    steps << QString("σ = F/A = %1/%2 = %3 Pa").arg(F).arg(A).arg(sigma,0,'g',8);
    steps << QString("ΔL = FL/AE = %1×%2/(%3×%4) = %5 m").arg(F).arg(L).arg(A).arg(E).arg(dL,0,'g',8);
    steps << QString("ε = ΔL/L = %1").arg(eps,0,'g',6);
    steps << QString("FOS = σ_yield/σ = %1/%2 = %3").arg(syield,0,'g',6).arg(sigma,0,'g',6).arg(FOS,0,'g',4);
    m_ssResult->setText(QString("σ = %1 Pa\nε = %2\nΔL = %3 m\nFOS = %4")
        .arg(sigma,0,'g',8).arg(eps,0,'g',6).arg(dL,0,'g',8).arg(FOS,0,'g',4));
    showSteps(m_ssSteps,m_ssShow,steps);
}

// ── Fluid Mechanics ───────────────────────────────────────────────────────────
QWidget* CivilMechWidget::buildFluidTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Bernoulli's equation: P₁+½ρv₁²+ρgh₁ = P₂+½ρv₂²+ρgh₂. Continuity: A₁v₁=A₂v₂. Reynolds number: Re=ρvD/μ.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_flV1=numEdit("2","v₁ (m/s)",w); m_flA1=numEdit("0.01","A₁ (m²)",w);
    m_flP1=numEdit("101325","P₁ (Pa)",w); m_flH=numEdit("0","h₁−h₂ (m)",w);
    m_flRho=numEdit("1000","ρ (kg/m³)",w); m_flA2=numEdit("0.005","A₂ (m²)",w);
    m_flD=numEdit("0.1","D (m)",w); m_flMu=numEdit("0.001","μ (Pa·s)",w);
    g->addWidget(new QLabel("v₁ (m/s):",w),0,0); g->addWidget(m_flV1,0,1);
    g->addWidget(new QLabel("A₁ (m²):",w),1,0); g->addWidget(m_flA1,1,1);
    g->addWidget(new QLabel("P₁ (Pa):",w),2,0); g->addWidget(m_flP1,2,1);
    g->addWidget(new QLabel("h₁−h₂ (m):",w),3,0); g->addWidget(m_flH,3,1);
    g->addWidget(new QLabel("ρ (kg/m³):",w),4,0); g->addWidget(m_flRho,4,1);
    g->addWidget(new QLabel("A₂ (m²):",w),5,0); g->addWidget(m_flA2,5,1);
    g->addWidget(new QLabel("D (m):",w),6,0); g->addWidget(m_flD,6,1);
    g->addWidget(new QLabel("μ (Pa·s):",w),7,0); g->addWidget(m_flMu,7,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_flResult=resultLbl(w); m_flShow=new QCheckBox("Show Steps",w); m_flSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_flResult);
    v->addWidget(m_flShow); v->addWidget(m_flSteps); v->addStretch();
    connect(m_flShow,&QCheckBox::toggled,this,[this](bool on){m_flSteps->setVisible(on&&!m_flSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&CivilMechWidget::computeFluid);
    return w;
}

void CivilMechWidget::computeFluid() {
    double v1=m_flV1->text().toDouble(), A1=m_flA1->text().toDouble();
    double P1=m_flP1->text().toDouble(), dh=m_flH->text().toDouble();
    double rho=m_flRho->text().toDouble(), A2=m_flA2->text().toDouble();
    double D=m_flD->text().toDouble(), mu=m_flMu->text().toDouble();
    const double g=9.81;
    QStringList steps;
    // Continuity
    double v2=A2>0?v1*A1/A2:0, Q=v1*A1;
    steps << QString("Continuity: A₁v₁=A₂v₂ → v₂=%1 m/s").arg(v2,0,'g',6);
    steps << QString("Flow rate Q = A₁v₁ = %1 m³/s").arg(Q,0,'g',6);
    // Bernoulli
    double P2=P1+0.5*rho*(v1*v1-v2*v2)+rho*g*dh;
    steps << QString("Bernoulli: P₂ = P₁+½ρ(v₁²−v₂²)+ρg(h₁−h₂) = %1 Pa").arg(P2,0,'g',8);
    // Reynolds
    double Re=rho*v1*D/mu;
    QString flow=Re<2300?"Laminar":Re<4000?"Transitional":"Turbulent";
    steps << QString("Re = ρvD/μ = %1×%2×%3/%4 = %5 (%6)").arg(rho).arg(v1).arg(D).arg(mu).arg(Re,0,'g',6).arg(flow);
    m_flResult->setText(QString("v₂ = %1 m/s\nQ = %2 m³/s\nP₂ = %3 Pa\nRe = %4 (%5)")
        .arg(v2,0,'g',8).arg(Q,0,'g',8).arg(P2,0,'g',8).arg(Re,0,'g',6).arg(flow));
    showSteps(m_flSteps,m_flShow,steps);
}

// ── Heat Transfer ─────────────────────────────────────────────────────────────
QWidget* CivilMechWidget::buildHeatTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Fourier's law of conduction: Q = kA(ΔT/L). Convection: Q = hAΔT. Thermal resistance: R = L/(kA) for conduction, R = 1/(hA) for convection.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_htK=numEdit("50","k (W/m·K)",w); m_htA=numEdit("1","A (m²)",w);
    m_htDT=numEdit("100","ΔT (K)",w); m_htL=numEdit("0.1","L (m)",w);
    m_hth=numEdit("25","h (W/m²·K)",w);
    g->addWidget(new QLabel("k (W/m·K):",w),0,0); g->addWidget(m_htK,0,1);
    g->addWidget(new QLabel("A (m²):",w),1,0); g->addWidget(m_htA,1,1);
    g->addWidget(new QLabel("ΔT (K):",w),2,0); g->addWidget(m_htDT,2,1);
    g->addWidget(new QLabel("L (m):",w),3,0); g->addWidget(m_htL,3,1);
    g->addWidget(new QLabel("h (W/m²·K):",w),4,0); g->addWidget(m_hth,4,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_htResult=resultLbl(w); m_htShow=new QCheckBox("Show Steps",w); m_htSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_htResult);
    v->addWidget(m_htShow); v->addWidget(m_htSteps); v->addStretch();
    connect(m_htShow,&QCheckBox::toggled,this,[this](bool on){m_htSteps->setVisible(on&&!m_htSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&CivilMechWidget::computeHeat);
    return w;
}

void CivilMechWidget::computeHeat() {
    double k=m_htK->text().toDouble(), A=m_htA->text().toDouble();
    double dT=m_htDT->text().toDouble(), L=m_htL->text().toDouble(), h=m_hth->text().toDouble();
    QStringList steps;
    double Qcond=k*A*dT/L, Rth_cond=L/(k*A);
    double Qconv=h*A*dT, Rth_conv=1.0/(h*A);
    steps << QString("Conduction: Q = kAΔT/L = %1×%2×%3/%4 = %5 W").arg(k).arg(A).arg(dT).arg(L).arg(Qcond,0,'g',8);
    steps << QString("Thermal resistance (cond): R = L/kA = %1 K/W").arg(Rth_cond,0,'g',6);
    steps << QString("Convection: Q = hAΔT = %1×%2×%3 = %4 W").arg(h).arg(A).arg(dT).arg(Qconv,0,'g',8);
    steps << QString("Thermal resistance (conv): R = 1/hA = %1 K/W").arg(Rth_conv,0,'g',6);
    m_htResult->setText(QString("Conduction Q = %1 W,  R = %2 K/W\nConvection Q = %3 W,  R = %4 K/W")
        .arg(Qcond,0,'g',8).arg(Rth_cond,0,'g',6).arg(Qconv,0,'g',8).arg(Rth_conv,0,'g',6));
    showSteps(m_htSteps,m_htShow,steps);
}

// ── Gear & Pulley ─────────────────────────────────────────────────────────────
QWidget* CivilMechWidget::buildGearTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Gear ratio: N₁/N₂ = ω₂/ω₁ = T₂/T₁. Speed ratio = N₁/N₂. Torque ratio = N₂/N₁. Mechanical advantage = output/input force.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_gN1=numEdit("20","N₁ (teeth)",w); m_gN2=numEdit("60","N₂ (teeth)",w);
    m_gT1=numEdit("10","T₁ (N·m)",w); m_gW1=numEdit("100","ω₁ (rpm)",w);
    g->addWidget(new QLabel("N₁ (driver teeth):",w),0,0); g->addWidget(m_gN1,0,1);
    g->addWidget(new QLabel("N₂ (driven teeth):",w),1,0); g->addWidget(m_gN2,1,1);
    g->addWidget(new QLabel("Input torque T₁:",w),2,0); g->addWidget(m_gT1,2,1);
    g->addWidget(new QLabel("Input speed ω₁:",w),3,0); g->addWidget(m_gW1,3,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_gResult=resultLbl(w); m_gShow=new QCheckBox("Show Steps",w); m_gSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_gResult);
    v->addWidget(m_gShow); v->addWidget(m_gSteps); v->addStretch();
    connect(m_gShow,&QCheckBox::toggled,this,[this](bool on){m_gSteps->setVisible(on&&!m_gSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&CivilMechWidget::computeGear);
    return w;
}

void CivilMechWidget::computeGear() {
    double N1=m_gN1->text().toDouble(), N2=m_gN2->text().toDouble();
    double T1=m_gT1->text().toDouble(), w1=m_gW1->text().toDouble();
    QStringList steps;
    double ratio=N1/N2, w2=w1*ratio, T2=T1/ratio;
    double P=T1*(w1*2*M_PI/60); // power in watts
    steps << QString("Gear ratio = N₁/N₂ = %1/%2 = %3").arg(N1).arg(N2).arg(ratio,0,'g',6);
    steps << QString("Output speed ω₂ = ω₁×(N₁/N₂) = %1 rpm").arg(w2,0,'g',6);
    steps << QString("Output torque T₂ = T₁×(N₂/N₁) = %1 N·m").arg(T2,0,'g',6);
    steps << QString("Power P = T₁×ω₁ = %1 W (assuming 100% efficiency)").arg(P,0,'g',6);
    m_gResult->setText(QString("Gear ratio = %1\nOutput speed ω₂ = %2 rpm\nOutput torque T₂ = %3 N·m\nPower = %4 W")
        .arg(ratio,0,'g',6).arg(w2,0,'g',8).arg(T2,0,'g',8).arg(P,0,'g',8));
    showSteps(m_gSteps,m_gShow,steps);
}
