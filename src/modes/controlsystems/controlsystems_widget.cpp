#include "controlsystems_widget.h"
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
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,p));
    e->setFixedWidth(110); return e;
}
static QLabel* resultLbl(QWidget* p) {
    auto* l = new QLabel("—", p); l->setWordWrap(true);
    l->setStyleSheet("font-size:13px;font-weight:bold;padding:8px;"); return l;
}
static QTextEdit* stepsBox(QWidget* p) {
    auto* t = new QTextEdit(p); t->setReadOnly(true);
    t->setMinimumHeight(100); t->setMaximumHeight(200);
    t->setStyleSheet("font-family:monospace;font-size:12px;"); t->hide(); return t;
}
static QLabel* descLbl(const QString& text, QWidget* p) {
    auto* l = new QLabel(text, p); l->setWordWrap(true);
    l->setStyleSheet("color:gray;font-size:12px;padding:4px 0;"); return l;
}

void ControlSystemsWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

ControlSystemsWidget::ControlSystemsWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Control Systems", this);
    title->setStyleSheet("font-size:18px;font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildTransferTab(),    "Transfer Function");
    tabs->addTab(buildRouthTab(),       "Routh-Hurwitz");
    tabs->addTab(buildPidTab(),         "PID Tuning");
    tabs->addTab(buildSteadyStateTab(), "Steady-State Error");
    tabs->addTab(buildStateSpaceTab(),  "State Space");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Transfer Function Analyzer ────────────────────────────────────────────────
QWidget* ControlSystemsWidget::buildTransferTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Analyzes G(s) = N(s)/D(s). Enter polynomial coefficients highest power first. Computes poles, zeros, DC gain, system type, and natural frequency for 2nd order systems.", w));
    v->addWidget(new QLabel("Numerator N(s) coefficients:", w));
    m_tfNum=new QTextEdit(w); m_tfNum->setPlaceholderText("e.g. 1, 2  →  s + 2"); m_tfNum->setMaximumHeight(50);
    v->addWidget(m_tfNum);
    v->addWidget(new QLabel("Denominator D(s) coefficients:", w));
    m_tfDen=new QTextEdit(w); m_tfDen->setPlaceholderText("e.g. 1, 3, 2  →  s² + 3s + 2"); m_tfDen->setMaximumHeight(50);
    v->addWidget(m_tfDen);
    m_tfResult=resultLbl(w); m_tfShow=new QCheckBox("Show Steps",w); m_tfSteps=stepsBox(w);
    v->addWidget(mkBtn("Analyze",w)); v->addWidget(m_tfResult);
    v->addWidget(m_tfShow); v->addWidget(m_tfSteps); v->addStretch();
    connect(m_tfShow,&QCheckBox::toggled,this,[this](bool on){m_tfSteps->setVisible(on&&!m_tfSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&ControlSystemsWidget::computeTransfer);
    return w;
}

void ControlSystemsWidget::computeTransfer() {
    auto parse=[](const QString& s)->QVector<double>{
        QVector<double> v;
        for(const QString& p:s.split(',',Qt::SkipEmptyParts)){bool ok;double d=p.trimmed().toDouble(&ok);if(ok)v<<d;}
        return v;
    };
    QVector<double> num=parse(m_tfNum->toPlainText()), den=parse(m_tfDen->toPlainText());
    if(num.isEmpty()||den.isEmpty()){m_tfResult->setText("Enter coefficients.");return;}
    QStringList steps;
    int numDeg=num.size()-1, denDeg=den.size()-1;
    steps<<QString("G(s) = N(s)/D(s), deg(N)=%1, deg(D)=%2").arg(numDeg).arg(denDeg);
    // DC gain H(0)
    double dcGain=den.last()!=0?num.last()/den.last():0;
    steps<<QString("DC gain G(0) = N(0)/D(0) = %1/%2 = %3").arg(num.last()).arg(den.last()).arg(dcGain,0,'g',6);
    // System type: number of poles at s=0 (trailing zeros in denominator)
    int sysType=0; for(int i=den.size()-1;i>=0&&std::abs(den[i])<1e-12;i--) sysType++;
    steps<<QString("System type = %1 (poles at s=0)").arg(sysType);
    // Poles (roots of denominator)
    QString poles, zeros;
    if(den.size()==2){ double p=-den.last()/den.first(); poles=QString("s=%1").arg(p,0,'g',6); steps<<QString("Pole: s=%1").arg(p,0,'g',6); }
    else if(den.size()==3){
        double a=den[0],b=den[1],c=den[2],disc=b*b-4*a*c;
        double wn=std::sqrt(std::abs(c/a)), zeta=b/(2*std::sqrt(a*c));
        steps<<QString("2nd order: ωn=%1 rad/s, ζ=%2").arg(wn,0,'g',6).arg(zeta,0,'g',6);
        if(disc>=0) poles=QString("s=%1, s=%2").arg((-b+std::sqrt(disc))/(2*a),0,'g',4).arg((-b-std::sqrt(disc))/(2*a),0,'g',4);
        else poles=QString("s=%1±%2j").arg(-b/(2*a),0,'g',4).arg(std::sqrt(-disc)/(2*a),0,'g',4);
        QString resp=zeta<0?"Unstable":zeta==0?"Undamped":zeta<1?"Underdamped":zeta==1?"Critically damped":"Overdamped";
        steps<<QString("Damping: ζ=%1 → %2").arg(zeta,0,'g',4).arg(resp);
        poles+=QString("\nωn=%1, ζ=%2 (%3)").arg(wn,0,'g',6).arg(zeta,0,'g',4).arg(resp);
    } else poles="(higher order — use Routh-Hurwitz tab)";
    if(num.size()==2){ double z=-num.last()/num.first(); zeros=QString("s=%1").arg(z,0,'g',6); }
    else if(num.size()==1) zeros="none";
    else zeros="(higher order)";
    // Stability check for 2nd order
    bool stable=true;
    if(den.size()==3){ double a=den[0],b=den[1],c=den[2]; stable=(a>0&&b>0&&c>0)||(a<0&&b<0&&c<0); }
    else if(den.size()==2){ stable=-den.last()/den.first()<0; }
    m_tfResult->setText(QString("DC Gain = %1\nSystem Type = %2\nPoles: %3\nZeros: %4\nStability: %5")
        .arg(dcGain,0,'g',6).arg(sysType).arg(poles).arg(zeros).arg(stable?"Stable":"Unstable"));
    showSteps(m_tfSteps,m_tfShow,steps);
}

// ── Routh-Hurwitz Stability ───────────────────────────────────────────────────
QWidget* ControlSystemsWidget::buildRouthTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Routh-Hurwitz stability criterion. Enter characteristic equation coefficients (highest power first). Builds the Routh array and counts sign changes in the first column.", w));
    v->addWidget(new QLabel("Characteristic equation coefficients (highest power first):", w));
    m_rhCoeffs=new QTextEdit(w); m_rhCoeffs->setPlaceholderText("e.g. 1, 2, 3, 4  →  s³+2s²+3s+4");
    m_rhCoeffs->setMaximumHeight(60);
    v->addWidget(m_rhCoeffs);
    m_rhResult=resultLbl(w); m_rhShow=new QCheckBox("Show Steps",w); m_rhSteps=stepsBox(w);
    v->addWidget(mkBtn("Build Routh Array",w)); v->addWidget(m_rhResult);
    v->addWidget(m_rhShow); v->addWidget(m_rhSteps); v->addStretch();
    connect(m_rhShow,&QCheckBox::toggled,this,[this](bool on){m_rhSteps->setVisible(on&&!m_rhSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&ControlSystemsWidget::computeRouth);
    return w;
}

void ControlSystemsWidget::computeRouth() {
    QVector<double> c;
    for(const QString& p:m_rhCoeffs->toPlainText().split(',',Qt::SkipEmptyParts)){
        bool ok; double v=p.trimmed().toDouble(&ok); if(ok) c<<v;
    }
    if(c.size()<2){m_rhResult->setText("Enter at least 2 coefficients.");return;}
    int n=c.size()-1; // degree
    QStringList steps;
    steps<<QString("Characteristic polynomial degree: %1").arg(n);
    // Build Routh array
    int rows=n+1;
    QVector<QVector<double>> R(rows, QVector<double>((n+2)/2+1, 0));
    // Fill first two rows
    for(int i=0;i<(int)c.size();i+=2) R[0][i/2]=c[i];
    for(int i=1;i<(int)c.size();i+=2) R[1][(i-1)/2]=c[i];
    // Fill remaining rows
    for(int i=2;i<rows;i++){
        double pivot=R[i-1][0];
        if(std::abs(pivot)<1e-12) pivot=1e-6; // small epsilon to avoid division by zero
        for(int j=0;j<(int)R[i].size()-1;j++){
            R[i][j]=(R[i-1][0]*R[i-2][j+1]-R[i-2][0]*R[i-1][j+1])/pivot;
        }
    }
    // Print array
    QString arrayStr="Routh Array:\n";
    for(int i=0;i<rows;i++){
        QString row=QString("s^%1: ").arg(n-i);
        for(int j=0;j<(int)R[i].size()&&(j==0||std::abs(R[i][j])>1e-12||j<2);j++)
            row+=QString("%1  ").arg(R[i][j],8,'f',4);
        arrayStr+=row.trimmed()+"\n";
        steps<<row.trimmed();
    }
    // Count sign changes in first column
    int signChanges=0;
    double prev=R[0][0];
    for(int i=1;i<rows;i++){
        if(R[i][0]*prev<0) signChanges++;
        if(std::abs(R[i][0])>1e-12) prev=R[i][0];
    }
    steps<<QString("Sign changes in first column: %1").arg(signChanges);
    steps<<(signChanges==0?"No sign changes → System is STABLE":"Sign changes → UNSTABLE, "+QString::number(signChanges)+" RHP poles");
    m_rhResult->setText(arrayStr+QString("\nSign changes: %1\n%2")
        .arg(signChanges).arg(signChanges==0?"✓ STABLE":"✗ UNSTABLE — "+QString::number(signChanges)+" poles in RHP"));
    showSteps(m_rhSteps,m_rhShow,steps);
}

// ── PID Tuning ────────────────────────────────────────────────────────────────
QWidget* ControlSystemsWidget::buildPidTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("PID tuning using Ziegler-Nichols method. Enter the ultimate gain Ku (gain at which system oscillates) and ultimate period Tu (oscillation period in seconds).", w));
    auto* row=new QHBoxLayout();
    m_pidMethod=new QComboBox(w);
    m_pidMethod->addItems({"P controller","PI controller","PID controller","PD controller"});
    row->addWidget(new QLabel("Controller:",w)); row->addWidget(m_pidMethod); row->addStretch();
    v->addLayout(row);
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_pidKu=numEdit("10","Ku (ultimate gain)",w); m_pidTu=numEdit("2","Tu (s)",w);
    g->addWidget(new QLabel("Ku (ultimate gain):",w),0,0); g->addWidget(m_pidKu,0,1);
    g->addWidget(new QLabel("Tu (ultimate period):",w),1,0); g->addWidget(m_pidTu,1,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_pidResult=resultLbl(w); m_pidShow=new QCheckBox("Show Steps",w); m_pidSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute PID",w)); v->addWidget(m_pidResult);
    v->addWidget(m_pidShow); v->addWidget(m_pidSteps); v->addStretch();
    connect(m_pidShow,&QCheckBox::toggled,this,[this](bool on){m_pidSteps->setVisible(on&&!m_pidSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&ControlSystemsWidget::computePid);
    return w;
}

void ControlSystemsWidget::computePid() {
    double Ku=m_pidKu->text().toDouble(), Tu=m_pidTu->text().toDouble();
    int idx=m_pidMethod->currentIndex();
    QStringList steps;
    steps<<QString("Ziegler-Nichols method: Ku=%1, Tu=%2 s").arg(Ku).arg(Tu);
    double Kp=0,Ki=0,Kd=0,Ti=0,Td=0;
    if(idx==0){ Kp=0.5*Ku; steps<<"P: Kp=0.5×Ku"; }
    else if(idx==1){ Kp=0.45*Ku; Ti=Tu/1.2; Ki=Kp/Ti; steps<<"PI: Kp=0.45Ku, Ti=Tu/1.2, Ki=Kp/Ti"; }
    else if(idx==2){ Kp=0.6*Ku; Ti=Tu/2; Td=Tu/8; Ki=Kp/Ti; Kd=Kp*Td; steps<<"PID: Kp=0.6Ku, Ti=Tu/2, Td=Tu/8"; }
    else { Kp=0.8*Ku; Td=Tu/8; Kd=Kp*Td; steps<<"PD: Kp=0.8Ku, Td=Tu/8"; }
    steps<<QString("Kp=%1, Ki=%2, Kd=%3").arg(Kp,0,'g',6).arg(Ki,0,'g',6).arg(Kd,0,'g',6);
    steps<<QString("Transfer function: C(s) = Kp + Ki/s + Kd×s");
    m_pidResult->setText(QString("Ziegler-Nichols %1:\nKp = %2\nKi = %3\nKd = %4%5")
        .arg(m_pidMethod->currentText()).arg(Kp,0,'g',6).arg(Ki,0,'g',6).arg(Kd,0,'g',6)
        .arg(Ti>0?QString("\nTi = %1 s,  Td = %2 s").arg(Ti,0,'g',6).arg(Td,0,'g',6):""));
    showSteps(m_pidSteps,m_pidShow,steps);
}

// ── Steady-State Error ────────────────────────────────────────────────────────
QWidget* ControlSystemsWidget::buildSteadyStateTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Steady-state error for unity feedback systems. Enter open-loop G(s) coefficients. Computes position (Kp), velocity (Kv), acceleration (Ka) error constants and steady-state errors.", w));
    auto* row=new QHBoxLayout();
    m_sseType=new QComboBox(w);
    m_sseType->addItems({"Step input (1/s)","Ramp input (1/s²)","Parabolic input (1/s³)"});
    row->addWidget(new QLabel("Input type:",w)); row->addWidget(m_sseType); row->addStretch();
    v->addLayout(row);
    v->addWidget(new QLabel("G(s) Numerator coefficients:", w));
    m_sseNum=new QTextEdit(w); m_sseNum->setPlaceholderText("e.g. 10"); m_sseNum->setMaximumHeight(50);
    v->addWidget(m_sseNum);
    v->addWidget(new QLabel("G(s) Denominator coefficients:", w));
    m_sseDen=new QTextEdit(w); m_sseDen->setPlaceholderText("e.g. 1, 2, 0  →  s(s+2)"); m_sseDen->setMaximumHeight(50);
    v->addWidget(m_sseDen);
    m_sseResult=resultLbl(w); m_sseShow=new QCheckBox("Show Steps",w); m_sseSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_sseResult);
    v->addWidget(m_sseShow); v->addWidget(m_sseSteps); v->addStretch();
    connect(m_sseShow,&QCheckBox::toggled,this,[this](bool on){m_sseSteps->setVisible(on&&!m_sseSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&ControlSystemsWidget::computeSteadyState);
    return w;
}

void ControlSystemsWidget::computeSteadyState() {
    auto parse=[](const QString& s)->QVector<double>{
        QVector<double> v;
        for(const QString& p:s.split(',',Qt::SkipEmptyParts)){bool ok;double d=p.trimmed().toDouble(&ok);if(ok)v<<d;}
        return v;
    };
    QVector<double> num=parse(m_sseNum->toPlainText()), den=parse(m_sseDen->toPlainText());
    if(num.isEmpty()||den.isEmpty()){m_sseResult->setText("Enter coefficients.");return;}
    QStringList steps;
    // System type: count trailing zeros in denominator
    int sysType=0;
    for(int i=den.size()-1;i>=0&&std::abs(den[i])<1e-12;i--) sysType++;
    steps<<QString("System type = %1").arg(sysType);
    // DC gain of G(s): lim s→0 of G(s) = num(0)/den_nonzero(0)
    double numDC=num.last();
    // Remove trailing zeros from den for DC gain
    QVector<double> denNZ=den;
    while(!denNZ.isEmpty()&&std::abs(denNZ.last())<1e-12) denNZ.removeLast();
    double denDC=denNZ.isEmpty()?1:denNZ.last();
    double Kp=sysType==0?numDC/denDC:1e300;
    double Kv=sysType==1?numDC/denDC:sysType>1?1e300:0;
    double Ka=sysType==2?numDC/denDC:sysType>2?1e300:0;
    steps<<QString("Kp = lim G(s) as s→0 = %1").arg(Kp>1e10?"∞":QString::number(Kp,'g',6));
    steps<<QString("Kv = lim s·G(s) as s→0 = %1").arg(Kv>1e10?"∞":Kv<1e-10?"0":QString::number(Kv,'g',6));
    steps<<QString("Ka = lim s²·G(s) as s→0 = %1").arg(Ka>1e10?"∞":Ka<1e-10?"0":QString::number(Ka,'g',6));
    double ess_step=Kp>1e10?0:1.0/(1+Kp);
    double ess_ramp=Kv>1e10?0:Kv<1e-10?1e300:1.0/Kv;
    double ess_para=Ka>1e10?0:Ka<1e-10?1e300:1.0/Ka;
    steps<<QString("e_ss (step) = 1/(1+Kp) = %1").arg(ess_step>1e10?"∞":QString::number(ess_step,'g',6));
    steps<<QString("e_ss (ramp) = 1/Kv = %1").arg(ess_ramp>1e10?"∞":QString::number(ess_ramp,'g',6));
    steps<<QString("e_ss (parabolic) = 1/Ka = %1").arg(ess_para>1e10?"∞":QString::number(ess_para,'g',6));
    m_sseResult->setText(QString("System Type = %1\nKp = %2\nKv = %3\nKa = %4\n\ne_ss (step) = %5\ne_ss (ramp) = %6\ne_ss (parabolic) = %7")
        .arg(sysType)
        .arg(Kp>1e10?"∞":QString::number(Kp,'g',6))
        .arg(Kv>1e10?"∞":Kv<1e-10?"0":QString::number(Kv,'g',6))
        .arg(Ka>1e10?"∞":Ka<1e-10?"0":QString::number(Ka,'g',6))
        .arg(ess_step>1e10?"∞":QString::number(ess_step,'g',6))
        .arg(ess_ramp>1e10?"∞":QString::number(ess_ramp,'g',6))
        .arg(ess_para>1e10?"∞":QString::number(ess_para,'g',6)));
    showSteps(m_sseSteps,m_sseShow,steps);
}

// ── State Space ───────────────────────────────────────────────────────────────
QWidget* ControlSystemsWidget::buildStateSpaceTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Converts a 2nd order transfer function G(s)=b₀/(s²+a₁s+a₀) to controllable canonical state-space form. Enter denominator and numerator coefficients.", w));
    v->addWidget(new QLabel("Numerator (b₀ only for 2nd order):", w));
    m_ssNum=new QTextEdit(w); m_ssNum->setPlaceholderText("e.g. 4"); m_ssNum->setMaximumHeight(50);
    v->addWidget(m_ssNum);
    v->addWidget(new QLabel("Denominator (s²+a₁s+a₀):", w));
    m_ssDen=new QTextEdit(w); m_ssDen->setPlaceholderText("e.g. 1, 3, 2  →  s²+3s+2"); m_ssDen->setMaximumHeight(50);
    v->addWidget(m_ssDen);
    m_ssResult=resultLbl(w); m_ssShow=new QCheckBox("Show Steps",w); m_ssSteps=stepsBox(w);
    v->addWidget(mkBtn("Convert",w)); v->addWidget(m_ssResult);
    v->addWidget(m_ssShow); v->addWidget(m_ssSteps); v->addStretch();
    connect(m_ssShow,&QCheckBox::toggled,this,[this](bool on){m_ssSteps->setVisible(on&&!m_ssSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&ControlSystemsWidget::computeStateSpace);
    return w;
}

void ControlSystemsWidget::computeStateSpace() {
    auto parse=[](const QString& s)->QVector<double>{
        QVector<double> v;
        for(const QString& p:s.split(',',Qt::SkipEmptyParts)){bool ok;double d=p.trimmed().toDouble(&ok);if(ok)v<<d;}
        return v;
    };
    QVector<double> num=parse(m_ssNum->toPlainText()), den=parse(m_ssDen->toPlainText());
    if(den.size()<3){m_ssResult->setText("Enter 3 denominator coefficients for 2nd order system.");return;}
    QStringList steps;
    double a1=den[1]/den[0], a0=den[2]/den[0];
    double b0=num.isEmpty()?1:num.last()/den[0];
    steps<<QString("G(s) = %1 / (s² + %2s + %3)").arg(b0*den[0]).arg(a1).arg(a0);
    steps<<"Controllable canonical form:";
    steps<<QString("A = [[0, 1], [-%1, -%2]]").arg(a0).arg(a1);
    steps<<QString("B = [[0], [1]]");
    steps<<QString("C = [%1, 0]").arg(b0);
    steps<<"D = [0]";
    // Eigenvalues (poles)
    double disc=a1*a1-4*a0;
    QString poles;
    if(disc>=0) poles=QString("λ₁=%1, λ₂=%2").arg((-a1+std::sqrt(disc))/2,0,'g',4).arg((-a1-std::sqrt(disc))/2,0,'g',4);
    else poles=QString("λ=%1±%2j").arg(-a1/2,0,'g',4).arg(std::sqrt(-disc)/2,0,'g',4);
    steps<<QString("Eigenvalues (poles): %1").arg(poles);
    // Controllability matrix [B, AB]
    // B=[0,1]^T, AB=A*B=[1,-a1]^T
    double detC=1*1-0*(-a1); // det([0,1;1,-a1]) = -a1 - 0 = ... actually det = 0*(-a1)-1*1 = -1
    steps<<QString("Controllability matrix det = %1 → %2").arg(-1).arg("Controllable (det≠0)");
    m_ssResult->setText(QString(
        "State-space (controllable canonical):\n\n"
        "A = ⎡  0      1  ⎤\n"
        "    ⎣ -%1   -%2  ⎦\n\n"
        "B = ⎡ 0 ⎤\n"
        "    ⎣ 1 ⎦\n\n"
        "C = [ %3   0 ]\n"
        "D = [ 0 ]\n\n"
        "Poles: %4\n"
        "System is controllable")
        .arg(a0,0,'g',6).arg(a1,0,'g',6).arg(b0,0,'g',6).arg(poles));
    showSteps(m_ssSteps,m_ssShow,steps);
}
