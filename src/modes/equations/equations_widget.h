#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QSpinBox;
class QTableWidget;

// Equations & Matrices mode
//  Tab 1 — Linear equation solver (1 and 2 variables)
//  Tab 2 — Quadratic equation solver
//  Tab 3 — System of equations (up to 4x4, Gaussian elimination)
//  Tab 4 — Matrix operations
class EquationsWidget : public QWidget {
    Q_OBJECT
public:
    explicit EquationsWidget(QWidget* parent = nullptr);

private:
    // Tab 1: Linear
    QWidget*   buildLinearTab();
    QLineEdit* m_linA, *m_linB;           // ax + b = 0
    QLineEdit* m_lin2A, *m_lin2B, *m_lin2C, *m_lin2D; // ax+by=c, dx+ey=f
    QLabel*    m_linResult;

    // Tab 2: Quadratic
    QWidget*   buildQuadraticTab();
    QLineEdit* m_quadA, *m_quadB, *m_quadC;
    QLabel*    m_quadResult;

    // Tab 3: System of equations
    QWidget*      buildSystemTab();
    QSpinBox*     m_sysSize;
    QTableWidget* m_sysMatrix;   // augmented matrix [A|b]
    QLabel*       m_sysResult;

    // Tab 4: Matrix ops
    QWidget*      buildMatrixTab();
    QSpinBox*     m_matRowsA, *m_matColsA;
    QSpinBox*     m_matRowsB, *m_matColsB;
    QTableWidget* m_matA;
    QTableWidget* m_matB;
    QTableWidget* m_matResult;
    QLabel*       m_matScalarResult;

    void solveLinear();
    void solveQuadratic();
    void solveSystem();
    void rebuildSysMatrix(int n);
    void rebuildMatrixTable(QTableWidget* tbl, int rows, int cols);
    void matOpAdd();
    void matOpSub();
    void matOpMul();
    void matOpTransposeA();
    void matOpDetA();
    void matOpInvA();

    // Matrix helpers
    using Matrix = QVector<QVector<double>>;
    Matrix readMatrix(QTableWidget* tbl) const;
    void   writeMatrix(QTableWidget* tbl, const Matrix& m);
    Matrix matAdd(const Matrix& a, const Matrix& b) const;
    Matrix matSub(const Matrix& a, const Matrix& b) const;
    Matrix matMul(const Matrix& a, const Matrix& b) const;
    Matrix matTranspose(const Matrix& a) const;
    double matDet(const Matrix& a) const;
    Matrix matInverse(const Matrix& a) const;
    Matrix gaussianElim(Matrix aug, int n) const; // returns solution vector as nx1
};
