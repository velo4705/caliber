#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;

class ElectricalWidget : public QWidget {
    Q_OBJECT
public:
    explicit ElectricalWidget(QWidget* parent = nullptr);

private:
    // ── Ohm's Law ─────────────────────────────────────────────────────────────
    QWidget*   buildOhmTab();
    QLineEdit* m_ohmV, *m_ohmI, *m_ohmR, *m_ohmP;
    QLabel*    m_ohmResult;
    QTextEdit* m_ohmSteps;
    QCheckBox* m_ohmShow;
    void computeOhm();

    // ── Series / Parallel Resistors ───────────────────────────────────────────
    QWidget*   buildResistorTab();
    QTextEdit* m_resValues;
    QComboBox* m_resMode;
    QLabel*    m_resResult;
    QTextEdit* m_resSteps;
    QCheckBox* m_resShow;
    void computeResistor();

    // ── RC / RL / RLC Circuits ────────────────────────────────────────────────
    QWidget*   buildRlcTab();
    QLineEdit* m_rlcR, *m_rlcL, *m_rlcC, *m_rlcF;
    QLabel*    m_rlcResult;
    QTextEdit* m_rlcSteps;
    QCheckBox* m_rlcShow;
    void computeRlc();

    // ── Voltage Divider ───────────────────────────────────────────────────────
    QWidget*   buildVdivTab();
    QLineEdit* m_vdivVin, *m_vdivR1, *m_vdivR2;
    QLabel*    m_vdivResult;
    QTextEdit* m_vdivSteps;
    QCheckBox* m_vdivShow;
    void computeVdiv();

    // ── Op-Amp ────────────────────────────────────────────────────────────────
    QWidget*   buildOpAmpTab();
    QComboBox* m_opAmpType;
    QLineEdit* m_opAmpR1, *m_opAmpR2, *m_opAmpVin;
    QLabel*    m_opAmpResult;
    QTextEdit* m_opAmpSteps;
    QCheckBox* m_opAmpShow;
    void computeOpAmp();

    // ── dB Converter ──────────────────────────────────────────────────────────
    QWidget*   buildDbTab();
    QLineEdit* m_dbValue;
    QComboBox* m_dbMode;
    QLabel*    m_dbResult;
    void computeDb();

    // ── Phasor Calculator ─────────────────────────────────────────────────────
    QWidget*   buildPhasorTab();
    QLineEdit* m_ph1Mag, *m_ph1Ang, *m_ph2Mag, *m_ph2Ang;
    QComboBox* m_phOp;
    QLabel*    m_phResult;
    QTextEdit* m_phSteps;
    QCheckBox* m_phShow;
    void computePhasor();

    // ── Power Factor ──────────────────────────────────────────────────────────
    QWidget*   buildPowerTab();
    QLineEdit* m_pwrP, *m_pwrQ, *m_pwrS, *m_pwrPf;
    QLabel*    m_pwrResult;
    QTextEdit* m_pwrSteps;
    QCheckBox* m_pwrShow;
    void computePower();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
