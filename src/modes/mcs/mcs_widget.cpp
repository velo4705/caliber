#include "mcs_widget.h"
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
#include <cmath>
#include <cstring>

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

void McsWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

McsWidget::McsWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Mathematics for Computer Science", this);
    title->setStyleSheet("font-size:18px;font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildAsymptoticTab(), "Asymptotic");
    tabs->addTab(buildMasterTab(),     "Master Theorem");
    tabs->addTab(buildFloatTab(),      "Floating Point");
    tabs->addTab(buildLogicTab(),      "Formal Logic");
    tabs->addTab(buildHashTab(),       "Hashing");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Asymptotic Analysis ───────────────────────────────────────────────────────
QWidget* McsWidget::buildAsymptoticTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Compares growth rates of common complexity functions at large n. Shows Big-O ordering and evaluates two functions at a given n to compare their values.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_asymN = numEdit("1000","n",w);
    m_asymCompare = new QComboBox(w);
    m_asymCompare->addItems({"O(1) vs O(log n)","O(log n) vs O(n)","O(n) vs O(n log n)",
                              "O(n log n) vs O(n²)","O(n²) vs O(n³)","O(n³) vs O(2^n)"});
    g->addWidget(new QLabel("n =",w),0,0); g->addWidget(m_asymN,0,1);
    g->addWidget(new QLabel("Compare:",w),1,0); g->addWidget(m_asymCompare,1,1,1,2);
    g->setColumnStretch(3,1);
    v->addLayout(g);
    m_asymResult=resultLbl(w); m_asymShow=new QCheckBox("Show Steps",w); m_asymSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_asymResult);
    v->addWidget(m_asymShow); v->addWidget(m_asymSteps); v->addStretch();
    connect(m_asymShow,&QCheckBox::toggled,this,[this](bool on){m_asymSteps->setVisible(on&&!m_asymSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&McsWidget::computeAsymptotic);
    return w;
}

void McsWidget::computeAsymptotic() {
    double n=m_asymN->text().toDouble();
    int idx=m_asymCompare->currentIndex();
    QStringList steps;
    steps<<"Big-O growth rate ordering: O(1) < O(log n) < O(n) < O(n log n) < O(n²) < O(n³) < O(2^n) < O(n!)";
    // Evaluate all complexities at n
    QStringList names={"O(1)","O(log n)","O(n)","O(n log n)","O(n²)","O(n³)","O(2^n)"};
    QVector<double> vals={1,std::log2(n),n,n*std::log2(n),n*n,n*n*n,n<=50?std::pow(2,n):1e300};
    for(int i=0;i<names.size();i++) steps<<QString("%1 at n=%2: %3").arg(names[i]).arg(n).arg(vals[i],0,'g',8);
    // Selected comparison
    QStringList parts=m_asymCompare->currentText().split(" vs ");
    QString f1=parts[0].trimmed(), f2=parts[1].trimmed();
    double v1=vals[idx], v2=vals[idx+1];
    double ratio=v1>0?v2/v1:0;
    steps<<QString("At n=%1: %2=%3, %4=%5, ratio=%6").arg(n).arg(f1).arg(v1,0,'g',8).arg(f2).arg(v2,0,'g',8).arg(ratio,0,'g',6);
    m_asymResult->setText(QString("At n = %1:\n%2 = %3\n%4 = %5\nRatio %4/%2 = %6\n\nGrowth order:\nO(1) < O(log n) < O(n) < O(n log n) < O(n²) < O(n³) < O(2^n)")
        .arg(n).arg(f1).arg(v1,0,'g',8).arg(f2).arg(v2,0,'g',8).arg(ratio,0,'g',6));
    showSteps(m_asymSteps,m_asymShow,steps);
}

// ── Master Theorem ────────────────────────────────────────────────────────────
QWidget* McsWidget::buildMasterTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Master Theorem for T(n) = a·T(n/b) + Θ(n^k·log^p(n)). Determines the asymptotic complexity by comparing log_b(a) with k.", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_mtA=numEdit("2","a (subproblems)",w); m_mtB=numEdit("2","b (size factor)",w);
    m_mtK=numEdit("1","k (n^k)",w); m_mtP=numEdit("0","p (log^p)",w);
    g->addWidget(new QLabel("a:",w),0,0); g->addWidget(m_mtA,0,1);
    g->addWidget(new QLabel("b:",w),1,0); g->addWidget(m_mtB,1,1);
    g->addWidget(new QLabel("k (n^k):",w),2,0); g->addWidget(m_mtK,2,1);
    g->addWidget(new QLabel("p (log^p):",w),3,0); g->addWidget(m_mtP,3,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_mtResult=resultLbl(w); m_mtShow=new QCheckBox("Show Steps",w); m_mtSteps=stepsBox(w);
    v->addWidget(mkBtn("Solve",w)); v->addWidget(m_mtResult);
    v->addWidget(m_mtShow); v->addWidget(m_mtSteps); v->addStretch();
    connect(m_mtShow,&QCheckBox::toggled,this,[this](bool on){m_mtSteps->setVisible(on&&!m_mtSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&McsWidget::computeMaster);
    return w;
}

void McsWidget::computeMaster() {
    double a=m_mtA->text().toDouble(), b=m_mtB->text().toDouble();
    double k=m_mtK->text().toDouble(), p=m_mtP->text().toDouble();
    QStringList steps;
    if(b<=1||a<=0){m_mtResult->setText("a>0 and b>1 required.");return;}
    double logba=std::log(a)/std::log(b);
    steps<<QString("T(n) = %1·T(n/%2) + Θ(n^%3·log^%4(n))").arg(a).arg(b).arg(k).arg(p);
    steps<<QString("log_b(a) = log_%1(%2) = %3").arg(b).arg(a).arg(logba,0,'g',6);
    steps<<QString("Compare log_b(a)=%1 with k=%2").arg(logba,0,'g',4).arg(k);
    QString result;
    if(logba>k+1e-9){
        result=QString("Case 1: log_b(a) > k → T(n) = Θ(n^%1)").arg(logba,0,'g',4);
        steps<<"Case 1 applies: f(n) is polynomially smaller than n^log_b(a)";
    } else if(std::abs(logba-k)<1e-9){
        if(p>-1) result=QString("Case 2: log_b(a) = k, p=%1 > -1 → T(n) = Θ(n^%2·log^%3(n))").arg(p).arg(k).arg(p+1);
        else if(std::abs(p+1)<1e-9) result=QString("Case 2: T(n) = Θ(n^%1·log(log(n)))").arg(k);
        else result=QString("Case 2: T(n) = Θ(n^%1)").arg(k);
        steps<<"Case 2 applies: f(n) = Θ(n^log_b(a)·log^p(n))";
    } else {
        if(p>=0) result=QString("Case 3: log_b(a) < k → T(n) = Θ(n^%1·log^%2(n))").arg(k).arg(p);
        else result=QString("Case 3: log_b(a) < k, p<0 → T(n) = Θ(n^%1)").arg(k);
        steps<<"Case 3 applies: f(n) is polynomially larger than n^log_b(a)";
    }
    steps<<result;
    m_mtResult->setText(result);
    showSteps(m_mtSteps,m_mtShow,steps);
}

// ── Floating Point Analysis ───────────────────────────────────────────────────
QWidget* McsWidget::buildFloatTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Analyzes floating-point representation. Shows machine epsilon, ULP (unit in last place), relative error, and IEEE 754 bit layout for single/double precision.", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_fpValue=new QLineEdit("0.1",w); m_fpValue->setFixedWidth(150);
    m_fpPrec=new QComboBox(w); m_fpPrec->addItems({"Single (32-bit)","Double (64-bit)"});
    g->addWidget(new QLabel("Value:",w),0,0); g->addWidget(m_fpValue,0,1);
    g->addWidget(new QLabel("Precision:",w),1,0); g->addWidget(m_fpPrec,1,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_fpResult=resultLbl(w); m_fpShow=new QCheckBox("Show Steps",w); m_fpSteps=stepsBox(w);
    v->addWidget(mkBtn("Analyze",w)); v->addWidget(m_fpResult);
    v->addWidget(m_fpShow); v->addWidget(m_fpSteps); v->addStretch();
    connect(m_fpShow,&QCheckBox::toggled,this,[this](bool on){m_fpSteps->setVisible(on&&!m_fpSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&McsWidget::computeFloat);
    return w;
}

void McsWidget::computeFloat() {
    bool ok; double val=m_fpValue->text().toDouble(&ok);
    if(!ok){m_fpResult->setText("Invalid number.");return;}
    QStringList steps;
    if(m_fpPrec->currentIndex()==0){
        float f=(float)val;
        uint32_t bits; memcpy(&bits,&f,4);
        int sign=(bits>>31)&1, exp=(bits>>23)&0xFF;
        uint32_t mant=bits&0x7FFFFF;
        float eps=std::numeric_limits<float>::epsilon();
        float stored=f;
        double err=std::abs((double)f-val);
        double relErr=val!=0?err/std::abs(val):0;
        steps<<QString("Stored as float: %1").arg((double)stored,0,'g',10);
        steps<<QString("Machine epsilon (float): %1").arg((double)eps,0,'g',6);
        steps<<QString("Absolute error: |stored−exact| = %1").arg(err,0,'g',6);
        steps<<QString("Relative error: %1").arg(relErr,0,'g',6);
        steps<<QString("Sign=%1, Exponent=%2 (actual=%3), Mantissa=%4").arg(sign).arg(exp).arg(exp-127).arg(mant);
        m_fpResult->setText(QString("Single precision:\nStored = %1\nExact  = %2\nAbs error = %3\nRel error = %4\nε_machine = %5\nBits: %6 | %7 | %8")
            .arg((double)stored,0,'g',10).arg(val,0,'g',10).arg(err,0,'g',6).arg(relErr,0,'g',6)
            .arg((double)eps,0,'g',6).arg(sign)
            .arg(QString::number(exp,2).rightJustified(8,'0'))
            .arg(QString::number(mant,2).rightJustified(23,'0')));
    } else {
        double eps=std::numeric_limits<double>::epsilon();
        uint64_t bits; memcpy(&bits,&val,8);
        int sign=(bits>>63)&1, exp=(bits>>52)&0x7FF;
        steps<<QString("Machine epsilon (double): %1").arg(eps,0,'g',6);
        steps<<QString("Sign=%1, Exponent=%2 (actual=%3)").arg(sign).arg(exp).arg(exp-1023);
        m_fpResult->setText(QString("Double precision:\nValue = %1\nε_machine = %2\nSign=%3, Exp=%4 (actual=%5)\nHex: %6")
            .arg(val,0,'g',15).arg(eps,0,'g',6).arg(sign).arg(exp).arg(exp-1023)
            .arg(QString::number(bits,16).toUpper().rightJustified(16,'0')));
    }
    showSteps(m_fpSteps,m_fpShow,steps);
}

// ── Formal Logic ──────────────────────────────────────────────────────────────
QWidget* McsWidget::buildLogicTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Propositional logic truth table and tautology checker. Use A,B,C with operators: && (AND), || (OR), ! (NOT), ^ (XOR). Checks if expression is tautology, contradiction, or contingency.", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_logExpr=new QLineEdit("(A||B)&&(!A||C)",w); m_logExpr->setMinimumWidth(200);
    m_logVars=new QSpinBox(w); m_logVars->setRange(1,3); m_logVars->setValue(2);
    g->addWidget(new QLabel("Expression:",w),0,0); g->addWidget(m_logExpr,0,1);
    g->addWidget(new QLabel("Variables:",w),1,0); g->addWidget(m_logVars,1,1);
    g->setColumnStretch(1,1);
    v->addLayout(g);
    m_logResult=resultLbl(w);
    v->addWidget(mkBtn("Evaluate",w)); v->addWidget(m_logResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&McsWidget::computeLogic);
    return w;
}

void McsWidget::computeLogic() {
    int n=m_logVars->value();
    QString expr=m_logExpr->text().trimmed();
    QStringList varNames; for(int i=0;i<n;i++) varNames<<QString(QChar('A'+i));
    int rows=1<<n; int trueCount=0;
    QString table;
    table+=varNames.join("  ")+"  | Result\n";
    table+=QString(n*4+10,'-')+"\n";

    auto evalExpr=[&](const QString& e, const QVector<bool>& vals)->bool{
        QString t=e.toUpper();
        for(int i=n-1;i>=0;i--) t.replace(varNames[i],vals[i]?"1":"0");
        std::function<bool(const QString&)> eval=[&](const QString& s)->bool{
            QString q=s.trimmed();
            while(q.startsWith('(')&&q.endsWith(')')){
                int d=0;bool w=true;
                for(int i=0;i<q.size()-1;i++){if(q[i]=='(')d++;else if(q[i]==')')d--;if(d==0){w=false;break;}}
                if(w)q=q.mid(1,q.size()-2).trimmed();else break;
            }
            int d=0;
            for(int i=q.size()-1;i>=0;i--){
                if(q[i]==')')d++;else if(q[i]=='(')d--;
                if(d==0&&i+1<q.size()&&q.mid(i,2)=="||") return eval(q.left(i))||eval(q.mid(i+2));
            }
            d=0;
            for(int i=q.size()-1;i>=0;i--){
                if(q[i]==')')d++;else if(q[i]=='(')d--;
                if(d==0&&i+1<q.size()&&q.mid(i,2)=="&&") return eval(q.left(i))&&eval(q.mid(i+2));
            }
            d=0;
            for(int i=q.size()-1;i>=0;i--){
                if(q[i]==')')d++;else if(q[i]=='(')d--;
                if(d==0&&q[i]=='^') return eval(q.left(i))^eval(q.mid(i+1));
            }
            if(q.startsWith('!')) return !eval(q.mid(1));
            return q=="1";
        };
        return eval(t);
    };

    for(int r=0;r<rows;r++){
        QVector<bool> vals(n);
        for(int i=0;i<n;i++) vals[i]=(r>>(n-1-i))&1;
        bool out=evalExpr(expr,vals);
        if(out) trueCount++;
        QStringList row;
        for(bool v:vals) row<<(v?"1":"0");
        table+=row.join("   ")+"  | "+(out?"1":"0")+"\n";
    }
    QString nature=trueCount==rows?"Tautology (always true)":trueCount==0?"Contradiction (always false)":"Contingency";
    m_logResult->setText(table+"\n"+nature);
}

// ── Hashing ───────────────────────────────────────────────────────────────────
QWidget* McsWidget::buildHashTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Demonstrates common hash functions: djb2, FNV-1a, and simple polynomial hash. Shows the hash value in decimal and hex for any input string.", w));
    auto* g=new QGridLayout(); g->setSpacing(6);
    m_hashInput=new QLineEdit("hello world",w); m_hashInput->setMinimumWidth(200);
    g->addWidget(new QLabel("Input:",w),0,0); g->addWidget(m_hashInput,0,1);
    g->setColumnStretch(1,1);
    v->addLayout(g);
    m_hashResult=resultLbl(w);
    v->addWidget(mkBtn("Hash",w)); v->addWidget(m_hashResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&McsWidget::computeHash);
    return w;
}

void McsWidget::computeHash() {
    QByteArray input=m_hashInput->text().toUtf8();
    // djb2
    uint32_t djb2=5381;
    for(unsigned char c:input) djb2=((djb2<<5)+djb2)+c;
    // FNV-1a 32-bit
    uint32_t fnv=2166136261u;
    for(unsigned char c:input){ fnv^=c; fnv*=16777619u; }
    // Simple polynomial hash
    uint32_t poly=0;
    for(unsigned char c:input) poly=poly*31+c;
    m_hashResult->setText(QString(
        "Input: \"%1\" (%2 bytes)\n\n"
        "djb2:    %3  (0x%4)\n"
        "FNV-1a:  %5  (0x%6)\n"
        "Poly-31: %7  (0x%8)")
        .arg(m_hashInput->text()).arg(input.size())
        .arg(djb2).arg(djb2,8,16,QChar('0')).toUpper()
        .arg(fnv).arg(fnv,8,16,QChar('0')).toUpper()
        .arg(poly).arg(poly,8,16,QChar('0')).toUpper());
}
