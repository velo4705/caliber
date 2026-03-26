#include "vectors_widget.h"
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
static QLineEdit* numEdit(const QString& def, QWidget* p) {
    auto* e = new QLineEdit(def);
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,p));
    e->setFixedWidth(80); return e;
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

void VectorsWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

VectorsWidget::Vec3 VectorsWidget::readA() const { return {m_ax->text().toDouble(), m_ay->text().toDouble(), m_az->text().toDouble()}; }
VectorsWidget::Vec3 VectorsWidget::readB() const { return {m_bx->text().toDouble(), m_by->text().toDouble(), m_bz->text().toDouble()}; }
QString VectorsWidget::fmt(Vec3 v) const { return QString("(%1, %2, %3)").arg(v.x,0,'g',6).arg(v.y,0,'g',6).arg(v.z,0,'g',6); }

VectorsWidget::Vec3  VectorsWidget::add(Vec3 a, Vec3 b)   { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
VectorsWidget::Vec3  VectorsWidget::sub(Vec3 a, Vec3 b)   { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
VectorsWidget::Vec3  VectorsWidget::scale(Vec3 a, double s){ return {a.x*s, a.y*s, a.z*s}; }
double VectorsWidget::dot(Vec3 a, Vec3 b)   { return a.x*b.x + a.y*b.y + a.z*b.z; }
VectorsWidget::Vec3  VectorsWidget::cross(Vec3 a, Vec3 b)  { return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }
double VectorsWidget::mag(Vec3 a)   { return std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }
VectorsWidget::Vec3  VectorsWidget::unit(Vec3 a){ double m=mag(a); return m>0?Vec3{a.x/m,a.y/m,a.z/m}:Vec3{0,0,0}; }
double VectorsWidget::angle(Vec3 a, Vec3 b){ double d=mag(a)*mag(b); return d>0?std::acos(std::max(-1.0,std::min(1.0,dot(a,b)/d)))*180.0/M_PI:0; }

// shared vector input row
static void addVecRow(QGridLayout* g, int row, const QString& lbl,
                      QLineEdit*& ex, QLineEdit*& ey, QLineEdit*& ez, QWidget* p) {
    g->addWidget(new QLabel(lbl, p), row, 0);
    ex = numEdit("1", p); ey = numEdit("0", p); ez = numEdit("0", p);
    g->addWidget(new QLabel("x:", p), row, 1); g->addWidget(ex, row, 2);
    g->addWidget(new QLabel("y:", p), row, 3); g->addWidget(ey, row, 4);
    g->addWidget(new QLabel("z:", p), row, 5); g->addWidget(ez, row, 6);
}

VectorsWidget::VectorsWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Vectors", this);
    title->setStyleSheet("font-size:18px;font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildArithTab(),     "Arithmetic");
    tabs->addTab(buildDotCrossTab(),  "Dot & Cross");
    tabs->addTab(buildMagUnitTab(),   "Magnitude & Unit");
    tabs->addTab(buildAngleProjTab(), "Angle & Projection");
    tabs->addTab(buildLinePlaneTab(), "Line & Plane");
    tabs->addTab(buildDistanceTab(),  "Distance");
    root->addWidget(tabs);
    setLayout(root);
}

QWidget* VectorsWidget::buildArithTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Vector addition, subtraction, and scalar multiplication. Enter 3D vectors A and B.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    addVecRow(g, 0, "A:", m_ax, m_ay, m_az, w);
    addVecRow(g, 1, "B:", m_bx, m_by, m_bz, w);
    v->addLayout(g);
    m_arithResult = resultLbl(w); m_arithShow = new QCheckBox("Show Steps", w); m_arithSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_arithResult);
    v->addWidget(m_arithShow); v->addWidget(m_arithSteps); v->addStretch();
    connect(m_arithShow, &QCheckBox::toggled, this, [this](bool on){ m_arithSteps->setVisible(on && !m_arithSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &VectorsWidget::computeArith);
    return w;
}

void VectorsWidget::computeArith() {
    Vec3 a=readA(), b=readB();
    QStringList steps;
    steps << QString("A = %1").arg(fmt(a));
    steps << QString("B = %1").arg(fmt(b));
    Vec3 sum=add(a,b), dif=sub(a,b), s2=scale(a,2);
    steps << QString("A + B = %1").arg(fmt(sum));
    steps << QString("A − B = %1").arg(fmt(dif));
    steps << QString("2A = %1").arg(fmt(s2));
    m_arithResult->setText(QString("A + B = %1\nA − B = %2\n2A = %3").arg(fmt(sum)).arg(fmt(dif)).arg(fmt(s2)));
    showSteps(m_arithSteps, m_arithShow, steps);
}

QWidget* VectorsWidget::buildDotCrossTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Dot product: A·B = AxBx + AyBy + AzBz (scalar). Cross product: A×B = vector perpendicular to both A and B.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    addVecRow(g, 0, "A:", m_ax, m_ay, m_az, w);
    addVecRow(g, 1, "B:", m_bx, m_by, m_bz, w);
    v->addLayout(g);
    m_dcResult = resultLbl(w); m_dcShow = new QCheckBox("Show Steps", w); m_dcSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_dcResult);
    v->addWidget(m_dcShow); v->addWidget(m_dcSteps); v->addStretch();
    connect(m_dcShow, &QCheckBox::toggled, this, [this](bool on){ m_dcSteps->setVisible(on && !m_dcSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &VectorsWidget::computeDotCross);
    return w;
}

void VectorsWidget::computeDotCross() {
    Vec3 a=readA(), b=readB();
    QStringList steps;
    steps << QString("A = %1,  B = %2").arg(fmt(a)).arg(fmt(b));
    double d = dot(a,b);
    steps << QString("A·B = Ax×Bx + Ay×By + Az×Bz = %1×%2 + %3×%4 + %5×%6 = %7")
             .arg(a.x).arg(b.x).arg(a.y).arg(b.y).arg(a.z).arg(b.z).arg(d,0,'g',8);
    Vec3 c = cross(a,b);
    steps << QString("A×B = (AyBz−AzBy, AzBx−AxBz, AxBy−AyBx)");
    steps << QString("A×B = (%1×%2−%3×%4, %3×%5−%1×%6, %1×%4−%2×%3) = %7")
             .arg(a.y).arg(b.z).arg(a.z).arg(b.y).arg(b.x).arg(b.y).arg(fmt(c));
    m_dcResult->setText(QString("A·B = %1\nA×B = %2\n|A×B| = %3").arg(d,0,'g',8).arg(fmt(c)).arg(mag(c),0,'g',8));
    showSteps(m_dcSteps, m_dcShow, steps);
}

QWidget* VectorsWidget::buildMagUnitTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Magnitude |A| = √(Ax²+Ay²+Az²). Unit vector Â = A/|A| (direction only, magnitude=1).", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    addVecRow(g, 0, "A:", m_ax, m_ay, m_az, w);
    addVecRow(g, 1, "B:", m_bx, m_by, m_bz, w);
    v->addLayout(g);
    m_magResult = resultLbl(w); m_magShow = new QCheckBox("Show Steps", w); m_magSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_magResult);
    v->addWidget(m_magShow); v->addWidget(m_magSteps); v->addStretch();
    connect(m_magShow, &QCheckBox::toggled, this, [this](bool on){ m_magSteps->setVisible(on && !m_magSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &VectorsWidget::computeMagUnit);
    return w;
}

void VectorsWidget::computeMagUnit() {
    Vec3 a=readA(), b=readB();
    QStringList steps;
    double ma=mag(a), mb=mag(b);
    steps << QString("|A| = √(%1²+%2²+%3²) = %4").arg(a.x).arg(a.y).arg(a.z).arg(ma,0,'g',8);
    steps << QString("|B| = √(%1²+%2²+%3²) = %4").arg(b.x).arg(b.y).arg(b.z).arg(mb,0,'g',8);
    Vec3 ua=unit(a), ub=unit(b);
    steps << QString("Â = A/|A| = %1").arg(fmt(ua));
    steps << QString("B̂ = B/|B| = %1").arg(fmt(ub));
    m_magResult->setText(QString("|A| = %1\n|B| = %2\nÂ = %3\nB̂ = %4").arg(ma,0,'g',8).arg(mb,0,'g',8).arg(fmt(ua)).arg(fmt(ub)));
    showSteps(m_magSteps, m_magShow, steps);
}

QWidget* VectorsWidget::buildAngleProjTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Angle between vectors: θ = arccos(A·B / |A||B|). Projection of A onto B: proj = (A·B/|B|²)×B. Rejection = A − projection.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    addVecRow(g, 0, "A:", m_ax, m_ay, m_az, w);
    addVecRow(g, 1, "B:", m_bx, m_by, m_bz, w);
    v->addLayout(g);
    m_apResult = resultLbl(w); m_apShow = new QCheckBox("Show Steps", w); m_apSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_apResult);
    v->addWidget(m_apShow); v->addWidget(m_apSteps); v->addStretch();
    connect(m_apShow, &QCheckBox::toggled, this, [this](bool on){ m_apSteps->setVisible(on && !m_apSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &VectorsWidget::computeAngleProj);
    return w;
}

void VectorsWidget::computeAngleProj() {
    Vec3 a=readA(), b=readB();
    QStringList steps;
    double theta = angle(a,b);
    steps << QString("A·B = %1").arg(dot(a,b),0,'g',8);
    steps << QString("|A| = %1,  |B| = %2").arg(mag(a),0,'g',6).arg(mag(b),0,'g',6);
    steps << QString("θ = arccos(A·B / |A||B|) = %1°").arg(theta,0,'f',4);
    double mb2 = mag(b)*mag(b);
    Vec3 proj = mb2>0 ? scale(b, dot(a,b)/mb2) : Vec3{0,0,0};
    Vec3 rej  = sub(a, proj);
    steps << QString("proj_B(A) = (A·B/|B|²)×B = %1").arg(fmt(proj));
    steps << QString("rejection = A − proj = %1").arg(fmt(rej));
    m_apResult->setText(QString("Angle θ = %1°\nproj_B(A) = %2\nrejection = %3").arg(theta,0,'f',4).arg(fmt(proj)).arg(fmt(rej)));
    showSteps(m_apSteps, m_apShow, steps);
}

QWidget* VectorsWidget::buildLinePlaneTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Vector equation of a line through point P with direction A: r = P + tA. Equation of a plane through points P, Q, R: normal = PQ×PR, then n·(r−P)=0.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    addVecRow(g, 0, "Direction A:", m_ax, m_ay, m_az, w);
    addVecRow(g, 1, "Point P:", m_lpPx, m_lpPy, m_lpPz, w);
    addVecRow(g, 2, "Point Q:", m_lpQx, m_lpQy, m_lpQz, w);
    v->addLayout(g);
    m_lpResult = resultLbl(w); m_lpShow = new QCheckBox("Show Steps", w); m_lpSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_lpResult);
    v->addWidget(m_lpShow); v->addWidget(m_lpSteps); v->addStretch();
    connect(m_lpShow, &QCheckBox::toggled, this, [this](bool on){ m_lpSteps->setVisible(on && !m_lpSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &VectorsWidget::computeLinePlane);
    return w;
}

void VectorsWidget::computeLinePlane() {
    Vec3 dir=readA();
    Vec3 P={m_lpPx->text().toDouble(), m_lpPy->text().toDouble(), m_lpPz->text().toDouble()};
    Vec3 Q={m_lpQx->text().toDouble(), m_lpQy->text().toDouble(), m_lpQz->text().toDouble()};
    QStringList steps;
    // Line
    steps << QString("Line: r = P + tA = %1 + t%2").arg(fmt(P)).arg(fmt(dir));
    steps << QString("Parametric: x=%1+%2t, y=%3+%4t, z=%5+%6t").arg(P.x).arg(dir.x).arg(P.y).arg(dir.y).arg(P.z).arg(dir.z);
    // Plane using P, Q, and B as third point
    Vec3 R=readB();
    Vec3 PQ=sub(Q,P), PR=sub(R,P);
    Vec3 n=cross(PQ,PR);
    double d=dot(n,P);
    steps << QString("Plane through P=%1, Q=%2, R=%3").arg(fmt(P)).arg(fmt(Q)).arg(fmt(R));
    steps << QString("PQ = %1,  PR = %2").arg(fmt(PQ)).arg(fmt(PR));
    steps << QString("Normal n = PQ×PR = %1").arg(fmt(n));
    steps << QString("Plane equation: %1x + %2y + %3z = %4").arg(n.x,0,'g',4).arg(n.y,0,'g',4).arg(n.z,0,'g',4).arg(d,0,'g',6);
    m_lpResult->setText(QString(
        "Line: r = %1 + t%2\n\nPlane normal: n = %3\nPlane: %4x + %5y + %6z = %7")
        .arg(fmt(P)).arg(fmt(dir)).arg(fmt(n))
        .arg(n.x,0,'g',4).arg(n.y,0,'g',4).arg(n.z,0,'g',4).arg(d,0,'g',6));
    showSteps(m_lpSteps, m_lpShow, steps);
}

QWidget* VectorsWidget::buildDistanceTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Distance from a point to a line or plane. Point P is the query point. Vector A is the line direction or plane normal. Vector B is a point on the line/plane.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    addVecRow(g, 0, "Line dir / Normal A:", m_ax, m_ay, m_az, w);
    addVecRow(g, 1, "Point on line/plane B:", m_bx, m_by, m_bz, w);
    m_distPx=numEdit("3",w); m_distPy=numEdit("4",w); m_distPz=numEdit("0",w);
    g->addWidget(new QLabel("Query point P:", w), 2, 0);
    g->addWidget(new QLabel("x:", w), 2, 1); g->addWidget(m_distPx, 2, 2);
    g->addWidget(new QLabel("y:", w), 2, 3); g->addWidget(m_distPy, 2, 4);
    g->addWidget(new QLabel("z:", w), 2, 5); g->addWidget(m_distPz, 2, 6);
    m_distType = new QComboBox(w); m_distType->addItems({"Point to Line", "Point to Plane"});
    g->addWidget(new QLabel("Type:", w), 3, 0); g->addWidget(m_distType, 3, 1, 1, 3);
    v->addLayout(g);
    m_distResult = resultLbl(w); m_distShow = new QCheckBox("Show Steps", w); m_distSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_distResult);
    v->addWidget(m_distShow); v->addWidget(m_distSteps); v->addStretch();
    connect(m_distShow, &QCheckBox::toggled, this, [this](bool on){ m_distSteps->setVisible(on && !m_distSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &VectorsWidget::computeDistance);
    return w;
}

void VectorsWidget::computeDistance() {
    Vec3 dir=readA(), B=readB();
    Vec3 P={m_distPx->text().toDouble(), m_distPy->text().toDouble(), m_distPz->text().toDouble()};
    QStringList steps;
    if (m_distType->currentIndex() == 0) {
        // Point to line: d = |BP × dir| / |dir|
        Vec3 BP = sub(P, B);
        Vec3 c  = cross(BP, dir);
        double d = mag(dir) > 0 ? mag(c)/mag(dir) : 0;
        steps << QString("BP = P − B = %1").arg(fmt(BP));
        steps << QString("BP × dir = %1").arg(fmt(c));
        steps << QString("d = |BP × dir| / |dir| = %1 / %2 = %3").arg(mag(c),0,'g',6).arg(mag(dir),0,'g',6).arg(d,0,'g',8);
        m_distResult->setText(QString("Distance from P to line = %1").arg(d,0,'g',8));
    } else {
        // Point to plane: d = |n·(P−B)| / |n|
        Vec3 BP = sub(P, B);
        double d = mag(dir) > 0 ? std::abs(dot(dir, BP))/mag(dir) : 0;
        steps << QString("n = %1 (plane normal)").arg(fmt(dir));
        steps << QString("P − B = %1").arg(fmt(BP));
        steps << QString("d = |n·(P−B)| / |n| = |%1| / %2 = %3").arg(dot(dir,BP),0,'g',6).arg(mag(dir),0,'g',6).arg(d,0,'g',8);
        m_distResult->setText(QString("Distance from P to plane = %1").arg(d,0,'g',8));
    }
    showSteps(m_distSteps, m_distShow, steps);
}
