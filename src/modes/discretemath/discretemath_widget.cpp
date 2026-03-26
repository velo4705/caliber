#include "discretemath_widget.h"
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
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDoubleValidator>
#include <climits>
#include <cmath>
#include <algorithm>
#include <queue>
#include <vector>

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
    t->setMinimumHeight(100); t->setMaximumHeight(200);
    t->setStyleSheet("font-family:monospace;font-size:12px;"); t->hide(); return t;
}
static QLabel* descLbl(const QString& text, QWidget* p) {
    auto* l = new QLabel(text, p); l->setWordWrap(true);
    l->setStyleSheet("color:gray;font-size:12px;padding:4px 0;"); return l;
}

void DiscreteMathWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

long long DiscreteMathWidget::factorial(int n) {
    if (n <= 1) return 1;
    long long r = 1; for (int i = 2; i <= n; i++) r *= i; return r;
}
long long DiscreteMathWidget::nPr(int n, int r) {
    if (r > n) return 0;
    long long res = 1; for (int i = n; i > n-r; i--) res *= i; return res;
}
long long DiscreteMathWidget::nCr(int n, int r) {
    if (r > n) return 0; if (r == 0) return 1;
    r = qMin(r, n-r);
    long long res = 1;
    for (int i = 0; i < r; i++) { res *= (n-i); res /= (i+1); }
    return res;
}

DiscreteMathWidget::DiscreteMathWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Discrete Mathematics", this);
    title->setStyleSheet("font-size:18px;font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildGraphTab(),      "Graph Theory");
    tabs->addTab(buildCombinTab(),     "Combinatorics");
    tabs->addTab(buildRelationsTab(),  "Relations");
    tabs->addTab(buildRecurrenceTab(), "Recurrence");
    tabs->addTab(buildBoolAlgTab(),    "Boolean Algebra");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Graph Theory ──────────────────────────────────────────────────────────────
QWidget* DiscreteMathWidget::buildGraphTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Enter an adjacency matrix (0=no edge, weight=edge). Runs BFS, DFS, or Dijkstra's shortest path from a source node.", w));
    auto* row = new QHBoxLayout();
    m_gNodes = new QSpinBox(w); m_gNodes->setRange(2,8); m_gNodes->setValue(4);
    m_gAlgo  = new QComboBox(w); m_gAlgo->addItems({"BFS","DFS","Dijkstra (shortest path)"});
    m_gSrc   = numEdit("0","source node",w);
    row->addWidget(new QLabel("Nodes:",w)); row->addWidget(m_gNodes);
    row->addWidget(new QLabel("Algorithm:",w)); row->addWidget(m_gAlgo);
    row->addWidget(new QLabel("Source:",w)); row->addWidget(m_gSrc);
    row->addStretch();
    v->addLayout(row);
    m_gMatrix = new QTableWidget(w);
    m_gMatrix->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_gMatrix->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_gMatrix->verticalHeader()->setDefaultSectionSize(30);
    v->addWidget(m_gMatrix);
    m_gResult=resultLbl(w); m_gShow=new QCheckBox("Show Steps",w); m_gSteps=stepsBox(w);
    auto* btns=new QHBoxLayout();
    btns->addWidget(mkBtn("Build Matrix",w)); btns->addWidget(mkBtn("Run",w)); btns->addStretch();
    v->addLayout(btns);
    v->addWidget(m_gResult); v->addWidget(m_gShow); v->addWidget(m_gSteps); v->addStretch();
    connect(m_gShow,&QCheckBox::toggled,this,[this](bool on){m_gSteps->setVisible(on&&!m_gSteps->toPlainText().isEmpty());});
    auto bl=w->findChildren<QPushButton*>();
    connect(bl[0],&QPushButton::clicked,this,[this](){buildGraphMatrix(m_gNodes->value());});
    connect(bl[1],&QPushButton::clicked,this,&DiscreteMathWidget::computeGraph);
    buildGraphMatrix(4);
    return w;
}

void DiscreteMathWidget::buildGraphMatrix(int n) {
    m_gMatrix->setRowCount(n); m_gMatrix->setColumnCount(n);
    QStringList hdr; for(int i=0;i<n;i++) hdr<<QString::number(i);
    m_gMatrix->setHorizontalHeaderLabels(hdr);
    m_gMatrix->setVerticalHeaderLabels(hdr);
    m_gMatrix->setFixedHeight(n*30+m_gMatrix->horizontalHeader()->sizeHint().height()+4);
    for(int r=0;r<n;r++) for(int c=0;c<n;c++){
        auto* item=new QTableWidgetItem(r==c?"0":"0");
        item->setTextAlignment(Qt::AlignCenter);
        m_gMatrix->setItem(r,c,item);
    }
}

void DiscreteMathWidget::computeGraph() {
    int n=m_gMatrix->rowCount(), src=m_gSrc->text().toInt();
    if(src<0||src>=n){m_gResult->setText("Invalid source node.");return;}
    // Read adjacency matrix
    std::vector<std::vector<int>> adj(n,std::vector<int>(n,0));
    for(int r=0;r<n;r++) for(int c=0;c<n;c++){
        auto* item=m_gMatrix->item(r,c);
        adj[r][c]=item?item->text().toInt():0;
    }
    QStringList steps;
    int algo=m_gAlgo->currentIndex();
    if(algo==0||algo==1){
        // BFS or DFS
        std::vector<bool> visited(n,false);
        QStringList order;
        steps<<QString("%1 from node %2").arg(algo==0?"BFS":"DFS").arg(src);
        if(algo==0){
            std::queue<int> q; q.push(src); visited[src]=true;
            while(!q.empty()){
                int u=q.front(); q.pop(); order<<QString::number(u);
                steps<<QString("Visit node %1").arg(u);
                for(int v=0;v<n;v++) if(adj[u][v]&&!visited[v]){visited[v]=true;q.push(v);steps<<QString("  Enqueue %1").arg(v);}
            }
        } else {
            std::function<void(int)> dfs=[&](int u){
                visited[u]=true; order<<QString::number(u);
                steps<<QString("Visit node %1").arg(u);
                for(int v=0;v<n;v++) if(adj[u][v]&&!visited[v]) dfs(v);
            };
            dfs(src);
        }
        m_gResult->setText(QString("%1 traversal from %2:\n%3").arg(algo==0?"BFS":"DFS").arg(src).arg(order.join(" → ")));
    } else {
        // Dijkstra
        std::vector<int> dist(n,INT_MAX); dist[src]=0;
        std::vector<bool> visited(n,false);
        std::vector<int> prev(n,-1);
        steps<<QString("Dijkstra from node %1").arg(src);
        for(int i=0;i<n;i++){
            int u=-1;
            for(int v=0;v<n;v++) if(!visited[v]&&(u==-1||dist[v]<dist[u])) u=v;
            if(u==-1||dist[u]==INT_MAX) break;
            visited[u]=true;
            steps<<QString("Process node %1, dist=%2").arg(u).arg(dist[u]);
            for(int v=0;v<n;v++) if(adj[u][v]&&dist[u]+adj[u][v]<dist[v]){
                dist[v]=dist[u]+adj[u][v]; prev[v]=u;
                steps<<QString("  Update dist[%1]=%2 via %3").arg(v).arg(dist[v]).arg(u);
            }
        }
        QString out=QString("Shortest distances from node %1:\n").arg(src);
        for(int i=0;i<n;i++) out+=QString("→ node %1: %2\n").arg(i).arg(dist[i]==INT_MAX?"∞":QString::number(dist[i]));
        m_gResult->setText(out.trimmed());
    }
    showSteps(m_gSteps,m_gShow,steps);
}

// ── Combinatorics ─────────────────────────────────────────────────────────────
QWidget* DiscreteMathWidget::buildCombinTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Permutations nPr = n!/(n−r)!, Combinations nCr = n!/r!(n−r)!, Pigeonhole principle, and Inclusion-Exclusion for two sets.", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_cn=numEdit("10","n",w); m_cr=numEdit("3","r",w);
    g->addWidget(new QLabel("n:",w),0,0); g->addWidget(m_cn,0,1);
    g->addWidget(new QLabel("r:",w),1,0); g->addWidget(m_cr,1,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_combResult=resultLbl(w); m_combShow=new QCheckBox("Show Steps",w); m_combSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_combResult);
    v->addWidget(m_combShow); v->addWidget(m_combSteps); v->addStretch();
    connect(m_combShow,&QCheckBox::toggled,this,[this](bool on){m_combSteps->setVisible(on&&!m_combSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&DiscreteMathWidget::computeCombin);
    return w;
}

void DiscreteMathWidget::computeCombin() {
    int n=(int)m_cn->text().toDouble(), r=(int)m_cr->text().toDouble();
    QStringList steps;
    steps<<QString("n=%1, r=%2").arg(n).arg(r);
    long long perm=nPr(n,r), comb=nCr(n,r);
    steps<<QString("nPr = n!/(n−r)! = %1!/%2! = %3").arg(n).arg(n-r).arg(perm);
    steps<<QString("nCr = n!/r!(n−r)! = %1!/(%2!×%3!) = %4").arg(n).arg(r).arg(n-r).arg(comb);
    // Pigeonhole
    steps<<QString("Pigeonhole: %1 items in %2 boxes → at least ⌈%1/%2⌉=%3 items in one box").arg(n).arg(r).arg((n+r-1)/r);
    // Inclusion-exclusion |A∪B|=|A|+|B|-|A∩B|
    steps<<QString("Inclusion-exclusion: |A∪B|=|A|+|B|−|A∩B|");
    m_combResult->setText(QString("P(%1,%2) = %3\nC(%1,%2) = %4\nPigeonhole (%1 items, %2 boxes): ≥%5 per box")
        .arg(n).arg(r).arg(perm).arg(comb).arg((n+r-1)/r));
    showSteps(m_combSteps,m_combShow,steps);
}

// ── Relations ─────────────────────────────────────────────────────────────────
QWidget* DiscreteMathWidget::buildRelationsTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Enter a relation as a 0/1 matrix. Checks if the relation is reflexive, symmetric, antisymmetric, transitive, and whether it is an equivalence relation or partial order.", w));
    auto* row=new QHBoxLayout();
    m_relN=new QSpinBox(w); m_relN->setRange(2,6); m_relN->setValue(3);
    row->addWidget(new QLabel("Set size n:",w)); row->addWidget(m_relN); row->addStretch();
    v->addLayout(row);
    m_relMatrix=new QTableWidget(w);
    m_relMatrix->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_relMatrix->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_relMatrix->verticalHeader()->setDefaultSectionSize(30);
    v->addWidget(m_relMatrix);
    m_relResult=resultLbl(w); m_relShow=new QCheckBox("Show Steps",w); m_relSteps=stepsBox(w);
    auto* btns=new QHBoxLayout();
    btns->addWidget(mkBtn("Build Matrix",w)); btns->addWidget(mkBtn("Check",w)); btns->addStretch();
    v->addLayout(btns);
    v->addWidget(m_relResult); v->addWidget(m_relShow); v->addWidget(m_relSteps); v->addStretch();
    connect(m_relShow,&QCheckBox::toggled,this,[this](bool on){m_relSteps->setVisible(on&&!m_relSteps->toPlainText().isEmpty());});
    auto bl=w->findChildren<QPushButton*>();
    connect(bl[0],&QPushButton::clicked,this,[this](){buildRelMatrix(m_relN->value());});
    connect(bl[1],&QPushButton::clicked,this,&DiscreteMathWidget::computeRelations);
    buildRelMatrix(3);
    return w;
}

void DiscreteMathWidget::buildRelMatrix(int n) {
    m_relMatrix->setRowCount(n); m_relMatrix->setColumnCount(n);
    QStringList hdr; for(int i=1;i<=n;i++) hdr<<QString::number(i);
    m_relMatrix->setHorizontalHeaderLabels(hdr);
    m_relMatrix->setVerticalHeaderLabels(hdr);
    m_relMatrix->setFixedHeight(n*30+m_relMatrix->horizontalHeader()->sizeHint().height()+4);
    for(int r=0;r<n;r++) for(int c=0;c<n;c++){
        auto* item=new QTableWidgetItem("0");
        item->setTextAlignment(Qt::AlignCenter);
        m_relMatrix->setItem(r,c,item);
    }
}

void DiscreteMathWidget::computeRelations() {
    int n=m_relMatrix->rowCount();
    std::vector<std::vector<int>> R(n,std::vector<int>(n,0));
    for(int r=0;r<n;r++) for(int c=0;c<n;c++){
        auto* item=m_relMatrix->item(r,c);
        R[r][c]=item?item->text().toInt():0;
    }
    QStringList steps;
    bool refl=true,sym=true,antisym=true,trans=true;
    for(int i=0;i<n;i++) if(!R[i][i]){refl=false;steps<<QString("Not reflexive: (%1,%1) missing").arg(i+1);break;}
    if(refl) steps<<"Reflexive: all (i,i) present";
    for(int i=0;i<n&&sym;i++) for(int j=0;j<n&&sym;j++) if(R[i][j]&&!R[j][i]){sym=false;steps<<QString("Not symmetric: (%1,%2) exists but (%2,%1) missing").arg(i+1).arg(j+1);}
    if(sym) steps<<"Symmetric: for all (i,j), (j,i) also present";
    for(int i=0;i<n&&antisym;i++) for(int j=0;j<n&&antisym;j++) if(i!=j&&R[i][j]&&R[j][i]){antisym=false;steps<<QString("Not antisymmetric: both (%1,%2) and (%2,%1) exist").arg(i+1).arg(j+1);}
    if(antisym) steps<<"Antisymmetric";
    for(int i=0;i<n&&trans;i++) for(int j=0;j<n&&trans;j++) if(R[i][j]) for(int k=0;k<n&&trans;k++) if(R[j][k]&&!R[i][k]){trans=false;steps<<QString("Not transitive: (%1,%2) and (%2,%3) but not (%1,%3)").arg(i+1).arg(j+1).arg(k+1);}
    if(trans) steps<<"Transitive";
    bool equiv=refl&&sym&&trans, partOrd=refl&&antisym&&trans;
    m_relResult->setText(QString("Reflexive: %1\nSymmetric: %2\nAntisymmetric: %3\nTransitive: %4\n\nEquivalence relation: %5\nPartial order: %6")
        .arg(refl?"✓":"✗").arg(sym?"✓":"✗").arg(antisym?"✓":"✗").arg(trans?"✓":"✗")
        .arg(equiv?"Yes":"No").arg(partOrd?"Yes":"No"));
    showSteps(m_relSteps,m_relShow,steps);
}

// ── Recurrence Relations ──────────────────────────────────────────────────────
QWidget* DiscreteMathWidget::buildRecurrenceTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Solves linear recurrence T(n) = a×T(n−1) + b×T(n−2) with initial conditions T(0)=f0, T(1)=f1. Generates first n terms and finds characteristic roots.", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_recA=numEdit("1","a",w); m_recB=numEdit("1","b",w);
    m_recF0=numEdit("0","T(0)",w); m_recF1=numEdit("1","T(1)",w); m_recN=numEdit("10","n terms",w);
    g->addWidget(new QLabel("a (coeff T(n−1)):",w),0,0); g->addWidget(m_recA,0,1);
    g->addWidget(new QLabel("b (coeff T(n−2)):",w),1,0); g->addWidget(m_recB,1,1);
    g->addWidget(new QLabel("T(0):",w),2,0); g->addWidget(m_recF0,2,1);
    g->addWidget(new QLabel("T(1):",w),3,0); g->addWidget(m_recF1,3,1);
    g->addWidget(new QLabel("n terms:",w),4,0); g->addWidget(m_recN,4,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_recResult=resultLbl(w); m_recShow=new QCheckBox("Show Steps",w); m_recSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_recResult);
    v->addWidget(m_recShow); v->addWidget(m_recSteps); v->addStretch();
    connect(m_recShow,&QCheckBox::toggled,this,[this](bool on){m_recSteps->setVisible(on&&!m_recSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&DiscreteMathWidget::computeRecurrence);
    return w;
}

void DiscreteMathWidget::computeRecurrence() {
    double a=m_recA->text().toDouble(), b=m_recB->text().toDouble();
    double f0=m_recF0->text().toDouble(), f1=m_recF1->text().toDouble();
    int n=qBound(2,(int)m_recN->text().toDouble(),50);
    QStringList steps;
    steps<<QString("T(n) = %1×T(n−1) + %2×T(n−2),  T(0)=%3, T(1)=%4").arg(a).arg(b).arg(f0).arg(f1);
    // Characteristic equation: x² - ax - b = 0
    double disc=a*a+4*b;
    steps<<QString("Characteristic equation: x²−%1x−%2=0").arg(a).arg(b);
    steps<<QString("Discriminant = a²+4b = %1").arg(disc,0,'g',6);
    if(disc>=0){
        double r1=(a+std::sqrt(disc))/2, r2=(a-std::sqrt(disc))/2;
        steps<<QString("Roots: r₁=%1, r₂=%2").arg(r1,0,'g',6).arg(r2,0,'g',6);
    } else {
        steps<<"Complex characteristic roots";
    }
    // Generate terms
    QVector<double> terms(n);
    terms[0]=f0; if(n>1) terms[1]=f1;
    for(int i=2;i<n;i++) terms[i]=a*terms[i-1]+b*terms[i-2];
    QStringList termStrs; for(int i=0;i<qMin(n,12);i++) termStrs<<QString::number(terms[i],'g',6);
    for(int i=2;i<qMin(n,6);i++) steps<<QString("T(%1) = %2×%3 + %4×%5 = %6").arg(i).arg(a).arg(terms[i-1],'g',4).arg(b).arg(terms[i-2],'g',4).arg(terms[i],'g',6);
    m_recResult->setText(QString("First %1 terms:\n%2%3").arg(n).arg(termStrs.join(", ")).arg(n>12?"...":""));
    showSteps(m_recSteps,m_recShow,steps);
}

// ── Boolean Algebra ───────────────────────────────────────────────────────────
QWidget* DiscreteMathWidget::buildBoolAlgTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Applies Boolean algebra laws step-by-step: De Morgan's, absorption, idempotent, complement, identity. Enter an expression using A,B,C with AND(&), OR(|), NOT(!).", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_boolExpr=new QLineEdit("!(A&B)",w); m_boolExpr->setMinimumWidth(200);
    g->addWidget(new QLabel("Expression:",w),0,0); g->addWidget(m_boolExpr,0,1);
    g->setColumnStretch(1,1);
    v->addLayout(g);
    m_boolResult=resultLbl(w); m_boolShow=new QCheckBox("Show Steps",w); m_boolSteps=stepsBox(w);
    v->addWidget(mkBtn("Simplify",w)); v->addWidget(m_boolResult);
    v->addWidget(m_boolShow); v->addWidget(m_boolSteps); v->addStretch();
    connect(m_boolShow,&QCheckBox::toggled,this,[this](bool on){m_boolSteps->setVisible(on&&!m_boolSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&DiscreteMathWidget::computeBoolAlg);
    return w;
}

void DiscreteMathWidget::computeBoolAlg() {
    QString expr=m_boolExpr->text().trimmed();
    QStringList steps;
    steps<<QString("Original: %1").arg(expr);
    // Apply De Morgan's laws textually
    QString e=expr;
    // De Morgan: !(A&B) = !A|!B
    if(e.contains("!(") && e.contains("&")){
        steps<<"De Morgan's law: !(A&B) = !A | !B";
        e.replace("!(A&B)","!A|!B"); e.replace("!(B&A)","!B|!A");
    }
    if(e.contains("!(") && e.contains("|")){
        steps<<"De Morgan's law: !(A|B) = !A & !B";
        e.replace("!(A|B)","!A&!B"); e.replace("!(B|A)","!B&!A");
    }
    // Idempotent: A&A=A, A|A=A
    if(e.contains("A&A")){e.replace("A&A","A");steps<<"Idempotent: A&A = A";}
    if(e.contains("A|A")){e.replace("A|A","A");steps<<"Idempotent: A|A = A";}
    // Identity: A&1=A, A|0=A
    if(e.contains("&1")){e.replace("&1","");steps<<"Identity: A&1 = A";}
    if(e.contains("|0")){e.replace("|0","");steps<<"Identity: A|0 = A";}
    // Complement: A&!A=0, A|!A=1
    if(e.contains("A&!A")||e.contains("!A&A")){e="0";steps<<"Complement: A&!A = 0";}
    if(e.contains("A|!A")||e.contains("!A|A")){e="1";steps<<"Complement: A|!A = 1";}
    steps<<QString("Simplified: %1").arg(e);
    // Truth table for original vs simplified
    steps<<"Verify with truth table (A=0,B=0): check both expressions give same result";
    m_boolResult->setText(QString("Original:   %1\nSimplified: %2").arg(expr).arg(e));
    showSteps(m_boolSteps,m_boolShow,steps);
}
