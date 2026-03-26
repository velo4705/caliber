#include "chemistry_widget.h"
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
#include <QMap>
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

void ChemistryWidget::showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s) {
    if (!t->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < s.size(); ++i) w->append(QString("Step %1: %2").arg(i+1).arg(s[i]));
    w->show();
}

// Atomic masses (g/mol)
static const QMap<QString,double> ATOMIC_MASS = {
    {"H",1.008},{"He",4.003},{"Li",6.941},{"Be",9.012},{"B",10.811},
    {"C",12.011},{"N",14.007},{"O",15.999},{"F",18.998},{"Ne",20.180},
    {"Na",22.990},{"Mg",24.305},{"Al",26.982},{"Si",28.086},{"P",30.974},
    {"S",32.065},{"Cl",35.453},{"Ar",39.948},{"K",39.098},{"Ca",40.078},
    {"Fe",55.845},{"Cu",63.546},{"Zn",65.38},{"Ag",107.868},{"Au",196.967},
    {"Pb",207.2},{"Hg",200.59},{"Br",79.904},{"I",126.904},{"Mn",54.938},
    {"Cr",51.996},{"Ni",58.693},{"Co",58.933},{"Ba",137.327},{"Sr",87.62},
    {"Sn",118.710},{"Ti",47.867},{"V",50.942},{"W",183.84},{"U",238.029}
};

double ChemistryWidget::parseMolarMass(const QString& formula, QStringList& steps) {
    double total = 0;
    int i = 0;
    while (i < formula.size()) {
        if (formula[i].isUpper()) {
            // Read element symbol
            QString sym = formula[i];
            i++;
            while (i < formula.size() && formula[i].isLower()) { sym += formula[i]; i++; }
            // Read count
            QString numStr;
            while (i < formula.size() && formula[i].isDigit()) { numStr += formula[i]; i++; }
            int count = numStr.isEmpty() ? 1 : numStr.toInt();
            if (ATOMIC_MASS.contains(sym)) {
                double mass = ATOMIC_MASS[sym] * count;
                total += mass;
                steps << QString("%1 × %2 = %3 g/mol").arg(count).arg(sym).arg(mass,0,'f',3);
            } else {
                steps << QString("Unknown element: %1").arg(sym);
            }
        } else {
            i++; // skip unknown chars
        }
    }
    return total;
}

ChemistryWidget::ChemistryWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16); root->setSpacing(12);
    auto* title = new QLabel("Chemistry", this);
    title->setStyleSheet("font-size:18px;font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildMolarMassTab(), "Molar Mass");
    tabs->addTab(buildIdealGasTab(),  "Ideal Gas");
    tabs->addTab(buildPhTab(),        "pH / pOH");
    tabs->addTab(buildDilutionTab(),  "Dilution");
    tabs->addTab(buildThermochemTab(),"Thermochemistry");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Molar Mass ────────────────────────────────────────────────────────────────
QWidget* ChemistryWidget::buildMolarMassTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Enter a chemical formula (e.g. H2O, NaCl, C6H12O6) to compute its molar mass. Supports all common elements.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_mmFormula = new QLineEdit("H2O", w); m_mmFormula->setMinimumWidth(200);
    g->addWidget(new QLabel("Formula:", w), 0, 0); g->addWidget(m_mmFormula, 0, 1);
    g->setColumnStretch(1, 1);
    v->addLayout(g);
    m_mmResult = resultLbl(w); m_mmShow = new QCheckBox("Show Steps", w); m_mmSteps = stepsBox(w);
    v->addWidget(mkBtn("Compute", w)); v->addWidget(m_mmResult);
    v->addWidget(m_mmShow); v->addWidget(m_mmSteps); v->addStretch();
    connect(m_mmShow, &QCheckBox::toggled, this, [this](bool on){ m_mmSteps->setVisible(on && !m_mmSteps->toPlainText().isEmpty()); });
    connect(w->findChildren<QPushButton*>().last(), &QPushButton::clicked, this, &ChemistryWidget::computeMolarMass);
    return w;
}

void ChemistryWidget::computeMolarMass() {
    QString formula = m_mmFormula->text().trimmed();
    if (formula.isEmpty()) { m_mmResult->setText("Enter a formula."); return; }
    QStringList steps;
    steps << QString("Formula: %1").arg(formula);
    double mm = parseMolarMass(formula, steps);
    steps << QString("Molar mass = %1 g/mol").arg(mm, 0, 'f', 3);
    m_mmResult->setText(QString("Molar mass of %1 = %2 g/mol").arg(formula).arg(mm, 0, 'f', 3));
    showSteps(m_mmSteps, m_mmShow, steps);
}

// ── Ideal Gas ─────────────────────────────────────────────────────────────────
QWidget* ChemistryWidget::buildIdealGasTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Ideal gas law: PV = nRT where R = 8.314 J/mol·K. Enter any three values — leave the unknown empty.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_igP = numEdit("101325","P (Pa)",w); m_igV = numEdit("0.001","V (m³)",w);
    m_ign = numEdit("","n (mol)",w);      m_igT = numEdit("298","T (K)",w);
    g->addWidget(new QLabel("P (Pa):",w),0,0); g->addWidget(m_igP,0,1);
    g->addWidget(new QLabel("V (m³):",w),1,0); g->addWidget(m_igV,1,1);
    g->addWidget(new QLabel("n (mol):",w),2,0); g->addWidget(m_ign,2,1);
    g->addWidget(new QLabel("T (K):",w),3,0);  g->addWidget(m_igT,3,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_igResult=resultLbl(w); m_igShow=new QCheckBox("Show Steps",w); m_igSteps=stepsBox(w);
    v->addWidget(mkBtn("Solve",w)); v->addWidget(m_igResult);
    v->addWidget(m_igShow); v->addWidget(m_igSteps); v->addStretch();
    connect(m_igShow,&QCheckBox::toggled,this,[this](bool on){m_igSteps->setVisible(on&&!m_igSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&ChemistryWidget::computeIdealGas);
    return w;
}

void ChemistryWidget::computeIdealGas() {
    const double R = 8.314;
    bool hP=!m_igP->text().isEmpty(), hV=!m_igV->text().isEmpty();
    bool hn=!m_ign->text().isEmpty(), hT=!m_igT->text().isEmpty();
    double P=m_igP->text().toDouble(), V=m_igV->text().toDouble();
    double n=m_ign->text().toDouble(), T=m_igT->text().toDouble();
    QStringList steps;
    steps << "PV = nRT,  R = 8.314 J/mol·K";
    if (!hP && hV&&hn&&hT) { P=n*R*T/V; steps<<QString("P=nRT/V=%1 Pa").arg(P,0,'g',8); }
    else if (!hV && hP&&hn&&hT) { V=n*R*T/P; steps<<QString("V=nRT/P=%1 m³").arg(V,0,'g',8); }
    else if (!hn && hP&&hV&&hT) { n=P*V/(R*T); steps<<QString("n=PV/RT=%1 mol").arg(n,0,'g',8); }
    else if (!hT && hP&&hV&&hn) { T=P*V/(n*R); steps<<QString("T=PV/nR=%1 K").arg(T,0,'g',8); }
    else { n=P*V/(R*T); steps<<QString("n=PV/RT=%1 mol").arg(n,0,'g',8); }
    steps << QString("Verification: PV=%1, nRT=%2").arg(P*V,0,'g',6).arg(n*R*T,0,'g',6);
    m_igResult->setText(QString("P = %1 Pa\nV = %2 m³\nn = %3 mol\nT = %4 K")
        .arg(P,0,'g',8).arg(V,0,'g',8).arg(n,0,'g',8).arg(T,0,'g',8));
    showSteps(m_igSteps,m_igShow,steps);
}

// ── pH / pOH ──────────────────────────────────────────────────────────────────
QWidget* ChemistryWidget::buildPhTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("pH = −log[H⁺]. For weak acids: [H⁺]=√(Ka×C). For weak bases: [OH⁻]=√(Kb×C), pOH=−log[OH⁻], pH=14−pOH. For strong acids/bases enter concentration directly.", w));
    auto* row = new QHBoxLayout();
    m_phType = new QComboBox(w);
    m_phType->addItems({"Strong Acid/Base [H⁺] or [OH⁻]", "Weak Acid (Ka)", "Weak Base (Kb)"});
    row->addWidget(new QLabel("Type:",w)); row->addWidget(m_phType); row->addStretch();
    v->addLayout(row);
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_phConc=numEdit("0.1","C (mol/L)",w); m_phKa=numEdit("1.8e-5","Ka",w); m_phKb=numEdit("1.8e-5","Kb",w);
    g->addWidget(new QLabel("Concentration C:",w),0,0); g->addWidget(m_phConc,0,1);
    g->addWidget(new QLabel("Ka:",w),1,0); g->addWidget(m_phKa,1,1);
    g->addWidget(new QLabel("Kb:",w),2,0); g->addWidget(m_phKb,2,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_phResult=resultLbl(w); m_phShow=new QCheckBox("Show Steps",w); m_phSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_phResult);
    v->addWidget(m_phShow); v->addWidget(m_phSteps); v->addStretch();
    connect(m_phShow,&QCheckBox::toggled,this,[this](bool on){m_phSteps->setVisible(on&&!m_phSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&ChemistryWidget::computePh);
    return w;
}

void ChemistryWidget::computePh() {
    double C=m_phConc->text().toDouble(), Ka=m_phKa->text().toDouble(), Kb=m_phKb->text().toDouble();
    int idx=m_phType->currentIndex();
    QStringList steps;
    double pH, pOH, hConc, ohConc;
    if (idx==0) {
        hConc=C; ohConc=1e-14/C;
        pH=-std::log10(hConc); pOH=14-pH;
        steps << QString("[H⁺] = %1 mol/L").arg(C,'g');
        steps << QString("pH = −log[H⁺] = −log(%1) = %2").arg(C,'g').arg(pH,0,'f',4);
    } else if (idx==1) {
        hConc=std::sqrt(Ka*C);
        pH=-std::log10(hConc); pOH=14-pH;
        steps << QString("Weak acid: [H⁺] = √(Ka×C) = √(%1×%2) = %3").arg(Ka,'g').arg(C).arg(hConc,'g');
        steps << QString("pH = −log(%1) = %2").arg(hConc,'g').arg(pH,0,'f',4);
    } else {
        ohConc=std::sqrt(Kb*C);
        pOH=-std::log10(ohConc); pH=14-pOH;
        steps << QString("Weak base: [OH⁻] = √(Kb×C) = √(%1×%2) = %3").arg(Kb,'g').arg(C).arg(ohConc,'g');
        steps << QString("pOH = −log(%1) = %2").arg(ohConc,'g').arg(pOH,0,'f',4);
        steps << QString("pH = 14 − pOH = %1").arg(pH,0,'f',4);
    }
    QString nature = pH<7?"Acidic":pH>7?"Basic":"Neutral";
    m_phResult->setText(QString("pH = %1\npOH = %2\n%3").arg(pH,0,'f',4).arg(pOH,0,'f',4).arg(nature));
    showSteps(m_phSteps,m_phShow,steps);
}

// ── Dilution ──────────────────────────────────────────────────────────────────
QWidget* ChemistryWidget::buildDilutionTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Dilution equation: C₁V₁ = C₂V₂. Enter any three values — leave the unknown empty.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_dilC1=numEdit("1.0","C₁ (mol/L)",w); m_dilV1=numEdit("0.1","V₁ (L)",w);
    m_dilC2=numEdit("","C₂ (mol/L)",w);    m_dilV2=numEdit("0.5","V₂ (L)",w);
    g->addWidget(new QLabel("C₁:",w),0,0); g->addWidget(m_dilC1,0,1);
    g->addWidget(new QLabel("V₁:",w),1,0); g->addWidget(m_dilV1,1,1);
    g->addWidget(new QLabel("C₂:",w),2,0); g->addWidget(m_dilC2,2,1);
    g->addWidget(new QLabel("V₂:",w),3,0); g->addWidget(m_dilV2,3,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    m_dilResult=resultLbl(w); m_dilShow=new QCheckBox("Show Steps",w); m_dilSteps=stepsBox(w);
    v->addWidget(mkBtn("Solve",w)); v->addWidget(m_dilResult);
    v->addWidget(m_dilShow); v->addWidget(m_dilSteps); v->addStretch();
    connect(m_dilShow,&QCheckBox::toggled,this,[this](bool on){m_dilSteps->setVisible(on&&!m_dilSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&ChemistryWidget::computeDilution);
    return w;
}

void ChemistryWidget::computeDilution() {
    bool hC1=!m_dilC1->text().isEmpty(), hV1=!m_dilV1->text().isEmpty();
    bool hC2=!m_dilC2->text().isEmpty(), hV2=!m_dilV2->text().isEmpty();
    double C1=m_dilC1->text().toDouble(), V1=m_dilV1->text().toDouble();
    double C2=m_dilC2->text().toDouble(), V2=m_dilV2->text().toDouble();
    QStringList steps;
    steps << "C₁V₁ = C₂V₂";
    if (!hC2&&hC1&&hV1&&hV2) { C2=C1*V1/V2; steps<<QString("C₂=C₁V₁/V₂=%1 mol/L").arg(C2,0,'g',8); }
    else if (!hV2&&hC1&&hV1&&hC2) { V2=C1*V1/C2; steps<<QString("V₂=C₁V₁/C₂=%1 L").arg(V2,0,'g',8); }
    else if (!hC1&&hC2&&hV1&&hV2) { C1=C2*V2/V1; steps<<QString("C₁=C₂V₂/V₁=%1 mol/L").arg(C1,0,'g',8); }
    else if (!hV1&&hC1&&hC2&&hV2) { V1=C2*V2/C1; steps<<QString("V₁=C₂V₂/C₁=%1 L").arg(V1,0,'g',8); }
    steps << QString("Dilution factor = V₂/V₁ = %1").arg(V2/V1,0,'g',4);
    m_dilResult->setText(QString("C₁ = %1 mol/L\nV₁ = %2 L\nC₂ = %3 mol/L\nV₂ = %4 L\nDilution factor = %5×")
        .arg(C1,0,'g',8).arg(V1,0,'g',8).arg(C2,0,'g',8).arg(V2,0,'g',8).arg(V2/V1,0,'g',4));
    showSteps(m_dilSteps,m_dilShow,steps);
}

// ── Thermochemistry ───────────────────────────────────────────────────────────
QWidget* ChemistryWidget::buildThermochemTab() {
    auto* w = new QWidget; auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(descLbl("Gibbs free energy: ΔG = ΔH − TΔS. If ΔG < 0 the reaction is spontaneous. Hess's law: enter ΔH values for each step (one per line) to get total ΔH.", w));
    auto* g = new QGridLayout(); g->setSpacing(6);
    m_tcDH=numEdit("-286","ΔH (kJ/mol)",w); m_tcDS=numEdit("-163","ΔS (J/mol·K)",w); m_tcT=numEdit("298","T (K)",w);
    g->addWidget(new QLabel("ΔH (kJ/mol):",w),0,0); g->addWidget(m_tcDH,0,1);
    g->addWidget(new QLabel("ΔS (J/mol·K):",w),1,0); g->addWidget(m_tcDS,1,1);
    g->addWidget(new QLabel("T (K):",w),2,0); g->addWidget(m_tcT,2,1);
    g->setColumnStretch(2,1);
    v->addLayout(g);
    v->addWidget(new QLabel("Hess's law — ΔH values per step (one per line):", w));
    m_tcHess = new QTextEdit(w); m_tcHess->setPlaceholderText("-286\n-394\n+44");
    m_tcHess->setMaximumHeight(80);
    v->addWidget(m_tcHess);
    m_tcResult=resultLbl(w); m_tcShow=new QCheckBox("Show Steps",w); m_tcSteps=stepsBox(w);
    v->addWidget(mkBtn("Compute",w)); v->addWidget(m_tcResult);
    v->addWidget(m_tcShow); v->addWidget(m_tcSteps); v->addStretch();
    connect(m_tcShow,&QCheckBox::toggled,this,[this](bool on){m_tcSteps->setVisible(on&&!m_tcSteps->toPlainText().isEmpty());});
    connect(w->findChildren<QPushButton*>().last(),&QPushButton::clicked,this,&ChemistryWidget::computeThermochem);
    return w;
}

void ChemistryWidget::computeThermochem() {
    double dH=m_tcDH->text().toDouble(), dS=m_tcDS->text().toDouble()/1000.0, T=m_tcT->text().toDouble();
    QStringList steps;
    double dG=dH-T*dS;
    steps << QString("ΔG = ΔH − TΔS");
    steps << QString("ΔG = %1 − %2 × %3 = %4 kJ/mol").arg(dH).arg(T).arg(dS*1000,0,'g',4).arg(dG,0,'g',8);
    QString spont = dG<0?"Spontaneous (ΔG < 0)":dG>0?"Non-spontaneous (ΔG > 0)":"At equilibrium (ΔG = 0)";
    steps << spont;
    // Hess's law
    double hessTotal=0; int step=1;
    for(const QString& line : m_tcHess->toPlainText().split('\n',Qt::SkipEmptyParts)){
        bool ok; double val=line.trimmed().toDouble(&ok);
        if(ok){ hessTotal+=val; steps<<QString("Step %1: ΔH = %2 kJ/mol").arg(step++).arg(val); }
    }
    if(step>1) steps<<QString("Hess's law total ΔH = %1 kJ/mol").arg(hessTotal,0,'g',8);
    m_tcResult->setText(QString("ΔG = %1 kJ/mol\n%2%3")
        .arg(dG,0,'g',8).arg(spont)
        .arg(step>1?QString("\n\nHess's law ΔH = %1 kJ/mol").arg(hessTotal,0,'g',8):""));
    showSteps(m_tcSteps,m_tcShow,steps);
}
