#pragma once
#include <QWidget>
#include <QVector>

class QTextEdit;
class QLineEdit;
class QLabel;
class QTabWidget;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QTableWidget;
class QChartView;

class StatisticsWidget : public QWidget {
    Q_OBJECT
public:
    explicit StatisticsWidget(QWidget* parent = nullptr);

private:
    // ── Data entry (shared across tabs) ──────────────────────────────────────
    QWidget*   buildDataTab();
    QTextEdit* m_dataInput;
    QLabel*    m_dataStatus;
    QVector<double> m_data;
    void parseData();

    // ── Descriptive statistics ────────────────────────────────────────────────
    QWidget*   buildDescriptiveTab();
    QLabel*    m_descResult;
    QTextEdit* m_descSteps;
    QCheckBox* m_descShowSteps;
    void computeDescriptive();

    // ── Probability distributions ─────────────────────────────────────────────
    QWidget*   buildDistributionTab();
    QComboBox* m_distType;
    QLineEdit* m_distP1, *m_distP2, *m_distP3;
    QLabel*    m_distP1Lbl, *m_distP2Lbl, *m_distP3Lbl;
    QLineEdit* m_distX;
    QLabel*    m_distResult;
    QTextEdit* m_distSteps;
    QCheckBox* m_distShowSteps;
    void onDistTypeChanged(int idx);
    void computeDistribution();

    // ── Hypothesis testing ────────────────────────────────────────────────────
    QWidget*   buildHypothesisTab();
    QComboBox* m_hypTestType;
    QLineEdit* m_hypMu, *m_hypSigma, *m_hypN, *m_hypAlpha, *m_hypMu2, *m_hypSigma2, *m_hypN2;
    QLabel*    m_hypResult;
    QTextEdit* m_hypSteps;
    QCheckBox* m_hypShowSteps;
    void computeHypothesis();

    // ── Confidence intervals ──────────────────────────────────────────────────
    QWidget*   buildConfidenceTab();
    QComboBox* m_ciType;
    QLineEdit* m_ciMean, *m_ciSigma, *m_ciN, *m_ciConf, *m_ciP;
    QLabel*    m_ciResult;
    QTextEdit* m_ciSteps;
    QCheckBox* m_ciShowSteps;
    void computeConfidence();

    // ── Chi-square test ───────────────────────────────────────────────────────
    QWidget*      buildChiSquareTab();
    QTableWidget* m_chiTable;
    QSpinBox*     m_chiCols;
    QLabel*       m_chiResult;
    QTextEdit*    m_chiSteps;
    QCheckBox*    m_chiShowSteps;
    void computeChiSquare();
    void rebuildChiTable(int cols);

    // ── ANOVA ─────────────────────────────────────────────────────────────────
    QWidget*   buildAnovaTab();
    QTextEdit* m_anovaGroups;
    QLabel*    m_anovaResult;
    QTextEdit* m_anovaSteps;
    QCheckBox* m_anovaShowSteps;
    void computeAnova();

    // ── Histogram / Box plot ──────────────────────────────────────────────────
    QWidget*         buildChartTab();
    QSpinBox*        m_chartBins;
    QWidget*         m_histWidget;  // HistogramWidget (forward declared in cpp)
    void buildHistogram();

    // ── Regression & Correlation ──────────────────────────────────────────────
    QWidget*   buildRegressionTab();
    QTextEdit* m_regYInput;
    QLabel*    m_regResult;
    QTextEdit* m_regSteps;
    QCheckBox* m_regShowSteps;
    void computeRegression();

    // ── Frequency distribution ────────────────────────────────────────────────
    QWidget*   buildFrequencyTab();
    QSpinBox*  m_freqBins;
    QLabel*    m_freqResult;
    void computeFrequency();

    // ── Step display helper ───────────────────────────────────────────────────
    void showSteps(QTextEdit* w, QCheckBox* toggle, const QStringList& steps);

    // ── Math helpers ──────────────────────────────────────────────────────────
    static double mean(const QVector<double>& d);
    static double variance(const QVector<double>& d, bool population = false);
    static double stddev(const QVector<double>& d, bool population = false);
    static double median(QVector<double> d);
    static double normalPDF(double x, double mu, double sigma);
    static double normalCDF(double x, double mu, double sigma);
    static double normalCDFInv(double p);
    static double tCDF(double t, int df);
    static double chiSquareCDF(double x, int df);
    static double binomialPMF(int k, int n, double p);
    static double poissonPMF(int k, double lambda);
    static double pearsonR(const QVector<double>& x, const QVector<double>& y);
    static double spearmanR(const QVector<double>& x, const QVector<double>& y);
};
