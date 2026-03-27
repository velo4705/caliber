#include "gradient_theme_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QSettings>
#include <QFrame>

GradientThemeDialog::GradientThemeDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Custom Gradient Theme");
    setMinimumWidth(420);

    m_startColor = QColor(0x1a, 0x1a, 0x2e);
    m_endColor   = QColor(0x16, 0x21, 0x3e);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 20, 20, 16);

    auto* title = new QLabel("Design Your Gradient Theme", this);
    title->setStyleSheet("font-size: 16px; font-weight: bold;");
    layout->addWidget(title);

    auto* desc = new QLabel("Pick two colors to form a gradient across the UI.\nThe gradient flows from top-left to bottom-right.", this);
    desc->setStyleSheet("color: #888; font-size: 12px;");
    layout->addWidget(desc);

    // ── Color pickers ─────────────────────────────────────────────────────
    auto* grid = new QGridLayout();
    grid->setSpacing(8);

    grid->addWidget(new QLabel("Start Color:", this), 0, 0);
    m_startBtn = new QPushButton(this);
    m_startBtn->setFixedSize(120, 32);
    grid->addWidget(m_startBtn, 0, 1);

    grid->addWidget(new QLabel("End Color:", this), 1, 0);
    m_endBtn = new QPushButton(this);
    m_endBtn->setFixedSize(120, 32);
    grid->addWidget(m_endBtn, 1, 1);

    layout->addLayout(grid);

    // ── Base mode (light/dark) ─────────────────────────────────────────────
    auto* baseLabel = new QLabel("Base Mode:", this);
    baseLabel->setStyleSheet("font-weight: bold; margin-top: 8px;");
    layout->addWidget(baseLabel);

    auto* baseRow = new QHBoxLayout();
    m_lightBase = new QRadioButton("Light (light text on dark gradient)", this);
    m_darkBase  = new QRadioButton("Dark (dark text on light gradient)", this);
    m_lightBase->setChecked(true);
    auto* baseGroup = new QButtonGroup(this);
    baseGroup->addButton(m_lightBase); baseGroup->addButton(m_darkBase);
    baseRow->addWidget(m_lightBase);
    baseRow->addWidget(m_darkBase);
    layout->addLayout(baseRow);

    // ── Angle ──────────────────────────────────────────────────────────────
    auto* angleRow = new QHBoxLayout();
    angleRow->addWidget(new QLabel("Gradient Angle:", this));
    m_angleCombo = new QComboBox(this);
    m_angleCombo->addItems({"Horizontal →", "Diagonal ↘", "Vertical ↓", "Diagonal ↙", "Horizontal ←"});
    m_angleCombo->setCurrentIndex(1); // default diagonal
    angleRow->addWidget(m_angleCombo);
    angleRow->addStretch();
    layout->addLayout(angleRow);

    // ── Preview ────────────────────────────────────────────────────────────
    auto* previewLabel = new QLabel("Preview:", this);
    previewLabel->setStyleSheet("font-weight: bold; margin-top: 8px;");
    layout->addWidget(previewLabel);

    m_previewLabel = new QLabel(this);
    m_previewLabel->setFixedHeight(60);
    m_previewLabel->setObjectName("gradientPreview");
    layout->addWidget(m_previewLabel);

    // ── Buttons ────────────────────────────────────────────────────────────
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_startBtn, &QPushButton::clicked, this, [this]{
        QColor c = QColorDialog::getColor(m_startColor, this, "Start Color");
        if (c.isValid()) { m_startColor = c; updatePreview(); }
    });
    connect(m_endBtn, &QPushButton::clicked, this, [this]{
        QColor c = QColorDialog::getColor(m_endColor, this, "End Color");
        if (c.isValid()) { m_endColor = c; updatePreview(); }
    });
    connect(m_angleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]{ updatePreview(); });
    connect(m_lightBase, &QRadioButton::toggled, this, [this]{ updatePreview(); });

    updatePreview();
}

QColor GradientThemeDialog::gradientStart() const { return m_startColor; }
QColor GradientThemeDialog::gradientEnd() const { return m_endColor; }
bool   GradientThemeDialog::darkBase() const { return m_lightBase->isChecked(); }

int GradientThemeDialog::angle() const {
    switch (m_angleCombo->currentIndex()) {
        case 0: return 0;
        case 1: return 45;
        case 2: return 90;
        case 3: return 135;
        case 4: return 180;
        default: return 45;
    }
}

void GradientThemeDialog::updatePreview() {
    m_startBtn->setStyleSheet(
        QString("background: %1; border: 1px solid #666; border-radius: 4px;").arg(m_startColor.name()));
    m_endBtn->setStyleSheet(
        QString("background: %1; border: 1px solid #666; border-radius: 4px;").arg(m_endColor.name()));

    // Compute gradient coordinates from angle
    double a = angle() * M_PI / 180.0;
    double cx = 0.5, cy = 0.5;
    double dx = std::cos(a) * 0.5, dy = std::sin(a) * 0.5;
    double x1 = cx - dx, y1 = cy - dy;
    double x2 = cx + dx, y2 = cy + dy;

    bool dark = darkBase();
    QString textColor = dark ? "#e8e8e8" : "#1a1a1a";

    m_previewLabel->setStyleSheet(QString(
        "#gradientPreview {"
        "  background: qlineargradient(x1:%1,y1:%2,x2:%3,y2:%4,"
        "    stop:0 %5, stop:1 %6);"
        "  border: 1px solid %7;"
        "  border-radius: 8px;"
        "  color: %8;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  padding: 8px;"
        "}"
    ).arg(x1, 0, 'f', 2).arg(y1, 0, 'f', 2)
     .arg(x2, 0, 'f', 2).arg(y2, 0, 'f', 2)
     .arg(m_startColor.name()).arg(m_endColor.name())
     .arg(dark ? "#444" : "#ccc")
     .arg(textColor));
    m_previewLabel->setText("  Caliber — Gradient Theme Preview");
}

void GradientThemeDialog::loadSettings() {
    QSettings s("Caliber", "Caliber");
    m_startColor = QColor(s.value("gradient/start", "#1a1a2e").toString());
    m_endColor   = QColor(s.value("gradient/end",   "#16213e").toString());
    bool dark = s.value("gradient/darkBase", true).toBool();
    m_lightBase->setChecked(dark);
    m_darkBase->setChecked(!dark);
    int ang = s.value("gradient/angle", 45).toInt();
    switch (ang) {
        case 0:   m_angleCombo->setCurrentIndex(0); break;
        case 45:  m_angleCombo->setCurrentIndex(1); break;
        case 90:  m_angleCombo->setCurrentIndex(2); break;
        case 135: m_angleCombo->setCurrentIndex(3); break;
        case 180: m_angleCombo->setCurrentIndex(4); break;
        default:  m_angleCombo->setCurrentIndex(1);
    }
    updatePreview();
}

void GradientThemeDialog::saveSettings() {
    QSettings s("Caliber", "Caliber");
    s.setValue("gradient/start",    m_startColor.name());
    s.setValue("gradient/end",      m_endColor.name());
    s.setValue("gradient/darkBase", m_lightBase->isChecked());
    s.setValue("gradient/angle",    angle());
}
