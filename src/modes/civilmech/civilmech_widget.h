#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;

class CivilMechWidget : public QWidget {
    Q_OBJECT
public:
    explicit CivilMechWidget(QWidget* parent = nullptr);

private:
    QWidget* buildBeamTab();
    QWidget* buildStressTab();
    QWidget* buildFluidTab();
    QWidget* buildHeatTab();
    QWidget* buildGearTab();

    // Beam
    QLineEdit* m_bL, *m_bW, *m_bA, *m_bP;
    QComboBox* m_bLoad;
    QLabel*    m_bResult; QTextEdit* m_bSteps; QCheckBox* m_bShow;
    void computeBeam();

    // Stress & Strain
    QLineEdit* m_ssF, *m_ssA, *m_ssE, *m_ssDL, *m_ssL, *m_ssFOS;
    QLabel*    m_ssResult; QTextEdit* m_ssSteps; QCheckBox* m_ssShow;
    void computeStress();

    // Fluid Mechanics
    QLineEdit* m_flV1, *m_flA1, *m_flV2, *m_flA2, *m_flP1, *m_flRho, *m_flH;
    QLineEdit* m_flD, *m_flMu;
    QLabel*    m_flResult; QTextEdit* m_flSteps; QCheckBox* m_flShow;
    void computeFluid();

    // Heat Transfer
    QLineEdit* m_htK, *m_htA, *m_htDT, *m_htL, *m_hth, *m_htQ;
    QLabel*    m_htResult; QTextEdit* m_htSteps; QCheckBox* m_htShow;
    void computeHeat();

    // Gear & Pulley
    QLineEdit* m_gN1, *m_gN2, *m_gT1, *m_gW1;
    QLabel*    m_gResult; QTextEdit* m_gSteps; QCheckBox* m_gShow;
    void computeGear();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
