#include "physics_widget.h"
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

static const double G_CONST = 9.81;
static const double PI = M_PI;
static const double K_E = 8.9875e9; // Coulomb's constant

static QPushButton* mkBtn(const QString& t, QWidget* p) {
    auto* b = new QPushButton(t, p); b->setProperty("class","actionButton");
    b->setMinimumHeight(36); b->setFocusPolicy(Qt::NoFocus); return b;
}
static QLineEdit* numEdit(const QString& def, const QString& ph, QWidget* p) {
    auto* e = new QLineEdit(def, p);
    e->setPlaceholderText(ph);
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,p));
    e->setFixedWidth(100); return e;
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

void PhysicsWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

PhysicsWidget::PhysicsWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Physics", this);
    title->setStyleSheet("font-size:18px;font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildKinematicsTab(),     "Kinematics");
    tabs->addTab(buildNewtonsTab(),        "Newton's Laws");
    tabs->addTab(buildEnergyTab(),         "Energy & Work");
    tabs->addTab(buildProjectileTab(),     "Projectile");
    tabs->addTab(buildCircularTab(),       "Circular Motion");
    tabs->addTab(buildWavesTab(),          "Waves & Optics");
    tabs->addTab(buildThermTab(),          "Thermodynamics");
    tabs->addTab(buildElectrostaticsTab(), "Electrostatics");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Kinematics ────────────────────────────────────────────────────────────────
QWidget* PhysicsWidget::buildKinematicsTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("SUVAT equations: s=displacement, u=initial velocity, v=final velocity, a=acceleration, t=time. Enter any 3 known values — leave unknowns empty.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_ks=numEdit("","s (m)",w); m_ku=numEdit("","u (m/s)",w); m_kv=numEdit("","v (m/s)",w);
    m_ka=numEdit("","a (m/s²)",w); m_kt=numEdit("","t (s)",w);
    g->addWidget(new QLabel("s:",w),0,0); g->addWidget(m_ks,0,1);
    g->addWidget(new QLabel("u:",w),1,0); g->addWidget(m_ku,1,1);
    g->addWidget(new QLabel("v:",w),2,0); g->addWidget(m_kv,2,1);
    g->addWidget(new QLabel("a:",w),3,0); g->addWidget(m_ka,3,1);
    g->addWidget(new QLabel("t:",w),4,0); g->addWidget(m_kt,4,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_kResult=resultLbl(w); m_kShow=new QCheckBox("Show Steps",w); m_kSteps=stepsBox(w);
    v->addWidget(mkBtn("Solve",w)); v->addWidget(m_kResult);
    v->addWidget(m_kShow); v->addWidget(m_kSteps); v->addStretch();
    connect(m_kShow,&QCheckBox::toggled,this,[this](bool on){m_kSteps->setVisible(on&&!m_kSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&PhysicsWidget::computeKinematics);
    return w;
}

void PhysicsWidget::computeKinematics() {
    bool hs=!m_ks->text().isEmpty(), hu=!m_ku->text().isEmpty(), hv=!m_kv->text().isEmpty();
    bool ha=!m_ka->text().isEmpty(), ht=!m_kt->text().isEmpty();
    double s=m_ks->text().toDouble(), u=m_ku->text().toDouble(), v=m_kv->text().toDouble();
    double a=m_ka->text().toDouble(), t=m_kt->text().toDouble();
    QStringList steps;
    steps << "SUVAT equations: v=u+at, s=ut+½at², v²=u²+2as, s=½(u+v)t";
    // Solve for unknowns
    if (hu&&ha&&ht&&!hv) { v=u+a*t; steps<<"v=u+at="+QString::number(v,'g',8); hv=true; }
    if (hu&&hv&&ht&&!hs) { s=0.5*(u+v)*t; steps<<"s=½(u+v)t="+QString::number(s,'g',8); hs=true; }
    if (hu&&ha&&ht&&!hs) { s=u*t+0.5*a*t*t; steps<<"s=ut+½at²="+QString::number(s,'g',8); hs=true; }
    if (hu&&hv&&ha&&!ht) { t=(v-u)/a; steps<<"t=(v-u)/a="+QString::number(t,'g',8); ht=true; }
    if (hu&&hv&&hs&&!ha) { a=(v*v-u*u)/(2*s); steps<<"a=(v²-u²)/2s="+QString::number(a,'g',8); ha=true; }
    if (hv&&ha&&ht&&!hu) { u=v-a*t; steps<<"u=v-at="+QString::number(u,'g',8); hu=true; }
    if (hu&&ha&&hs&&!hv) { double disc=u*u+2*a*s; v=disc>=0?std::sqrt(disc):-std::sqrt(-disc); steps<<"v=√(u²+2as)="+QString::number(v,'g',8); hv=true; }
    m_kResult->setText(QString("s = %1 m\nu = %2 m/s\nv = %3 m/s\na = %4 m/s²\nt = %5 s")
        .arg(s,0,'g',8).arg(u,0,'g',8).arg(v,0,'g',8).arg(a,0,'g',8).arg(t,0,'g',8));
    showSteps(m_kSteps,m_kShow,steps);
}

// ── Newton's Laws ─────────────────────────────────────────────────────────────
QWidget* PhysicsWidget::buildNewtonsTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("F=ma solver. Also computes friction force (f=μN), normal force on incline, and net force. Leave any one field empty to solve for it.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_nF=numEdit("","F (N)",w); m_nM=numEdit("","m (kg)",w); m_nA=numEdit("","a (m/s²)",w);
    m_nMu=numEdit("0.3","μ (friction)",w); m_nN=numEdit("","N (normal, N)",w);
    g->addWidget(new QLabel("Force F:",w),0,0); g->addWidget(m_nF,0,1);
    g->addWidget(new QLabel("Mass m:",w),1,0); g->addWidget(m_nM,1,1);
    g->addWidget(new QLabel("Accel a:",w),2,0); g->addWidget(m_nA,2,1);
    g->addWidget(new QLabel("μ (coeff):",w),3,0); g->addWidget(m_nMu,3,1);
    g->addWidget(new QLabel("Normal N:",w),4,0); g->addWidget(m_nN,4,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_nResult=resultLbl(w); m_nShow=new QCheckBox("Show Steps",w); m_nSteps=stepsBox(w);
    v->addWidget(mkBtn("Solve",w)); v->addWidget(m_nResult);
    v->addWidget(m_nShow); v->addWidget(m_nSteps); v->addStretch();
    connect(m_nShow,&QCheckBox::toggled,this,[this](bool on){m_nSteps->setVisible(on&&!m_nSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&PhysicsWidget::computeNewtons);
    return w;
}

void PhysicsWidget::computeNewtons() {
    bool hF=!m_nF->text().isEmpty(), hM=!m_nM->text().isEmpty(), hA=!m_nA->text().isEmpty();
    double F=m_nF->text().toDouble(), m=m_nM->text().toDouble(), a=m_nA->text().toDouble();
    double mu=m_nMu->text().toDouble(), N=m_nN->text().isEmpty()?m*G_CONST:m_nN->text().toDouble();
    QStringList steps;
    steps << "Newton's 2nd law: F = m × a";
    if (hM&&hA&&!hF) { F=m*a; steps<<"F=m×a="+QString::number(F,'g',8)+" N"; }
    else if (hF&&hA&&!hM) { m=F/a; steps<<"m=F/a="+QString::number(m,'g',8)+" kg"; }
    else if (hF&&hM&&!hA) { a=F/m; steps<<"a=F/m="+QString::number(a,'g',8)+" m/s²"; }
    double friction = mu * N;
    steps << QString("Normal force N = %1 N").arg(N,0,'g',6);
    steps << QString("Friction f = μN = %1×%2 = %3 N").arg(mu).arg(N,0,'g',6).arg(friction,0,'g',6);
    steps << QString("Net force = F − f = %1 − %2 = %3 N").arg(F,0,'g',6).arg(friction,0,'g',6).arg(F-friction,0,'g',6);
    m_nResult->setText(QString("F = %1 N\nm = %2 kg\na = %3 m/s²\nFriction = %4 N\nNet force = %5 N")
        .arg(F,0,'g',8).arg(m,0,'g',8).arg(a,0,'g',8).arg(friction,0,'g',6).arg(F-friction,0,'g',6));
    showSteps(m_nSteps,m_nShow,steps);
}

// ── Energy & Work ─────────────────────────────────────────────────────────────
QWidget* PhysicsWidget::buildEnergyTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("KE = ½mv², PE = mgh, Work = F×d×cos(θ), Power = W/t. Enter known values to compute all energy quantities.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_em=numEdit("10","m (kg)",w); m_ev=numEdit("5","v (m/s)",w);
    m_eh=numEdit("3","h (m)",w); m_eF=numEdit("20","F (N)",w); m_ed=numEdit("4","d (m)",w);
    g->addWidget(new QLabel("Mass m:",w),0,0); g->addWidget(m_em,0,1);
    g->addWidget(new QLabel("Velocity v:",w),1,0); g->addWidget(m_ev,1,1);
    g->addWidget(new QLabel("Height h:",w),2,0); g->addWidget(m_eh,2,1);
    g->addWidget(new QLabel("Force F:",w),3,0); g->addWidget(m_eF,3,1);
    g->addWidget(new QLabel("Distance d:",w),4,0); g->addWidget(m_ed,4,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_eResult=resultLbl(w); m_eShow=new QCheckBox("Show Steps",w); m_eSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_eResult);
    v->addWidget(m_eShow); v->addWidget(m_eSteps); v->addStretch();
    connect(m_eShow,&QCheckBox::toggled,this,[this](bool on){m_eSteps->setVisible(on&&!m_eSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&PhysicsWidget::computeEnergy);
    return w;
}

void PhysicsWidget::computeEnergy() {
    double m=m_em->text().toDouble(), v=m_ev->text().toDouble();
    double h=m_eh->text().toDouble(), F=m_eF->text().toDouble(), d=m_ed->text().toDouble();
    QStringList steps;
    double KE=0.5*m*v*v, PE=m*G_CONST*h, W=F*d;
    steps << QString("KE = ½mv² = ½×%1×%2² = %3 J").arg(m).arg(v).arg(KE,0,'g',8);
    steps << QString("PE = mgh = %1×%2×%3 = %4 J").arg(m).arg(G_CONST).arg(h).arg(PE,0,'g',8);
    steps << QString("W = F×d = %1×%2 = %3 J").arg(F).arg(d).arg(W,0,'g',8);
    steps << QString("Total mechanical energy = KE + PE = %1 J").arg(KE+PE,0,'g',8);
    m_eResult->setText(QString("KE = %1 J\nPE = %2 J\nWork W = %3 J\nTotal E = %4 J")
        .arg(KE,0,'g',8).arg(PE,0,'g',8).arg(W,0,'g',8).arg(KE+PE,0,'g',8));
    showSteps(m_eSteps,m_eShow,steps);
}

// ── Projectile Motion ─────────────────────────────────────────────────────────
QWidget* PhysicsWidget::buildProjectileTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Projectile launched at angle θ with initial speed v₀ from height h₀. Computes range, max height, time of flight, and velocity components.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_pv0=numEdit("20","v₀ (m/s)",w); m_pAngle=numEdit("45","θ (°)",w); m_ph0=numEdit("0","h₀ (m)",w);
    g->addWidget(new QLabel("v₀ (m/s):",w),0,0); g->addWidget(m_pv0,0,1);
    g->addWidget(new QLabel("Angle θ (°):",w),1,0); g->addWidget(m_pAngle,1,1);
    g->addWidget(new QLabel("Initial h₀ (m):",w),2,0); g->addWidget(m_ph0,2,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_pResult=resultLbl(w); m_pShow=new QCheckBox("Show Steps",w); m_pSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_pResult);
    v->addWidget(m_pShow); v->addWidget(m_pSteps); v->addStretch();
    connect(m_pShow,&QCheckBox::toggled,this,[this](bool on){m_pSteps->setVisible(on&&!m_pSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&PhysicsWidget::computeProjectile);
    return w;
}

void PhysicsWidget::computeProjectile() {
    double v0=m_pv0->text().toDouble(), theta=m_pAngle->text().toDouble()*PI/180, h0=m_ph0->text().toDouble();
    double vx=v0*std::cos(theta), vy=v0*std::sin(theta);
    QStringList steps;
    steps << QString("v₀=%1 m/s, θ=%2°, h₀=%3 m").arg(v0).arg(m_pAngle->text()).arg(h0);
    steps << QString("vx = v₀cos(θ) = %1 m/s").arg(vx,0,'g',6);
    steps << QString("vy = v₀sin(θ) = %1 m/s").arg(vy,0,'g',6);
    double tFlight = (vy + std::sqrt(vy*vy + 2*G_CONST*h0)) / G_CONST;
    steps << QString("Time of flight: t = (vy + √(vy²+2gh₀))/g = %1 s").arg(tFlight,0,'g',6);
    double range = vx * tFlight;
    steps << QString("Range R = vx × t = %1 m").arg(range,0,'g',6);
    double hMax = h0 + vy*vy/(2*G_CONST);
    steps << QString("Max height H = h₀ + vy²/2g = %1 m").arg(hMax,0,'g',6);
    m_pResult->setText(QString("Range = %1 m\nMax height = %2 m\nTime of flight = %3 s\nvx = %4 m/s,  vy = %5 m/s")
        .arg(range,0,'g',8).arg(hMax,0,'g',8).arg(tFlight,0,'g',8).arg(vx,0,'g',6).arg(vy,0,'g',6));
    showSteps(m_pSteps,m_pShow,steps);
}

// ── Circular Motion ───────────────────────────────────────────────────────────
QWidget* PhysicsWidget::buildCircularTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Circular motion: centripetal force Fc=mv²/r, angular velocity ω=2π/T, period T=2πr/v. Enter mass, radius, and either speed or period.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_cm=numEdit("2","m (kg)",w); m_cr=numEdit("5","r (m)",w);
    m_cv=numEdit("10","v (m/s)",w); m_cT=numEdit("","T (s)",w);
    g->addWidget(new QLabel("Mass m:",w),0,0); g->addWidget(m_cm,0,1);
    g->addWidget(new QLabel("Radius r:",w),1,0); g->addWidget(m_cr,1,1);
    g->addWidget(new QLabel("Speed v:",w),2,0); g->addWidget(m_cv,2,1);
    g->addWidget(new QLabel("Period T:",w),3,0); g->addWidget(m_cT,3,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_cResult=resultLbl(w); m_cShow=new QCheckBox("Show Steps",w); m_cSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_cResult);
    v->addWidget(m_cShow); v->addWidget(m_cSteps); v->addStretch();
    connect(m_cShow,&QCheckBox::toggled,this,[this](bool on){m_cSteps->setVisible(on&&!m_cSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&PhysicsWidget::computeCircular);
    return w;
}

void PhysicsWidget::computeCircular() {
    double m=m_cm->text().toDouble(), r=m_cr->text().toDouble();
    double v=m_cv->text().isEmpty()?0:m_cv->text().toDouble();
    double T=m_cT->text().isEmpty()?0:m_cT->text().toDouble();
    if (v==0 && T>0) v=2*PI*r/T;
    if (T==0 && v>0) T=2*PI*r/v;
    QStringList steps;
    double omega=v>0?v/r:2*PI/T;
    double Fc=m*v*v/r;
    double ac=v*v/r;
    steps << QString("v=%1 m/s, r=%2 m, m=%3 kg").arg(v,0,'g',6).arg(r).arg(m);
    steps << QString("ω = v/r = %1 rad/s").arg(omega,0,'g',6);
    steps << QString("T = 2πr/v = %1 s").arg(T,0,'g',6);
    steps << QString("ac = v²/r = %1 m/s²").arg(ac,0,'g',6);
    steps << QString("Fc = mac = mv²/r = %1 N").arg(Fc,0,'g',6);
    m_cResult->setText(QString("ω = %1 rad/s\nT = %2 s\nac = %3 m/s²\nFc = %4 N")
        .arg(omega,0,'g',8).arg(T,0,'g',8).arg(ac,0,'g',8).arg(Fc,0,'g',8));
    showSteps(m_cSteps,m_cShow,steps);
}

// ── Waves & Optics ────────────────────────────────────────────────────────────
QWidget* PhysicsWidget::buildWavesTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Wave equation: v=fλ. Snell's law: n₁sin(θ₁)=n₂sin(θ₂). Enter wave speed and frequency to get wavelength, or refractive indices and angle for Snell's law.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_wv=numEdit("340","v (m/s)",w); m_wf=numEdit("440","f (Hz)",w); m_wlambda=numEdit("","λ (m)",w);
    m_on1=numEdit("1.0","n₁",w); m_on2=numEdit("1.5","n₂",w); m_oTheta1=numEdit("30","θ₁ (°)",w);
    g->addWidget(new QLabel("Wave speed v:",w),0,0); g->addWidget(m_wv,0,1);
    g->addWidget(new QLabel("Frequency f:",w),1,0); g->addWidget(m_wf,1,1);
    g->addWidget(new QLabel("Wavelength λ:",w),2,0); g->addWidget(m_wlambda,2,1);
    g->addWidget(new QLabel("n₁:",w),3,0); g->addWidget(m_on1,3,1);
    g->addWidget(new QLabel("n₂:",w),4,0); g->addWidget(m_on2,4,1);
    g->addWidget(new QLabel("θ₁ (°):",w),5,0); g->addWidget(m_oTheta1,5,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_wResult=resultLbl(w); m_wShow=new QCheckBox("Show Steps",w); m_wSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_wResult);
    v->addWidget(m_wShow); v->addWidget(m_wSteps); v->addStretch();
    connect(m_wShow,&QCheckBox::toggled,this,[this](bool on){m_wSteps->setVisible(on&&!m_wSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&PhysicsWidget::computeWaves);
    return w;
}

void PhysicsWidget::computeWaves() {
    double wv=m_wv->text().toDouble(), wf=m_wf->text().toDouble();
    double wl=m_wlambda->text().isEmpty()?0:m_wlambda->text().toDouble();
    double n1=m_on1->text().toDouble(), n2=m_on2->text().toDouble();
    double t1=m_oTheta1->text().toDouble()*PI/180;
    QStringList steps;
    // Wave
    if (wf>0&&wv>0) wl=wv/wf;
    else if (wf>0&&wl>0) wv=wf*wl;
    else if (wv>0&&wl>0) wf=wv/wl;
    steps << QString("v = f×λ: v=%1, f=%2, λ=%3").arg(wv,0,'g',6).arg(wf,0,'g',6).arg(wl,0,'g',6);
    double T=wf>0?1.0/wf:0;
    steps << QString("Period T = 1/f = %1 s").arg(T,0,'g',6);
    // Snell's law
    double sinT2=n1*std::sin(t1)/n2;
    QString snell;
    if (std::abs(sinT2)<=1) {
        double t2=std::asin(sinT2)*180/PI;
        steps << QString("Snell's law: n₁sin(θ₁)=n₂sin(θ₂)");
        steps << QString("sin(θ₂)=n₁sin(θ₁)/n₂=%1×sin(%2°)/%3=%4").arg(n1).arg(m_oTheta1->text()).arg(n2).arg(sinT2,0,'g',4);
        steps << QString("θ₂ = %1°").arg(t2,0,'f',4);
        snell = QString("θ₂ = %1°").arg(t2,0,'f',4);
    } else {
        snell = "Total internal reflection (no refracted ray)";
        steps << snell;
    }
    m_wResult->setText(QString("v = %1 m/s\nf = %2 Hz\nλ = %3 m\nT = %4 s\nSnell: %5")
        .arg(wv,0,'g',8).arg(wf,0,'g',8).arg(wl,0,'g',8).arg(T,0,'g',6).arg(snell));
    showSteps(m_wSteps,m_wShow,steps);
}

// ── Thermodynamics ────────────────────────────────────────────────────────────
QWidget* PhysicsWidget::buildThermTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Ideal gas law: PV=nRT (R=8.314 J/mol·K). Specific heat: Q=mcΔT. Enter known values — leave unknowns empty.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_tP=numEdit("101325","P (Pa)",w); m_tV=numEdit("0.001","V (m³)",w);
    m_tn=numEdit("","n (mol)",w); m_tT=numEdit("300","T (K)",w);
    m_tm=numEdit("1","m (kg)",w); m_tc=numEdit("4186","c (J/kg·K)",w); m_tdT=numEdit("10","ΔT (K)",w);
    g->addWidget(new QLabel("P (Pa):",w),0,0); g->addWidget(m_tP,0,1);
    g->addWidget(new QLabel("V (m³):",w),1,0); g->addWidget(m_tV,1,1);
    g->addWidget(new QLabel("n (mol):",w),2,0); g->addWidget(m_tn,2,1);
    g->addWidget(new QLabel("T (K):",w),3,0); g->addWidget(m_tT,3,1);
    g->addWidget(new QLabel("m (kg):",w),4,0); g->addWidget(m_tm,4,1);
    g->addWidget(new QLabel("c (J/kg·K):",w),5,0); g->addWidget(m_tc,5,1);
    g->addWidget(new QLabel("ΔT (K):",w),6,0); g->addWidget(m_tdT,6,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_tResult=resultLbl(w); m_tShow=new QCheckBox("Show Steps",w); m_tSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_tResult);
    v->addWidget(m_tShow); v->addWidget(m_tSteps); v->addStretch();
    connect(m_tShow,&QCheckBox::toggled,this,[this](bool on){m_tSteps->setVisible(on&&!m_tSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&PhysicsWidget::computeTherm);
    return w;
}

void PhysicsWidget::computeTherm() {
    double P=m_tP->text().toDouble(), V=m_tV->text().toDouble();
    double T=m_tT->text().toDouble();
    double m=m_tm->text().toDouble(), c=m_tc->text().toDouble(), dT=m_tdT->text().toDouble();
    const double R=8.314;
    QStringList steps;
    double n=m_tn->text().isEmpty()?(P*V/(R*T)):m_tn->text().toDouble();
    steps << QString("Ideal gas: PV=nRT, R=8.314 J/mol·K");
    steps << QString("n = PV/RT = %1×%2/(%3×%4) = %5 mol").arg(P).arg(V).arg(R).arg(T).arg(n,0,'g',6);
    double Q=m*c*dT;
    steps << QString("Q = mcΔT = %1×%2×%3 = %4 J").arg(m).arg(c).arg(dT).arg(Q,0,'g',8);
    m_tResult->setText(QString("Ideal gas:\nn = %1 mol\nPV = %2 J\n\nHeat transfer:\nQ = %3 J")
        .arg(n,0,'g',8).arg(P*V,0,'g',8).arg(Q,0,'g',8));
    showSteps(m_tSteps,m_tShow,steps);
}

// ── Electrostatics ────────────────────────────────────────────────────────────
QWidget* PhysicsWidget::buildElectrostaticsTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Coulomb's law: F=kq₁q₂/r². Electric field E=kq/r². Electric potential V=kq/r. k=8.9875×10⁹ N·m²/C².", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_esq1=numEdit("1e-6","q₁ (C)",w); m_esq2=numEdit("2e-6","q₂ (C)",w); m_esr=numEdit("0.1","r (m)",w);
    g->addWidget(new QLabel("q₁ (C):",w),0,0); g->addWidget(m_esq1,0,1);
    g->addWidget(new QLabel("q₂ (C):",w),1,0); g->addWidget(m_esq2,1,1);
    g->addWidget(new QLabel("r (m):",w),2,0); g->addWidget(m_esr,2,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_esResult=resultLbl(w); m_esShow=new QCheckBox("Show Steps",w); m_esSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_esResult);
    v->addWidget(m_esShow); v->addWidget(m_esSteps); v->addStretch();
    connect(m_esShow,&QCheckBox::toggled,this,[this](bool on){m_esSteps->setVisible(on&&!m_esSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&PhysicsWidget::computeElectrostatics);
    return w;
}

void PhysicsWidget::computeElectrostatics() {
    double q1=m_esq1->text().toDouble(), q2=m_esq2->text().toDouble(), r=m_esr->text().toDouble();
    QStringList steps;
    double F=K_E*q1*q2/(r*r);
    double E1=K_E*q1/(r*r), E2=K_E*q2/(r*r);
    double V1=K_E*q1/r, V2=K_E*q2/r;
    double U=K_E*q1*q2/r;
    steps << QString("k = %1 N·m²/C²").arg(K_E,'g');
    steps << QString("F = kq₁q₂/r² = %1×%2×%3/%4² = %5 N").arg(K_E,'g').arg(q1,'g').arg(q2,'g').arg(r).arg(F,0,'g',6);
    steps << QString("E₁ = kq₁/r² = %1 N/C").arg(E1,0,'g',6);
    steps << QString("V₁ = kq₁/r = %1 V").arg(V1,0,'g',6);
    steps << QString("PE = kq₁q₂/r = %1 J").arg(U,0,'g',6);
    m_esResult->setText(QString("Coulomb force F = %1 N\nE₁ = %2 N/C\nE₂ = %3 N/C\nV₁ = %4 V\nV₂ = %5 V\nPE = %6 J")
        .arg(F,0,'g',8).arg(E1,0,'g',6).arg(E2,0,'g',6).arg(V1,0,'g',6).arg(V2,0,'g',6).arg(U,0,'g',6));
    showSteps(m_esSteps,m_esShow,steps);
}
