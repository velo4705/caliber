#include "calculus_widget.h"
#include "../../modes/graphing/function_parser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QDoubleValidator>
#include <cmath>
#include <stdexcept>

// ── shared helpers ────────────────────────────────────────────────────────────
static QPushButton* mkBtn(const QString& t, QWidget* p) {
    auto* b = new QPushButton(t, p);
    b->setProperty("class", "actionButton");
    b->setMinimumHeight(36);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}
static QLineEdit* exprEdit(const QString& ph, QWidget* p) {
    auto* e = new QLineEdit(p);
    e->setPlaceholderText(ph);
    return e;
}
static QLineEdit* numEdit(const QString& def, QWidget* p) {
    auto* e = new QLineEdit(def, p);
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,p));
    e->setFixedWidth(80);
    return e;
}
static QLabel* resultLbl(QWidget* p) {
    auto* l = new QLabel("—", p);
    l->setObjectName("resultLabel");
    l->setWordWrap(true);
    l->setStyleSheet("font-size:14px; font-weight:bold; padding:8px;");
    return l;
}
static QTextEdit* stepsBox(QWidget* p) {
    auto* t = new QTextEdit(p);
    t->setReadOnly(true);
    t->setMinimumHeight(100); t->setMaximumHeight(180);
    t->setStyleSheet("font-family:monospace; font-size:12px;");
    t->hide();
    return t;
}
static QLabel* descLbl(const QString& text, QWidget* p) {
    auto* l = new QLabel(text, p);
    l->setWordWrap(true);
    l->setStyleSheet("color:gray; font-size:12px; padding:4px 0;");
    return l;
}

void CalculusWidget::showSteps(QTextEdit* w, QCheckBox* toggle, const QStringList& steps) {
    if (!toggle->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < steps.size(); ++i)
        w->append(QString("Step %1: %2").arg(i+1).arg(steps[i]));
    w->show();
}

double CalculusWidget::evalExpr(const QString& expr, double x, double y, double z) {
    FunctionParser parser;
    // Substitute z first (before y to avoid partial replacement)
    QString e = expr;
    e.replace("z", QString::number(z, 'g', 15));
    return parser.evaluate(e, x, y);
}

// ── Constructor ───────────────────────────────────────────────────────────────
CalculusWidget::CalculusWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Calculus", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildDiffTab(),        "Differentiation");
    tabs->addTab(buildIntegTab(),       "Integration");
    tabs->addTab(buildDoubleIntegTab(), "Double Integral");
    tabs->addTab(buildTripleIntegTab(), "Triple Integral");
    tabs->addTab(buildPartialTab(),     "Partial Deriv.");
    tabs->addTab(buildLimitTab(),       "Limits");
    tabs->addTab(buildTaylorTab(),      "Taylor Series");
    tabs->addTab(buildRootTab(),        "Root Finder");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Tab: Differentiation ──────────────────────────────────────────────────────
QWidget* CalculusWidget::buildDiffTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Numerically computes f'(x) and f''(x) at a given point using the central difference method. Enter a function of x.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_diffExpr = exprEdit("e.g. sin(x) or x^3 - 2*x", w);
    m_diffX    = numEdit("1", w);
    m_diffH    = numEdit("1e-5", w);
    g->addWidget(new QLabel("f(x) =", w), 0, 0); g->addWidget(m_diffExpr, 0, 1, 1, 3);
    g->addWidget(new QLabel("x =",    w), 1, 0); g->addWidget(m_diffX,    1, 1);
    g->addWidget(new QLabel("h =",    w), 1, 2); g->addWidget(m_diffH,    1, 3);
    v->addLayout(g);
    m_diffResult    = resultLbl(w);
    m_diffShowSteps = new QCheckBox("Show Steps", w);
    m_diffSteps     = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_diffResult);
    v->addWidget(m_diffShowSteps); v->addWidget(m_diffSteps); v->addStretch();
    connect(m_diffShowSteps, &QCheckBox::toggled, this, [this](bool on){ m_diffSteps->setVisible(on && !m_diffSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &CalculusWidget::computeDiff);
    return w;
}

void CalculusWidget::computeDiff() {
    QString expr = m_diffExpr->text().trimmed();
    if (expr.isEmpty()) { m_diffResult->setText("Enter a function."); return; }
    double x = m_diffX->text().toDouble();
    double h = m_diffH->text().toDouble();
    if (h <= 0) h = 1e-5;
    QStringList steps;
    steps << QString("f(x) = %1,  x = %2,  h = %3").arg(expr).arg(x).arg(h);
    try {
        double fx  = evalExpr(expr, x);
        double fxh = evalExpr(expr, x+h);
        double fxmh= evalExpr(expr, x-h);
        double fxh2= evalExpr(expr, x+2*h);
        double fxmh2=evalExpr(expr, x-2*h);
        steps << QString("f(x) = %1").arg(fx, 0,'g',10);
        steps << QString("f(x+h) = %1,  f(x-h) = %2").arg(fxh,0,'g',10).arg(fxmh,0,'g',10);
        double d1 = (fxh - fxmh) / (2*h);
        steps << QString("f'(x) ≈ [f(x+h) − f(x−h)] / 2h = %1").arg(d1,0,'g',10);
        double d2 = (fxh - 2*fx + fxmh) / (h*h);
        steps << QString("f''(x) ≈ [f(x+h) − 2f(x) + f(x−h)] / h² = %1").arg(d2,0,'g',10);
        // Higher order check
        double d1_4 = (-fxh2 + 8*fxh - 8*fxmh + fxmh2) / (12*h);
        steps << QString("f'(x) (5-point, more accurate) = %1").arg(d1_4,0,'g',10);
        m_diffResult->setText(QString("f'(%1)  = %2\nf''(%1) = %3").arg(x,0,'g',6).arg(d1,0,'g',10).arg(d2,0,'g',10));
    } catch (...) { m_diffResult->setText("Error evaluating expression."); }
    showSteps(m_diffSteps, m_diffShowSteps, steps);
}

// ── Tab: Integration (Simpson's Rule) ────────────────────────────────────────
QWidget* CalculusWidget::buildIntegTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes the definite integral ∫f(x)dx from a to b using Simpson's 1/3 rule. Higher n = more accurate. n must be even.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_integExpr = exprEdit("e.g. x^2 or sin(x)", w);
    m_integA    = numEdit("0", w); m_integB = numEdit("1", w);
    m_integN    = numEdit("1000", w);
    g->addWidget(new QLabel("f(x) =", w), 0, 0); g->addWidget(m_integExpr, 0, 1, 1, 3);
    g->addWidget(new QLabel("a =",    w), 1, 0); g->addWidget(m_integA,    1, 1);
    g->addWidget(new QLabel("b =",    w), 1, 2); g->addWidget(m_integB,    1, 3);
    g->addWidget(new QLabel("n =",    w), 2, 0); g->addWidget(m_integN,    2, 1);
    v->addLayout(g);
    m_integResult    = resultLbl(w);
    m_integShowSteps = new QCheckBox("Show Steps", w);
    m_integSteps     = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_integResult);
    v->addWidget(m_integShowSteps); v->addWidget(m_integSteps); v->addStretch();
    connect(m_integShowSteps, &QCheckBox::toggled, this, [this](bool on){ m_integSteps->setVisible(on && !m_integSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &CalculusWidget::computeInteg);
    return w;
}

void CalculusWidget::computeInteg() {
    QString expr = m_integExpr->text().trimmed();
    if (expr.isEmpty()) { m_integResult->setText("Enter a function."); return; }
    double a = m_integA->text().toDouble();
    double b = m_integB->text().toDouble();
    int n = qMax(2, (int)m_integN->text().toDouble());
    if (n % 2 != 0) n++;
    QStringList steps;
    steps << QString("∫f(x)dx from %1 to %2,  n = %3 subintervals").arg(a).arg(b).arg(n);
    steps << "Using Simpson's 1/3 rule: h/3 × [f(a) + 4f(x₁) + 2f(x₂) + ... + f(b)]";
    try {
        double h = (b - a) / n;
        steps << QString("h = (b−a)/n = (%1−%2)/%3 = %4").arg(b).arg(a).arg(n).arg(h,0,'g',8);
        double sum = evalExpr(expr, a) + evalExpr(expr, b);
        for (int i = 1; i < n; i++) {
            double xi = a + i * h;
            sum += (i % 2 == 0 ? 2 : 4) * evalExpr(expr, xi);
        }
        double result = h / 3.0 * sum;
        steps << QString("Result = h/3 × Σ = %1").arg(result, 0,'g',12);
        m_integResult->setText(QString("∫f(x)dx = %1").arg(result, 0,'g',12));
    } catch (...) { m_integResult->setText("Error evaluating expression."); }
    showSteps(m_integSteps, m_integShowSteps, steps);
}

// ── Tab: Double Integration ───────────────────────────────────────────────────
QWidget* CalculusWidget::buildDoubleIntegTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes ∬f(x,y)dxdy over a rectangular region [xa,xb]×[ya,yb] using nested Simpson's rule. Use x and y as variables.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_dblExpr = exprEdit("e.g. x^2 + y^2", w);
    m_dblXa = numEdit("0",w); m_dblXb = numEdit("1",w);
    m_dblYa = numEdit("0",w); m_dblYb = numEdit("1",w);
    m_dblN  = numEdit("50",w);
    g->addWidget(new QLabel("f(x,y) =",w),0,0); g->addWidget(m_dblExpr,0,1,1,3);
    g->addWidget(new QLabel("x: a =",w),1,0); g->addWidget(m_dblXa,1,1);
    g->addWidget(new QLabel("b =",w),1,2);     g->addWidget(m_dblXb,1,3);
    g->addWidget(new QLabel("y: a =",w),2,0); g->addWidget(m_dblYa,2,1);
    g->addWidget(new QLabel("b =",w),2,2);     g->addWidget(m_dblYb,2,3);
    g->addWidget(new QLabel("n =",w),3,0);     g->addWidget(m_dblN,3,1);
    v->addLayout(g);
    m_dblResult    = resultLbl(w);
    m_dblShowSteps = new QCheckBox("Show Steps", w);
    m_dblSteps     = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_dblResult);
    v->addWidget(m_dblShowSteps); v->addWidget(m_dblSteps); v->addStretch();
    connect(m_dblShowSteps, &QCheckBox::toggled, this, [this](bool on){ m_dblSteps->setVisible(on && !m_dblSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &CalculusWidget::computeDoubleInteg);
    return w;
}

void CalculusWidget::computeDoubleInteg() {
    QString expr = m_dblExpr->text().trimmed();
    if (expr.isEmpty()) { m_dblResult->setText("Enter a function."); return; }
    double xa=m_dblXa->text().toDouble(), xb=m_dblXb->text().toDouble();
    double ya=m_dblYa->text().toDouble(), yb=m_dblYb->text().toDouble();
    int n = qMax(2,(int)m_dblN->text().toDouble()); if(n%2) n++;
    QStringList steps;
    steps << QString("∬f(x,y)dxdy over [%1,%2]×[%3,%4], n=%5").arg(xa).arg(xb).arg(ya).arg(yb).arg(n);
    steps << "Nested Simpson's rule: integrate over y first, then x";
    try {
        double hx=(xb-xa)/n, hy=(yb-ya)/n;
        steps << QString("hx = %1,  hy = %2").arg(hx,0,'g',6).arg(hy,0,'g',6);
        double total = 0;
        for(int i=0;i<=n;i++){
            double x=xa+i*hx, wx=(i==0||i==n)?1:(i%2?4:2);
            double inner=0;
            for(int j=0;j<=n;j++){
                double y=ya+j*hy, wy=(j==0||j==n)?1:(j%2?4:2);
                inner += wy * evalExpr(expr,x,y);
            }
            total += wx * (hy/3.0) * inner;
        }
        double result = (hx/3.0) * total;
        steps << QString("Result = %1").arg(result,0,'g',12);
        m_dblResult->setText(QString("∬f(x,y)dxdy = %1").arg(result,0,'g',12));
    } catch(...){ m_dblResult->setText("Error evaluating expression."); }
    showSteps(m_dblSteps, m_dblShowSteps, steps);
}

// ── Tab: Triple Integration ───────────────────────────────────────────────────
QWidget* CalculusWidget::buildTripleIntegTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes ∭f(x,y,z)dxdydz over a rectangular region using nested Simpson's rule. Use x, y, z as variables. Use smaller n (e.g. 20) for speed.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_triExpr = exprEdit("e.g. x*y*z", w);
    m_triXa=numEdit("0",w); m_triXb=numEdit("1",w);
    m_triYa=numEdit("0",w); m_triYb=numEdit("1",w);
    m_triZa=numEdit("0",w); m_triZb=numEdit("1",w);
    m_triN =numEdit("20",w);
    g->addWidget(new QLabel("f(x,y,z)=",w),0,0); g->addWidget(m_triExpr,0,1,1,3);
    g->addWidget(new QLabel("x: a=",w),1,0); g->addWidget(m_triXa,1,1); g->addWidget(new QLabel("b=",w),1,2); g->addWidget(m_triXb,1,3);
    g->addWidget(new QLabel("y: a=",w),2,0); g->addWidget(m_triYa,2,1); g->addWidget(new QLabel("b=",w),2,2); g->addWidget(m_triYb,2,3);
    g->addWidget(new QLabel("z: a=",w),3,0); g->addWidget(m_triZa,3,1); g->addWidget(new QLabel("b=",w),3,2); g->addWidget(m_triZb,3,3);
    g->addWidget(new QLabel("n=",w),4,0); g->addWidget(m_triN,4,1);
    v->addLayout(g);
    m_triResult    = resultLbl(w);
    m_triShowSteps = new QCheckBox("Show Steps", w);
    m_triSteps     = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_triResult);
    v->addWidget(m_triShowSteps); v->addWidget(m_triSteps); v->addStretch();
    connect(m_triShowSteps, &QCheckBox::toggled, this, [this](bool on){ m_triSteps->setVisible(on && !m_triSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &CalculusWidget::computeTripleInteg);
    return w;
}

void CalculusWidget::computeTripleInteg() {
    QString expr = m_triExpr->text().trimmed();
    if(expr.isEmpty()){ m_triResult->setText("Enter a function."); return; }
    double xa=m_triXa->text().toDouble(),xb=m_triXb->text().toDouble();
    double ya=m_triYa->text().toDouble(),yb=m_triYb->text().toDouble();
    double za=m_triZa->text().toDouble(),zb=m_triZb->text().toDouble();
    int n=qMax(2,(int)m_triN->text().toDouble()); if(n%2)n++;
    QStringList steps;
    steps << QString("∭f(x,y,z)dxdydz over [%1,%2]×[%3,%4]×[%5,%6], n=%7").arg(xa).arg(xb).arg(ya).arg(yb).arg(za).arg(zb).arg(n);
    steps << "Nested Simpson's rule over three dimensions";
    try {
        double hx=(xb-xa)/n, hy=(yb-ya)/n, hz=(zb-za)/n;
        steps << QString("hx=%1, hy=%2, hz=%3").arg(hx,0,'g',4).arg(hy,0,'g',4).arg(hz,0,'g',4);
        double total=0;
        for(int i=0;i<=n;i++){
            double x=xa+i*hx, wx=(i==0||i==n)?1:(i%2?4:2);
            for(int j=0;j<=n;j++){
                double y=ya+j*hy, wy=(j==0||j==n)?1:(j%2?4:2);
                for(int k=0;k<=n;k++){
                    double z=za+k*hz, wz=(k==0||k==n)?1:(k%2?4:2);
                    total += wx*wy*wz*evalExpr(expr,x,y,z);
                }
            }
        }
        double result=(hx*hy*hz/27.0)*total;
        steps << QString("Result = %1").arg(result,0,'g',12);
        m_triResult->setText(QString("∭f(x,y,z)dxdydz = %1").arg(result,0,'g',12));
    } catch(...){ m_triResult->setText("Error evaluating expression."); }
    showSteps(m_triSteps, m_triShowSteps, steps);
}

// ── Tab: Partial Derivatives ──────────────────────────────────────────────────
QWidget* CalculusWidget::buildPartialTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Computes ∂f/∂x and ∂f/∂y at a given point (x,y) using central differences. Use x and y as variables in your function.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_partExpr = exprEdit("e.g. x^2 + y^2 or sin(x)*cos(y)", w);
    m_partX = numEdit("1",w); m_partY = numEdit("1",w); m_partH = numEdit("1e-5",w);
    g->addWidget(new QLabel("f(x,y) =",w),0,0); g->addWidget(m_partExpr,0,1,1,3);
    g->addWidget(new QLabel("x =",w),1,0); g->addWidget(m_partX,1,1);
    g->addWidget(new QLabel("y =",w),1,2); g->addWidget(m_partY,1,3);
    g->addWidget(new QLabel("h =",w),2,0); g->addWidget(m_partH,2,1);
    v->addLayout(g);
    m_partResult    = resultLbl(w);
    m_partShowSteps = new QCheckBox("Show Steps", w);
    m_partSteps     = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_partResult);
    v->addWidget(m_partShowSteps); v->addWidget(m_partSteps); v->addStretch();
    connect(m_partShowSteps, &QCheckBox::toggled, this, [this](bool on){ m_partSteps->setVisible(on && !m_partSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &CalculusWidget::computePartial);
    return w;
}

void CalculusWidget::computePartial() {
    QString expr = m_partExpr->text().trimmed();
    if(expr.isEmpty()){ m_partResult->setText("Enter a function."); return; }
    double x=m_partX->text().toDouble(), y=m_partY->text().toDouble();
    double h=m_partH->text().toDouble(); if(h<=0) h=1e-5;
    QStringList steps;
    steps << QString("f(x,y) = %1  at  (x=%2, y=%3)").arg(expr).arg(x).arg(y);
    try {
        double dfdx = (evalExpr(expr,x+h,y) - evalExpr(expr,x-h,y)) / (2*h);
        double dfdy = (evalExpr(expr,x,y+h) - evalExpr(expr,x,y-h)) / (2*h);
        steps << QString("∂f/∂x ≈ [f(x+h,y) − f(x−h,y)] / 2h = %1").arg(dfdx,0,'g',10);
        steps << QString("∂f/∂y ≈ [f(x,y+h) − f(x,y−h)] / 2h = %1").arg(dfdy,0,'g',10);
        // Mixed partial
        double d2fdxdy = (evalExpr(expr,x+h,y+h) - evalExpr(expr,x+h,y-h)
                        - evalExpr(expr,x-h,y+h) + evalExpr(expr,x-h,y-h)) / (4*h*h);
        steps << QString("∂²f/∂x∂y ≈ %1").arg(d2fdxdy,0,'g',10);
        m_partResult->setText(QString("∂f/∂x = %1\n∂f/∂y = %2\n∂²f/∂x∂y = %3")
            .arg(dfdx,0,'g',10).arg(dfdy,0,'g',10).arg(d2fdxdy,0,'g',10));
    } catch(...){ m_partResult->setText("Error evaluating expression."); }
    showSteps(m_partSteps, m_partShowSteps, steps);
}

// ── Tab: Limits ───────────────────────────────────────────────────────────────
QWidget* CalculusWidget::buildLimitTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Estimates lim(x→c) f(x) by evaluating f at values approaching c from both sides. Also detects if the limit does not exist (left ≠ right).", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_limExpr = exprEdit("e.g. sin(x)/x or (x^2-1)/(x-1)", w);
    m_limC    = numEdit("0", w);
    m_limH    = numEdit("1e-6", w);
    g->addWidget(new QLabel("f(x) =",w),0,0); g->addWidget(m_limExpr,0,1,1,3);
    g->addWidget(new QLabel("c =",   w),1,0); g->addWidget(m_limC,   1,1);
    g->addWidget(new QLabel("h =",   w),1,2); g->addWidget(m_limH,   1,3);
    v->addLayout(g);
    m_limResult    = resultLbl(w);
    m_limShowSteps = new QCheckBox("Show Steps", w);
    m_limSteps     = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_limResult);
    v->addWidget(m_limShowSteps); v->addWidget(m_limSteps); v->addStretch();
    connect(m_limShowSteps, &QCheckBox::toggled, this, [this](bool on){ m_limSteps->setVisible(on && !m_limSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &CalculusWidget::computeLimit);
    return w;
}

void CalculusWidget::computeLimit() {
    QString expr = m_limExpr->text().trimmed();
    if(expr.isEmpty()){ m_limResult->setText("Enter a function."); return; }
    double c=m_limC->text().toDouble(), h=m_limH->text().toDouble();
    if(h<=0) h=1e-6;
    QStringList steps;
    steps << QString("lim(x→%1) %2").arg(c).arg(expr);
    try {
        // Approach from both sides with decreasing h
        double left=0, right=0;
        for(double d : {h*100, h*10, h}) {
            left  = evalExpr(expr, c-d);
            right = evalExpr(expr, c+d);
            steps << QString("x = %1: f = %2  |  x = %3: f = %4")
                     .arg(c-d,0,'g',6).arg(left,0,'g',8).arg(c+d,0,'g',6).arg(right,0,'g',8);
        }
        if(std::abs(left-right) < 1e-6) {
            double lim = (left+right)/2.0;
            steps << QString("Left limit ≈ right limit → lim = %1").arg(lim,0,'g',10);
            m_limResult->setText(QString("lim(x→%1) f(x) = %2").arg(c,0,'g',6).arg(lim,0,'g',10));
        } else {
            steps << QString("Left limit (%1) ≠ right limit (%2) → limit does not exist").arg(left,0,'g',8).arg(right,0,'g',8);
            m_limResult->setText("Limit does not exist (left ≠ right)");
        }
    } catch(...){ m_limResult->setText("Error evaluating expression."); }
    showSteps(m_limSteps, m_limShowSteps, steps);
}

// ── Tab: Taylor Series ────────────────────────────────────────────────────────
QWidget* CalculusWidget::buildTaylorTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Approximates f(x) as a Taylor polynomial around point a up to order n. Shows each term and the approximation value at a given x.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_tayExpr = exprEdit("e.g. sin(x) or e^x", w);
    m_tayA    = numEdit("0", w);
    m_tayX    = numEdit("1", w);
    m_tayN    = new QSpinBox(w); m_tayN->setRange(1,10); m_tayN->setValue(5);
    g->addWidget(new QLabel("f(x) =",w),0,0); g->addWidget(m_tayExpr,0,1,1,3);
    g->addWidget(new QLabel("a =",   w),1,0); g->addWidget(m_tayA,   1,1);
    g->addWidget(new QLabel("x =",   w),1,2); g->addWidget(m_tayX,   1,3);
    g->addWidget(new QLabel("order n =",w),2,0); g->addWidget(m_tayN, 2,1);
    v->addLayout(g);
    m_tayResult    = resultLbl(w);
    m_tayShowSteps = new QCheckBox("Show Steps", w);
    m_taySteps     = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_tayResult);
    v->addWidget(m_tayShowSteps); v->addWidget(m_taySteps); v->addStretch();
    connect(m_tayShowSteps, &QCheckBox::toggled, this, [this](bool on){ m_taySteps->setVisible(on && !m_taySteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &CalculusWidget::computeTaylor);
    return w;
}

void CalculusWidget::computeTaylor() {
    QString expr = m_tayExpr->text().trimmed();
    if(expr.isEmpty()){ m_tayResult->setText("Enter a function."); return; }
    double a=m_tayA->text().toDouble(), x=m_tayX->text().toDouble();
    int n=m_tayN->value();
    double h=1e-5;
    QStringList steps;
    steps << QString("Taylor series of f(x) = %1 around a = %2, order %3").arg(expr).arg(a).arg(n);
    steps << "P(x) = Σ f⁽ⁿ⁾(a)/n! × (x−a)ⁿ";
    try {
        // Compute derivatives numerically using central differences
        QVector<double> derivs(n+1);
        derivs[0] = evalExpr(expr, a);
        // Use finite differences for higher derivatives
        QVector<double> vals(2*n+3);
        for(int i=0;i<(int)vals.size();i++)
            vals[i] = evalExpr(expr, a + (i-n-1)*h);
        // Compute nth derivative via finite difference table
        QVector<double> diff = vals;
        for(int k=1;k<=n;k++){
            QVector<double> next(diff.size()-1);
            for(int i=0;i<(int)next.size();i++)
                next[i]=(diff[i+1]-diff[i])/h;
            diff=next;
        }
        double approx=0; double factorial=1;
        for(int k=0;k<=n;k++){
            if(k>0) factorial*=k;
            double dk = k==0 ? derivs[0] : diff[diff.size()/2];
            if(k>0){
                // recompute kth derivative properly
                QVector<double> d2=vals;
                for(int j=0;j<k;j++){
                    QVector<double> nd(d2.size()-1);
                    for(int i=0;i<(int)nd.size();i++) nd[i]=(d2[i+1]-d2[i])/h;
                    d2=nd;
                }
                dk=d2[d2.size()/2];
            }
            double term = dk/factorial * std::pow(x-a, k);
            approx += term;
            steps << QString("Term %1: f^(%1)(a)/%1! × (x−a)^%1 = %2/%3 × %4 = %5")
                     .arg(k).arg(dk,0,'g',6).arg(factorial,0,'g',4)
                     .arg(std::pow(x-a,k),0,'g',4).arg(term,0,'g',8);
        }
        steps << QString("P(%1) ≈ %2").arg(x,0,'g',6).arg(approx,0,'g',10);
        double exact = evalExpr(expr, x);
        steps << QString("Exact f(%1) = %2,  error = %3").arg(x,0,'g',6).arg(exact,0,'g',10).arg(std::abs(exact-approx),0,'g',6);
        m_tayResult->setText(QString("Taylor approximation P(%1) = %2\nExact f(%1) = %3\nError = %4")
            .arg(x,0,'g',6).arg(approx,0,'g',10).arg(exact,0,'g',10).arg(std::abs(exact-approx),0,'g',6));
    } catch(...){ m_tayResult->setText("Error evaluating expression."); }
    showSteps(m_taySteps, m_tayShowSteps, steps);
}

// ── Tab: Root Finder ──────────────────────────────────────────────────────────
QWidget* CalculusWidget::buildRootTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Finds a root of f(x) = 0 in interval [a,b]. Bisection: always converges, slower. Newton-Raphson: faster but needs a good starting point (uses midpoint).", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_rootExpr = exprEdit("e.g. x^3 - x - 2", w);
    m_rootA    = numEdit("1", w); m_rootB = numEdit("2", w);
    m_rootMethod = new QComboBox(w);
    m_rootMethod->addItems({"Bisection", "Newton-Raphson"});
    g->addWidget(new QLabel("f(x) =",w),0,0); g->addWidget(m_rootExpr,0,1,1,3);
    g->addWidget(new QLabel("a =",   w),1,0); g->addWidget(m_rootA,   1,1);
    g->addWidget(new QLabel("b =",   w),1,2); g->addWidget(m_rootB,   1,3);
    g->addWidget(new QLabel("Method:",w),2,0); g->addWidget(m_rootMethod,2,1);
    v->addLayout(g);
    m_rootResult    = resultLbl(w);
    m_rootShowSteps = new QCheckBox("Show Steps", w);
    m_rootSteps     = stepsBox(w);
    v->addWidget(mkBtn("Find Root", w)); v->addWidget(m_rootResult);
    v->addWidget(m_rootShowSteps); v->addWidget(m_rootSteps); v->addStretch();
    connect(m_rootShowSteps, &QCheckBox::toggled, this, [this](bool on){ m_rootSteps->setVisible(on && !m_rootSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &CalculusWidget::computeRoot);
    return w;
}

void CalculusWidget::computeRoot() {
    QString expr = m_rootExpr->text().trimmed();
    if(expr.isEmpty()){ m_rootResult->setText("Enter a function."); return; }
    double a=m_rootA->text().toDouble(), b=m_rootB->text().toDouble();
    QStringList steps;
    try {
        if(m_rootMethod->currentIndex()==0) {
            // Bisection
            steps << QString("Bisection method on [%1, %2]").arg(a).arg(b);
            double fa=evalExpr(expr,a), fb=evalExpr(expr,b);
            if(fa*fb > 0){ m_rootResult->setText("f(a) and f(b) must have opposite signs."); return; }
            double mid=a;
            for(int i=0;i<60;i++){
                mid=(a+b)/2.0;
                double fm=evalExpr(expr,mid);
                if(i<8) steps << QString("Iter %1: [%2, %3], mid=%4, f(mid)=%5").arg(i+1).arg(a,0,'g',6).arg(b,0,'g',6).arg(mid,0,'g',8).arg(fm,0,'g',6);
                if(std::abs(fm)<1e-12 || (b-a)/2<1e-12) break;
                if(fa*fm<0){ b=mid; fb=fm; } else { a=mid; fa=fm; }
            }
            steps << QString("Root ≈ %1,  f(root) = %2").arg(mid,0,'g',12).arg(evalExpr(expr,mid),0,'g',6);
            m_rootResult->setText(QString("Root = %1\nf(root) = %2").arg(mid,0,'g',12).arg(evalExpr(expr,mid),0,'g',8));
        } else {
            // Newton-Raphson
            double x=(a+b)/2.0, h=1e-7;
            steps << QString("Newton-Raphson starting at x₀ = %1").arg(x,0,'g',6);
            for(int i=0;i<50;i++){
                double fx=evalExpr(expr,x);
                double fpx=(evalExpr(expr,x+h)-evalExpr(expr,x-h))/(2*h);
                if(std::abs(fpx)<1e-15){ steps << "f'(x) ≈ 0, stopping"; break; }
                double xn=x-fx/fpx;
                if(i<8) steps << QString("Iter %1: x=%2, f(x)=%3, f'(x)=%4, x_new=%5").arg(i+1).arg(x,0,'g',8).arg(fx,0,'g',6).arg(fpx,0,'g',6).arg(xn,0,'g',8);
                if(std::abs(xn-x)<1e-12) { x=xn; break; }
                x=xn;
            }
            steps << QString("Root ≈ %1,  f(root) = %2").arg(x,0,'g',12).arg(evalExpr(expr,x),0,'g',6);
            m_rootResult->setText(QString("Root = %1\nf(root) = %2").arg(x,0,'g',12).arg(evalExpr(expr,x),0,'g',8));
        }
    } catch(...){ m_rootResult->setText("Error evaluating expression."); }
    showSteps(m_rootSteps, m_rootShowSteps, steps);
}
