#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;
class QSpinBox;

class McsWidget : public QWidget {
    Q_OBJECT
public:
    explicit McsWidget(QWidget* parent = nullptr);

private:
    QWidget* buildAsymptoticTab();
    QWidget* buildMasterTab();
    QWidget* buildFloatTab();
    QWidget* buildLogicTab();
    QWidget* buildHashTab();

    // Asymptotic Analysis
    QLineEdit* m_asymExpr, *m_asymN;
    QComboBox* m_asymCompare;
    QLabel*    m_asymResult; QTextEdit* m_asymSteps; QCheckBox* m_asymShow;
    void computeAsymptotic();

    // Master Theorem
    QLineEdit* m_mtA, *m_mtB, *m_mtK, *m_mtP;
    QLabel*    m_mtResult; QTextEdit* m_mtSteps; QCheckBox* m_mtShow;
    void computeMaster();

    // Floating Point
    QLineEdit* m_fpValue;
    QComboBox* m_fpPrec;
    QLabel*    m_fpResult; QTextEdit* m_fpSteps; QCheckBox* m_fpShow;
    void computeFloat();

    // Formal Logic
    QLineEdit* m_logExpr;
    QSpinBox*  m_logVars;
    QLabel*    m_logResult;
    void computeLogic();

    // Hashing
    QLineEdit* m_hashInput;
    QLabel*    m_hashResult;
    void computeHash();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
