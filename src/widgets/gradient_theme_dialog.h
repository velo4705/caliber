#pragma once
#include <QDialog>

class QColor;
class QPushButton;
class QRadioButton;
class QComboBox;
class QLabel;

class GradientThemeDialog : public QDialog {
    Q_OBJECT
public:
    explicit GradientThemeDialog(QWidget* parent = nullptr);

    QColor gradientStart() const;
    QColor gradientEnd() const;
    bool   darkBase() const;
    int    angle() const; // 0, 45, 90, 135, 180

    // Load from saved settings
    void loadSettings();
    // Save to QSettings
    void saveSettings();

private:
    void updatePreview();

    QPushButton* m_startBtn;
    QPushButton* m_endBtn;
    QRadioButton* m_lightBase;
    QRadioButton* m_darkBase;
    QComboBox*   m_angleCombo;
    QLabel*      m_previewLabel;

    QColor m_startColor;
    QColor m_endColor;
};
