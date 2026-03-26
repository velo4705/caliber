#include "advancedmath_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDoubleValidator>
#include <QSet>
#include <cmath>

static QPushButton* mkBtn(const QString& t, QWidget* p) {
    auto* b = new QPushButton(t, p); b->setProperty("class","actionButton");
    b->setMinimumHeight(36); b->setFocusPolicy(Qt::NoFocus); return b;
}
static QLineEdit* numEdit(const QString& def, const QString& ph, QWidget* p) {
    auto* e = new QLineEdit(def); e->setPlaceholderText(ph);
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

void AdvancedMathWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

AdvancedMathWidget::AdvancedMathWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Advanced Mathematics", this);
    title->setStyleSheet("font-size:18px;font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildComplexTab(),    "Complex Numbers");
    tabs->addTab(buildLinAlgTab(),     "Linear Algebra");
    tabs->addTab(buildSeriesTab(),     "Sequences & Series");
    tabs->addTab(buildTrigSolverTab(), "Triangle Solver");
    tabs->addTab(buildPolyTab(),       "Polynomials");
    tabs->addTab(buildSetTab(),        "Set Theory");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Complex Numbers ───────────────────────────────────────────────────────────
QWidget* AdvancedMathWidget::buildComplexTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Complex number arithmetic in rectangular form (a+bi). Also computes polar form (r∠θ), Euler's formula (re^iθ), modulus, argument, and conjugate.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_ca1=numEdit("3","a₁",w); m_cb1=numEdit("4","b₁",w);
    m_ca2=numEdit("1","a₂",w); m_cb2=numEdit("2","b₂",w);
    m_cOp=new QComboBox(w); m_cOp->addItems({"Add","Subtract","Multiply","Divide","Power (z₁^n)"});
    g->addWidget(new QLabel("z₁ = a₁+b₁i:",w),0,0);
    g->addWidget(new QLabel("a₁:",w),0,1); g->addWidget(m_ca1,0,2);
    g->addWidget(new QLabel("b₁:",w),0,3); g->addWidget(m_cb1,0,4);
    g->addWidget(new QLabel("z₂ = a₂+b₂i:",w),1,0);
    g->addWidget(new QLabel("a₂:",w),1,1); g->addWidget(m_ca2,1,2);
    g->addWidget(new QLabel("b₂:",w),1,3); g->addWidget(m_cb2,1,4);
    g->addWidget(new QLabel("Operation:",w),2,0); g->addWidget(m_cOp,2,1,1,3);
    g->setColumnStretch(5,1);
    v->addLayout(g);
    m_cResult=resultLbl(w); m_cShow=new QCheckBox("Show Steps",w); m_cSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_cResult);
    v->addWidget(m_cShow); v->addWidget(m_cSteps); v->addStretch();
    connect(m_cShow,&QCheckBox::toggled,this,[this](bool on){m_cSteps->setVisible(on&&!m_cSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&AdvancedMathWidget::computeComplex);
    return w;
}

void AdvancedMathWidget::computeComplex() {
    double a1=m_ca1->text().toDouble(), b1=m_cb1->text().toDouble();
    double a2=m_ca2->text().toDouble(), b2=m_cb2->text().toDouble();
    int idx=m_cOp->currentIndex();
    QStringList steps;
    double ra,rb;
    steps << QString("z₁ = %1 + %2i").arg(a1).arg(b1);
    steps << QString("z₂ = %1 + %2i").arg(a2).arg(b2);
    if(idx==0){ra=a1+a2;rb=b1+b2;steps<<"Add: (a₁+a₂)+(b₁+b₂)i";}
    else if(idx==1){ra=a1-a2;rb=b1-b2;steps<<"Subtract: (a₁-a₂)+(b₁-b₂)i";}
    else if(idx==2){
        ra=a1*a2-b1*b2; rb=a1*b2+b1*a2;
        steps<<QString("Multiply: (%1×%2−%3×%4)+(%1×%4+%3×%2)i").arg(a1).arg(a2).arg(b1).arg(b2);
    } else if(idx==3){
        double denom=a2*a2+b2*b2;
        if(denom==0){m_cResult->setText("Division by zero.");return;}
        ra=(a1*a2+b1*b2)/denom; rb=(b1*a2-a1*b2)/denom;
        steps<<QString("Divide: multiply by conjugate, denom=%1").arg(denom);
    } else { // power z1^n where n=a2
        int n=(int)a2;
        double r=std::sqrt(a1*a1+b1*b1), theta=std::atan2(b1,a1);
        double rn=std::pow(r,n); double ntheta=n*theta;
        ra=rn*std::cos(ntheta); rb=rn*std::sin(ntheta);
        steps<<QString("z₁^%1: r=%2, θ=%3 rad → r^n=%4, nθ=%5 rad").arg(n).arg(r,0,'g',4).arg(theta,0,'g',4).arg(rn,0,'g',6).arg(ntheta,0,'g',6);
    }
    double r=std::sqrt(ra*ra+rb*rb), theta=std::atan2(rb,ra)*180/M_PI;
    steps<<QString("Result: %1 + %2i").arg(ra,0,'g',8).arg(rb,0,'g',8);
    steps<<QString("Polar: r=%1, θ=%2°").arg(r,0,'g',8).arg(theta,0,'f',4);
    // Also show z1 properties
    double r1=std::sqrt(a1*a1+b1*b1), t1=std::atan2(b1,a1)*180/M_PI;
    steps<<QString("z₁ properties: |z₁|=%1, arg=%2°, conj=%3−%4i").arg(r1,0,'g',6).arg(t1,0,'f',4).arg(a1).arg(b1);
    m_cResult->setText(QString("Result = %1 + %2i\nPolar: %3 ∠ %4°\nEuler: %3 × e^(i×%5 rad)")
        .arg(ra,0,'g',8).arg(rb,0,'g',8).arg(r,0,'g',8).arg(theta,0,'f',4).arg(theta*M_PI/180,0,'g',6));
    showSteps(m_cSteps,m_cShow,steps);
}

// ── Linear Algebra (2×2 eigenvalues) ─────────────────────────────────────────
QWidget* AdvancedMathWidget::buildLinAlgTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes eigenvalues and eigenvectors of a 2×2 matrix using the characteristic equation det(A−λI)=0. Also shows trace, determinant, and rank.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_la[0]=numEdit("4","a",w); m_la[1]=numEdit("1","b",w);
    m_la[2]=numEdit("2","c",w); m_la[3]=numEdit("3","d",w);
    g->addWidget(new QLabel("Matrix A:",w),0,0);
    g->addWidget(m_la[0],0,1); g->addWidget(m_la[1],0,2);
    g->addWidget(m_la[2],1,1); g->addWidget(m_la[3],1,2);
    g->setColumnStretch(3,1);
    v->addLayout(g);
    m_laResult=resultLbl(w); m_laShow=new QCheckBox("Show Steps",w); m_laSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_laResult);
    v->addWidget(m_laShow); v->addWidget(m_laSteps); v->addStretch();
    connect(m_laShow,&QCheckBox::toggled,this,[this](bool on){m_laSteps->setVisible(on&&!m_laSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&AdvancedMathWidget::computeLinAlg);
    return w;
}

void AdvancedMathWidget::computeLinAlg() {
    double a=m_la[0]->text().toDouble(), b=m_la[1]->text().toDouble();
    double c=m_la[2]->text().toDouble(), d=m_la[3]->text().toDouble();
    QStringList steps;
    double trace=a+d, det=a*d-b*c;
    steps<<QString("A = [[%1,%2],[%3,%4]]").arg(a).arg(b).arg(c).arg(d);
    steps<<QString("Trace = a+d = %1").arg(trace,0,'g',8);
    steps<<QString("det(A) = ad−bc = %1×%2−%3×%4 = %5").arg(a).arg(d).arg(b).arg(c).arg(det,0,'g',8);
    steps<<"Characteristic equation: λ²−trace×λ+det=0";
    double disc=trace*trace-4*det;
    steps<<QString("Discriminant = trace²−4det = %1").arg(disc,0,'g',8);
    QString eigenStr, eigenvecStr;
    if(disc>=0){
        double l1=(trace+std::sqrt(disc))/2, l2=(trace-std::sqrt(disc))/2;
        steps<<QString("λ₁ = %1,  λ₂ = %2").arg(l1,0,'g',8).arg(l2,0,'g',8);
        // Eigenvectors: (A−λI)v=0
        auto eigvec=[&](double lam)->QString{
            // (a-lam)x + b*y = 0 → y/x = -(a-lam)/b if b≠0
            if(std::abs(b)>1e-12) return QString("[%1, 1]").arg(-(a-lam)/b,0,'g',4);
            if(std::abs(c)>1e-12) return QString("[1, %1]").arg(-(d-lam)/c,0,'g',4);
            return "[1, 0]";
        };
        eigenvecStr=QString("v₁=%1, v₂=%2").arg(eigvec(l1)).arg(eigvec(l2));
        eigenStr=QString("λ₁ = %1\nλ₂ = %2\n%3").arg(l1,0,'g',8).arg(l2,0,'g',8).arg(eigenvecStr);
    } else {
        double re=trace/2, im=std::sqrt(-disc)/2;
        steps<<QString("Complex eigenvalues: λ = %1 ± %2i").arg(re,0,'g',6).arg(im,0,'g',6);
        eigenStr=QString("λ₁ = %1 + %2i\nλ₂ = %1 − %2i").arg(re,0,'g',8).arg(im,0,'g',8);
    }
    int rank=(std::abs(det)>1e-12)?2:(std::abs(a)>1e-12||std::abs(b)>1e-12||std::abs(c)>1e-12||std::abs(d)>1e-12)?1:0;
    m_laResult->setText(QString("Trace = %1\ndet(A) = %2\nRank = %3\n\nEigenvalues:\n%4")
        .arg(trace,0,'g',8).arg(det,0,'g',8).arg(rank).arg(eigenStr));
    showSteps(m_laSteps,m_laShow,steps);
}

// ── Sequences & Series ────────────────────────────────────────────────────────
QWidget* AdvancedMathWidget::buildSeriesTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Arithmetic sequence: aₙ=a+(n−1)d, Sₙ=n/2(2a+(n−1)d). Geometric sequence: aₙ=ar^(n−1), Sₙ=a(1−rⁿ)/(1−r). Also checks convergence of geometric series.", w));
    auto* row=new QHBoxLayout();
    m_seqType=new QComboBox(w); m_seqType->addItems({"Arithmetic","Geometric"});
    row->addWidget(new QLabel("Type:",w)); row->addWidget(m_seqType); row->addStretch();
    v->addLayout(row);
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_seqA=numEdit("2","a (first term)",w); m_seqD=numEdit("3","d (common diff)",w);
    m_seqR=numEdit("2","r (common ratio)",w); m_seqN=numEdit("10","n (terms)",w);
    g->addWidget(new QLabel("a (first term):",w),0,0); g->addWidget(m_seqA,0,1);
    g->addWidget(new QLabel("d (diff):",w),1,0); g->addWidget(m_seqD,1,1);
    g->addWidget(new QLabel("r (ratio):",w),2,0); g->addWidget(m_seqR,2,1);
    g->addWidget(new QLabel("n (terms):",w),3,0); g->addWidget(m_seqN,3,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_seqResult=resultLbl(w); m_seqShow=new QCheckBox("Show Steps",w); m_seqSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_seqResult);
    v->addWidget(m_seqShow); v->addWidget(m_seqSteps); v->addStretch();
    connect(m_seqShow,&QCheckBox::toggled,this,[this](bool on){m_seqSteps->setVisible(on&&!m_seqSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&AdvancedMathWidget::computeSeries);
    return w;
}

void AdvancedMathWidget::computeSeries() {
    double a=m_seqA->text().toDouble(), d=m_seqD->text().toDouble();
    double r=m_seqR->text().toDouble(); int n=(int)m_seqN->text().toDouble();
    QStringList steps;
    if(m_seqType->currentIndex()==0){
        double an=a+(n-1)*d, Sn=n/2.0*(2*a+(n-1)*d);
        steps<<QString("Arithmetic: a=%1, d=%2, n=%3").arg(a).arg(d).arg(n);
        steps<<QString("aₙ = a+(n−1)d = %1+%2×%3 = %4").arg(a).arg(d).arg(n-1).arg(an,0,'g',8);
        steps<<QString("Sₙ = n/2×(2a+(n−1)d) = %1").arg(Sn,0,'g',8);
        // First 5 terms
        QStringList terms; for(int i=1;i<=qMin(n,8);i++) terms<<QString::number(a+(i-1)*d,'g',4);
        m_seqResult->setText(QString("aₙ = %1\nSₙ = %2\nFirst terms: %3%4")
            .arg(an,0,'g',8).arg(Sn,0,'g',8).arg(terms.join(", ")).arg(n>8?"...":""));
    } else {
        double an=a*std::pow(r,n-1);
        double Sn=std::abs(r-1)>1e-12?a*(1-std::pow(r,n))/(1-r):a*n;
        steps<<QString("Geometric: a=%1, r=%2, n=%3").arg(a).arg(r).arg(n);
        steps<<QString("aₙ = ar^(n−1) = %1×%2^%3 = %4").arg(a).arg(r).arg(n-1).arg(an,0,'g',8);
        steps<<QString("Sₙ = a(1−rⁿ)/(1−r) = %1").arg(Sn,0,'g',8);
        QString inf_sum=std::abs(r)<1?QString("S∞ = a/(1−r) = %1").arg(a/(1-r),0,'g',8):"Series diverges (|r|≥1)";
        steps<<inf_sum;
        QStringList terms; for(int i=1;i<=qMin(n,8);i++) terms<<QString::number(a*std::pow(r,i-1),'g',4);
        m_seqResult->setText(QString("aₙ = %1\nSₙ = %2\n%3\nFirst terms: %4%5")
            .arg(an,0,'g',8).arg(Sn,0,'g',8).arg(inf_sum).arg(terms.join(", ")).arg(n>8?"...":""));
    }
    showSteps(m_seqSteps,m_seqShow,steps);
}

// ── Triangle Solver ───────────────────────────────────────────────────────────
QWidget* AdvancedMathWidget::buildTrigSolverTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Solves any triangle given SSS, SAS, ASA, or AAS. Enter known sides (a,b,c) and angles (A,B,C in degrees). Leave unknowns empty.", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_trA=numEdit("","a",w); m_trB=numEdit("","b",w); m_trC=numEdit("","c",w);
    m_trAngA=numEdit("","A°",w); m_trAngB=numEdit("","B°",w); m_trAngC=numEdit("","C°",w);
    g->addWidget(new QLabel("Side a:",w),0,0); g->addWidget(m_trA,0,1);
    g->addWidget(new QLabel("Side b:",w),1,0); g->addWidget(m_trB,1,1);
    g->addWidget(new QLabel("Side c:",w),2,0); g->addWidget(m_trC,2,1);
    g->addWidget(new QLabel("Angle A°:",w),3,0); g->addWidget(m_trAngA,3,1);
    g->addWidget(new QLabel("Angle B°:",w),4,0); g->addWidget(m_trAngB,4,1);
    g->addWidget(new QLabel("Angle C°:",w),5,0); g->addWidget(m_trAngC,5,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_trResult=resultLbl(w); m_trShow=new QCheckBox("Show Steps",w); m_trSteps=stepsBox(w);
    v->addWidget(mkBtn("Solve",w)); v->addWidget(m_trResult);
    v->addWidget(m_trShow); v->addWidget(m_trSteps); v->addStretch();
    connect(m_trShow,&QCheckBox::toggled,this,[this](bool on){m_trSteps->setVisible(on&&!m_trSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&AdvancedMathWidget::computeTrigSolver);
    return w;
}

void AdvancedMathWidget::computeTrigSolver() {
    auto get=[](QLineEdit* e,bool& has)->double{ has=!e->text().isEmpty(); return e->text().toDouble(); };
    bool ha,hb,hc,hA,hB,hC;
    double a=get(m_trA,ha),b=get(m_trB,hb),c=get(m_trC,hc);
    double A=get(m_trAngA,hA)*M_PI/180,B=get(m_trAngB,hB)*M_PI/180,C=get(m_trAngC,hC)*M_PI/180;
    QStringList steps;
    steps<<"Using law of sines: a/sin(A)=b/sin(B)=c/sin(C)";
    steps<<"Using law of cosines: c²=a²+b²−2ab×cos(C)";
    // Fill missing angle if two known
    if(hA&&hB&&!hC){C=M_PI-A-B;hC=true;steps<<QString("C=180°−A−B=%1°").arg(C*180/M_PI,0,'f',4);}
    if(hA&&hC&&!hB){B=M_PI-A-C;hB=true;steps<<QString("B=180°−A−C=%1°").arg(B*180/M_PI,0,'f',4);}
    if(hB&&hC&&!hA){A=M_PI-B-C;hA=true;steps<<QString("A=180°−B−C=%1°").arg(A*180/M_PI,0,'f',4);}
    // Law of sines to find sides
    if(ha&&hA&&hB&&!hb){b=a*std::sin(B)/std::sin(A);hb=true;steps<<QString("b=a×sin(B)/sin(A)=%1").arg(b,0,'g',8);}
    if(ha&&hA&&hC&&!hc){c=a*std::sin(C)/std::sin(A);hc=true;steps<<QString("c=a×sin(C)/sin(A)=%1").arg(c,0,'g',8);}
    if(hb&&hB&&hA&&!ha){a=b*std::sin(A)/std::sin(B);ha=true;steps<<QString("a=b×sin(A)/sin(B)=%1").arg(a,0,'g',8);}
    // Law of cosines SSS→angles
    if(ha&&hb&&hc&&!hA){A=std::acos((b*b+c*c-a*a)/(2*b*c));hA=true;steps<<QString("A=arccos((b²+c²−a²)/2bc)=%1°").arg(A*180/M_PI,0,'f',4);}
    if(ha&&hb&&hc&&!hB){B=std::acos((a*a+c*c-b*b)/(2*a*c));hB=true;steps<<QString("B=arccos((a²+c²−b²)/2ac)=%1°").arg(B*180/M_PI,0,'f',4);}
    if(ha&&hb&&hc&&!hC){C=M_PI-A-B;hC=true;}
    // SAS: two sides + included angle
    if(ha&&hb&&hC&&!hc){c=std::sqrt(a*a+b*b-2*a*b*std::cos(C));hc=true;steps<<QString("c=√(a²+b²−2ab×cos(C))=%1").arg(c,0,'g',8);}
    if(ha&&hc&&hB&&!hb){b=std::sqrt(a*a+c*c-2*a*c*std::cos(B));hb=true;steps<<QString("b=√(a²+c²−2ac×cos(B))=%1").arg(b,0,'g',8);}
    double area=0.5*a*b*std::sin(C);
    steps<<QString("Area = ½ab×sin(C) = %1").arg(area,0,'g',8);
    m_trResult->setText(QString("a=%1  b=%2  c=%3\nA=%4°  B=%5°  C=%6°\nArea=%7")
        .arg(a,0,'g',8).arg(b,0,'g',8).arg(c,0,'g',8)
        .arg(A*180/M_PI,0,'f',4).arg(B*180/M_PI,0,'f',4).arg(C*180/M_PI,0,'f',4)
        .arg(area,0,'g',8));
    showSteps(m_trSteps,m_trShow,steps);
}

// ── Polynomial Tools ──────────────────────────────────────────────────────────
QWidget* AdvancedMathWidget::buildPolyTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Finds roots of polynomials. Quadratic: ax²+bx+c=0 (exact). Cubic: ax³+bx²+cx+d=0 (Cardano's method). Shows discriminant and nature of roots.", w));
    auto* row=new QHBoxLayout();
    m_polyDeg=new QComboBox(w); m_polyDeg->addItems({"Quadratic (ax²+bx+c)","Cubic (ax³+bx²+cx+d)"});
    row->addWidget(new QLabel("Degree:",w)); row->addWidget(m_polyDeg); row->addStretch();
    v->addLayout(row);
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_polyA=numEdit("1","a",w); m_polyB=numEdit("-3","b",w);
    m_polyC=numEdit("2","c",w); m_polyD=numEdit("0","d",w);
    g->addWidget(new QLabel("a:",w),0,0); g->addWidget(m_polyA,0,1);
    g->addWidget(new QLabel("b:",w),1,0); g->addWidget(m_polyB,1,1);
    g->addWidget(new QLabel("c:",w),2,0); g->addWidget(m_polyC,2,1);
    g->addWidget(new QLabel("d:",w),3,0); g->addWidget(m_polyD,3,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_polyResult=resultLbl(w); m_polyShow=new QCheckBox("Show Steps",w); m_polySteps=stepsBox(w);
    v->addWidget(mkBtn("Find Roots",w)); v->addWidget(m_polyResult);
    v->addWidget(m_polyShow); v->addWidget(m_polySteps); v->addStretch();
    connect(m_polyShow,&QCheckBox::toggled,this,[this](bool on){m_polySteps->setVisible(on&&!m_polySteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&AdvancedMathWidget::computePoly);
    return w;
}

void AdvancedMathWidget::computePoly() {
    double a=m_polyA->text().toDouble(), b=m_polyB->text().toDouble();
    double c=m_polyC->text().toDouble(), d=m_polyD->text().toDouble();
    QStringList steps;
    if(m_polyDeg->currentIndex()==0){
        steps<<QString("Quadratic: %1x²+%2x+%3=0").arg(a).arg(b).arg(c);
        if(a==0){m_polyResult->setText("a=0, not quadratic.");return;}
        double disc=b*b-4*a*c;
        steps<<QString("Δ=b²−4ac=%1²−4×%2×%3=%4").arg(b).arg(a).arg(c).arg(disc,0,'g',8);
        if(disc>0){
            double x1=(-b+std::sqrt(disc))/(2*a), x2=(-b-std::sqrt(disc))/(2*a);
            steps<<QString("Two real roots: x₁=%1, x₂=%2").arg(x1,0,'g',10).arg(x2,0,'g',10);
            m_polyResult->setText(QString("Δ=%1 (two real roots)\nx₁=%2\nx₂=%3").arg(disc,0,'g',8).arg(x1,0,'g',10).arg(x2,0,'g',10));
        } else if(disc==0){
            double x=-b/(2*a);
            steps<<QString("One repeated root: x=%1").arg(x,0,'g',10);
            m_polyResult->setText(QString("Δ=0 (repeated root)\nx=%1").arg(x,0,'g',10));
        } else {
            double re=-b/(2*a), im=std::sqrt(-disc)/(2*a);
            steps<<QString("Complex roots: %1±%2i").arg(re,0,'g',8).arg(im,0,'g',8);
            m_polyResult->setText(QString("Δ=%1 (complex roots)\nx=%2±%3i").arg(disc,0,'g',8).arg(re,0,'g',8).arg(im,0,'g',8));
        }
    } else {
        // Cubic: depressed form via substitution x=t-b/(3a)
        steps<<QString("Cubic: %1x³+%2x²+%3x+%4=0").arg(a).arg(b).arg(c).arg(d);
        if(a==0){m_polyResult->setText("a=0, not cubic.");return;}
        double p=(3*a*c-b*b)/(3*a*a), q=(2*b*b*b-9*a*b*c+27*a*a*d)/(27*a*a*a);
        double D=q*q/4+p*p*p/27;
        steps<<QString("Depressed cubic: t³+pt+q=0, p=%1, q=%2").arg(p,0,'g',6).arg(q,0,'g',6);
        steps<<QString("Discriminant D=q²/4+p³/27=%1").arg(D,0,'g',8);
        double shift=-b/(3*a);
        if(D>0){
            double u=std::cbrt(-q/2+std::sqrt(D)), v=std::cbrt(-q/2-std::sqrt(D));
            double x1=u+v+shift;
            steps<<QString("One real root: x=%1").arg(x1,0,'g',10);
            m_polyResult->setText(QString("D>0: one real root\nx₁=%1").arg(x1,0,'g',10));
        } else if(std::abs(D)<1e-12){
            double x1=2*std::cbrt(-q/2)+shift, x2=-std::cbrt(-q/2)+shift;
            m_polyResult->setText(QString("D=0: repeated roots\nx₁=%1, x₂=%2").arg(x1,0,'g',10).arg(x2,0,'g',10));
        } else {
            double r=std::sqrt(-p*p*p/27), theta=std::acos(-q/(2*r))/3;
            double m=2*std::cbrt(r);
            double x1=m*std::cos(theta)+shift;
            double x2=m*std::cos(theta+2*M_PI/3)+shift;
            double x3=m*std::cos(theta+4*M_PI/3)+shift;
            steps<<QString("Three real roots");
            m_polyResult->setText(QString("D<0: three real roots\nx₁=%1\nx₂=%2\nx₃=%3").arg(x1,0,'g',10).arg(x2,0,'g',10).arg(x3,0,'g',10));
        }
    }
    showSteps(m_polySteps,m_polyShow,steps);
}

// ── Set Theory ────────────────────────────────────────────────────────────────
QWidget* AdvancedMathWidget::buildSetTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Set operations on integer sets. Enter comma-separated integers for sets A, B, and universal set U. Computes union, intersection, difference, complement, and power set size.", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_setA=new QLineEdit("1,2,3,4,5",w); m_setB=new QLineEdit("3,4,5,6,7",w);
    m_setU=new QLineEdit("1,2,3,4,5,6,7,8,9,10",w);
    g->addWidget(new QLabel("Set A:",w),0,0); g->addWidget(m_setA,0,1);
    g->addWidget(new QLabel("Set B:",w),1,0); g->addWidget(m_setB,1,1);
    g->addWidget(new QLabel("Universal U:",w),2,0); g->addWidget(m_setU,2,1);
    g->setColumnStretch(1,1);
    v->addLayout(g);
    m_setResult=resultLbl(w); m_setShow=new QCheckBox("Show Steps",w); m_setSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_setResult);
    v->addWidget(m_setShow); v->addWidget(m_setSteps); v->addStretch();
    connect(m_setShow,&QCheckBox::toggled,this,[this](bool on){m_setSteps->setVisible(on&&!m_setSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&AdvancedMathWidget::computeSet);
    return w;
}

void AdvancedMathWidget::computeSet() {
    auto parse=[](const QString& s)->QSet<int>{
        QSet<int> set;
        for(const QString& p:s.split(',',Qt::SkipEmptyParts)){bool ok;int v=p.trimmed().toInt(&ok);if(ok)set<<v;}
        return set;
    };
    auto fmt=[](const QSet<int>& s)->QString{
        QList<int> l=s.values(); std::sort(l.begin(),l.end());
        QStringList sl; for(int v:l) sl<<QString::number(v);
        return "{"+sl.join(", ")+"}";
    };
    QSet<int> A=parse(m_setA->text()), B=parse(m_setB->text()), U=parse(m_setU->text());
    QStringList steps;
    steps<<QString("|A|=%1, |B|=%2, |U|=%3").arg(A.size()).arg(B.size()).arg(U.size());
    QSet<int> uni=A|B, inter=A&B, diff=A-B, diffBA=B-A, compA=U-A, compB=U-B;
    steps<<QString("A∪B = %1").arg(fmt(uni));
    steps<<QString("A∩B = %1").arg(fmt(inter));
    steps<<QString("A−B = %1").arg(fmt(diff));
    steps<<QString("A' (complement in U) = %1").arg(fmt(compA));
    long long powerSetSize=1LL<<A.size();
    steps<<QString("|P(A)| = 2^%1 = %2").arg(A.size()).arg(powerSetSize);
    // Cartesian product size
    steps<<QString("|A×B| = |A|×|B| = %1×%2 = %3").arg(A.size()).arg(B.size()).arg(A.size()*B.size());
    m_setResult->setText(QString(
        "A∪B = %1\nA∩B = %2\nA−B = %3\nB−A = %4\nA' = %5\nB' = %6\n|P(A)| = %7\n|A×B| = %8")
        .arg(fmt(uni)).arg(fmt(inter)).arg(fmt(diff)).arg(fmt(diffBA))
        .arg(fmt(compA)).arg(fmt(compB)).arg(powerSetSize).arg(A.size()*B.size()));
    showSteps(m_setSteps,m_setShow,steps);
}
