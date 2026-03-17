#include "graphing_widget.h"
#include "function_parser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QFrame>
#include <QPixmap>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QStackedWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DInputHandler>
#include <QtDataVisualization/Q3DTheme>
#include <cmath>
#include <limits>
#include <algorithm>

// ── Color palettes ────────────────────────────────────────────────────────────
static const QColor COLORS_DARK[]  = {
    {0x42,0x9e,0xf5},{0xff,0x6e,0x40},{0x69,0xf0,0xae},
    {0xff,0xd7,0x40},{0xea,0x80,0xfc},{0xff,0x40,0x81},
    {0x40,0xc4,0xff},{0xff,0x52,0x52},
};
static const QColor COLORS_LIGHT[] = {
    {0x19,0x65,0xb0},{0xbf,0x36,0x0c},{0x1b,0x5e,0x20},
    {0xf5,0x7f,0x17},{0x6a,0x1b,0x9a},{0x88,0x0e,0x4f},
    {0x00,0x60,0x64},{0xb7,0x1c,0x1c},
};
static constexpr int NUM_COLORS = 8;

// ── Zoom + Pan chart view ─────────────────────────────────────────────────────
class ZoomChartView : public QChartView {
public:
    explicit ZoomChartView(QChart* chart, QWidget* parent = nullptr)
        : QChartView(chart, parent)
    {
        setMouseTracking(true);
    }

protected:
    void wheelEvent(QWheelEvent* e) override {
        double f = e->angleDelta().y() > 0 ? 0.85 : 1.0 / 0.85;
        chart()->zoom(f);
        e->accept();
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton || e->button() == Qt::MiddleButton) {
            m_lastPan = e->pos();
            m_panning = true;
            setCursor(Qt::ClosedHandCursor);
            e->accept();
        } else {
            QChartView::mousePressEvent(e);
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (m_panning) {
            QPoint delta = e->pos() - m_lastPan;
            // scroll() takes pixel offsets and pans the chart axes
            chart()->scroll(-delta.x(), delta.y());
            m_lastPan = e->pos();
            e->accept();
        } else {
            QChartView::mouseMoveEvent(e);
        }
    }

    void mouseReleaseEvent(QMouseEvent* e) override {
        if (m_panning) {
            m_panning = false;
            setCursor(Qt::ArrowCursor);
            e->accept();
        } else {
            QChartView::mouseReleaseEvent(e);
        }
    }

private:
    bool   m_panning = false;
    QPoint m_lastPan;
};

static QPushButton* mkBtn(const QString& t, const QString& cls, QWidget* p) {
    auto* b = new QPushButton(t, p);
    b->setProperty("class", cls);
    b->setMinimumHeight(32);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}

GraphingWidget::GraphingWidget(QWidget* parent)
    : QWidget(parent), m_parser(new FunctionParser())
{
    buildUI();
}

void GraphingWidget::buildUI() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // 2D chart
    m_chart = new QChart();
    m_chart->legend()->hide();
    m_chart->setAnimationOptions(QChart::NoAnimation);
    m_chart->setMargins(QMargins(4,4,4,4));

    m_axisX = new QValueAxis();
    m_axisX->setRange(-10,10); m_axisX->setTickCount(11);
    m_axisX->setLabelFormat("%.1f"); m_axisX->setTitleText("x");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY = new QValueAxis();
    m_axisY->setRange(-10,10); m_axisY->setTickCount(11);
    m_axisY->setLabelFormat("%.1f"); m_axisY->setTitleText("y");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    // Cartesian axes at origin
    QPen axisPen(Qt::black); axisPen.setWidth(2);
    auto* hAxis = new QLineSeries(); hAxis->setName("__haxis__"); hAxis->setPen(axisPen);
    hAxis->append(-1e9,0); hAxis->append(1e9,0);
    m_chart->addSeries(hAxis); hAxis->attachAxis(m_axisX); hAxis->attachAxis(m_axisY);

    auto* vAxis = new QLineSeries(); vAxis->setName("__vaxis__"); vAxis->setPen(axisPen);
    vAxis->append(0,-1e9); vAxis->append(0,1e9);
    m_chart->addSeries(vAxis); vAxis->attachAxis(m_axisX); vAxis->attachAxis(m_axisY);

    QPen gridPen(QColor(180,180,180)); gridPen.setWidthF(0.5);
    m_axisX->setGridLinePen(gridPen);
    m_axisY->setGridLinePen(gridPen);

    m_chartView = new ZoomChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setRubberBand(QChartView::NoRubberBand);
    root->addWidget(m_chartView, 1);
    setLayout(root);

    // 3D surface is lazy-initialized on first switch to 3D mode
    // (avoids OpenGL context creation at startup — important for VMs / no-GPU)

    applyChartTheme();

    // ── Bottom panel ─────────────────────────────────────────────────────────
    m_panel = new QWidget(this);
    m_panel->setObjectName("graphBottomPanel");
    m_panel->setAttribute(Qt::WA_StyledBackground, true);
    m_panel->setFixedHeight(PANEL_HEIGHT_3D);
    // No inline stylesheet — themed via app QSS (#graphBottomPanel)

    auto* panelVL = new QVBoxLayout(m_panel);
    panelVL->setContentsMargins(0,0,0,0);
    panelVL->setSpacing(0);

    // ── Chip row (always visible, both 2D and 3D) ─────────────────────────
    m_3dChipRow = new QWidget(m_panel);
    m_3dChipRow->setFixedHeight(40);
    m_3dChipRow->setStyleSheet("background:transparent;");
    m_3dChipRowLayout = new QHBoxLayout(m_3dChipRow);
    m_3dChipRowLayout->setContentsMargins(12,4,12,4);
    m_3dChipRowLayout->setSpacing(6);
    m_3dChipRowLayout->addStretch();
    panelVL->addWidget(m_3dChipRow);

    // ── Controls row ──────────────────────────────────────────────────────
    auto* controlsRow = new QWidget(m_panel);
    controlsRow->setFixedHeight(PANEL_HEIGHT);
    panelVL->addWidget(controlsRow);

    const int H = 32;
    auto* pl = new QHBoxLayout(controlsRow);
    pl->setContentsMargins(12,0,12,0);
    pl->setSpacing(10);

    // 2D/3D radio buttons
    m_radio2D = new QRadioButton("2D", controlsRow);
    m_radio3D = new QRadioButton("3D", controlsRow);
    m_radio2D->setChecked(true);
    auto* dimGroup = new QButtonGroup(controlsRow);
    dimGroup->addButton(m_radio2D); dimGroup->addButton(m_radio3D);
    pl->addWidget(m_radio2D, 0, Qt::AlignVCenter);
    pl->addWidget(m_radio3D, 0, Qt::AlignVCenter);

    // Divider
    auto* div0 = new QFrame(controlsRow);
    div0->setFrameShape(QFrame::VLine); div0->setFrameShadow(QFrame::Sunken);
    pl->addWidget(div0);

    // Function input
    m_funcInput = new QLineEdit(controlsRow);
    m_funcInput->setPlaceholderText("e.g.  sin(x)  or  x^2 - 4");
    m_funcInput->setFixedHeight(H);
    m_funcInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto* addBtn = mkBtn("+", "actionButton", controlsRow);
    addBtn->setFixedSize(H, H);
    addBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pl->addWidget(m_funcInput, 4, Qt::AlignVCenter);
    pl->addWidget(addBtn,      0, Qt::AlignVCenter);

    // Divider
    auto* div1 = new QFrame(controlsRow);
    div1->setFrameShape(QFrame::VLine); div1->setFrameShadow(QFrame::Sunken);
    pl->addWidget(div1);

    // X/Y range
    auto* rangeWidget = new QWidget(controlsRow);
    rangeWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    auto* rl = new QVBoxLayout(rangeWidget);
    rl->setContentsMargins(0,0,0,0); rl->setSpacing(4);

    auto* xRow = new QHBoxLayout(); xRow->setSpacing(4); xRow->setContentsMargins(0,0,0,0);
    xRow->addWidget(new QLabel("X:", controlsRow));
    m_xMin = new QDoubleSpinBox(controlsRow); m_xMin->setRange(-1e6,1e6); m_xMin->setValue(-10); m_xMin->setDecimals(1); m_xMin->setFixedSize(68,H);
    m_xMax = new QDoubleSpinBox(controlsRow); m_xMax->setRange(-1e6,1e6); m_xMax->setValue(10);  m_xMax->setDecimals(1); m_xMax->setFixedSize(68,H);
    xRow->addWidget(m_xMin); xRow->addWidget(new QLabel("→",controlsRow)); xRow->addWidget(m_xMax);
    rl->addLayout(xRow);

    auto* yRow = new QHBoxLayout(); yRow->setSpacing(4); yRow->setContentsMargins(0,0,0,0);
    yRow->addWidget(new QLabel("Y:", controlsRow));
    m_yMin = new QDoubleSpinBox(controlsRow); m_yMin->setRange(-1e6,1e6); m_yMin->setValue(-10); m_yMin->setDecimals(1); m_yMin->setFixedSize(68,H);
    m_yMax = new QDoubleSpinBox(controlsRow); m_yMax->setRange(-1e6,1e6); m_yMax->setValue(10);  m_yMax->setDecimals(1); m_yMax->setFixedSize(68,H);
    yRow->addWidget(m_yMin); yRow->addWidget(new QLabel("→",controlsRow)); yRow->addWidget(m_yMax);
    rl->addLayout(yRow);
    pl->addWidget(rangeWidget, 0, Qt::AlignVCenter);

    // Divider
    auto* div2 = new QFrame(controlsRow);
    div2->setFrameShape(QFrame::VLine); div2->setFrameShadow(QFrame::Sunken);
    pl->addWidget(div2);

    // Chart theme toggle
    m_themeBtn = new QToolButton(controlsRow);
    m_themeBtn->setText("☀");
    m_themeBtn->setToolTip("Toggle chart light/dark");
    m_themeBtn->setFixedSize(H, H);
    m_themeBtn->setObjectName("historyToggleBtn");
    pl->addWidget(m_themeBtn, 0, Qt::AlignVCenter);

    // Divider
    auto* div3 = new QFrame(controlsRow);
    div3->setFrameShape(QFrame::VLine); div3->setFrameShadow(QFrame::Sunken);
    pl->addWidget(div3);

    // Reset + Export
    auto* resetBtn  = mkBtn("Reset",  "operatorButton", controlsRow); resetBtn->setFixedHeight(H);  resetBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto* exportBtn = mkBtn("Export", "calcButton",     controlsRow); exportBtn->setFixedHeight(H); exportBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pl->addWidget(resetBtn,  1, Qt::AlignVCenter);
    pl->addWidget(exportBtn, 1, Qt::AlignVCenter);

    // ── Toggle button ─────────────────────────────────────────────────────────
    m_toggleBtn = new QToolButton(this);
    m_toggleBtn->setText("⌃  Controls");
    m_toggleBtn->setObjectName("historyToggleBtn");
    m_toggleBtn->setFixedHeight(28);
    m_toggleBtn->setFocusPolicy(Qt::NoFocus);

    m_anim = new QPropertyAnimation(this, "panelY", this);
    m_anim->setDuration(220);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);

    // Connections
    connect(addBtn,      &QPushButton::clicked,     this, &GraphingWidget::addFunction);
    connect(m_funcInput, &QLineEdit::returnPressed,  this, &GraphingWidget::addFunction);
    connect(resetBtn,    &QPushButton::clicked,      this, &GraphingWidget::resetView);
    connect(exportBtn,   &QPushButton::clicked,      this, &GraphingWidget::exportGraph);
    connect(m_toggleBtn, &QToolButton::clicked,      this, &GraphingWidget::togglePanel);
    connect(m_themeBtn,  &QToolButton::clicked,      this, [this]{
        m_chartDark = !m_chartDark;
        m_themeBtn->setText(m_chartDark ? "☀" : "🌙");
        applyChartTheme();
        plotAll();
    });
    connect(m_radio2D, &QRadioButton::toggled, this, [this](bool on){ if (on) switchDimension(false); });
    connect(m_radio3D, &QRadioButton::toggled, this, [this](bool on){ if (on) switchDimension(true);  });
    connect(m_xMin, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
    connect(m_xMax, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
    connect(m_yMin, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
    connect(m_yMax, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });

    // No default functions — start with a blank canvas
}

// ── Lazy 3D surface init ──────────────────────────────────────────────────────
void GraphingWidget::init3DSurface() {
    if (m_surface) return;  // already initialized

    m_surface = new Q3DSurface();

    auto* inputHandler = new Q3DInputHandler(m_surface);
    inputHandler->setRotationEnabled(true);
    inputHandler->setZoomEnabled(true);
    inputHandler->setSelectionEnabled(true);
    m_surface->setActiveInputHandler(inputHandler);

    m_surface3DContainer = QWidget::createWindowContainer(m_surface, this);
    m_surface3DContainer->setMinimumSize(200, 200);
    m_surface3DContainer->setFocusPolicy(Qt::StrongFocus);
    m_surface3DContainer->setMouseTracking(true);
    m_surface3DContainer->hide();

    m_series3D = new QSurface3DSeries();
    m_series3D->setDrawMode(QSurface3DSeries::DrawSurface);
    m_series3D->setFlatShadingEnabled(false);

    QLinearGradient gradient;
    gradient.setColorAt(0.0,  QColor(0x00, 0x40, 0xff));
    gradient.setColorAt(0.25, QColor(0x00, 0xc8, 0xff));
    gradient.setColorAt(0.5,  QColor(0x00, 0xe0, 0x60));
    gradient.setColorAt(0.75, QColor(0xff, 0xd0, 0x00));
    gradient.setColorAt(1.0,  QColor(0xff, 0x20, 0x00));
    m_series3D->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
    m_series3D->setBaseGradient(gradient);

    m_surface->addSeries(m_series3D);
    m_surface->axisX()->setTitle("X"); m_surface->axisX()->setTitleVisible(true);
    m_surface->axisY()->setTitle("Z"); m_surface->axisY()->setTitleVisible(true);
    m_surface->axisZ()->setTitle("Y"); m_surface->axisZ()->setTitleVisible(true);
}

// ── Chart theme ───────────────────────────────────────────────────────────────
void GraphingWidget::applyChartTheme() {
    if (m_chartDark) {
        m_chart->setTheme(QChart::ChartThemeDark);
        m_chart->setBackgroundBrush(QBrush(QColor(0x0d,0x0d,0x0d)));
        m_chart->setPlotAreaBackgroundBrush(QBrush(QColor(0x13,0x13,0x13)));
        m_chart->setPlotAreaBackgroundVisible(true);
    } else {
        m_chart->setTheme(QChart::ChartThemeLight);
        m_chart->setBackgroundBrush(QBrush(QColor(0xff,0xff,0xff)));
        m_chart->setPlotAreaBackgroundBrush(QBrush(QColor(0xfa,0xfa,0xfa)));
        m_chart->setPlotAreaBackgroundVisible(true);
    }

    // Re-apply black Cartesian axes (theme resets pens)
    QPen axisPen(Qt::black); axisPen.setWidth(2);
    for (auto* s : m_chart->series()) {
        if (s->name() == "__haxis__" || s->name() == "__vaxis__")
            static_cast<QLineSeries*>(s)->setPen(axisPen);
    }

    // Hairline grid
    QPen gridPen(m_chartDark ? QColor(50,50,50) : QColor(200,200,200));
    gridPen.setWidthF(0.5);
    if (m_axisX) m_axisX->setGridLinePen(gridPen);
    if (m_axisY) m_axisY->setGridLinePen(gridPen);

    // Axis label colors
    QColor labelColor = m_chartDark ? QColor(180,180,180) : QColor(60,60,60);
    if (m_axisX) { m_axisX->setLabelsColor(labelColor); m_axisX->setTitleBrush(QBrush(labelColor)); }
    if (m_axisY) { m_axisY->setLabelsColor(labelColor); m_axisY->setTitleBrush(QBrush(labelColor)); }
}

// ── Dimension switch ──────────────────────────────────────────────────────────
void GraphingWidget::switchDimension(bool is3D) {
    m_is3D = is3D;

    if (is3D) {
        init3DSurface();  // lazy init — only creates OpenGL context on first use
        m_chartView->hide();
        if (!m_entries.isEmpty()) {
            int ph = m_panelOpen ? PANEL_HEIGHT_3D : 0;
            m_surface3DContainer->setGeometry(0, 0, width(), height() - ph);
            m_surface3DContainer->show();
            m_surface3DContainer->setFocus();
            plot3D();
        }
    } else {
        if (m_surface3DContainer) m_surface3DContainer->hide();
        m_chartView->show();
        plotAll();
    }

    updateFunctionList();
    m_panel->raise();
    m_toggleBtn->raise();
}

// ── Overlay positioning ───────────────────────────────────────────────────────
int  GraphingWidget::panelY() const   { return m_panel->y(); }
void GraphingWidget::setPanelY(int y) { m_panel->move(0, y); }

void GraphingWidget::repositionOverlays() {
    int w = width(), h = height();
    int ph = PANEL_HEIGHT_3D;
    int panelY = m_panelOpen ? h - ph : h;

    m_panel->setFixedWidth(w);
    m_panel->move(0, panelY);

    int btnW = 120;
    m_toggleBtn->setFixedWidth(btnW);
    m_toggleBtn->move(w - btnW - 8, panelY - 34);
    m_toggleBtn->setText(m_panelOpen ? "⌄  Controls" : "⌃  Controls");

    if (m_is3D && m_surface3DContainer && m_surface3DContainer->isVisible())
        m_surface3DContainer->setGeometry(0, 0, w, panelY);

    m_panel->raise();
    m_toggleBtn->raise();
}

void GraphingWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    repositionOverlays();
}

void GraphingWidget::adjustFor3DOverlap(bool historyOpen, int historyWidth) {
    if (!m_is3D || !m_surface3DContainer || !m_surface3DContainer->isVisible()) return;
    int w = width(), h = height();
    int ph = m_panelOpen ? PANEL_HEIGHT_3D : 0;
    int containerW = historyOpen ? w - historyWidth : w;
    m_surface3DContainer->setGeometry(0, 0, containerW, h - ph);
}

void GraphingWidget::syncToAppTheme(bool dark) {
    m_chartDark = dark;
    m_themeBtn->setText(dark ? "☀" : "🌙");
    applyChartTheme();
    if (!m_is3D) plotAll();
}

void GraphingWidget::togglePanel() {
    m_anim->stop();
    int w = width(), h = height(), ph = PANEL_HEIGHT_3D;
    int openY = h - ph;
    m_panel->setFixedWidth(w);
    if (!m_panelOpen) {
        m_panel->show(); m_panel->raise();
        m_anim->setStartValue(h); m_anim->setEndValue(openY);
        m_panelOpen = true;
        m_toggleBtn->setText("⌄  Controls");
        m_toggleBtn->move(w - 128, openY - 34);
    } else {
        m_anim->setStartValue(openY); m_anim->setEndValue(h);
        m_panelOpen = false;
        m_toggleBtn->setText("⌃  Controls");
        m_toggleBtn->move(w - 128, h - 34);
    }
    m_anim->start();
    if (m_is3D && m_surface3DContainer)
        m_surface3DContainer->setGeometry(0, 0, w, m_panelOpen ? openY : h);
}

// ── Function chips ────────────────────────────────────────────────────────────
// Builds a chip widget (color dot + label + remove button) for one entry.
// parent can be m_chipOverlay (2D) or m_3dChipRow (3D).
static QWidget* makeChip(QWidget* parent, const QString& expr, const QColor& color) {
    auto* chip = new QWidget(parent);
    chip->setStyleSheet("QWidget{background:rgba(255,255,255,0.10);border-radius:4px;}");
    auto* rl = new QHBoxLayout(chip);
    rl->setContentsMargins(6,3,6,3); rl->setSpacing(5);

    auto* dot = new QPushButton(chip);
    dot->setFixedSize(12,12);
    dot->setStyleSheet(QString("QPushButton{background:%1;border-radius:6px;border:none;}").arg(color.name()));
    dot->setFocusPolicy(Qt::NoFocus);

    auto* lbl = new QLabel(expr, chip);
    lbl->setStyleSheet("font-size:11px;background:transparent;");

    auto* rm = new QPushButton("✕", chip);
    rm->setFixedSize(14,14);
    rm->setStyleSheet("QPushButton{background:transparent;color:#888;border:none;font-size:9px;}"
                      "QPushButton:hover{color:#ff5555;}");
    rm->setFocusPolicy(Qt::NoFocus);

    rl->addWidget(dot); rl->addWidget(lbl); rl->addWidget(rm);
    chip->setProperty("dotBtn", QVariant::fromValue(static_cast<QObject*>(dot)));
    chip->setProperty("rmBtn",  QVariant::fromValue(static_cast<QObject*>(rm)));
    return chip;
}

void GraphingWidget::updateFunctionList() {
    // Clear chip row
    while (m_3dChipRowLayout->count() > 0) {
        auto* it = m_3dChipRowLayout->takeAt(0);
        if (it->widget()) it->widget()->deleteLater();
        delete it;
    }

    for (int i = 0; i < m_entries.size(); ++i) {
        auto& entry = m_entries[i];
        auto* chip = makeChip(m_3dChipRow, entry.expression, entry.color);
        m_3dChipRowLayout->addWidget(chip);

        auto* dot = qobject_cast<QPushButton*>(chip->property("dotBtn").value<QObject*>());
        auto* rm  = qobject_cast<QPushButton*>(chip->property("rmBtn").value<QObject*>());

        int idx = i;
        if (rm)  connect(rm,  &QPushButton::clicked, this, [this,idx]{ removeFunction(idx); });
        if (dot) connect(dot, &QPushButton::clicked, this, [this,idx,dot]{
            QColor c = QColorDialog::getColor(m_entries[idx].color, this, "Pick color");
            if (c.isValid()) {
                m_entries[idx].color = c;
                dot->setStyleSheet(QString("QPushButton{background:%1;border-radius:6px;border:none;}").arg(c.name()));
                if (m_entries[idx].series) m_entries[idx].series->setColor(c);
            }
        });
    }

    m_3dChipRowLayout->addStretch();
}

// ── Add / remove ──────────────────────────────────────────────────────────────
void GraphingWidget::addFunction() {
    QString expr = m_funcInput->text().trimmed();
    if (expr.isEmpty()) return;
    PlotEntry e; e.expression = expr; e.color = nextColor(); e.visible = true;
    m_entries.append(e);
    m_funcInput->clear();
    updateFunctionList();
    if (m_is3D) {
        // Show container on first function added in 3D mode
        if (m_surface3DContainer && !m_surface3DContainer->isVisible()) {
            int ph = m_panelOpen ? PANEL_HEIGHT_3D : 0;
            m_surface3DContainer->setGeometry(0, 0, width(), height() - ph);
            m_surface3DContainer->show();
            m_surface3DContainer->setFocus();
            m_panel->raise();
            m_toggleBtn->raise();
        }
        plot3D();
    } else {
        plotAll();
    }
}

void GraphingWidget::removeFunction(int index) {
    if (index < 0 || index >= m_entries.size()) return;
    if (m_entries[index].series) {
        m_chart->removeSeries(m_entries[index].series);
        delete m_entries[index].series;
    }
    m_entries.removeAt(index);
    updateFunctionList();
    if (m_is3D) {
        if (m_entries.isEmpty() && m_surface3DContainer)
            m_surface3DContainer->hide();
        else
            plot3D();
    } else {
        plotAll();
    }
}

// ── 2D plotting ───────────────────────────────────────────────────────────────
void GraphingWidget::plotAll() {
    for (auto& e : m_entries) {
        if (e.series) { m_chart->removeSeries(e.series); delete e.series; e.series = nullptr; }
    }
    for (auto& e : m_entries) plotEntry(e);
    m_axisX->setRange(m_xMin->value(), m_xMax->value());
    m_axisY->setRange(m_yMin->value(), m_yMax->value());
}

void GraphingWidget::plotEntry(PlotEntry& entry) {
    auto* series = new QLineSeries();
    series->setName(entry.expression);
    const auto& palette = m_chartDark ? COLORS_DARK : COLORS_LIGHT;
    // Find index of this entry's color in the palette, or use as-is
    QPen pen(entry.color); pen.setWidth(2); series->setPen(pen);

    static constexpr double PLOT_RANGE = 1e4;
    double step = (PLOT_RANGE * 2.0) / (SAMPLE_POINTS * 10);
    double prevY = std::numeric_limits<double>::quiet_NaN();
    bool hasError = false;

    for (double x = -PLOT_RANGE; x <= PLOT_RANGE; x += step) {
        try {
            double y = m_parser->evaluate(entry.expression, x);
            if (!std::isfinite(y)) { prevY = std::numeric_limits<double>::quiet_NaN(); continue; }
            double yRange = m_axisY->max() - m_axisY->min();
            if (!std::isnan(prevY) && std::abs(y - prevY) > yRange * 10)
                series->append(x, std::numeric_limits<double>::quiet_NaN());
            series->append(x, y);
            prevY = y;
        } catch (...) { hasError = true; prevY = std::numeric_limits<double>::quiet_NaN(); }
    }

    if (hasError) { /* expression has errors — series may be partial */ }
    m_chart->addSeries(series);
    series->attachAxis(m_axisX); series->attachAxis(m_axisY);
    series->setVisible(entry.visible);
    entry.series = series;
}

// ── 3D plotting ───────────────────────────────────────────────────────────────
void GraphingWidget::plot3D() {
    if (m_entries.isEmpty()) {
        m_series3D->dataProxy()->resetArray(new QSurfaceDataArray());
        return;
    }
    QString expr;
    for (auto& e : m_entries) { if (e.visible) { expr = e.expression; break; } }
    if (expr.isEmpty()) return;

    double xMin = m_xMin->value(), xMax = m_xMax->value();
    double yMin = m_yMin->value(), yMax = m_yMax->value();
    int N = SAMPLES_3D;

    // Pass 1: evaluate all Z values into a flat grid
    QVector<double> zGrid(N * N);
    double zMin = std::numeric_limits<double>::max();
    double zMax = std::numeric_limits<double>::lowest();
    double zFiniteSum = 0; int zFiniteCount = 0;

    for (int j = 0; j < N; ++j) {
        double y = yMin + j * (yMax - yMin) / (N - 1);
        for (int i = 0; i < N; ++i) {
            double x = xMin + i * (xMax - xMin) / (N - 1);
            double z = 0;
            try { z = m_parser->evaluate(expr, x, y); } catch (...) {}
            if (std::isfinite(z)) {
                zMin = std::min(zMin, z);
                zMax = std::max(zMax, z);
                zFiniteSum += z;
                ++zFiniteCount;
            } else {
                z = std::numeric_limits<double>::quiet_NaN(); // mark for pass 2
            }
            zGrid[j * N + i] = z;
        }
    }

    // Fallback if everything is non-finite
    double zFallback = zFiniteCount > 0 ? (zFiniteSum / zFiniteCount) : 0.0;
    if (zMin > zMax) { zMin = -1; zMax = 1; }

    // Pass 2: replace NaN with fallback, clamp extreme spikes to ±3× range
    double zRange = zMax - zMin;
    double zClampLo = zMin - zRange * 2;
    double zClampHi = zMax + zRange * 2;
    for (double& z : zGrid) {
        if (!std::isfinite(z)) z = zFallback;
        else z = std::clamp(z, zClampLo, zClampHi);
    }

    // Pass 3: fill QSurfaceDataArray
    auto* dataArray = new QSurfaceDataArray();
    dataArray->reserve(N);
    for (int j = 0; j < N; ++j) {
        double y = yMin + j * (yMax - yMin) / (N - 1);
        auto* row = new QSurfaceDataRow(N);
        for (int i = 0; i < N; ++i) {
            double x = xMin + i * (xMax - xMin) / (N - 1);
            (*row)[i].setPosition(QVector3D(
                static_cast<float>(x),
                static_cast<float>(zGrid[j * N + i]),
                static_cast<float>(y)));
        }
        dataArray->append(row);
    }

    m_series3D->dataProxy()->resetArray(dataArray);
    m_surface->axisX()->setRange(xMin, xMax);
    m_surface->axisZ()->setRange(yMin, yMax);
    m_surface->axisY()->setRange(zMin, zMax);
}

// ── Range / view ──────────────────────────────────────────────────────────────
void GraphingWidget::onRangeChanged() {
    if (m_xMin->value() >= m_xMax->value() || m_yMin->value() >= m_yMax->value()) return;
    m_is3D ? plot3D() : plotAll();
}

void GraphingWidget::resetView() {
    m_xMin->setValue(-10); m_xMax->setValue(10);
    m_yMin->setValue(-10); m_yMax->setValue(10);
    m_is3D ? plot3D() : plotAll();
}

void GraphingWidget::exportGraph() {
    QString path = QFileDialog::getSaveFileName(this, "Export Graph", "graph.png",
        "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (path.isEmpty()) return;
    if (!m_is3D)
        m_chartView->grab().save(path);
    else
        m_surface3DContainer->grab().save(path);
}

QColor GraphingWidget::nextColor() {
    const auto& palette = m_chartDark ? COLORS_DARK : COLORS_LIGHT;
    return palette[m_colorIndex++ % NUM_COLORS];
}
