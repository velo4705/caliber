#pragma once
#include <QWidget>
#include <QVector>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QTableWidget;

class DiscreteMathWidget : public QWidget {
    Q_OBJECT
public:
    explicit DiscreteMathWidget(QWidget* parent = nullptr);

private:
    QWidget* buildGraphTab();
    QWidget* buildCombinTab();
    QWidget* buildRelationsTab();
    QWidget* buildRecurrenceTab();
    QWidget* buildBoolAlgTab();

    // Graph Theory
    QSpinBox*     m_gNodes;
    QTableWidget* m_gMatrix;
    QComboBox*    m_gAlgo;
    QLineEdit*    m_gSrc;
    QLabel*       m_gResult; QTextEdit* m_gSteps; QCheckBox* m_gShow;
    void buildGraphMatrix(int n);
    void computeGraph();

    // Combinatorics
    QLineEdit* m_cn, *m_cr;
    QLabel*    m_combResult; QTextEdit* m_combSteps; QCheckBox* m_combShow;
    void computeCombin();

    // Relations
    QSpinBox*     m_relN;
    QTableWidget* m_relMatrix;
    QLabel*       m_relResult; QTextEdit* m_relSteps; QCheckBox* m_relShow;
    void buildRelMatrix(int n);
    void computeRelations();

    // Recurrence
    QLineEdit* m_recA, *m_recB, *m_recF0, *m_recF1, *m_recN;
    QLabel*    m_recResult; QTextEdit* m_recSteps; QCheckBox* m_recShow;
    void computeRecurrence();

    // Boolean Algebra
    QLineEdit* m_boolExpr;
    QLabel*    m_boolResult; QTextEdit* m_boolSteps; QCheckBox* m_boolShow;
    void computeBoolAlg();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);

    static long long factorial(int n);
    static long long nPr(int n, int r);
    static long long nCr(int n, int r);
};
