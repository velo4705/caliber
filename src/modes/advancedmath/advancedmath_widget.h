#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;
class QSpinBox;

class AdvancedMathWidget : public QWidget {
    Q_OBJECT
public:
    explicit AdvancedMathWidget(QWidget* parent = nullptr);

private:
    QWidget* buildComplexTab();
    QWidget* buildLinAlgTab();
    QWidget* buildSeriesTab();
    QWidget* buildTrigSolverTab();
    QWidget* buildPolyTab();
    QWidget* buildSetTab();

    // Complex Numbers
    QLineEdit* m_ca1, *m_cb1, *m_ca2, *m_cb2;
    QComboBox* m_cOp;
    QLabel*    m_cResult; QTextEdit* m_cSteps; QCheckBox* m_cShow;
    void computeComplex();

    // Linear Algebra (2x2 eigenvalues)
    QLineEdit* m_la[4]; // 2x2 matrix
    QLabel*    m_laResult; QTextEdit* m_laSteps; QCheckBox* m_laShow;
    void computeLinAlg();

    // Sequences & Series
    QLineEdit* m_seqA, *m_seqD, *m_seqR, *m_seqN;
    QComboBox* m_seqType;
    QLabel*    m_seqResult; QTextEdit* m_seqSteps; QCheckBox* m_seqShow;
    void computeSeries();

    // Trigonometry Solver (triangles)
    QLineEdit* m_trA, *m_trB, *m_trC, *m_trAngA, *m_trAngB, *m_trAngC;
    QComboBox* m_trCase;
    QLabel*    m_trResult; QTextEdit* m_trSteps; QCheckBox* m_trShow;
    void computeTrigSolver();

    // Polynomial Tools
    QLineEdit* m_polyA, *m_polyB, *m_polyC, *m_polyD;
    QComboBox* m_polyDeg;
    QLabel*    m_polyResult; QTextEdit* m_polySteps; QCheckBox* m_polyShow;
    void computePoly();

    // Set Theory
    QLineEdit* m_setA, *m_setB, *m_setU;
    QLabel*    m_setResult; QTextEdit* m_setSteps; QCheckBox* m_setShow;
    void computeSet();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
