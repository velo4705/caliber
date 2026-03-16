#include "equations_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>
#include <QDoubleValidator>
#include <cmath>
#include <stdexcept>

// ── shared helpers ────────────────────────────────────────────────────────────
static QFrame* card(QWidget* p) {
    auto* f = new QFrame(p);
    f->setObjectName("displayWidget");
    f->setFrameShape(QFrame::StyledPanel);
    return f;
}
static QPushButton* btn(const QString& t, const QString& cls, QWidget* p) {
    auto* b = new QPushButton(t, p);
    b->setProperty("class", cls);
    b->setMinimumHeight(38);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}
static QLineEdit* coefEdit(QWidget* p) {
    auto* e = new QLineEdit("0", p);
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,p));
    e->setFixedWidth(80);
    e->setAlignment(Qt::AlignCenter);
    return e;
}
static QLabel* resultLbl(QWidget* p) {
    auto* l = new QLabel("—", p);
    l->setObjectName("resultLabel");
    l->setAlignment(Qt::AlignCenter);
    l->setWordWrap(true);
    l->setStyleSheet("font-size:16px; font-weight:bold; padding:10px;");
    return l;
}
static QTableWidget* makeTable(int rows, int cols, QWidget* p) {
    auto* t = new QTableWidget(rows, cols, p);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->setMaximumHeight(rows * 36 + 28);
    return t;
}

// ── constructor ───────────────────────────────────────────────────────────────
EquationsWidget::EquationsWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(12);
    auto* title = new QLabel("Equations & Matrices", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->addTab(buildLinearTab(),    "Linear");
    tabs->addTab(buildQuadraticTab(), "Quadratic");
    tabs->addTab(buildSystemTab(),    "System");
    tabs->addTab(buildMatrixTab(),    "Matrix");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Tab 1: Linear ─────────────────────────────────────────────────────────────
QWidget* EquationsWidget::buildLinearTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(10);

    // 1-variable: ax + b = 0
    auto* c1 = card(w);
    auto* g1 = new QHBoxLayout(c1);
    g1->setContentsMargins(12,10,12,10);
    m_linA = coefEdit(w); m_linB = coefEdit(w);
    g1->addWidget(m_linA); g1->addWidget(new QLabel("x  +", w));
    g1->addWidget(m_linB); g1->addWidget(new QLabel("= 0", w));
    g1->addStretch();
    v->addWidget(new QLabel("One variable:  ax + b = 0", w));
    v->addWidget(c1);

    // 2-variable: ax+by=c, dx+ey=f
    auto* c2 = card(w);
    auto* g2 = new QGridLayout(c2);
    g2->setContentsMargins(12,10,12,10); g2->setSpacing(6);
    m_lin2A=coefEdit(w); m_lin2B=coefEdit(w); m_lin2C=coefEdit(w);
    m_lin2D=coefEdit(w);
    auto* m_lin2E=coefEdit(w); auto* m_lin2F=coefEdit(w);
    // store E and F via lambda capture
    g2->addWidget(m_lin2A,0,0); g2->addWidget(new QLabel("x +",w),0,1);
    g2->addWidget(m_lin2B,0,2); g2->addWidget(new QLabel("y =",w),0,3);
    g2->addWidget(m_lin2C,0,4);
    g2->addWidget(m_lin2D,1,0); g2->addWidget(new QLabel("x +",w),1,1);
    g2->addWidget(m_lin2E,1,2); g2->addWidget(new QLabel("y =",w),1,3);
    g2->addWidget(m_lin2F,1,4);
    v->addWidget(new QLabel("Two variables:  ax+by=c  /  dx+ey=f", w));
    v->addWidget(c2);

    m_linResult = resultLbl(w);
    auto* solveBtn = btn("Solve", "actionButton", w);
    v->addWidget(solveBtn);
    v->addWidget(m_linResult);
    v->addStretch();

    connect(solveBtn, &QPushButton::clicked, this, [this, m_lin2E, m_lin2F]() {
        QString out;
        // 1-var
        double a = m_linA->text().toDouble();
        double b = m_linB->text().toDouble();
        if (a != 0) out += QString("1-var:  x = %1\n").arg(-b/a);
        else        out += QString("1-var:  %1\n").arg(b==0?"Infinite solutions":"No solution");
        // 2-var
        double A=m_lin2A->text().toDouble(), B=m_lin2B->text().toDouble(), C=m_lin2C->text().toDouble();
        double D=m_lin2D->text().toDouble(), E=m_lin2E->text().toDouble(), F=m_lin2F->text().toDouble();
        double det = A*E - B*D;
        if (std::abs(det) < 1e-12) {
            out += "2-var:  No unique solution (det=0)";
        } else {
            double x = (C*E - B*F)/det;
            double y = (A*F - C*D)/det;
            out += QString("2-var:  x = %1,  y = %2").arg(x,0,'g',8).arg(y,0,'g',8);
        }
        m_linResult->setText(out);
    });
    return w;
}

// ── Tab 2: Quadratic ──────────────────────────────────────────────────────────
QWidget* EquationsWidget::buildQuadraticTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(10);

    auto* c = card(w);
    auto* row = new QHBoxLayout(c);
    row->setContentsMargins(12,10,12,10);
    m_quadA=coefEdit(w); m_quadB=coefEdit(w); m_quadC=coefEdit(w);
    row->addWidget(m_quadA); row->addWidget(new QLabel("x²  +",w));
    row->addWidget(m_quadB); row->addWidget(new QLabel("x  +",w));
    row->addWidget(m_quadC); row->addWidget(new QLabel("= 0",w));
    row->addStretch();
    v->addWidget(new QLabel("ax² + bx + c = 0", w));
    v->addWidget(c);

    m_quadResult = resultLbl(w);
    auto* solveBtn = btn("Solve", "actionButton", w);
    v->addWidget(solveBtn);
    v->addWidget(m_quadResult);
    v->addStretch();

    connect(solveBtn, &QPushButton::clicked, this, &EquationsWidget::solveQuadratic);
    return w;
}

void EquationsWidget::solveQuadratic() {
    double a = m_quadA->text().toDouble();
    double b = m_quadB->text().toDouble();
    double c = m_quadC->text().toDouble();
    if (a == 0) { m_quadResult->setText("Not quadratic (a=0)"); return; }
    double disc = b*b - 4*a*c;
    if (disc > 0) {
        double x1 = (-b + std::sqrt(disc)) / (2*a);
        double x2 = (-b - std::sqrt(disc)) / (2*a);
        m_quadResult->setText(QString("Two real roots:\nx₁ = %1\nx₂ = %2")
            .arg(x1,0,'g',10).arg(x2,0,'g',10));
    } else if (disc == 0) {
        m_quadResult->setText(QString("One real root:\nx = %1").arg(-b/(2*a),0,'g',10));
    } else {
        double re = -b/(2*a);
        double im = std::sqrt(-disc)/(2*a);
        m_quadResult->setText(QString("Complex roots:\nx₁ = %1 + %2i\nx₂ = %1 − %2i")
            .arg(re,0,'g',8).arg(im,0,'g',8));
    }
}

// ── Tab 3: System of equations ────────────────────────────────────────────────
QWidget* EquationsWidget::buildSystemTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(10);

    auto* row = new QHBoxLayout();
    m_sysSize = new QSpinBox(w);
    m_sysSize->setRange(2,4); m_sysSize->setValue(2);
    row->addWidget(new QLabel("Variables:", w));
    row->addWidget(m_sysSize); row->addStretch();
    v->addLayout(row);

    v->addWidget(new QLabel("Enter augmented matrix [A | b]:", w));
    m_sysMatrix = makeTable(2, 3, w);
    v->addWidget(m_sysMatrix);

    m_sysResult = resultLbl(w);
    auto* solveBtn = btn("Solve (Gaussian Elimination)", "actionButton", w);
    v->addWidget(solveBtn);
    v->addWidget(m_sysResult);
    v->addStretch();

    connect(m_sysSize, &QSpinBox::valueChanged, this, &EquationsWidget::rebuildSysMatrix);
    connect(solveBtn,  &QPushButton::clicked,   this, &EquationsWidget::solveSystem);
    rebuildSysMatrix(2);
    return w;
}

void EquationsWidget::rebuildSysMatrix(int n) {
    m_sysMatrix->setRowCount(n);
    m_sysMatrix->setColumnCount(n+1);
    QStringList hdr;
    for (int i=0;i<n;i++) hdr << QString("x%1").arg(i+1);
    hdr << "b";
    m_sysMatrix->setHorizontalHeaderLabels(hdr);
    m_sysMatrix->setMaximumHeight(n*36+28);
    for (int r=0;r<n;r++)
        for (int c=0;c<=n;c++) {
            auto* item = new QTableWidgetItem("0");
            item->setTextAlignment(Qt::AlignCenter);
            m_sysMatrix->setItem(r,c,item);
        }
}

void EquationsWidget::solveSystem() {
    int n = m_sysSize->value();
    Matrix aug(n, QVector<double>(n+1, 0));
    for (int r=0;r<n;r++)
        for (int c=0;c<=n;c++) {
            auto* item = m_sysMatrix->item(r,c);
            aug[r][c] = item ? item->text().toDouble() : 0.0;
        }
    try {
        Matrix sol = gaussianElim(aug, n);
        QString out;
        for (int i=0;i<n;i++)
            out += QString("x%1 = %2\n").arg(i+1).arg(sol[i][0],0,'g',8);
        m_sysResult->setText(out.trimmed());
    } catch (const std::exception& e) {
        m_sysResult->setText(QString("Error: %1").arg(e.what()));
    }
}

EquationsWidget::Matrix EquationsWidget::gaussianElim(Matrix aug, int n) const {
    // Forward elimination with partial pivoting
    for (int col=0;col<n;col++) {
        int pivot=col;
        for (int row=col+1;row<n;row++)
            if (std::abs(aug[row][col]) > std::abs(aug[pivot][col])) pivot=row;
        std::swap(aug[col], aug[pivot]);
        if (std::abs(aug[col][col]) < 1e-12)
            throw std::runtime_error("No unique solution (singular matrix)");
        for (int row=col+1;row<n;row++) {
            double f = aug[row][col]/aug[col][col];
            for (int k=col;k<=n;k++) aug[row][k] -= f*aug[col][k];
        }
    }
    // Back substitution
    Matrix sol(n, QVector<double>(1,0));
    for (int i=n-1;i>=0;i--) {
        sol[i][0] = aug[i][n];
        for (int j=i+1;j<n;j++) sol[i][0] -= aug[i][j]*sol[j][0];
        sol[i][0] /= aug[i][i];
    }
    return sol;
}

// ── Tab 4: Matrix operations ──────────────────────────────────────────────────
QWidget* EquationsWidget::buildMatrixTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);

    // Matrix A size
    auto* rowA = new QHBoxLayout();
    m_matRowsA=new QSpinBox(w); m_matRowsA->setRange(1,6); m_matRowsA->setValue(2);
    m_matColsA=new QSpinBox(w); m_matColsA->setRange(1,6); m_matColsA->setValue(2);
    rowA->addWidget(new QLabel("Matrix A:", w));
    rowA->addWidget(m_matRowsA); rowA->addWidget(new QLabel("×",w));
    rowA->addWidget(m_matColsA); rowA->addStretch();
    v->addLayout(rowA);

    m_matA = makeTable(2,2,w);
    v->addWidget(m_matA);

    // Matrix B size
    auto* rowB = new QHBoxLayout();
    m_matRowsB=new QSpinBox(w); m_matRowsB->setRange(1,6); m_matRowsB->setValue(2);
    m_matColsB=new QSpinBox(w); m_matColsB->setRange(1,6); m_matColsB->setValue(2);
    rowB->addWidget(new QLabel("Matrix B:", w));
    rowB->addWidget(m_matRowsB); rowB->addWidget(new QLabel("×",w));
    rowB->addWidget(m_matColsB); rowB->addStretch();
    v->addLayout(rowB);

    m_matB = makeTable(2,2,w);
    v->addWidget(m_matB);

    // Operation buttons
    auto* opRow = new QHBoxLayout();
    opRow->addWidget(btn("A+B",  "operatorButton", w));
    opRow->addWidget(btn("A−B",  "operatorButton", w));
    opRow->addWidget(btn("A×B",  "operatorButton", w));
    opRow->addWidget(btn("Aᵀ",   "operatorButton", w));
    opRow->addWidget(btn("det(A)","operatorButton", w));
    opRow->addWidget(btn("A⁻¹",  "operatorButton", w));
    v->addLayout(opRow);

    // Result table + scalar label
    m_matResult = makeTable(2,2,w);
    m_matResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
    v->addWidget(m_matResult);

    m_matScalarResult = resultLbl(w);
    v->addWidget(m_matScalarResult);
    v->addStretch();

    // Resize tables when spinboxes change
    auto resizeA = [this]{ rebuildMatrixTable(m_matA, m_matRowsA->value(), m_matColsA->value()); };
    auto resizeB = [this]{ rebuildMatrixTable(m_matB, m_matRowsB->value(), m_matColsB->value()); };
    connect(m_matRowsA,&QSpinBox::valueChanged,this,[resizeA](int){resizeA();});
    connect(m_matColsA,&QSpinBox::valueChanged,this,[resizeA](int){resizeA();});
    connect(m_matRowsB,&QSpinBox::valueChanged,this,[resizeB](int){resizeB();});
    connect(m_matColsB,&QSpinBox::valueChanged,this,[resizeB](int){resizeB();});

    // Wire op buttons
    QList<QPushButton*> btns = w->findChildren<QPushButton*>();
    for (auto* b : btns) {
        QString t = b->text();
        if      (t=="A+B")   connect(b,&QPushButton::clicked,this,&EquationsWidget::matOpAdd);
        else if (t=="A−B")   connect(b,&QPushButton::clicked,this,&EquationsWidget::matOpSub);
        else if (t=="A×B")   connect(b,&QPushButton::clicked,this,&EquationsWidget::matOpMul);
        else if (t=="Aᵀ")    connect(b,&QPushButton::clicked,this,&EquationsWidget::matOpTransposeA);
        else if (t=="det(A)") connect(b,&QPushButton::clicked,this,&EquationsWidget::matOpDetA);
        else if (t=="A⁻¹")   connect(b,&QPushButton::clicked,this,&EquationsWidget::matOpInvA);
    }

    rebuildMatrixTable(m_matA,2,2);
    rebuildMatrixTable(m_matB,2,2);
    return w;
}

void EquationsWidget::rebuildMatrixTable(QTableWidget* tbl, int rows, int cols) {
    tbl->setRowCount(rows); tbl->setColumnCount(cols);
    tbl->setMaximumHeight(rows*36+28);
    for (int r=0;r<rows;r++)
        for (int c=0;c<cols;c++) {
            auto* item = new QTableWidgetItem("0");
            item->setTextAlignment(Qt::AlignCenter);
            tbl->setItem(r,c,item);
        }
}

EquationsWidget::Matrix EquationsWidget::readMatrix(QTableWidget* tbl) const {
    int r=tbl->rowCount(), c=tbl->columnCount();
    Matrix m(r, QVector<double>(c,0));
    for (int i=0;i<r;i++)
        for (int j=0;j<c;j++) {
            auto* item=tbl->item(i,j);
            m[i][j] = item ? item->text().toDouble() : 0.0;
        }
    return m;
}

void EquationsWidget::writeMatrix(QTableWidget* tbl, const Matrix& m) {
    tbl->setRowCount(m.size());
    tbl->setColumnCount(m.isEmpty()?0:m[0].size());
    tbl->setMaximumHeight(m.size()*36+28);
    for (int i=0;i<(int)m.size();i++)
        for (int j=0;j<(int)m[i].size();j++) {
            auto* item=new QTableWidgetItem(QString::number(m[i][j],'g',8));
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            tbl->setItem(i,j,item);
        }
}

EquationsWidget::Matrix EquationsWidget::matAdd(const Matrix& a, const Matrix& b) const {
    if (a.size()!=b.size()||a[0].size()!=b[0].size()) throw std::runtime_error("Dimension mismatch");
    Matrix r=a;
    for (int i=0;i<(int)a.size();i++) for (int j=0;j<(int)a[0].size();j++) r[i][j]+=b[i][j];
    return r;
}
EquationsWidget::Matrix EquationsWidget::matSub(const Matrix& a, const Matrix& b) const {
    if (a.size()!=b.size()||a[0].size()!=b[0].size()) throw std::runtime_error("Dimension mismatch");
    Matrix r=a;
    for (int i=0;i<(int)a.size();i++) for (int j=0;j<(int)a[0].size();j++) r[i][j]-=b[i][j];
    return r;
}
EquationsWidget::Matrix EquationsWidget::matMul(const Matrix& a, const Matrix& b) const {
    if (a[0].size()!=b.size()) throw std::runtime_error("Incompatible dimensions for multiplication");
    int m=a.size(),n=b[0].size(),k=b.size();
    Matrix r(m,QVector<double>(n,0));
    for (int i=0;i<m;i++) for (int j=0;j<n;j++) for (int p=0;p<k;p++) r[i][j]+=a[i][p]*b[p][j];
    return r;
}
EquationsWidget::Matrix EquationsWidget::matTranspose(const Matrix& a) const {
    int m=a.size(),n=a[0].size();
    Matrix r(n,QVector<double>(m));
    for (int i=0;i<m;i++) for (int j=0;j<n;j++) r[j][i]=a[i][j];
    return r;
}
double EquationsWidget::matDet(const Matrix& a) const {
    int n=a.size();
    if (n==1) return a[0][0];
    if (n==2) return a[0][0]*a[1][1]-a[0][1]*a[1][0];
    // LU decomposition for larger
    Matrix lu=a; double det=1.0;
    for (int col=0;col<n;col++) {
        int pivot=col;
        for (int row=col+1;row<n;row++) if (std::abs(lu[row][col])>std::abs(lu[pivot][col])) pivot=row;
        if (pivot!=col) { std::swap(lu[col],lu[pivot]); det*=-1; }
        if (std::abs(lu[col][col])<1e-12) return 0.0;
        det*=lu[col][col];
        for (int row=col+1;row<n;row++) {
            double f=lu[row][col]/lu[col][col];
            for (int k=col;k<n;k++) lu[row][k]-=f*lu[col][k];
        }
    }
    return det;
}
EquationsWidget::Matrix EquationsWidget::matInverse(const Matrix& a) const {
    int n=a.size();
    if ((int)a[0].size()!=n) throw std::runtime_error("Matrix must be square");
    // Augment with identity
    Matrix aug(n,QVector<double>(2*n,0));
    for (int i=0;i<n;i++) { for (int j=0;j<n;j++) aug[i][j]=a[i][j]; aug[i][n+i]=1.0; }
    for (int col=0;col<n;col++) {
        int pivot=col;
        for (int row=col+1;row<n;row++) if (std::abs(aug[row][col])>std::abs(aug[pivot][col])) pivot=row;
        std::swap(aug[col],aug[pivot]);
        if (std::abs(aug[col][col])<1e-12) throw std::runtime_error("Matrix is singular");
        double s=aug[col][col];
        for (int k=0;k<2*n;k++) aug[col][k]/=s;
        for (int row=0;row<n;row++) if (row!=col) {
            double f=aug[row][col];
            for (int k=0;k<2*n;k++) aug[row][k]-=f*aug[col][k];
        }
    }
    Matrix inv(n,QVector<double>(n));
    for (int i=0;i<n;i++) for (int j=0;j<n;j++) inv[i][j]=aug[i][n+j];
    return inv;
}

void EquationsWidget::matOpAdd() {
    try { writeMatrix(m_matResult, matAdd(readMatrix(m_matA),readMatrix(m_matB))); m_matScalarResult->setText("A + B"); }
    catch(const std::exception& e){ m_matScalarResult->setText(QString("Error: %1").arg(e.what())); }
}
void EquationsWidget::matOpSub() {
    try { writeMatrix(m_matResult, matSub(readMatrix(m_matA),readMatrix(m_matB))); m_matScalarResult->setText("A − B"); }
    catch(const std::exception& e){ m_matScalarResult->setText(QString("Error: %1").arg(e.what())); }
}
void EquationsWidget::matOpMul() {
    try { writeMatrix(m_matResult, matMul(readMatrix(m_matA),readMatrix(m_matB))); m_matScalarResult->setText("A × B"); }
    catch(const std::exception& e){ m_matScalarResult->setText(QString("Error: %1").arg(e.what())); }
}
void EquationsWidget::matOpTransposeA() {
    try { writeMatrix(m_matResult, matTranspose(readMatrix(m_matA))); m_matScalarResult->setText("Aᵀ"); }
    catch(const std::exception& e){ m_matScalarResult->setText(QString("Error: %1").arg(e.what())); }
}
void EquationsWidget::matOpDetA() {
    try {
        Matrix a=readMatrix(m_matA);
        if (a.size()!=a[0].size()) throw std::runtime_error("Matrix must be square");
        m_matScalarResult->setText(QString("det(A) = %1").arg(matDet(a),0,'g',10));
    } catch(const std::exception& e){ m_matScalarResult->setText(QString("Error: %1").arg(e.what())); }
}
void EquationsWidget::matOpInvA() {
    try { writeMatrix(m_matResult, matInverse(readMatrix(m_matA))); m_matScalarResult->setText("A⁻¹"); }
    catch(const std::exception& e){ m_matScalarResult->setText(QString("Error: %1").arg(e.what())); }
}

void EquationsWidget::solveLinear() {}  // handled inline via lambda
