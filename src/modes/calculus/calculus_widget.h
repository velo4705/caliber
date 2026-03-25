#pragma once
#include <QWidget>

class QTextEdit;
class QLineEdit;
class QLabel;
class QTabWidget;
class QSpinBox;
class QCheckBox;
class QComboBox;

class CalculusWidget : public QWidget {
    Q_OBJECT
public:
    explicit CalculusWidget(QWidget* parent = nullptr);

private:
    // ── Differentiation ───────────────────────────────────────────────────────
    QWidget*   buildDiffTab();
    QLineEdit* m_diffExpr, *m_diffX, *m_diffH;
    QLabel*    m_diffResult;
    QTextEdit* m_diffSteps;
    QCheckBox* m_diffShowSteps;
    void computeDiff();

    // ── Integration ───────────────────────────────────────────────────────────
    QWidget*   buildIntegTab();
    QLineEdit* m_integExpr, *m_integA, *m_integB, *m_integN;
    QLabel*    m_integResult;
    QTextEdit* m_integSteps;
    QCheckBox* m_integShowSteps;
    void computeInteg();

    // ── Double Integration ────────────────────────────────────────────────────
    QWidget*   buildDoubleIntegTab();
    QLineEdit* m_dblExpr, *m_dblXa, *m_dblXb, *m_dblYa, *m_dblYb, *m_dblN;
    QLabel*    m_dblResult;
    QTextEdit* m_dblSteps;
    QCheckBox* m_dblShowSteps;
    void computeDoubleInteg();

    // ── Triple Integration ────────────────────────────────────────────────────
    QWidget*   buildTripleIntegTab();
    QLineEdit* m_triExpr, *m_triXa, *m_triXb, *m_triYa, *m_triYb, *m_triZa, *m_triZb, *m_triN;
    QLabel*    m_triResult;
    QTextEdit* m_triSteps;
    QCheckBox* m_triShowSteps;
    void computeTripleInteg();

    // ── Partial Derivatives ───────────────────────────────────────────────────
    QWidget*   buildPartialTab();
    QLineEdit* m_partExpr, *m_partX, *m_partY, *m_partH;
    QLabel*    m_partResult;
    QTextEdit* m_partSteps;
    QCheckBox* m_partShowSteps;
    void computePartial();

    // ── Limits ────────────────────────────────────────────────────────────────
    QWidget*   buildLimitTab();
    QLineEdit* m_limExpr, *m_limC, *m_limH;
    QLabel*    m_limResult;
    QTextEdit* m_limSteps;
    QCheckBox* m_limShowSteps;
    void computeLimit();

    // ── Taylor Series ─────────────────────────────────────────────────────────
    QWidget*   buildTaylorTab();
    QLineEdit* m_tayExpr, *m_tayA, *m_tayX;
    QSpinBox*  m_tayN;
    QLabel*    m_tayResult;
    QTextEdit* m_taySteps;
    QCheckBox* m_tayShowSteps;
    void computeTaylor();

    // ── Root Finder ───────────────────────────────────────────────────────────
    QWidget*   buildRootTab();
    QLineEdit* m_rootExpr, *m_rootA, *m_rootB;
    QComboBox* m_rootMethod;
    QLabel*    m_rootResult;
    QTextEdit* m_rootSteps;
    QCheckBox* m_rootShowSteps;
    void computeRoot();

    // ── Helpers ───────────────────────────────────────────────────────────────
    void showSteps(QTextEdit* w, QCheckBox* toggle, const QStringList& steps);
    double evalExpr(const QString& expr, double x, double y = 0, double z = 0);
};
