#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;

class ControlSystemsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ControlSystemsWidget(QWidget* parent = nullptr);

private:
    QWidget* buildTransferTab();
    QWidget* buildRouthTab();
    QWidget* buildPidTab();
    QWidget* buildSteadyStateTab();
    QWidget* buildStateSpaceTab();

    // Transfer Function
    QTextEdit* m_tfNum, *m_tfDen;
    QLabel*    m_tfResult; QTextEdit* m_tfSteps; QCheckBox* m_tfShow;
    void computeTransfer();

    // Routh-Hurwitz
    QTextEdit* m_rhCoeffs;
    QLabel*    m_rhResult; QTextEdit* m_rhSteps; QCheckBox* m_rhShow;
    void computeRouth();

    // PID Tuning
    QLineEdit* m_pidKu, *m_pidTu;
    QComboBox* m_pidMethod;
    QLabel*    m_pidResult; QTextEdit* m_pidSteps; QCheckBox* m_pidShow;
    void computePid();

    // Steady-State Error
    QTextEdit* m_sseNum, *m_sseDen;
    QComboBox* m_sseType;
    QLabel*    m_sseResult; QTextEdit* m_sseSteps; QCheckBox* m_sseShow;
    void computeSteadyState();

    // State Space
    QTextEdit* m_ssNum, *m_ssDen;
    QLabel*    m_ssResult; QTextEdit* m_ssSteps; QCheckBox* m_ssShow;
    void computeStateSpace();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
