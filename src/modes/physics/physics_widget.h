#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;

class PhysicsWidget : public QWidget {
    Q_OBJECT
public:
    explicit PhysicsWidget(QWidget* parent = nullptr);

private:
    QWidget* buildKinematicsTab();
    QWidget* buildNewtonsTab();
    QWidget* buildEnergyTab();
    QWidget* buildProjectileTab();
    QWidget* buildCircularTab();
    QWidget* buildWavesTab();
    QWidget* buildThermTab();
    QWidget* buildElectrostaticsTab();

    // Kinematics
    QLineEdit* m_ks, *m_ku, *m_kv, *m_ka, *m_kt;
    QLabel*    m_kResult; QTextEdit* m_kSteps; QCheckBox* m_kShow;
    void computeKinematics();

    // Newton's Laws
    QLineEdit* m_nF, *m_nM, *m_nA, *m_nMu, *m_nN;
    QLabel*    m_nResult; QTextEdit* m_nSteps; QCheckBox* m_nShow;
    void computeNewtons();

    // Energy & Work
    QLineEdit* m_em, *m_ev, *m_eh, *m_eF, *m_ed;
    QLabel*    m_eResult; QTextEdit* m_eSteps; QCheckBox* m_eShow;
    void computeEnergy();

    // Projectile
    QLineEdit* m_pv0, *m_pAngle, *m_ph0;
    QLabel*    m_pResult; QTextEdit* m_pSteps; QCheckBox* m_pShow;
    void computeProjectile();

    // Circular Motion
    QLineEdit* m_cm, *m_cr, *m_cv, *m_cT;
    QLabel*    m_cResult; QTextEdit* m_cSteps; QCheckBox* m_cShow;
    void computeCircular();

    // Waves & Optics
    QLineEdit* m_wv, *m_wf, *m_wlambda;
    QLineEdit* m_on1, *m_on2, *m_oTheta1;
    QLabel*    m_wResult; QTextEdit* m_wSteps; QCheckBox* m_wShow;
    void computeWaves();

    // Thermodynamics
    QLineEdit* m_tP, *m_tV, *m_tn, *m_tT, *m_tm, *m_tc, *m_tdT;
    QLabel*    m_tResult; QTextEdit* m_tSteps; QCheckBox* m_tShow;
    void computeTherm();

    // Electrostatics
    QLineEdit* m_esq1, *m_esq2, *m_esr;
    QLabel*    m_esResult; QTextEdit* m_esSteps; QCheckBox* m_esShow;
    void computeElectrostatics();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
