#pragma once
#include <QWidget>
#include <QVector>

class QLineEdit;
class QLabel;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class QComboBox;

class SignalProcessingWidget : public QWidget {
    Q_OBJECT
public:
    explicit SignalProcessingWidget(QWidget* parent = nullptr);

private:
    QWidget* buildDftTab();
    QWidget* buildFilterTab();
    QWidget* buildSamplingTab();
    QWidget* buildConvolutionTab();
    QWidget* buildTransferTab();

    // DFT
    QTextEdit* m_dftInput;
    QLabel*    m_dftResult; QTextEdit* m_dftSteps; QCheckBox* m_dftShow;
    void computeDft();

    // Filter Design
    QLineEdit* m_filtFs, *m_filtFc, *m_filtFc2;
    QComboBox* m_filtType;
    QLabel*    m_filtResult; QTextEdit* m_filtSteps; QCheckBox* m_filtShow;
    void computeFilter();

    // Sampling
    QLineEdit* m_sampFmax, *m_sampFs;
    QLabel*    m_sampResult; QTextEdit* m_sampSteps; QCheckBox* m_sampShow;
    void computeSampling();

    // Convolution
    QTextEdit* m_convX, *m_convH;
    QLabel*    m_convResult; QTextEdit* m_convSteps; QCheckBox* m_convShow;
    void computeConvolution();

    // Transfer Function
    QTextEdit* m_tfNum, *m_tfDen;
    QLabel*    m_tfResult; QTextEdit* m_tfSteps; QCheckBox* m_tfShow;
    void computeTransfer();

    void showSteps(QTextEdit* w, QCheckBox* t, const QStringList& s);
};
