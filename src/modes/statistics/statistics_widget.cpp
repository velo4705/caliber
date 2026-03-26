#include "statistics_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QFrame>
#include <QPainter>
#include <QPaintEvent>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>

// ── Custom histogram widget (QPainter — pixel-perfect, no gaps) ───────────────
class HistogramWidget : public QWidget {
public:
    explicit HistogramWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    void setData(const QVector<int>& counts, double minV, double binWidth) {
        m_counts = counts; m_minV = minV; m_binWidth = binWidth;
        update();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        if (m_counts.isEmpty()) return;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, false);

        const int pad = 48, padTop = 16, padRight = 16, padBottom = 36;
        int W = width() - pad - padRight;
        int H = height() - padTop - padBottom;
        int n = m_counts.size();
        int maxC = *std::max_element(m_counts.begin(), m_counts.end());
        if (maxC == 0) return;

        // Background
        p.fillRect(rect(), palette().window());

        // Draw bars — pixel-perfect, no gaps
        for (int i = 0; i < n; i++) {
            // Use integer pixel boundaries to avoid sub-pixel gaps
            int x0 = pad + (int)std::round((double)i * W / n);
            int x1 = pad + (int)std::round((double)(i+1) * W / n);
            int barH = (int)std::round((double)m_counts[i] / maxC * H);
            int y0 = padTop + H - barH;

            QRect bar(x0, y0, x1 - x0, barH);
            p.fillRect(bar, QColor(0x42, 0x9e, 0xf5, 200));
            p.setPen(QColor(0x1a, 0x7a, 0xc4));
            p.drawRect(bar);
        }

        // X axis labels
        p.setPen(palette().text().color());
        QFont f = p.font(); f.setPointSize(9); p.setFont(f);
        int labelStep = qMax(1, n / 10);
        for (int i = 0; i <= n; i += labelStep) {
            int x = pad + (int)std::round((double)i * W / n);
            double val = m_minV + i * m_binWidth;
            QString lbl = QString::number(val, 'g', 4);
            p.drawText(x - 20, padTop + H + 4, 40, 20, Qt::AlignCenter, lbl);
        }

        // Y axis labels
        for (int i = 0; i <= maxC; i++) {
            int y = padTop + H - (int)std::round((double)i / maxC * H);
            p.drawText(0, y - 8, pad - 4, 16, Qt::AlignRight | Qt::AlignVCenter, QString::number(i));
        }

        // Axes
        p.setPen(QPen(palette().text().color(), 1));
        p.drawLine(pad, padTop, pad, padTop + H);
        p.drawLine(pad, padTop + H, pad + W, padTop + H);
    }
private:
    QVector<int> m_counts;
    double m_minV = 0, m_binWidth = 1;
};

// ── shared UI helpers ─────────────────────────────────────────────────────────
static QPushButton* mkBtn(const QString& t, const QString& cls, QWidget* p) {
    auto* b = new QPushButton(t, p);
    b->setProperty("class", cls);
    b->setMinimumHeight(36);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}
static QLabel* resultLbl(QWidget* p) {
    auto* l = new QLabel("—", p);
    l->setObjectName("resultLabel");
    l->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    l->setWordWrap(true);
    l->setStyleSheet("font-size:13px; padding:8px;");
    return l;
}
static QTextEdit* stepsBox(QWidget* p) {
    auto* t = new QTextEdit(p);
    t->setReadOnly(true);
    t->setMinimumHeight(100);
    t->setMaximumHeight(200);
    t->setStyleSheet("font-family:monospace; font-size:12px;");
    t->hide();
    return t;
}
static QLineEdit* numEdit(const QString& def, QWidget* p) {
    auto* e = new QLineEdit(def);
    e->setValidator(new QDoubleValidator(-1e15,1e15,10,p));
    e->setFixedWidth(90);
    return e;
}

// ── step helper ───────────────────────────────────────────────────────────────
void StatisticsWidget::showSteps(QTextEdit* w, QCheckBox* toggle, const QStringList& steps) {
    if (!toggle->isChecked()) { w->hide(); return; }
    w->clear();
    for (int i = 0; i < steps.size(); ++i)
        w->append(QString("Step %1: %2").arg(i+1).arg(steps[i]));
    w->show();
}

// ── constructor ───────────────────────────────────────────────────────────────
StatisticsWidget::StatisticsWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(12);
    auto* title = new QLabel("Statistics", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    root->addWidget(title);
    auto* tabs = new QTabWidget(this);
    tabs->setTabPosition(QTabWidget::North);
    tabs->setUsesScrollButtons(true);
    tabs->addTab(buildDataTab(),        "Data");
    tabs->addTab(buildDescriptiveTab(), "Descriptive");
    tabs->addTab(buildDistributionTab(),"Distributions");
    tabs->addTab(buildHypothesisTab(),  "Hypothesis");
    tabs->addTab(buildConfidenceTab(),  "Confidence");
    tabs->addTab(buildChiSquareTab(),   "Chi-Square");
    tabs->addTab(buildAnovaTab(),       "ANOVA");
    tabs->addTab(buildChartTab(),       "Charts");
    tabs->addTab(buildRegressionTab(),  "Regression");
    tabs->addTab(buildFrequencyTab(),   "Frequency");
    root->addWidget(tabs);
    setLayout(root);
}

// ── Tab: Data Entry ───────────────────────────────────────────────────────────
QWidget* StatisticsWidget::buildDataTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    auto* desc = new QLabel("Enter your dataset here. All other tabs use this data to compute statistics.", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);
    m_dataInput = new QTextEdit(w);
    m_dataInput->setPlaceholderText("e.g.  4, 7, 13, 2, 1, 9, 15, 3");
    m_dataInput->setMaximumHeight(140);
    v->addWidget(m_dataInput);
    auto* parseBtn = mkBtn("Load Data", "actionButton", w);
    m_dataStatus = new QLabel("No data loaded.", w);
    m_dataStatus->setStyleSheet("color: gray; font-size:12px;");
    v->addWidget(parseBtn);
    v->addWidget(m_dataStatus);
    v->addWidget(new QLabel("Once loaded, switch to other tabs to compute statistics.", w));
    v->addStretch();
    connect(parseBtn, &QPushButton::clicked, this, &StatisticsWidget::parseData);
    return w;
}

void StatisticsWidget::parseData() {
    m_data.clear();
    QString raw = m_dataInput->toPlainText();
    raw.replace('\n', ',');
    const QStringList parts = raw.split(',', Qt::SkipEmptyParts);
    for (const QString& p : parts) {
        bool ok;
        double v = p.trimmed().toDouble(&ok);
        if (ok) m_data.append(v);
    }
    if (m_data.isEmpty())
        m_dataStatus->setText("No valid numbers found.");
    else {
        m_dataStatus->setText(QString("%1 values loaded.").arg(m_data.size()));
        // Set smart default bin count using Sturges' rule: ceil(log2(n) + 1)
        int sturges = (int)std::ceil(std::log2(m_data.size()) + 1);
        m_chartBins->setValue(qMin(sturges, m_chartBins->maximum()));
        m_freqBins->setValue(qMin(sturges, m_freqBins->maximum()));
    }
}

// ── Tab: Descriptive Statistics ───────────────────────────────────────────────
QWidget* StatisticsWidget::buildDescriptiveTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    auto* desc = new QLabel("Computes summary statistics for your dataset: mean, median, mode, range, variance, standard deviation, and quartiles (Q1, Q3, IQR).", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);
    auto* btn = mkBtn("Compute", "actionButton", w);
    m_descResult    = resultLbl(w);
    m_descShowSteps = new QCheckBox("Show Steps", w);
    m_descSteps     = stepsBox(w);
    v->addWidget(btn);
    v->addWidget(m_descResult);
    v->addWidget(m_descShowSteps);
    v->addWidget(m_descSteps);
    v->addStretch();
    connect(m_descShowSteps, &QCheckBox::toggled, this, [this](bool on){
        m_descSteps->setVisible(on && !m_descSteps->toPlainText().isEmpty());
    });
    connect(btn, &QPushButton::clicked, this, &StatisticsWidget::computeDescriptive);
    return w;
}

void StatisticsWidget::computeDescriptive() {
    if (m_data.isEmpty()) { m_descResult->setText("Load data first (Data tab)."); return; }
    QStringList steps;
    int n = m_data.size();
    steps << QString("Dataset: n = %1 values").arg(n);

    double mu = mean(m_data);
    steps << QString("Mean = Σx / n = %1 / %2 = %3").arg(
        [&]{ double s=0; for(auto v:m_data) s+=v; return s; }(), 0, 'g', 8).arg(n).arg(mu, 0, 'g', 8);

    double med = median(m_data);
    steps << QString("Median: sort values, pick middle → %1").arg(med, 0, 'g', 8);

    // Mode
    QMap<double,int> freq;
    for (auto v : m_data) freq[v]++;
    int maxF = *std::max_element(freq.begin(), freq.end());
    QStringList modes;
    for (auto it = freq.begin(); it != freq.end(); ++it)
        if (it.value() == maxF) modes << QString::number(it.key(), 'g', 8);
    QString modeStr = maxF == 1 ? "No mode (all unique)" : modes.join(", ");
    steps << QString("Mode: most frequent value(s) → %1").arg(modeStr);

    QVector<double> sorted = m_data;
    std::sort(sorted.begin(), sorted.end());
    double range = sorted.last() - sorted.first();
    steps << QString("Range = max − min = %1 − %2 = %3").arg(sorted.last(),0,'g',8).arg(sorted.first(),0,'g',8).arg(range,0,'g',8);

    double varPop  = variance(m_data, true);
    double varSamp = variance(m_data, false);
    steps << QString("Population variance σ² = Σ(x−μ)²/n = %1").arg(varPop, 0, 'g', 8);
    steps << QString("Sample variance s² = Σ(x−μ)²/(n−1) = %1").arg(varSamp, 0, 'g', 8);
    steps << QString("Population std dev σ = √σ² = %1").arg(std::sqrt(varPop), 0, 'g', 8);
    steps << QString("Sample std dev s = √s² = %1").arg(std::sqrt(varSamp), 0, 'g', 8);

    // Quartiles
    auto q = [&](double p) {
        double idx = p * (n - 1);
        int lo = (int)idx;
        double frac = idx - lo;
        return lo+1 < n ? sorted[lo] + frac*(sorted[lo+1]-sorted[lo]) : sorted[lo];
    };
    double q1 = q(0.25), q3 = q(0.75), iqr = q3 - q1;
    steps << QString("Q1 = %1,  Q3 = %2,  IQR = Q3−Q1 = %3").arg(q1,0,'g',8).arg(q3,0,'g',8).arg(iqr,0,'g',8);

    m_descResult->setText(QString(
        "n = %1\nMean = %2\nMedian = %3\nMode = %4\n"
        "Range = %5\nVariance (pop) = %6\nVariance (samp) = %7\n"
        "Std Dev (pop) = %8\nStd Dev (samp) = %9\n"
        "Q1 = %10  Q3 = %11  IQR = %12")
        .arg(n).arg(mu,0,'g',8).arg(med,0,'g',8).arg(modeStr)
        .arg(range,0,'g',8).arg(varPop,0,'g',8).arg(varSamp,0,'g',8)
        .arg(std::sqrt(varPop),0,'g',8).arg(std::sqrt(varSamp),0,'g',8)
        .arg(q1,0,'g',8).arg(q3,0,'g',8).arg(iqr,0,'g',8));

    showSteps(m_descSteps, m_descShowSteps, steps);
}

// ── Tab: Probability Distributions ───────────────────────────────────────────
QWidget* StatisticsWidget::buildDistributionTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    auto* desc = new QLabel("Computes PDF and CDF for common probability distributions. Normal: continuous bell curve. Binomial: discrete success/failure trials. Poisson: discrete event count over time.", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);

    auto* typeRow = new QHBoxLayout();
    m_distType = new QComboBox(w);
    m_distType->addItems({"Normal", "Binomial", "Poisson"});
    typeRow->addWidget(new QLabel("Distribution:", w));
    typeRow->addWidget(m_distType); typeRow->addStretch();
    v->addLayout(typeRow);

    auto* paramGrid = new QHBoxLayout();
    m_distP1Lbl = new QLabel("μ:", w); m_distP1 = numEdit("0", w);
    m_distP2Lbl = new QLabel("σ:", w); m_distP2 = numEdit("1", w);
    m_distP3Lbl = new QLabel("",  w);  m_distP3 = numEdit("0", w);
    m_distP3->hide(); m_distP3Lbl->hide();
    paramGrid->addWidget(m_distP1Lbl); paramGrid->addWidget(m_distP1);
    paramGrid->addWidget(m_distP2Lbl); paramGrid->addWidget(m_distP2);
    paramGrid->addWidget(m_distP3Lbl); paramGrid->addWidget(m_distP3);
    paramGrid->addStretch();
    v->addLayout(paramGrid);

    auto* xRow = new QHBoxLayout();
    xRow->addWidget(new QLabel("x =", w));
    m_distX = numEdit("0", w);
    xRow->addWidget(m_distX); xRow->addStretch();
    v->addLayout(xRow);

    auto* btn = mkBtn("Compute PDF / CDF", "actionButton", w);
    m_distResult    = resultLbl(w);
    m_distShowSteps = new QCheckBox("Show Steps", w);
    m_distSteps     = stepsBox(w);
    v->addWidget(btn);
    v->addWidget(m_distResult);
    v->addWidget(m_distShowSteps);
    v->addWidget(m_distSteps);
    v->addStretch();

    connect(m_distType, &QComboBox::currentIndexChanged, this, &StatisticsWidget::onDistTypeChanged);
    connect(m_distShowSteps, &QCheckBox::toggled, this, [this](bool on){
        m_distSteps->setVisible(on && !m_distSteps->toPlainText().isEmpty());
    });
    connect(btn, &QPushButton::clicked, this, &StatisticsWidget::computeDistribution);
    return w;
}

void StatisticsWidget::onDistTypeChanged(int idx) {
    if (idx == 0) { // Normal
        m_distP1Lbl->setText("μ:"); m_distP1->setText("0");
        m_distP2Lbl->setText("σ:"); m_distP2->setText("1");
        m_distP3->hide(); m_distP3Lbl->hide();
    } else if (idx == 1) { // Binomial
        m_distP1Lbl->setText("n:"); m_distP1->setText("10");
        m_distP2Lbl->setText("p:"); m_distP2->setText("0.5");
        m_distP3Lbl->setText("k:"); m_distP3->setText("5");
        m_distP3->show(); m_distP3Lbl->show();
    } else { // Poisson
        m_distP1Lbl->setText("λ:"); m_distP1->setText("3");
        m_distP2Lbl->setText("k:"); m_distP2->setText("2");
        m_distP3->hide(); m_distP3Lbl->hide();
    }
}

void StatisticsWidget::computeDistribution() {
    QStringList steps;
    int idx = m_distType->currentIndex();
    if (idx == 0) { // Normal
        double mu = m_distP1->text().toDouble();
        double sigma = m_distP2->text().toDouble();
        double x = m_distX->text().toDouble();
        steps << QString("Normal distribution: μ = %1, σ = %2").arg(mu).arg(sigma);
        steps << QString("Standardize: z = (x − μ) / σ = (%1 − %2) / %3 = %4").arg(x).arg(mu).arg(sigma).arg((x-mu)/sigma, 0,'g',6);
        double pdf = normalPDF(x, mu, sigma);
        double cdf = normalCDF(x, mu, sigma);
        steps << QString("PDF = (1/σ√2π) · e^(−z²/2) = %1").arg(pdf, 0,'g',8);
        steps << QString("CDF = P(X ≤ %1) = %2").arg(x).arg(cdf, 0,'g',8);
        m_distResult->setText(QString("Normal(μ=%1, σ=%2)\nPDF(x=%3) = %4\nCDF(x=%3) = %5").arg(mu).arg(sigma).arg(x).arg(pdf,0,'g',8).arg(cdf,0,'g',8));
    } else if (idx == 1) { // Binomial
        int n = (int)m_distP1->text().toDouble();
        double p = m_distP2->text().toDouble();
        int k = (int)m_distP3->text().toDouble();
        steps << QString("Binomial distribution: n = %1, p = %2, k = %3").arg(n).arg(p).arg(k);
        steps << QString("PMF = C(n,k) · p^k · (1−p)^(n−k)");
        double pmf = binomialPMF(k, n, p);
        steps << QString("C(%1,%2) = %3").arg(n).arg(k).arg([&]{ double c=1; for(int i=0;i<k;i++) c=c*(n-i)/(i+1); return c; }(), 0,'g',8);
        steps << QString("PMF = %1").arg(pmf, 0,'g',8);
        double cdf = 0; for(int i=0;i<=k;i++) cdf += binomialPMF(i,n,p);
        steps << QString("CDF = P(X ≤ %1) = %2").arg(k).arg(cdf, 0,'g',8);
        m_distResult->setText(QString("Binomial(n=%1, p=%2)\nP(X=%3) = %4\nP(X≤%3) = %5").arg(n).arg(p).arg(k).arg(pmf,0,'g',8).arg(cdf,0,'g',8));
    } else { // Poisson
        double lambda = m_distP1->text().toDouble();
        int k = (int)m_distP2->text().toDouble();
        steps << QString("Poisson distribution: λ = %1, k = %2").arg(lambda).arg(k);
        steps << QString("PMF = (λ^k · e^−λ) / k!");
        double pmf = poissonPMF(k, lambda);
        steps << QString("PMF = (%1^%2 · e^−%1) / %2! = %3").arg(lambda).arg(k).arg(pmf,0,'g',8);
        double cdf = 0; for(int i=0;i<=k;i++) cdf += poissonPMF(i,lambda);
        steps << QString("CDF = P(X ≤ %1) = %2").arg(k).arg(cdf,0,'g',8);
        m_distResult->setText(QString("Poisson(λ=%1)\nP(X=%2) = %3\nP(X≤%2) = %4").arg(lambda).arg(k).arg(pmf,0,'g',8).arg(cdf,0,'g',8));
    }
    showSteps(m_distSteps, m_distShowSteps, steps);
}

// ── Tab: Hypothesis Testing ───────────────────────────────────────────────────
QWidget* StatisticsWidget::buildHypothesisTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    auto* desc = new QLabel("Tests whether a sample mean is significantly different from a known value (H₀: μ = 0). Z-test: use when population σ is known. T-test: use when σ is estimated from sample. Two-sample: compare two group means.", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);

    auto* typeRow = new QHBoxLayout();
    m_hypTestType = new QComboBox(w);
    m_hypTestType->addItems({"One-sample Z-test", "One-sample T-test", "Two-sample Z-test"});
    typeRow->addWidget(new QLabel("Test:", w)); typeRow->addWidget(m_hypTestType); typeRow->addStretch();
    v->addLayout(typeRow);

    auto* g = new QHBoxLayout();
    m_hypMu    = numEdit("0",    w); m_hypSigma = numEdit("1",  w);
    m_hypN     = numEdit("30",   w); m_hypAlpha = numEdit("0.05", w);
    m_hypMu2   = numEdit("0",    w); m_hypSigma2= numEdit("1",  w);
    m_hypN2    = numEdit("30",   w);
    g->addWidget(new QLabel("x̄:", w)); g->addWidget(m_hypMu);
    g->addWidget(new QLabel("σ/s:", w)); g->addWidget(m_hypSigma);
    g->addWidget(new QLabel("n:", w)); g->addWidget(m_hypN);
    g->addWidget(new QLabel("α:", w)); g->addWidget(m_hypAlpha);
    g->addStretch();
    v->addLayout(g);

    auto* g2 = new QHBoxLayout();
    g2->addWidget(new QLabel("x̄₂:", w)); g2->addWidget(m_hypMu2);
    g2->addWidget(new QLabel("σ₂:", w)); g2->addWidget(m_hypSigma2);
    g2->addWidget(new QLabel("n₂:", w)); g2->addWidget(m_hypN2);
    g2->addStretch();
    v->addLayout(g2);
    v->addWidget(new QLabel("H₀: μ = 0  (two-tailed test)", w));

    auto* btn = mkBtn("Compute", "actionButton", w);
    m_hypResult    = resultLbl(w);
    m_hypShowSteps = new QCheckBox("Show Steps", w);
    m_hypSteps     = stepsBox(w);
    v->addWidget(btn);
    v->addWidget(m_hypResult);
    v->addWidget(m_hypShowSteps);
    v->addWidget(m_hypSteps);
    v->addStretch();

    connect(m_hypShowSteps, &QCheckBox::toggled, this, [this](bool on){
        m_hypSteps->setVisible(on && !m_hypSteps->toPlainText().isEmpty());
    });
    connect(btn, &QPushButton::clicked, this, &StatisticsWidget::computeHypothesis);
    return w;
}

void StatisticsWidget::computeHypothesis() {
    QStringList steps;
    int idx = m_hypTestType->currentIndex();
    double xbar  = m_hypMu->text().toDouble();
    double sigma = m_hypSigma->text().toDouble();
    int    n     = qMax(1, (int)m_hypN->text().toDouble());
    double alpha = m_hypAlpha->text().toDouble();
    double mu0   = 0.0; // H0: mu = 0

    if (idx == 0 || idx == 1) { // One-sample Z or T
        double se = sigma / std::sqrt(n);
        double stat = (xbar - mu0) / se;
        steps << QString("H₀: μ = %1,  H₁: μ ≠ %1 (two-tailed)").arg(mu0);
        steps << QString("Standard error: SE = σ/√n = %1/√%2 = %3").arg(sigma).arg(n).arg(se,0,'g',6);
        steps << QString("%1 statistic = (x̄ − μ₀) / SE = (%2 − %3) / %4 = %5")
                 .arg(idx==0?"Z":"t").arg(xbar).arg(mu0).arg(se,0,'g',6).arg(stat,0,'g',6);

        double pval;
        if (idx == 0) {
            pval = 2.0 * (1.0 - normalCDF(std::abs(stat), 0, 1));
            steps << QString("p-value = 2 × P(Z > |%1|) = %2").arg(stat,0,'g',4).arg(pval,0,'g',6);
        } else {
            pval = 2.0 * tCDF(-std::abs(stat), n-1);
            steps << QString("p-value (t, df=%1) = %2").arg(n-1).arg(pval,0,'g',6);
        }
        steps << QString("α = %1 → %2").arg(alpha).arg(pval < alpha ? "Reject H₀ (significant)" : "Fail to reject H₀");
        m_hypResult->setText(QString("%1 = %2\np-value = %3\n%4")
            .arg(idx==0?"Z":"t").arg(stat,0,'g',6).arg(pval,0,'g',6)
            .arg(pval < alpha ? "Reject H₀ (p < α)" : "Fail to reject H₀ (p ≥ α)"));
    } else { // Two-sample Z
        double xbar2  = m_hypMu2->text().toDouble();
        double sigma2 = m_hypSigma2->text().toDouble();
        int    n2     = qMax(1, (int)m_hypN2->text().toDouble());
        double se = std::sqrt((sigma*sigma/n) + (sigma2*sigma2/n2));
        double z  = (xbar - xbar2) / se;
        steps << "H₀: μ₁ = μ₂,  H₁: μ₁ ≠ μ₂ (two-tailed)";
        steps << QString("SE = √(σ₁²/n₁ + σ₂²/n₂) = %1").arg(se,0,'g',6);
        steps << QString("Z = (x̄₁ − x̄₂) / SE = (%1 − %2) / %3 = %4").arg(xbar).arg(xbar2).arg(se,0,'g',6).arg(z,0,'g',6);
        double pval = 2.0 * (1.0 - normalCDF(std::abs(z), 0, 1));
        steps << QString("p-value = %1").arg(pval,0,'g',6);
        steps << QString(pval < alpha ? "Reject H₀" : "Fail to reject H₀");
        m_hypResult->setText(QString("Z = %1\np-value = %2\n%3").arg(z,0,'g',6).arg(pval,0,'g',6)
            .arg(pval < alpha ? "Reject H₀ (p < α)" : "Fail to reject H₀ (p ≥ α)"));
    }
    showSteps(m_hypSteps, m_hypShowSteps, steps);
}

// ── Tab: Regression & Correlation ────────────────────────────────────────────
QWidget* StatisticsWidget::buildRegressionTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    auto* desc = new QLabel("Fits a straight line ŷ = bx + a through your X (Data tab) and Y values. Reports slope, intercept, R² (how well the line fits), Pearson r (linear correlation), and Spearman ρ (rank correlation).", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);
    v->addWidget(new QLabel("X values: loaded from Data tab.  Enter Y values below:", w));
    m_regYInput = new QTextEdit(w);
    m_regYInput->setPlaceholderText("e.g.  2, 4, 5, 4, 5");
    m_regYInput->setMaximumHeight(80);
    v->addWidget(m_regYInput);
    auto* btn = mkBtn("Compute Regression & Correlation", "actionButton", w);
    m_regResult    = resultLbl(w);
    m_regShowSteps = new QCheckBox("Show Steps", w);
    m_regSteps     = stepsBox(w);
    v->addWidget(btn);
    v->addWidget(m_regResult);
    v->addWidget(m_regShowSteps);
    v->addWidget(m_regSteps);
    v->addStretch();
    connect(m_regShowSteps, &QCheckBox::toggled, this, [this](bool on){
        m_regSteps->setVisible(on && !m_regSteps->toPlainText().isEmpty());
    });
    connect(btn, &QPushButton::clicked, this, &StatisticsWidget::computeRegression);
    return w;
}

void StatisticsWidget::computeRegression() {
    if (m_data.isEmpty()) { m_regResult->setText("Load X data first (Data tab)."); return; }
    QVector<double> y;
    QString raw = m_regYInput->toPlainText(); raw.replace('\n',',');
    for (const QString& p : raw.split(',', Qt::SkipEmptyParts)) {
        bool ok; double v = p.trimmed().toDouble(&ok);
        if (ok) y.append(v);
    }
    if (y.size() != m_data.size()) {
        m_regResult->setText(QString("X has %1 values, Y has %2. Must match.").arg(m_data.size()).arg(y.size()));
        return;
    }
    QStringList steps;
    int n = m_data.size();
    const QVector<double>& x = m_data;
    double xbar = mean(x), ybar = mean(y);
    steps << QString("n = %1,  x̄ = %2,  ȳ = %3").arg(n).arg(xbar,0,'g',6).arg(ybar,0,'g',6);

    double sxy = 0, sxx = 0;
    for (int i=0;i<n;i++) { sxy += (x[i]-xbar)*(y[i]-ybar); sxx += (x[i]-xbar)*(x[i]-xbar); }
    steps << QString("Σ(x−x̄)(y−ȳ) = %1").arg(sxy,0,'g',8);
    steps << QString("Σ(x−x̄)² = %1").arg(sxx,0,'g',8);

    double slope = sxy / sxx;
    double intercept = ybar - slope * xbar;
    steps << QString("Slope b = Σ(x−x̄)(y−ȳ) / Σ(x−x̄)² = %1").arg(slope,0,'g',8);
    steps << QString("Intercept a = ȳ − b·x̄ = %1 − %2·%3 = %4").arg(ybar,0,'g',6).arg(slope,0,'g',6).arg(xbar,0,'g',6).arg(intercept,0,'g',8);

    double r = pearsonR(x, y);
    double r2 = r * r;
    steps << QString("Pearson r = %1").arg(r,0,'g',8);
    steps << QString("R² = %1 (%2% of variance explained)").arg(r2,0,'g',6).arg(r2*100,0,'f',1);

    double rho = spearmanR(x, y);
    steps << QString("Spearman ρ = %1").arg(rho,0,'g',8);

    m_regResult->setText(QString(
        "Linear regression: ŷ = %1·x + %2\n"
        "Slope (b) = %1\nIntercept (a) = %2\n"
        "Pearson r = %3\nR² = %4\nSpearman ρ = %5")
        .arg(slope,0,'g',8).arg(intercept,0,'g',8)
        .arg(r,0,'g',8).arg(r2,0,'g',6).arg(rho,0,'g',8));
    showSteps(m_regSteps, m_regShowSteps, steps);
}

// ── Tab: Frequency Distribution ──────────────────────────────────────────────
QWidget* StatisticsWidget::buildFrequencyTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    auto* desc = new QLabel("Divides your dataset into equal-width bins and counts how many values fall in each. Shows absolute frequency and relative frequency (%) per bin.", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);
    auto* row = new QHBoxLayout();
    row->addWidget(new QLabel("Number of bins:", w));
    m_freqBins = new QSpinBox(w); m_freqBins->setRange(2, 100); m_freqBins->setValue(5);
    row->addWidget(m_freqBins); row->addStretch();
    v->addLayout(row);
    auto* btn = mkBtn("Compute Frequency Table", "actionButton", w);
    m_freqResult = resultLbl(w);
    v->addWidget(btn);
    v->addWidget(m_freqResult);
    v->addStretch();
    connect(btn, &QPushButton::clicked, this, &StatisticsWidget::computeFrequency);
    return w;
}

void StatisticsWidget::computeFrequency() {
    if (m_data.isEmpty()) { m_freqResult->setText("Load data first (Data tab)."); return; }
    QVector<double> sorted = m_data;
    std::sort(sorted.begin(), sorted.end());
    int bins = m_freqBins->value();
    double minV = sorted.first(), maxV = sorted.last();
    double width = (maxV - minV) / bins;
    if (width == 0) { m_freqResult->setText("All values are identical."); return; }

    QVector<int> counts(bins, 0);
    for (double v : m_data) {
        int b = (int)((v - minV) / width);
        if (b >= bins) b = bins - 1;
        counts[b]++;
    }

    QString out = QString("Bin width = %1\n\n").arg(width, 0, 'g', 6);
    out += QString("%-20s  %5s  %8s\n").arg("Interval").arg("Freq").arg("Rel.Freq");
    out += QString(40, '-') + "\n";
    for (int i = 0; i < bins; i++) {
        double lo = minV + i*width, hi = lo + width;
        double rel = (double)counts[i] / m_data.size();
        out += QString("[%1, %2)  %3  %4%\n")
            .arg(lo,6,'g',4).arg(hi,6,'g',4)
            .arg(counts[i],5)
            .arg(rel*100,7,'f',1);
    }
    m_freqResult->setText(out);
}

// ── Tab: Confidence Intervals ─────────────────────────────────────────────────
QWidget* StatisticsWidget::buildConfidenceTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(10);
    auto* desc = new QLabel("Computes a confidence interval — a range likely to contain the true population parameter. For means (Z or T) and proportions. Higher confidence = wider interval.", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);

    auto* typeRow = new QHBoxLayout();
    m_ciType = new QComboBox(w);
    m_ciType->addItems({"Mean (Z, known σ)", "Mean (T, unknown σ)", "Proportion"});
    typeRow->addWidget(new QLabel("Type:", w)); typeRow->addWidget(m_ciType); typeRow->addStretch();
    v->addLayout(typeRow);

    // Inputs in a form-style grid
    auto* grid = new QGridLayout();
    grid->setSpacing(8);
    m_ciMean  = new QLineEdit("50",   w); m_ciMean->setFixedWidth(90);
    m_ciSigma = new QLineEdit("10",   w); m_ciSigma->setFixedWidth(90);
    m_ciN     = new QLineEdit("30",   w); m_ciN->setFixedWidth(90);
    m_ciConf  = new QLineEdit("0.95", w); m_ciConf->setFixedWidth(90);
    m_ciP     = new QLineEdit("0.5",  w); m_ciP->setFixedWidth(90); m_ciP->hide();
    grid->addWidget(new QLabel("x̄ (or p̂):", w), 0, 0); grid->addWidget(m_ciMean,  0, 1);
    grid->addWidget(new QLabel("σ or s:",    w), 1, 0); grid->addWidget(m_ciSigma, 1, 1);
    grid->addWidget(new QLabel("n:",         w), 2, 0); grid->addWidget(m_ciN,     2, 1);
    grid->addWidget(new QLabel("Confidence (e.g. 0.95):", w), 3, 0); grid->addWidget(m_ciConf, 3, 1);
    grid->setColumnStretch(2, 1);
    v->addLayout(grid);

    auto* btn   = mkBtn("Compute", "actionButton", w);
    m_ciResult    = resultLbl(w);
    m_ciShowSteps = new QCheckBox("Show Steps", w);
    m_ciSteps     = stepsBox(w);
    v->addWidget(btn); v->addWidget(m_ciResult);
    v->addWidget(m_ciShowSteps); v->addWidget(m_ciSteps);
    v->addStretch();
    connect(m_ciShowSteps, &QCheckBox::toggled, this, [this](bool on){
        m_ciSteps->setVisible(on && !m_ciSteps->toPlainText().isEmpty());
    });
    connect(btn, &QPushButton::clicked, this, &StatisticsWidget::computeConfidence);
    return w;
}

void StatisticsWidget::computeConfidence() {
    QStringList steps;
    int idx   = m_ciType->currentIndex();
    double xbar = m_ciMean->text().toDouble();
    double sigma= m_ciSigma->text().toDouble();
    int    n    = qMax(1,(int)m_ciN->text().toDouble());
    double conf = m_ciConf->text().toDouble();
    double alpha= 1.0 - conf;

    steps << QString("Confidence level: %1% → α = %2").arg(conf*100,0,'f',1).arg(alpha,0,'g',4);

    if (idx == 0) { // Z interval
        double z = normalCDFInv(1.0 - alpha/2.0);
        double se = sigma / std::sqrt(n);
        double me = z * se;
        steps << QString("Z* for %1% = %2").arg(conf*100,0,'f',1).arg(z,0,'g',6);
        steps << QString("SE = σ/√n = %1/√%2 = %3").arg(sigma).arg(n).arg(se,0,'g',6);
        steps << QString("Margin of error = Z* × SE = %1 × %2 = %3").arg(z,0,'g',4).arg(se,0,'g',4).arg(me,0,'g',6);
        steps << QString("CI = x̄ ± ME = %1 ± %2").arg(xbar).arg(me,0,'g',6);
        m_ciResult->setText(QString("Z-interval (%1%)\n(%2,  %3)\nMargin of error = ±%4")
            .arg(conf*100,0,'f',1).arg(xbar-me,0,'g',8).arg(xbar+me,0,'g',8).arg(me,0,'g',6));
    } else if (idx == 1) { // T interval
        double t = 0;
        // Approximate t* using normal for large df, else simple lookup
        if (n-1 >= 30) t = normalCDFInv(1.0 - alpha/2.0);
        else {
            // Bisection to find t such that tCDF(t, df) = 1-alpha/2
            double lo=0, hi=10;
            for(int i=0;i<60;i++){
                double mid=(lo+hi)/2;
                if(tCDF(mid,n-1) < 1.0-alpha/2.0) lo=mid; else hi=mid;
            }
            t=(lo+hi)/2;
        }
        double se = sigma / std::sqrt(n);
        double me = t * se;
        steps << QString("t* (df=%1, %2%) = %3").arg(n-1).arg(conf*100,0,'f',1).arg(t,0,'g',6);
        steps << QString("SE = s/√n = %1/√%2 = %3").arg(sigma).arg(n).arg(se,0,'g',6);
        steps << QString("Margin of error = t* × SE = %1").arg(me,0,'g',6);
        m_ciResult->setText(QString("T-interval (%1%, df=%2)\n(%3,  %4)\nMargin of error = ±%5")
            .arg(conf*100,0,'f',1).arg(n-1).arg(xbar-me,0,'g',8).arg(xbar+me,0,'g',8).arg(me,0,'g',6));
    } else { // Proportion
        double p = xbar; // reuse mean field for p̂
        double z = normalCDFInv(1.0 - alpha/2.0);
        double se = std::sqrt(p*(1-p)/n);
        double me = z * se;
        steps << QString("p̂ = %1,  n = %2").arg(p).arg(n);
        steps << QString("SE = √(p̂(1−p̂)/n) = %1").arg(se,0,'g',6);
        steps << QString("Z* = %1,  ME = Z* × SE = %2").arg(z,0,'g',4).arg(me,0,'g',6);
        m_ciResult->setText(QString("Proportion interval (%1%)\n(%2,  %3)\nMargin of error = ±%4")
            .arg(conf*100,0,'f',1).arg(p-me,0,'g',8).arg(p+me,0,'g',8).arg(me,0,'g',6));
    }
    showSteps(m_ciSteps, m_ciShowSteps, steps);
}

// ── Tab: Chi-Square Test ──────────────────────────────────────────────────────
QWidget* StatisticsWidget::buildChiSquareTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    auto* desc = new QLabel("Goodness-of-fit test: checks if observed frequencies match expected frequencies. Enter observed counts in row 1 and expected counts in row 2.", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);

    auto* colRow = new QHBoxLayout();
    m_chiCols = new QSpinBox(w); m_chiCols->setRange(2,10); m_chiCols->setValue(4);
    colRow->addWidget(new QLabel("Categories:", w)); colRow->addWidget(m_chiCols); colRow->addStretch();
    v->addLayout(colRow);

    m_chiTable = new QTableWidget(2, 4, w);
    m_chiTable->setVerticalHeaderLabels({"Observed", "Expected"});
    m_chiTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_chiTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_chiTable->verticalHeader()->setDefaultSectionSize(32);
    m_chiTable->setFixedHeight(2 * 32 + m_chiTable->horizontalHeader()->height() + 4);
    m_chiTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    v->addWidget(m_chiTable);
    rebuildChiTable(4);

    auto* btn     = mkBtn("Compute Chi-Square", "actionButton", w);
    m_chiResult    = resultLbl(w);
    m_chiShowSteps = new QCheckBox("Show Steps", w);
    m_chiSteps     = stepsBox(w);
    v->addWidget(btn); v->addWidget(m_chiResult);
    v->addWidget(m_chiShowSteps); v->addWidget(m_chiSteps);
    v->addStretch();

    connect(m_chiCols, &QSpinBox::valueChanged, this, &StatisticsWidget::rebuildChiTable);
    connect(m_chiShowSteps, &QCheckBox::toggled, this, [this](bool on){
        m_chiSteps->setVisible(on && !m_chiSteps->toPlainText().isEmpty());
    });
    connect(btn, &QPushButton::clicked, this, &StatisticsWidget::computeChiSquare);
    return w;
}

void StatisticsWidget::rebuildChiTable(int cols) {
    m_chiTable->setColumnCount(cols);
    QStringList hdr; for(int i=0;i<cols;i++) hdr << QString("C%1").arg(i+1);
    m_chiTable->setHorizontalHeaderLabels(hdr);
    for(int r=0;r<2;r++) for(int c=0;c<cols;c++){
        auto* item = new QTableWidgetItem("0");
        item->setTextAlignment(Qt::AlignCenter);
        m_chiTable->setItem(r,c,item);
    }
}

void StatisticsWidget::computeChiSquare() {
    int k = m_chiTable->columnCount();
    QVector<double> obs(k), exp_(k);
    for(int c=0;c<k;c++){
        obs[c]  = m_chiTable->item(0,c) ? m_chiTable->item(0,c)->text().toDouble() : 0;
        exp_[c] = m_chiTable->item(1,c) ? m_chiTable->item(1,c)->text().toDouble() : 0;
    }
    QStringList steps;
    steps << QString("k = %1 categories,  df = k−1 = %2").arg(k).arg(k-1);
    steps << "χ² = Σ (O − E)² / E";
    double chi2 = 0;
    for(int c=0;c<k;c++){
        if(exp_[c] <= 0){ m_chiResult->setText("Error: expected values must be > 0"); return; }
        double term = (obs[c]-exp_[c])*(obs[c]-exp_[c]) / exp_[c];
        steps << QString("Category %1: (O=%2, E=%3) → (%4)²/%3 = %5")
                 .arg(c+1).arg(obs[c],0,'g',4).arg(exp_[c],0,'g',4).arg(obs[c]-exp_[c],0,'g',4).arg(term,0,'g',6);
        chi2 += term;
    }
    steps << QString("χ² = %1").arg(chi2,0,'g',8);
    double pval = 1.0 - chiSquareCDF(chi2, k-1);
    steps << QString("p-value = P(χ² > %1, df=%2) = %3").arg(chi2,0,'g',4).arg(k-1).arg(pval,0,'g',6);
    steps << (pval < 0.05 ? "p < 0.05 → Reject H₀ (distributions differ)" : "p ≥ 0.05 → Fail to reject H₀");
    m_chiResult->setText(QString("χ² = %1\ndf = %2\np-value = %3\n%4")
        .arg(chi2,0,'g',8).arg(k-1).arg(pval,0,'g',6)
        .arg(pval < 0.05 ? "Reject H₀ (p < 0.05)" : "Fail to reject H₀ (p ≥ 0.05)"));
    showSteps(m_chiSteps, m_chiShowSteps, steps);
}

// ── Tab: ANOVA ────────────────────────────────────────────────────────────────
QWidget* StatisticsWidget::buildAnovaTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    auto* desc = new QLabel("One-way ANOVA tests whether the means of 3+ groups are equal. Enter each group on a separate line as comma-separated values. H₀: all group means are equal.", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);
    v->addWidget(new QLabel("Enter groups (one per line, comma-separated):", w));
    m_anovaGroups = new QTextEdit(w);
    m_anovaGroups->setPlaceholderText("Group 1: 4, 5, 6, 7\nGroup 2: 2, 3, 4, 5\nGroup 3: 6, 7, 8, 9");
    m_anovaGroups->setMaximumHeight(100);
    v->addWidget(m_anovaGroups);

    auto* btn      = mkBtn("Compute ANOVA", "actionButton", w);
    m_anovaResult    = resultLbl(w);
    m_anovaShowSteps = new QCheckBox("Show Steps", w);
    m_anovaSteps     = stepsBox(w);
    v->addWidget(btn); v->addWidget(m_anovaResult);
    v->addWidget(m_anovaShowSteps); v->addWidget(m_anovaSteps);
    v->addStretch();
    connect(m_anovaShowSteps, &QCheckBox::toggled, this, [this](bool on){
        m_anovaSteps->setVisible(on && !m_anovaSteps->toPlainText().isEmpty());
    });
    connect(btn, &QPushButton::clicked, this, &StatisticsWidget::computeAnova);
    return w;
}

void StatisticsWidget::computeAnova() {
    QStringList lines = m_anovaGroups->toPlainText().split('\n', Qt::SkipEmptyParts);
    if(lines.size() < 2){ m_anovaResult->setText("Enter at least 2 groups."); return; }
    QVector<QVector<double>> groups;
    for(const QString& line : lines){
        QVector<double> g;
        for(const QString& p : line.split(',', Qt::SkipEmptyParts)){
            bool ok; double v=p.trimmed().toDouble(&ok); if(ok) g.append(v);
        }
        if(!g.isEmpty()) groups.append(g);
    }
    if(groups.size() < 2){ m_anovaResult->setText("Need at least 2 valid groups."); return; }

    QStringList steps;
    int k = groups.size();
    int N = 0; for(auto& g:groups) N+=g.size();
    steps << QString("k = %1 groups,  N = %2 total observations").arg(k).arg(N);

    // Grand mean
    double grandSum = 0;
    for(auto& g:groups) for(double v:g) grandSum+=v;
    double grandMean = grandSum / N;
    steps << QString("Grand mean x̄ = %1").arg(grandMean,0,'g',8);

    // SSB (between groups)
    double SSB = 0;
    for(auto& g:groups){ double gm=mean(g); SSB += g.size()*(gm-grandMean)*(gm-grandMean); }
    int dfB = k-1;
    double MSB = SSB/dfB;
    steps << QString("SS_between = %1,  df_between = %2,  MS_between = %3").arg(SSB,0,'g',8).arg(dfB).arg(MSB,0,'g',8);

    // SSW (within groups)
    double SSW = 0;
    for(auto& g:groups){ double gm=mean(g); for(double v:g) SSW+=(v-gm)*(v-gm); }
    int dfW = N-k;
    double MSW = SSW/dfW;
    steps << QString("SS_within = %1,  df_within = %2,  MS_within = %3").arg(SSW,0,'g',8).arg(dfW).arg(MSW,0,'g',8);

    double F = MSB/MSW;
    steps << QString("F = MS_between / MS_within = %1 / %2 = %3").arg(MSB,0,'g',6).arg(MSW,0,'g',6).arg(F,0,'g',8);

    // p-value approximation via F distribution (use chi-square approximation)
    double pval = 1.0 - chiSquareCDF(F * dfB, dfB);
    steps << QString("p-value ≈ %1").arg(pval,0,'g',6);
    steps << (pval < 0.05 ? "p < 0.05 → Reject H₀ (group means differ)" : "p ≥ 0.05 → Fail to reject H₀");

    QString out = QString("One-way ANOVA\nF(%1, %2) = %3\np-value ≈ %4\n%5\n\nGroup means:")
        .arg(dfB).arg(dfW).arg(F,0,'g',8).arg(pval,0,'g',6)
        .arg(pval < 0.05 ? "Reject H₀" : "Fail to reject H₀");
    for(int i=0;i<groups.size();i++)
        out += QString("\n  Group %1: x̄ = %2  (n=%3)").arg(i+1).arg(mean(groups[i]),0,'g',6).arg(groups[i].size());
    m_anovaResult->setText(out);
    showSteps(m_anovaSteps, m_anovaShowSteps, steps);
}

// ── Tab: Charts (Histogram + Box Plot) ───────────────────────────────────────
QWidget* StatisticsWidget::buildChartTab() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    auto* desc = new QLabel("Visualize your dataset. Histogram shows value distribution across bins. Bars are pixel-perfect with no gaps.", w);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: gray; font-size: 12px; padding: 4px 0;");
    v->addWidget(desc);

    auto* row = new QHBoxLayout();
    row->addWidget(new QLabel("Bins:", w));
    m_chartBins = new QSpinBox(w); m_chartBins->setRange(2, 100); m_chartBins->setValue(8);
    row->addWidget(m_chartBins);
    auto* btn = mkBtn("Build Histogram", "actionButton", w);
    row->addWidget(btn); row->addStretch();
    v->addLayout(row);

    m_histWidget = new HistogramWidget(w);
    v->addWidget(m_histWidget, 1);

    connect(btn, &QPushButton::clicked, this, &StatisticsWidget::buildHistogram);
    return w;
}

void StatisticsWidget::buildHistogram() {
    if (m_data.isEmpty()) return;

    QVector<double> sorted = m_data;
    std::sort(sorted.begin(), sorted.end());
    int n = m_data.size();
    int bins = qMin(m_chartBins->value(), n);
    double minV = sorted.first(), maxV = sorted.last();
    double width = (maxV - minV) / bins;
    if (width == 0) width = 1;

    QVector<int> counts(bins, 0);
    for (double v : m_data) {
        int b = (int)((v - minV) / width);
        if (b >= bins) b = bins - 1;
        counts[b]++;
    }

    static_cast<HistogramWidget*>(m_histWidget)->setData(counts, minV, width);
}

// ── Math helpers ──────────────────────────────────────────────────────────────
double StatisticsWidget::mean(const QVector<double>& d) {
    if (d.isEmpty()) return 0;
    return std::accumulate(d.begin(), d.end(), 0.0) / d.size();
}
double StatisticsWidget::variance(const QVector<double>& d, bool population) {
    if (d.size() < 2) return 0;
    double mu = mean(d);
    double sum = 0;
    for (double v : d) sum += (v-mu)*(v-mu);
    return sum / (population ? d.size() : d.size()-1);
}
double StatisticsWidget::stddev(const QVector<double>& d, bool population) {
    return std::sqrt(variance(d, population));
}
double StatisticsWidget::median(QVector<double> d) {
    std::sort(d.begin(), d.end());
    int n = d.size();
    return n%2 ? d[n/2] : (d[n/2-1]+d[n/2])/2.0;
}

// Normal distribution using Abramowitz & Stegun approximation
double StatisticsWidget::normalPDF(double x, double mu, double sigma) {
    double z = (x-mu)/sigma;
    return std::exp(-0.5*z*z) / (sigma * std::sqrt(2*M_PI));
}
double StatisticsWidget::normalCDF(double x, double mu, double sigma) {
    return 0.5 * std::erfc(-(x-mu) / (sigma * std::sqrt(2.0)));
}

// Inverse normal CDF (probit) — rational approximation
double StatisticsWidget::normalCDFInv(double p) {
    // Beasley-Springer-Moro algorithm
    static const double a[] = {-3.969683028665376e+01, 2.209460984245205e+02,
        -2.759285104469687e+02, 1.383577518672690e+02, -3.066479806614716e+01, 2.506628277459239e+00};
    static const double b[] = {-5.447609879822406e+01, 1.615858368580409e+02,
        -1.556989798598866e+02, 6.680131188771972e+01, -1.328068155288572e+01};
    static const double c[] = {-7.784894002430293e-03,-3.223964580411365e-01,
        -2.400758277161838e+00,-2.549732539343734e+00, 4.374664141464968e+00, 2.938163982698783e+00};
    static const double d[] = {7.784695709041462e-03, 3.224671290700398e-01,
        2.445134137142996e+00, 3.754408661907416e+00};
    double q, r;
    if(p < 0.02425){
        q = std::sqrt(-2*std::log(p));
        return (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
               ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
    } else if(p <= 0.97575){
        q = p-0.5; r = q*q;
        return (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q /
               (((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1);
    } else {
        q = std::sqrt(-2*std::log(1-p));
        return -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
                ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
    }
}

// Chi-square CDF using regularized incomplete gamma function
double StatisticsWidget::chiSquareCDF(double x, int df) {
    if(x <= 0) return 0;
    // P(df/2, x/2) via series expansion
    double a = df/2.0, xx = x/2.0;
    double term = std::exp(-xx + a*std::log(xx) - std::lgamma(a+1));
    double sum = term;
    for(int i=1;i<200;i++){
        term *= xx/(a+i);
        sum += term;
        if(term < sum*1e-10) break;
    }
    return std::min(sum, 1.0);
}

// t-distribution CDF approximation (two-tailed lower tail)
double StatisticsWidget::tCDF(double t, int df) {
    // Use regularized incomplete beta function approximation
    double x = (double)df / (df + t*t);
    // Simple approximation via normal for large df
    if (df > 30) return normalCDF(t, 0, 1);
    // Iterative approximation
    double a = 0.5 * df, b = 0.5;
    double bt = std::exp(std::lgamma(a+b) - std::lgamma(a) - std::lgamma(b)
                         + a*std::log(x) + b*std::log(1-x));
    // continued fraction
    double qab = a+b, qap = a+1, qam = a-1;
    double c=1, d=1-qab*x/qap; if(std::abs(d)<1e-30) d=1e-30; d=1/d;
    double h=d;
    for(int m=1;m<=100;m++){
        double m2=2*m;
        double aa=m*(b-m)*x/((qam+m2)*(a+m2));
        d=1+aa*d; if(std::abs(d)<1e-30)d=1e-30; c=1+aa/c; if(std::abs(c)<1e-30)c=1e-30;
        d=1/d; h*=d*c;
        aa=-(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
        d=1+aa*d; if(std::abs(d)<1e-30)d=1e-30; c=1+aa/c; if(std::abs(c)<1e-30)c=1e-30;
        d=1/d; double del=d*c; h*=del;
        if(std::abs(del-1)<1e-10) break;
    }
    double ibeta = bt*h/a;
    return t < 0 ? 0.5*ibeta : 1 - 0.5*ibeta;
}

double StatisticsWidget::binomialPMF(int k, int n, double p) {
    if (k < 0 || k > n) return 0;
    // log-space to avoid overflow
    double logC = std::lgamma(n+1) - std::lgamma(k+1) - std::lgamma(n-k+1);
    return std::exp(logC + k*std::log(p) + (n-k)*std::log(1-p));
}
double StatisticsWidget::poissonPMF(int k, double lambda) {
    if (k < 0) return 0;
    return std::exp(-lambda + k*std::log(lambda) - std::lgamma(k+1));
}

double StatisticsWidget::pearsonR(const QVector<double>& x, const QVector<double>& y) {
    int n = x.size();
    double xbar = mean(x), ybar = mean(y);
    double sxy=0, sxx=0, syy=0;
    for(int i=0;i<n;i++){
        sxy += (x[i]-xbar)*(y[i]-ybar);
        sxx += (x[i]-xbar)*(x[i]-xbar);
        syy += (y[i]-ybar)*(y[i]-ybar);
    }
    double denom = std::sqrt(sxx*syy);
    return denom < 1e-12 ? 0 : sxy/denom;
}

double StatisticsWidget::spearmanR(const QVector<double>& x, const QVector<double>& y) {
    int n = x.size();
    // Rank x and y
    auto rank = [&](const QVector<double>& v) {
        QVector<int> idx(n); std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&](int a, int b){ return v[a] < v[b]; });
        QVector<double> r(n);
        for(int i=0;i<n;i++) r[idx[i]] = i+1;
        return r;
    };
    QVector<double> rx = rank(x), ry = rank(y);
    return pearsonR(rx, ry);
}

