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

class DigitalLogicWidget : public QWidget {
    Q_OBJECT
public:
    explicit DigitalLogicWidget(QWidget* parent = nullptr);

private:
    // ── Truth Table ───────────────────────────────────────────────────────────
    QWidget*      buildTruthTab();
    QLineEdit*    m_ttExpr;
    QSpinBox*     m_ttVars;
    QTableWidget* m_ttTable;
    QLabel*       m_ttResult;
    void buildTruthTable();
    bool evalBool(const QString& expr, const QVector<bool>& vals, const QStringList& vars);

    // ── K-Map ─────────────────────────────────────────────────────────────────
    QWidget*      buildKmapTab();
    QSpinBox*     m_kmapVars;
    QTableWidget* m_kmapTable;
    QLabel*       m_kmapResult;
    QTextEdit*    m_kmapSteps;
    QCheckBox*    m_kmapShow;
    void buildKmap();
    void computeKmap();
    QString simplifyKmap(const QVector<int>& minterms, int vars);

    // ── Number System Converter ───────────────────────────────────────────────
    QWidget*   buildNumSysTab();
    QLineEdit* m_nsValue;
    QLabel*    m_nsResult;
    void computeNumSys();

    // ── Flip-Flop Analyzer ────────────────────────────────────────────────────
    QWidget*   buildFlipFlopTab();
    QComboBox* m_ffType;
    QLineEdit* m_ffInputs;
    QLabel*    m_ffResult;
    void computeFlipFlop();

    // ── Adder / Subtractor ────────────────────────────────────────────────────
    QWidget*   buildAdderTab();
    QLineEdit* m_addA, *m_addB;
    QComboBox* m_addType;
    QLabel*    m_addResult;
    QTextEdit* m_addSteps;
    QCheckBox* m_addShow;
    void computeAdder();

    // ── IEEE 754 Visualizer ───────────────────────────────────────────────────
    QWidget*   buildIeeeTab();
    QLineEdit* m_ieeeValue;
    QComboBox* m_ieeePrec;
    QLabel*    m_ieeeResult;
    void computeIeee();

    // ── MUX / DEMUX ───────────────────────────────────────────────────────────
    QWidget*   buildMuxTab();
    QComboBox* m_muxType;
    QSpinBox*  m_muxSel;
    QLineEdit* m_muxInputs;
    QLabel*    m_muxResult;
    QTextEdit* m_muxSteps;
    QCheckBox* m_muxShow;
    void computeMux();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
