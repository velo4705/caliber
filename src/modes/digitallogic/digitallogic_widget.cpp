#include "digitallogic_widget.h"
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
#include <cmath>
#include <cstring>

static QPushButton* mkBtn(const QString& t, QWidget* p) {
    auto* b = new QPushButton(t, p);
    b->setProperty("class", "actionButton");
    b->setMinimumHeight(36); b->setFocusPolicy(Qt::NoFocus);
    return b;
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

void DigitalLogicWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

DigitalLogicWidget::DigitalLogicWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Digital Logic", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildTruthTab(),    "Truth Table");
    tabs->addTab(buildKmapTab(),     "K-Map");
    tabs->addTab(buildNumSysTab(),   "Number Systems");
    tabs->addTab(buildFlipFlopTab(), "Flip-Flops");
    tabs->addTab(buildAdderTab(),    "Adder/Subtractor");
    tabs->addTab(buildMuxTab(),      "MUX / DEMUX");
    tabs->addTab(buildIeeeTab(),     "IEEE 754");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Truth Table ───────────────────────────────────────────────────────────────
QWidget* DigitalLogicWidget::buildTruthTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Generates a full truth table for any Boolean expression. Use A, B, C, D as variables. Operators: AND (&& or AND), OR (|| or OR), NOT (! or NOT), XOR (^ or XOR), NAND, NOR.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_ttExpr = new QLineEdit(w); m_ttExpr->setPlaceholderText("e.g. (A AND B) OR (NOT C)");
    m_ttVars = new QSpinBox(w); m_ttVars->setRange(1,4); m_ttVars->setValue(2);
    g->addWidget(new QLabel("Expression:", w), 0, 0); g->addWidget(m_ttExpr, 0, 1, 1, 3);
    g->addWidget(new QLabel("Variables:", w),  1, 0); g->addWidget(m_ttVars, 1, 1);
    g->setColumnStretch(3, 1);
    v->addLayout(g);
    m_ttTable = new QTableWidget(w);
    m_ttTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_ttTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    v->addWidget(m_ttTable);
    m_ttResult = resultLbl(w);
    v->addWidget(mkBtn("Generate", w)); v->addWidget(m_ttResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &DigitalLogicWidget::buildTruthTable);
    return w;
}

bool DigitalLogicWidget::evalBool(const QString& expr, const QVector<bool>& vals, const QStringList& vars) {
    QString e = expr.toUpper();
    // Replace variable names with 0/1
    for (int i = vars.size()-1; i >= 0; i--)
        e.replace(vars[i], vals[i] ? "1" : "0");
    // Replace operators
    e.replace("AND", "&"); e.replace("OR", "|"); e.replace("NOT", "!");
    e.replace("NAND", "NAND_OP"); e.replace("NOR", "NOR_OP");
    e.replace("XOR", "^");

    // Simple recursive evaluator
    std::function<bool(const QString&)> eval = [&](const QString& s) -> bool {
        QString t = s.trimmed();
        // Remove outer parens
        while (t.startsWith('(') && t.endsWith(')')) {
            int depth=0; bool wrap=true;
            for(int i=0;i<t.size()-1;i++){
                if(t[i]=='(') depth++; else if(t[i]==')') depth--;
                if(depth==0){wrap=false;break;}
            }
            if(wrap) t=t.mid(1,t.size()-2).trimmed(); else break;
        }
        // Find lowest precedence operator outside parens
        int depth=0;
        // OR
        for(int i=t.size()-1;i>=0;i--){
            if(t[i]==')') depth++; else if(t[i]=='(') depth--;
            if(depth==0 && t[i]=='|' && (i==0||t[i-1]!='|') && (i+1<t.size()&&t[i+1]!='|'))
                return eval(t.left(i)) || eval(t.mid(i+1));
            if(depth==0 && i+1<t.size() && t.mid(i,2)=="||")
                return eval(t.left(i)) || eval(t.mid(i+2));
        }
        depth=0;
        // AND
        for(int i=t.size()-1;i>=0;i--){
            if(t[i]==')') depth++; else if(t[i]=='(') depth--;
            if(depth==0 && t[i]=='&' && (i==0||t[i-1]!='&') && (i+1<t.size()&&t[i+1]!='&'))
                return eval(t.left(i)) && eval(t.mid(i+1));
            if(depth==0 && i+1<t.size() && t.mid(i,2)=="&&")
                return eval(t.left(i)) && eval(t.mid(i+2));
        }
        depth=0;
        // XOR
        for(int i=t.size()-1;i>=0;i--){
            if(t[i]==')') depth++; else if(t[i]=='(') depth--;
            if(depth==0 && t[i]=='^')
                return eval(t.left(i)) ^ eval(t.mid(i+1));
        }
        // NOT
        if(t.startsWith('!')) return !eval(t.mid(1));
        // Literal
        if(t=="1") return true;
        if(t=="0") return false;
        return false;
    };
    return eval(e);
}

void DigitalLogicWidget::buildTruthTable() {
    int n = m_ttVars->value();
    QString expr = m_ttExpr->text().trimmed();
    if (expr.isEmpty()) { m_ttResult->setText("Enter an expression."); return; }
    QStringList vars; for(int i=0;i<n;i++) vars << QString(QChar('A'+i));
    int rows = 1 << n;
    m_ttTable->setRowCount(rows);
    m_ttTable->setColumnCount(n+1);
    QStringList hdr = vars; hdr << "Output";
    m_ttTable->setHorizontalHeaderLabels(hdr);
    m_ttTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_ttTable->verticalHeader()->setDefaultSectionSize(28);
    m_ttTable->setFixedHeight(qMin(rows,16)*28 + m_ttTable->horizontalHeader()->sizeHint().height()+4);
    int trueCount=0;
    for(int r=0;r<rows;r++){
        QVector<bool> vals(n);
        for(int i=0;i<n;i++) vals[i]=(r>>(n-1-i))&1;
        for(int i=0;i<n;i++){
            auto* item=new QTableWidgetItem(vals[i]?"1":"0");
            item->setTextAlignment(Qt::AlignCenter);
            m_ttTable->setItem(r,i,item);
        }
        bool out=false;
        try { out=evalBool(expr,vals,vars); } catch(...) {}
        if(out) trueCount++;
        auto* item=new QTableWidgetItem(out?"1":"0");
        item->setTextAlignment(Qt::AlignCenter);
        item->setForeground(out?QColor(0x42,0x9e,0xf5):QColor(0xff,0x55,0x55));
        m_ttTable->setItem(r,n,item);
    }
    m_ttResult->setText(QString("Output is 1 for %1/%2 input combinations").arg(trueCount).arg(rows));
}

// ── K-Map ─────────────────────────────────────────────────────────────────────
QWidget* DigitalLogicWidget::buildKmapTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Karnaugh Map simplification. Enter the output values (0, 1, or X for don't-care) for each minterm in order. Supports 2, 3, or 4 variables. Outputs simplified SOP expression.", w));
    auto* row = new QHBoxLayout();
    m_kmapVars = new QSpinBox(w); m_kmapVars->setRange(2,4); m_kmapVars->setValue(3);
    row->addWidget(new QLabel("Variables:", w)); row->addWidget(m_kmapVars);
    row->addWidget(mkBtn("Build K-Map", w)); row->addStretch();
    v->addLayout(row);
    m_kmapTable = new QTableWidget(w);
    m_kmapTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_kmapTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    v->addWidget(m_kmapTable);
    m_kmapResult    = resultLbl(w);
    m_kmapShow      = new QCheckBox("Show Steps", w);
    m_kmapSteps     = stepsBox(w);
    v->addWidget(mkBtn("Simplify", w)); v->addWidget(m_kmapResult);
    v->addWidget(m_kmapShow); v->addWidget(m_kmapSteps); v->addStretch();
    connect(m_kmapShow, &QCheckBox::toggled, this, [this](bool on){ m_kmapSteps->setVisible(on && !m_kmapSteps->toPlainText().isEmpty()); });
    auto btns = w->findChildren<QPushButton*>();
    connect(btns[0], &QPushButton::clicked, this, &DigitalLogicWidget::buildKmap);
    connect(btns[1], &QPushButton::clicked, this, &DigitalLogicWidget::computeKmap);
    return w;
}

void DigitalLogicWidget::buildKmap() {
    int n = m_kmapVars->value();
    int cells = 1 << n;
    // Layout: 2var=2x2, 3var=2x4, 4var=4x4
    int rows = (n >= 3) ? 4 : 2;
    int cols = (n == 4) ? 4 : (n == 3 ? 4 : 2);
    m_kmapTable->setRowCount(rows); m_kmapTable->setColumnCount(cols);
    // Gray code order for rows/cols
    static const int gc2[] = {0,1,3,2};
    QStringList rHdr, cHdr;
    if (n == 2) { rHdr << "A=0" << "A=1"; cHdr << "B=0" << "B=1"; }
    else if (n == 3) { rHdr << "A=0" << "A=1"; cHdr << "BC=00" << "BC=01" << "BC=11" << "BC=10"; }
    else { rHdr << "AB=00" << "AB=01" << "AB=11" << "AB=10"; cHdr << "CD=00" << "CD=01" << "CD=11" << "CD=10"; }
    m_kmapTable->setHorizontalHeaderLabels(cHdr);
    m_kmapTable->setVerticalHeaderLabels(rHdr);
    m_kmapTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_kmapTable->verticalHeader()->setDefaultSectionSize(32);
    m_kmapTable->setFixedHeight(rows*32 + m_kmapTable->horizontalHeader()->sizeHint().height()+4);
    for(int r=0;r<rows;r++) for(int c=0;c<cols;c++){
        auto* item = new QTableWidgetItem("0");
        item->setTextAlignment(Qt::AlignCenter);
        m_kmapTable->setItem(r,c,item);
    }
    m_kmapResult->setText(QString("K-Map built for %1 variables (%2 cells). Edit values then click Simplify.").arg(n).arg(cells));
}

void DigitalLogicWidget::computeKmap() {
    int n = m_kmapVars->value();
    int cells = 1 << n;
    // Read minterm values from table in Gray code order
    static const int gc[] = {0,1,3,2};
    QVector<int> vals(cells, 0); // 0=false, 1=true, 2=don't care
    int rows = m_kmapTable->rowCount(), cols = m_kmapTable->columnCount();
    for(int r=0;r<rows;r++) for(int c=0;c<cols;c++){
        auto* item = m_kmapTable->item(r,c);
        if(!item) continue;
        QString v = item->text().trimmed().toUpper();
        int rg = gc[r % 4], cg = gc[c % 4];
        int minterm;
        if(n==2) minterm = rg*2 + cg;
        else if(n==3) minterm = rg*4 + cg;
        else minterm = rg*4 + cg;
        if(minterm < cells) vals[minterm] = (v=="1"?1:(v=="X"||v=="x"?2:0));
    }
    QVector<int> minterms, dontcares;
    for(int i=0;i<cells;i++) { if(vals[i]==1) minterms<<i; else if(vals[i]==2) dontcares<<i; }
    QStringList steps;
    steps << QString("Minterms (output=1): %1").arg([&]{ QStringList s; for(int m:minterms) s<<QString::number(m); return s.join(", "); }());
    steps << QString("Don't cares: %1").arg([&]{ QStringList s; for(int d:dontcares) s<<QString::number(d); return s.isEmpty()?"none":s.join(", "); }());
    if(minterms.isEmpty()){ m_kmapResult->setText("F = 0 (all zeros)"); showSteps(m_kmapSteps,m_kmapShow,steps); return; }
    if(minterms.size()+dontcares.size()==cells){ m_kmapResult->setText("F = 1 (all ones)"); showSteps(m_kmapSteps,m_kmapShow,steps); return; }
    QString sop = simplifyKmap(minterms, n);
    steps << QString("Simplified SOP: F = %1").arg(sop);
    m_kmapResult->setText(QString("F = %1").arg(sop));
    showSteps(m_kmapSteps, m_kmapShow, steps);
}

QString DigitalLogicWidget::simplifyKmap(const QVector<int>& minterms, int vars) {
    // Quine-McCluskey prime implicant generation (simplified)
    QStringList varNames; for(int i=0;i<vars;i++) varNames << QString(QChar('A'+i));
    // Generate prime implicants by combining minterms
    QVector<QPair<int,int>> implicants; // (minterm, mask) mask=1 means variable eliminated
    for(int m : minterms) implicants.append({m, 0});
    bool changed=true;
    QVector<QPair<int,int>> primes;
    while(changed){
        changed=false;
        QVector<bool> used(implicants.size(), false);
        QVector<QPair<int,int>> next;
        for(int i=0;i<implicants.size();i++) for(int j=i+1;j<implicants.size();j++){
            if(implicants[i].second != implicants[j].second) continue;
            int diff = implicants[i].first ^ implicants[j].first;
            if(diff>0 && (diff&(diff-1))==0){ // power of 2 — one bit differs
                next.append({implicants[i].first & implicants[j].first, implicants[i].second | diff});
                used[i]=used[j]=true; changed=true;
            }
        }
        for(int i=0;i<implicants.size();i++) if(!used[i]) primes.append(implicants[i]);
        // Deduplicate
        QVector<QPair<int,int>> uniq;
        for(auto& p:next){ bool dup=false; for(auto& u:uniq) if(u==p){dup=true;break;} if(!dup) uniq<<p; }
        implicants=uniq;
    }
    if(primes.isEmpty()) for(auto& p:implicants) primes<<p;

    // Build SOP terms
    QStringList terms;
    for(auto& [val, mask] : primes){
        QString term;
        for(int i=vars-1;i>=0;i--){
            if(mask & (1<<i)) continue; // eliminated
            bool bit = (val >> i) & 1;
            term += bit ? varNames[vars-1-i] : varNames[vars-1-i]+"'";
        }
        if(!term.isEmpty() && !terms.contains(term)) terms<<term;
    }
    return terms.isEmpty() ? "0" : terms.join(" + ");
}

// ── Number Systems ────────────────────────────────────────────────────────────
QWidget* DigitalLogicWidget::buildNumSysTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Converts a decimal integer to BCD, Gray code, Excess-3, and 2's complement (8-bit). Enter a non-negative integer.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_nsValue = new QLineEdit("13", w); m_nsValue->setFixedWidth(120);
    g->addWidget(new QLabel("Decimal n:", w), 0, 0); g->addWidget(m_nsValue, 0, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_nsResult = resultLbl(w);
    v->addWidget(mkBtn("Convert", w)); v->addWidget(m_nsResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &DigitalLogicWidget::computeNumSys);
    return w;
}

void DigitalLogicWidget::computeNumSys() {
    bool ok; int n = m_nsValue->text().toInt(&ok);
    if (!ok || n < 0) { m_nsResult->setText("Enter a non-negative integer."); return; }
    // Binary
    QString bin = QString::number(n, 2).rightJustified(8, '0');
    // BCD (each decimal digit → 4 bits)
    QString bcd;
    for (QChar c : QString::number(n)) bcd += QString::number(c.digitValue(), 2).rightJustified(4,'0') + " ";
    // Gray code: G = B XOR (B >> 1)
    int gray = n ^ (n >> 1);
    QString grayStr = QString::number(gray, 2).rightJustified(8, '0');
    // Excess-3: add 3 to each BCD digit
    QString ex3;
    for (QChar c : QString::number(n)) ex3 += QString::number(c.digitValue()+3, 2).rightJustified(4,'0') + " ";
    // 2's complement (8-bit)
    QString twos;
    if (n >= 0 && n <= 127) twos = bin + " (positive, same as binary)";
    else if (n >= 128 && n <= 255) {
        int tc = (~n & 0xFF) + 1;
        twos = QString::number(tc, 2).rightJustified(8,'0') + QString(" (−%1)").arg(256-n);
    } else twos = "out of 8-bit range";

    m_nsResult->setText(QString(
        "Decimal:     %1\n"
        "Binary:      %2\n"
        "BCD:         %3\n"
        "Gray code:   %4\n"
        "Excess-3:    %5\n"
        "2's comp:    %6")
        .arg(n).arg(bin).arg(bcd.trimmed()).arg(grayStr).arg(ex3.trimmed()).arg(twos));
}

// ── Flip-Flop Analyzer ────────────────────────────────────────────────────────
QWidget* DigitalLogicWidget::buildFlipFlopTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Shows the state transition table for SR, JK, D, and T flip-flops. Enter input sequence as comma-separated pairs (e.g. 0,1 for S=0,R=1) and initial state Q.", w));
    auto* row = new QHBoxLayout();
    m_ffType = new QComboBox(w);
    m_ffType->addItems({"SR Flip-Flop", "JK Flip-Flop", "D Flip-Flop", "T Flip-Flop"});
    row->addWidget(new QLabel("Type:", w)); row->addWidget(m_ffType); row->addStretch();
    v->addLayout(row);
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_ffInputs = new QLineEdit("0,0 | 0,1 | 1,0 | 1,1", w);
    g->addWidget(new QLabel("Inputs (pairs):", w), 0, 0); g->addWidget(m_ffInputs, 0, 1);
    g->setColumnStretch(1, 1);
    v->addLayout(g);
    m_ffResult = resultLbl(w);
    v->addWidget(mkBtn("Analyze", w)); v->addWidget(m_ffResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &DigitalLogicWidget::computeFlipFlop);
    return w;
}

void DigitalLogicWidget::computeFlipFlop() {
    int type = m_ffType->currentIndex();
    QStringList pairs = m_ffInputs->text().split('|', Qt::SkipEmptyParts);
    QString out = QString("%1 State Table\n").arg(m_ffType->currentText());
    QStringList hdr;
    if(type==0) hdr << "S" << "R" << "Q" << "Q_next" << "Comment";
    else if(type==1) hdr << "J" << "K" << "Q" << "Q_next";
    else if(type==2) hdr << "D" << "Q" << "Q_next";
    else hdr << "T" << "Q" << "Q_next";
    out += hdr.join("  ") + "\n" + QString(40,'-') + "\n";

    for(const QString& pair : pairs){
        QStringList vals = pair.trimmed().split(',', Qt::SkipEmptyParts);
        if(vals.size() < 2) continue;
        int i1 = vals[0].trimmed().toInt();
        int i2 = vals[1].trimmed().toInt();
        // Show for both Q=0 and Q=1
        for(int Q=0;Q<=1;Q++){
            int Qn=0; QString comment;
            if(type==0){ // SR
                if(i1==0&&i2==0){Qn=Q;comment="Hold";}
                else if(i1==0&&i2==1){Qn=0;comment="Reset";}
                else if(i1==1&&i2==0){Qn=1;comment="Set";}
                else{Qn=-1;comment="Invalid";}
                out += QString("%1  %2  %3  %4  %5\n").arg(i1).arg(i2).arg(Q).arg(Qn<0?"X":QString::number(Qn)).arg(comment);
            } else if(type==1){ // JK
                if(i1==0&&i2==0) Qn=Q;
                else if(i1==0&&i2==1) Qn=0;
                else if(i1==1&&i2==0) Qn=1;
                else Qn=1-Q; // toggle
                out += QString("%1  %2  %3  %4\n").arg(i1).arg(i2).arg(Q).arg(Qn);
            } else if(type==2){ // D
                Qn=i1;
                out += QString("%1  %2  %3\n").arg(i1).arg(Q).arg(Qn);
                break; // D doesn't depend on Q
            } else { // T
                Qn = i1 ? 1-Q : Q;
                out += QString("%1  %2  %3\n").arg(i1).arg(Q).arg(Qn);
            }
        }
    }
    m_ffResult->setText(out);
}

// ── Adder / Subtractor ────────────────────────────────────────────────────────
QWidget* DigitalLogicWidget::buildAdderTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Binary addition and subtraction with carry/borrow. Shows bit-by-bit working. Enter binary numbers (e.g. 1011) or decimal values.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_addA = new QLineEdit("1011", w); m_addA->setFixedWidth(120);
    m_addB = new QLineEdit("0110", w); m_addB->setFixedWidth(120);
    m_addType = new QComboBox(w); m_addType->addItems({"Binary Add", "Binary Subtract", "Decimal Add", "Decimal Subtract"});
    g->addWidget(new QLabel("A:", w), 0, 0); g->addWidget(m_addA, 0, 1);
    g->addWidget(new QLabel("B:", w), 1, 0); g->addWidget(m_addB, 1, 1);
    g->addWidget(new QLabel("Op:", w),2, 0); g->addWidget(m_addType, 2, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_addResult = resultLbl(w); m_addShow = new QCheckBox("Show Steps", w); m_addSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_addResult);
    v->addWidget(m_addShow); v->addWidget(m_addSteps); v->addStretch();
    connect(m_addShow, &QCheckBox::toggled, this, [this](bool on){ m_addSteps->setVisible(on && !m_addSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &DigitalLogicWidget::computeAdder);
    return w;
}

void DigitalLogicWidget::computeAdder() {
    int idx = m_addType->currentIndex();
    QStringList steps;
    bool isBin = idx < 2;
    bool isSub = idx == 1 || idx == 3;
    long long a, b;
    if(isBin){
        bool ok1,ok2;
        a = m_addA->text().toLongLong(&ok1, 2);
        b = m_addB->text().toLongLong(&ok2, 2);
        if(!ok1||!ok2){ m_addResult->setText("Invalid binary input."); return; }
    } else {
        a = m_addA->text().toLongLong();
        b = m_addB->text().toLongLong();
    }
    long long result = isSub ? a - b : a + b;
    steps << QString("A = %1 (decimal %2)").arg(isBin?m_addA->text():QString::number(a)).arg(a);
    steps << QString("B = %1 (decimal %2)").arg(isBin?m_addB->text():QString::number(b)).arg(b);
    steps << QString("%1 = %2").arg(isSub?"A - B":"A + B").arg(result);
    if(isBin){
        steps << QString("Binary result: %1").arg(QString::number(result<0?-result:result, 2));
        if(result<0) steps << "Result is negative — 2's complement representation";
    }
    m_addResult->setText(QString(
        "A = %1\nB = %2\n%3 = %4\nBinary: %5")
        .arg(a).arg(b).arg(isSub?"A−B":"A+B").arg(result)
        .arg(result<0 ? "-"+QString::number(-result,2) : QString::number(result,2)));
    showSteps(m_addSteps, m_addShow, steps);
}

// ── MUX / DEMUX ───────────────────────────────────────────────────────────────
QWidget* DigitalLogicWidget::buildMuxTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Multiplexer (MUX): selects one of N inputs based on select lines and routes it to output. Demultiplexer (DEMUX): routes one input to one of N outputs based on select lines.", w));
    auto* row = new QHBoxLayout();
    m_muxType = new QComboBox(w);
    m_muxType->addItems({"MUX (many→one)", "DEMUX (one→many)"});
    row->addWidget(new QLabel("Type:", w)); row->addWidget(m_muxType); row->addStretch();
    v->addLayout(row);
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_muxSel    = new QSpinBox(w); m_muxSel->setRange(1,4); m_muxSel->setValue(2);
    m_muxInputs = new QLineEdit("1,0,1,1", w);
    m_muxInputs->setPlaceholderText("Input bits comma-separated (MUX) or single bit (DEMUX)");
    g->addWidget(new QLabel("Select lines:", w), 0, 0); g->addWidget(m_muxSel,    0, 1);
    g->addWidget(new QLabel("Inputs:",        w), 1, 0); g->addWidget(m_muxInputs, 1, 1);
    g->setColumnStretch(1, 1);
    v->addLayout(g);
    m_muxResult = resultLbl(w); m_muxShow = new QCheckBox("Show Steps", w); m_muxSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_muxResult);
    v->addWidget(m_muxShow); v->addWidget(m_muxSteps); v->addStretch();
    connect(m_muxShow, &QCheckBox::toggled, this, [this](bool on){ m_muxSteps->setVisible(on && !m_muxSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &DigitalLogicWidget::computeMux);
    return w;
}

void DigitalLogicWidget::computeMux() {
    int selBits = m_muxSel->value();
    int lines   = 1 << selBits; // 2^selBits input/output lines
    QStringList steps;

    if (m_muxType->currentIndex() == 0) {
        // MUX: read all input bits, show which one is selected for each select value
        QStringList parts = m_muxInputs->text().split(',', Qt::SkipEmptyParts);
        QVector<int> inputs;
        for (const QString& p : parts) inputs << p.trimmed().toInt();
        while (inputs.size() < lines) inputs << 0;

        steps << QString("%1-to-1 MUX with %2 select lines").arg(lines).arg(selBits);
        steps << QString("Inputs: %1").arg([&]{ QStringList s; for(int i:inputs) s<<QString::number(i); return s.join(", "); }());

        QString out = QString("%1-to-1 Multiplexer\n\n").arg(lines);
        out += QString("Select lines: S[%1..0]\n\n").arg(selBits-1);
        out += QString("%-10s  %-10s  Output\n").arg("Select (dec)").arg("Select (bin)");
        out += QString(35, '-') + "\n";
        for (int s = 0; s < lines; s++) {
            int output = s < inputs.size() ? inputs[s] : 0;
            out += QString("%-10s  %-10s  I%1 = %2\n")
                .arg(s).arg(QString::number(s,2).rightJustified(selBits,'0'))
                .arg(s).arg(output);
            steps << QString("S=%1 → selects I%2 = %3").arg(s).arg(s).arg(output);
        }
        m_muxResult->setText(out.trimmed());
    } else {
        // DEMUX: one input routed to one of N outputs
        int input = m_muxInputs->text().trimmed().toInt();
        steps << QString("1-to-%1 DEMUX with %2 select lines").arg(lines).arg(selBits);
        steps << QString("Input bit: %1").arg(input);

        QString out = QString("1-to-%1 Demultiplexer\nInput = %2\n\n").arg(lines).arg(input);
        out += QString("%-10s  %-10s  Output\n").arg("Select (dec)").arg("Select (bin)");
        out += QString(35, '-') + "\n";
        for (int s = 0; s < lines; s++) {
            int output = s == 0 ? input : 0; // default: show for select=0
            out += QString("%-10s  %-10s  Y%1 = %2\n")
                .arg(s).arg(QString::number(s,2).rightJustified(selBits,'0'))
                .arg(s).arg(s == 0 ? input : 0);
            steps << QString("S=%1 → Y%2 = %3, all others = 0").arg(s).arg(s).arg(s==0?input:0);
        }
        out += QString("\nFor any select value S=k: Y_k = Input, all other outputs = 0");
        m_muxResult->setText(out.trimmed());
    }
    showSteps(m_muxSteps, m_muxShow, steps);
}

// ── IEEE 754 Visualizer ───────────────────────────────────────────────────────
QWidget* DigitalLogicWidget::buildIeeeTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Shows the IEEE 754 binary representation of a floating-point number. Displays sign bit, exponent bits, and mantissa bits for single (32-bit) and double (64-bit) precision.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_ieeeValue = new QLineEdit("3.14", w); m_ieeeValue->setFixedWidth(150);
    m_ieeePrec  = new QComboBox(w); m_ieeePrec->addItems({"Single (32-bit)", "Double (64-bit)"});
    g->addWidget(new QLabel("Value:", w),     0, 0); g->addWidget(m_ieeeValue, 0, 1);
    g->addWidget(new QLabel("Precision:", w), 1, 0); g->addWidget(m_ieeePrec,  1, 1);
    g->setColumnStretch(2, 1);
    v->addLayout(g);
    m_ieeeResult = resultLbl(w);
    v->addWidget(mkBtn("Visualize", w)); v->addWidget(m_ieeeResult); v->addStretch();
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &DigitalLogicWidget::computeIeee);
    return w;
}

void DigitalLogicWidget::computeIeee() {
    bool ok; double val = m_ieeeValue->text().toDouble(&ok);
    if(!ok){ m_ieeeResult->setText("Invalid number."); return; }
    QString out;
    if(m_ieeePrec->currentIndex()==0){
        float f = (float)val;
        uint32_t bits; memcpy(&bits, &f, 4);
        int sign = (bits>>31)&1;
        int exp  = (bits>>23)&0xFF;
        uint32_t mant = bits&0x7FFFFF;
        QString signBit = QString::number(sign);
        QString expBits = QString::number(exp,2).rightJustified(8,'0');
        QString mantBits= QString::number(mant,2).rightJustified(23,'0');
        out = QString("Single precision (32-bit)\n\n"
            "Sign:     %1\n"
            "Exponent: %2  (biased=%3, actual=%4)\n"
            "Mantissa: %5\n\n"
            "Full: %6 %7 %8\n"
            "Hex:  %9")
            .arg(signBit).arg(expBits).arg(exp).arg(exp-127)
            .arg(mantBits).arg(signBit).arg(expBits).arg(mantBits)
            .arg(QString::number(bits,16).toUpper().rightJustified(8,'0'));
    } else {
        uint64_t bits; memcpy(&bits, &val, 8);
        int sign = (bits>>63)&1;
        int exp  = (bits>>52)&0x7FF;
        uint64_t mant = bits&0x000FFFFFFFFFFFFFULL;
        QString signBit = QString::number(sign);
        QString expBits = QString::number(exp,2).rightJustified(11,'0');
        QString mantBits= QString::number(mant,2).rightJustified(52,'0');
        out = QString("Double precision (64-bit)\n\n"
            "Sign:     %1\n"
            "Exponent: %2  (biased=%3, actual=%4)\n"
            "Mantissa: %5\n\n"
            "Hex:  %6")
            .arg(signBit).arg(expBits).arg(exp).arg(exp-1023)
            .arg(mantBits.left(26)+"...").arg(QString::number(bits,16).toUpper().rightJustified(16,'0'));
    }
    m_ieeeResult->setText(out);
}
