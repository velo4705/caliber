#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;

class FinancialWidget : public QWidget {
    Q_OBJECT
public:
    explicit FinancialWidget(QWidget* parent = nullptr);

private:
    // ── Compound Interest ─────────────────────────────────────────────────────
    QWidget*   buildCompoundTab();
    QLineEdit* m_ciP, *m_ciR, *m_ciN, *m_ciT;
    QComboBox* m_ciComp;
    QLabel*    m_ciResult;
    QTextEdit* m_ciSteps;
    QCheckBox* m_ciShow;
    void computeCompound();

    // ── Loan / Mortgage ───────────────────────────────────────────────────────
    QWidget*   buildLoanTab();
    QLineEdit* m_loanP, *m_loanR, *m_loanN;
    QLabel*    m_loanResult;
    QTextEdit* m_loanSteps;
    QCheckBox* m_loanShow;
    void computeLoan();

    // ── NPV / IRR ─────────────────────────────────────────────────────────────
    QWidget*   buildNpvTab();
    QLineEdit* m_npvRate, *m_npvInitial;
    QTextEdit* m_npvCashflows;
    QLabel*    m_npvResult;
    QTextEdit* m_npvSteps;
    QCheckBox* m_npvShow;
    void computeNpv();

    // ── Percentage Tools ──────────────────────────────────────────────────────
    QWidget*   buildPercentTab();
    QLineEdit* m_pctValue, *m_pctPct;
    QLabel*    m_pctResult;
    void computePercent();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
