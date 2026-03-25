#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QSpinBox;

class NumberTheoryWidget : public QWidget {
    Q_OBJECT
public:
    explicit NumberTheoryWidget(QWidget* parent = nullptr);

private:
    QWidget*   buildPrimeTab();
    QLineEdit* m_primeN;
    QLabel*    m_primeResult;
    QTextEdit* m_primeSteps;
    QCheckBox* m_primeShow;
    void computePrime();

    QWidget*   buildGcdTab();
    QLineEdit* m_gcdA, *m_gcdB;
    QLabel*    m_gcdResult;
    QTextEdit* m_gcdSteps;
    QCheckBox* m_gcdShow;
    void computeGcd();

    QWidget*   buildModTab();
    QLineEdit* m_modA, *m_modM, *m_modExp;
    QLabel*    m_modResult;
    QTextEdit* m_modSteps;
    QCheckBox* m_modShow;
    void computeMod();

    QWidget*   buildBaseTab();
    QLineEdit* m_baseValue;
    QSpinBox*  m_baseFrom, *m_baseTo;
    QLabel*    m_baseResult;
    void computeBase();

    QWidget*   buildSeqTab();
    QLineEdit* m_seqN;
    QLabel*    m_seqResult;
    void computeSeq();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);

    static long long gcd(long long a, long long b);
    static long long modpow(long long base, long long exp, long long mod);
    static long long modInverse(long long a, long long m);
};
