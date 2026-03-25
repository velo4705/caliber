#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;

class ChemistryWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChemistryWidget(QWidget* parent = nullptr);

private:
    QWidget* buildMolarMassTab();
    QWidget* buildIdealGasTab();
    QWidget* buildPhTab();
    QWidget* buildDilutionTab();
    QWidget* buildThermochemTab();

    // Molar Mass
    QLineEdit* m_mmFormula;
    QLabel*    m_mmResult;
    QTextEdit* m_mmSteps; QCheckBox* m_mmShow;
    void computeMolarMass();
    double parseMolarMass(const QString& formula, QStringList& steps);

    // Ideal Gas
    QLineEdit* m_igP, *m_igV, *m_ign, *m_igT;
    QLabel*    m_igResult; QTextEdit* m_igSteps; QCheckBox* m_igShow;
    void computeIdealGas();

    // pH
    QLineEdit* m_phConc, *m_phKa, *m_phKb;
    QComboBox* m_phType;
    QLabel*    m_phResult; QTextEdit* m_phSteps; QCheckBox* m_phShow;
    void computePh();

    // Dilution
    QLineEdit* m_dilC1, *m_dilV1, *m_dilC2, *m_dilV2;
    QLabel*    m_dilResult; QTextEdit* m_dilSteps; QCheckBox* m_dilShow;
    void computeDilution();

    // Thermochemistry
    QLineEdit* m_tcDH, *m_tcDS, *m_tcT;
    QTextEdit* m_tcHess;
    QLabel*    m_tcResult; QTextEdit* m_tcSteps; QCheckBox* m_tcShow;
    void computeThermochem();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
