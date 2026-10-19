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
#include <QComboBox>
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
#include <QTimer>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#ifdef HAVE_DATAVISUALIZATION
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DInputHandler>
#include <QtDataVisualization/Q3DTheme>
#endif
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
        setFocusPolicy(Qt::StrongFocus);

        // Trace tooltip label
        m_tooltip = new QLabel(this);
        m_tooltip->setObjectName("traceTooltip");
        m_tooltip->setStyleSheet(
            "QLabel { background: rgba(30,30,30,220); color: #fff;"
            " border: 1px solid #555; border-radius: 4px;"
            " padding: 3px 7px; font-size: 12px; font-family: monospace; }");
        m_tooltip->hide();
        m_tooltip->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    // Trace marker accessor — plotAll() may delete it, so ensureTraceMarker()
    // recreates it on demand.
    QScatterSeries* ensureTraceMarker() {
        if (!m_traceMarker) {
            m_traceMarker = new QScatterSeries();
            m_traceMarker->setName("__trace__");
            m_traceMarker->setMarkerSize(12);
            m_traceMarker->setBorderColor(Qt::white);
            m_traceMarker->setBrush(Qt::white);
            chart()->addSeries(m_traceMarker);
            // Attach to same axes as first visible series (or defaults)
            auto axes = chart()->axes();
            for (auto* a : axes) {
                if (auto* va = qobject_cast<QValueAxis*>(a))
                    m_traceMarker->attachAxis(va);
            }
        }
        return m_traceMarker;
    }

    void hideTrace() {
        if (m_traceMarker) m_traceMarker->setVisible(false);
        m_tooltip->hide();
        m_traceSeriesIndex = -1;
    }

protected:
    void wheelEvent(QWheelEvent* e) override {
        double f = e->angleDelta().y() > 0 ? 0.85 : 1.0 / 0.85;
        chart()->zoom(f);
        m_tooltip->hide();
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
            chart()->scroll(-delta.x(), delta.y());
            m_lastPan = e->pos();
            m_tooltip->hide();
            e->accept();
        } else {
            QChartView::mouseMoveEvent(e);
            updateMouseTrace(e->pos());
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

    void leaveEvent(QEvent* e) override {
        m_tooltip->hide();
        QChartView::leaveEvent(e);
    }

    void keyPressEvent(QKeyEvent* e) override {
        const Qt::Key key = static_cast<Qt::Key>(e->key());
        const Qt::KeyboardModifiers mod = e->modifiers();
        const QList<QLineSeries*> slist = visibleSeries();

        // ── Shift + Arrow → Pan ───────────────────────────────────────────
        if (mod & Qt::ShiftModifier) {
            switch (key) {
            case Qt::Key_Left:  chart()->scroll( 30, 0); e->accept(); return;
            case Qt::Key_Right: chart()->scroll(-30, 0); e->accept(); return;
            case Qt::Key_Up:    chart()->scroll(0,  30); e->accept(); return;
            case Qt::Key_Down:  chart()->scroll(0, -30); e->accept(); return;
            default: break;
            }
        }

        switch (key) {
        // ── Left / Right → Trace along curve ──────────────────────────────
        case Qt::Key_Left:
        case Qt::Key_Right: {
            if (slist.isEmpty()) { e->accept(); return; }

            // Initialize trace on first press
            if (m_traceSeriesIndex < 0 || m_traceSeriesIndex >= slist.size())
                m_traceSeriesIndex = 0;

            // Step = 1/50 of visible x range
            auto xAxes = chart()->axes(Qt::Horizontal);
            double step = 1.0;
            if (!xAxes.isEmpty()) {
                if (auto* xa = qobject_cast<QValueAxis*>(xAxes.first()))
                    step = (xa->max() - xa->min()) / 50.0;
            }

            if (key == Qt::Key_Left) m_traceX -= step;
            else                     m_traceX += step;

            // Clamp to visible range
            if (!xAxes.isEmpty()) {
                if (auto* xa = qobject_cast<QValueAxis*>(xAxes.first()))
                    m_traceX = qBound(xa->min(), m_traceX, xa->max());
            }

            updateKeyboardTrace();
            e->accept();
            return;
        }

        // ── Up / Down → Pan vertically ────────────────────────────────────
        case Qt::Key_Up:    chart()->scroll(0,  30); e->accept(); return;
        case Qt::Key_Down:  chart()->scroll(0, -30); e->accept(); return;

        // ── Tab / Shift+Tab → Cycle curves ────────────────────────────────
        case Qt::Key_Tab: {
            if (slist.isEmpty()) { e->accept(); return; }
            // Initialize trace position on first tab
            if (m_traceSeriesIndex < 0 || m_traceSeriesIndex >= slist.size()) {
                m_traceSeriesIndex = 0;
                m_traceX = centerVisibleX();
            }
            m_traceSeriesIndex = (m_traceSeriesIndex + 1) % slist.size();
            updateKeyboardTrace();
            e->accept();
            return;
        }
        case Qt::Key_Backtab: {
            if (slist.isEmpty()) { e->accept(); return; }
            if (m_traceSeriesIndex < 0 || m_traceSeriesIndex >= slist.size()) {
                m_traceSeriesIndex = 0;
                m_traceX = centerVisibleX();
            }
            m_traceSeriesIndex = (m_traceSeriesIndex - 1 + slist.size()) % slist.size();
            updateKeyboardTrace();
            e->accept();
            return;
        }

        // ── + / - → Zoom ──────────────────────────────────────────────────
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            chart()->zoom(0.85);
            m_tooltip->hide();
            e->accept();
            return;
        case Qt::Key_Minus:
            chart()->zoom(1.0 / 0.85);
            m_tooltip->hide();
            e->accept();
            return;

        // ── Escape → Clear trace ──────────────────────────────────────────
        case Qt::Key_Escape:
            hideTrace();
            e->accept();
            return;

        default:
            QChartView::keyPressEvent(e);
        }
    }

private:
    // ── Helpers ────────────────────────────────────────────────────────────
    QList<QLineSeries*> visibleSeries() const {
        QList<QLineSeries*> out;
        for (auto* s : chart()->series()) {
            auto* ls = qobject_cast<QLineSeries*>(s);
            if (ls && ls->isVisible() && !ls->name().startsWith("__"))
                out.append(ls);
        }
        return out;
    }

    double centerVisibleX() const {
        auto xAxes = chart()->axes(Qt::Horizontal);
        if (!xAxes.isEmpty()) {
            if (auto* xa = qobject_cast<QValueAxis*>(xAxes.first()))
                return (xa->min() + xa->max()) / 2.0;
        }
        return 0.0;
    }

    int findNearestPointIndex(QLineSeries* series, double x) const {
        const auto& pts = series->points();
        if (pts.isEmpty()) return -1;
        int best = 0;
        double bestDist = std::abs(pts[0].x() - x);
        for (int i = 1; i < pts.size(); ++i) {
            double d = std::abs(pts[i].x() - x);
            if (d < bestDist) { bestDist = d; best = i; }
        }
        return best;
    }

    void updateKeyboardTrace() {
        const QList<QLineSeries*> slist = visibleSeries();
        if (slist.isEmpty() || m_traceSeriesIndex < 0 || m_traceSeriesIndex >= slist.size()) {
            hideTrace();
            return;
        }

        QLineSeries* series = slist[m_traceSeriesIndex];
        const int idx = findNearestPointIndex(series, m_traceX);
        if (idx < 0) { hideTrace(); return; }

        const QPointF pt = series->points()[idx];

        // Update marker
        auto* marker = ensureTraceMarker();
        marker->clear();
        marker->append(pt.x(), pt.y());
        marker->setColor(series->pen().color());
        marker->setBorderColor(series->pen().color());
        marker->setBrush(series->pen().color());
        marker->setVisible(true);

        // Update tooltip
        const QPointF px = chart()->mapToPosition(pt);
        m_tooltip->setText(QString("x = %1\ny = %2\nCurve: %3")
            .arg(pt.x(), 0, 'g', 6).arg(pt.y(), 0, 'g', 6)
            .arg(series->name()));
        m_tooltip->adjustSize();

        int tx = int(px.x()) + 14;
        int ty = int(px.y()) - m_tooltip->height() - 4;
        if (tx + m_tooltip->width() > width())  tx = int(px.x()) - m_tooltip->width() - 8;
        if (ty < 0) ty = int(px.y()) + 14;
        m_tooltip->move(tx, ty);
        m_tooltip->show();
        m_tooltip->raise();
    }

    // ── Mouse trace (hover) ───────────────────────────────────────────────
    void updateMouseTrace(const QPoint& pos) {
        const QPointF chartPt = chart()->mapToValue(pos);
        const double cx = chartPt.x();

        double bestDist = 1e300, bestX = 0, bestY = 0;
        bool found = false;

        for (auto* s : chart()->series()) {
            auto* ls = qobject_cast<QLineSeries*>(s);
            if (!ls || !ls->isVisible() || ls->name().startsWith("__")) continue;
            const auto& pts = ls->points();
            if (pts.isEmpty()) continue;
            for (const QPointF& p : pts) {
                double dist = std::abs(p.x() - cx);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestX = p.x(); bestY = p.y();
                    found = true;
                }
            }
        }

        if (!found) { m_tooltip->hide(); return; }

        const QPointF snappedPx = chart()->mapToPosition(QPointF(bestX, bestY));
        if (std::hypot(snappedPx.x() - pos.x(), snappedPx.y() - pos.y()) > 40) {
            m_tooltip->hide(); return;
        }

        m_tooltip->setText(QString("x = %1\ny = %2")
            .arg(bestX, 0, 'g', 6).arg(bestY, 0, 'g', 6));
        m_tooltip->adjustSize();

        int tx = pos.x() + 14;
        int ty = pos.y() - m_tooltip->height() - 4;
        if (tx + m_tooltip->width() > width())  tx = pos.x() - m_tooltip->width() - 8;
        if (ty < 0) ty = pos.y() + 14;
        m_tooltip->move(tx, ty);
        m_tooltip->show();
        m_tooltip->raise();
    }

    bool              m_panning = false;
    QPoint            m_lastPan;
    QLabel*           m_tooltip;
    int               m_traceSeriesIndex = -1;
    double            m_traceX = 0;
    QScatterSeries*   m_traceMarker = nullptr;
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
    const int H = 32;
    const int SIDEBAR_W = 200;

    // ── Root: center (chart+panel) + sidebar ───────────────────────────────
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── Center: chart (70%) + bottom panel (30%) ──────────────────────────
    auto* centerContainer = new QWidget();
    auto* centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setContentsMargins(0,0,0,0);
    centerLayout->setSpacing(0);

    // 2D chart
    m_chart = new QChart();
    m_chart->legend()->hide();
    m_chart->setAnimationOptions(QChart::NoAnimation);
    m_chart->setMargins(QMargins(4,4,4,4));

    m_axisX = new QValueAxis();
    m_axisX->setRange(-10,10); m_axisX->setTickCount(11);
    m_axisX->setLabelFormat("%.4g"); m_axisX->setTitleText("x");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY = new QValueAxis();
    m_axisY->setRange(-10,10); m_axisY->setTickCount(11);
    m_axisY->setLabelFormat("%.4g"); m_axisY->setTitleText("y");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    // Cartesian axes at origin
    QPen axisPen(QColor(180,180,180)); axisPen.setWidth(1);
    auto* hAxis = new QLineSeries(); hAxis->setName("__haxis__"); hAxis->setPen(axisPen);
    hAxis->append(-1e9,0); hAxis->append(1e9,0);
    m_chart->addSeries(hAxis); hAxis->attachAxis(m_axisX); hAxis->attachAxis(m_axisY);

    auto* vAxis = new QLineSeries(); vAxis->setName("__vaxis__"); vAxis->setPen(axisPen);
    vAxis->append(0,-1e9); vAxis->append(0,1e9);
    m_chart->addSeries(vAxis); vAxis->attachAxis(m_axisX); vAxis->attachAxis(m_axisY);

    QPen gridPen(QColor(180,180,180)); gridPen.setWidthF(0.5);
    m_axisX->setGridLinePen(gridPen);
    m_axisY->setGridLinePen(gridPen);

    m_chartView = new ZoomChartView(m_chart, centerContainer);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setRubberBand(QChartView::NoRubberBand);

    m_stackedWidget = new QStackedWidget(centerContainer);
    m_stackedWidget->addWidget(m_chartView);  // page 0 = 2D chart
    centerLayout->addWidget(m_stackedWidget, 7);

    // ── Right sidebar ──────────────────────────────────────────────────────
    auto* sidebar = new QWidget();
    sidebar->setFixedWidth(SIDEBAR_W);
    sidebar->setObjectName("graphSidebar");
    auto* sbLayout = new QVBoxLayout(sidebar);
    sbLayout->setContentsMargins(8, 8, 8, 8);
    sbLayout->setSpacing(8);

    // Divider helper
    auto mkDiv = [sidebar]() -> QFrame* {
        auto* d = new QFrame(sidebar);
        d->setFrameShape(QFrame::HLine); d->setFrameShadow(QFrame::Sunken);
        return d;
    };

    // X range
    sbLayout->addWidget(new QLabel("X Range:", sidebar));
    auto* xRow = new QHBoxLayout(); xRow->setSpacing(4);
    m_xMin = new QDoubleSpinBox(sidebar); m_xMin->setRange(-1e6,1e6); m_xMin->setValue(-10); m_xMin->setDecimals(1); m_xMin->setFixedWidth(68); m_xMin->setFixedHeight(H);
    m_xMax = new QDoubleSpinBox(sidebar); m_xMax->setRange(-1e6,1e6); m_xMax->setValue(10);  m_xMax->setDecimals(1); m_xMax->setFixedWidth(68); m_xMax->setFixedHeight(H);
    xRow->addWidget(m_xMin);
    auto* xArrow = new QLabel("\xe2\x86\x92"); xArrow->setFixedWidth(16); xArrow->setAlignment(Qt::AlignCenter);
    xRow->addWidget(xArrow);
    xRow->addWidget(m_xMax);
    sbLayout->addLayout(xRow);

    // Y range
    sbLayout->addWidget(new QLabel("Y Range:", sidebar));
    auto* yRow = new QHBoxLayout(); yRow->setSpacing(4);
    m_yMin = new QDoubleSpinBox(sidebar); m_yMin->setRange(-1e6,1e6); m_yMin->setValue(-10); m_yMin->setDecimals(1); m_yMin->setFixedWidth(68); m_yMin->setFixedHeight(H);
    m_yMax = new QDoubleSpinBox(sidebar); m_yMax->setRange(-1e6,1e6); m_yMax->setValue(10);  m_yMax->setDecimals(1); m_yMax->setFixedWidth(68); m_yMax->setFixedHeight(H);
    yRow->addWidget(m_yMin);
    auto* yArrow = new QLabel("\xe2\x86\x92"); yArrow->setFixedWidth(16); yArrow->setAlignment(Qt::AlignCenter);
    yRow->addWidget(yArrow);
    yRow->addWidget(m_yMax);
    sbLayout->addLayout(yRow);

    // Z range (3D only)
#ifdef HAVE_DATAVISUALIZATION
    sbLayout->addWidget(new QLabel("Z Range:", sidebar));
    auto* zRow = new QHBoxLayout(); zRow->setSpacing(4);
    m_zMin = new QDoubleSpinBox(sidebar); m_zMin->setRange(-1e6,1e6); m_zMin->setValue(-10); m_zMin->setDecimals(1); m_zMin->setFixedWidth(68); m_zMin->setFixedHeight(H);
    m_zMax = new QDoubleSpinBox(sidebar); m_zMax->setRange(-1e6,1e6); m_zMax->setValue(10);  m_zMax->setDecimals(1); m_zMax->setFixedWidth(68); m_zMax->setFixedHeight(H);
    zRow->addWidget(m_zMin);
    auto* zArrow = new QLabel("\xe2\x86\x92"); zArrow->setFixedWidth(16); zArrow->setAlignment(Qt::AlignCenter);
    zRow->addWidget(zArrow);
    zRow->addWidget(m_zMax);
    sbLayout->addLayout(zRow);
#endif

    sbLayout->addWidget(mkDiv());

    // Toggle buttons — full width, stacked vertically
    auto mkFullToggle = [sidebar, sbLayout](const QString& text, const QString& tip, bool checkable) -> QToolButton* {
        auto* btn = new QToolButton(sidebar);
        btn->setText(text); btn->setToolTip(tip);
        btn->setCheckable(checkable);
        btn->setObjectName("graphToggleBtn");
        btn->setFixedHeight(H);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sbLayout->addWidget(btn);
        return btn;
    };

    m_themeBtn     = mkFullToggle("\xe2\x98\x80  Theme", "Toggle chart light/dark", true);
    m_derivBtn     = mkFullToggle("f'  Derivative", "Toggle f'(x) derivative", true);
    m_intersectBtn = mkFullToggle("\xe2\x88\xa9  Intersect",  "Find intersections", true);
    m_shadeBtn     = mkFullToggle("\xe2\x88\xab  Shade Area",  "Shade area under curve", true);

    // Shade range inputs — full width, side by side
    sbLayout->addWidget(new QLabel("Shade Range:", sidebar));
    auto* shadeRow = new QHBoxLayout(); shadeRow->setSpacing(4);
    m_shadeA = new QLineEdit(sidebar); m_shadeA->setPlaceholderText("a"); m_shadeA->setAlignment(Qt::AlignCenter); m_shadeA->setStyleSheet("font-size:12px;"); m_shadeA->setFixedHeight(H);
    m_shadeB = new QLineEdit(sidebar); m_shadeB->setPlaceholderText("b"); m_shadeB->setAlignment(Qt::AlignCenter); m_shadeB->setStyleSheet("font-size:12px;"); m_shadeB->setFixedHeight(H);
    shadeRow->addWidget(m_shadeA);
    shadeRow->addWidget(m_shadeB);
    sbLayout->addLayout(shadeRow);

    sbLayout->addWidget(mkDiv());

    // Reset + Export
    auto* resetBtn  = mkBtn("Reset",  "operatorButton", sidebar); resetBtn->setFixedHeight(H);
    auto* exportBtn = mkBtn("Export", "calcButton",     sidebar); exportBtn->setFixedHeight(H);
    sbLayout->addWidget(resetBtn);
    sbLayout->addWidget(exportBtn);
    sbLayout->addSpacing(16);

    // ── Bottom panel (full width of center): chips + input ──────────────────
    m_panel = new QWidget();
    m_panel->setObjectName("graphBottomPanel");
    m_panel->setAttribute(Qt::WA_StyledBackground, true);
    m_panel->setFixedHeight(PANEL_HEIGHT_3D);

    auto* panelVL = new QVBoxLayout(m_panel);
    panelVL->setContentsMargins(0,0,0,0);
    panelVL->setSpacing(0);

    // Chip row
    auto* chipContainer = new QWidget();
    chipContainer->setObjectName("graphChipContainer");
    chipContainer->setStyleSheet("background:transparent;");
    m_3dChipRowLayout = new QHBoxLayout(chipContainer);
    m_3dChipRowLayout->setContentsMargins(12,4,12,4);
    m_3dChipRowLayout->setSpacing(6);
    m_3dChipRowLayout->addStretch();

    auto* chipScroll = new QScrollArea(m_panel);
    chipScroll->setWidget(chipContainer);
    chipScroll->setWidgetResizable(true);
    chipScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    chipScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chipScroll->setFixedHeight(30);
    chipScroll->setFrameShape(QFrame::NoFrame);
    chipScroll->setStyleSheet("background:transparent;");
    panelVL->addWidget(chipScroll);

    // Input row
    auto* inputRow = new QWidget(m_panel);
    inputRow->setFixedHeight(PANEL_HEIGHT);
    panelVL->addWidget(inputRow);

    auto* il = new QHBoxLayout(inputRow);
    il->setContentsMargins(12,0,12,0);
    il->setSpacing(10);

    // 2D / 3D radio buttons
    m_radio2D = new QRadioButton("2D", inputRow);
    m_radio2D->setChecked(true);
    il->addWidget(m_radio2D, 0, Qt::AlignVCenter);
#ifdef HAVE_DATAVISUALIZATION
    m_radio3D = new QRadioButton("3D", inputRow);
    auto* dimGroup = new QButtonGroup(inputRow);
    dimGroup->addButton(m_radio2D); dimGroup->addButton(m_radio3D);
    il->addWidget(m_radio3D, 0, Qt::AlignVCenter);
#endif

    // Divider
    auto* ilDiv = new QFrame(inputRow);
    ilDiv->setFrameShape(QFrame::VLine); ilDiv->setFrameShadow(QFrame::Sunken);
    il->addWidget(ilDiv);

    m_plotModeCombo = new QComboBox(inputRow);
    m_plotModeCombo->addItems({"Auto", "Cartesian", "Polar", "Parametric", "Implicit"});
    m_plotModeCombo->setFixedHeight(H);
    m_plotModeCombo->setToolTip("Plot mode (Auto detects from expression)");
    il->addWidget(m_plotModeCombo, 0, Qt::AlignVCenter);

    m_funcInput = new QLineEdit(inputRow);
    m_funcInput->setPlaceholderText("e.g.  sin(x)  or  x^2 - 4");
    m_funcInput->setFixedHeight(H);
    m_funcInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_paramLabel = new QLabel("y(t) =", inputRow);
    m_paramLabel->hide();
    m_funcInputY = new QLineEdit(inputRow);
    m_funcInputY->setPlaceholderText("e.g.  cos(t)");
    m_funcInputY->setFixedHeight(H);
    m_funcInputY->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_funcInputY->hide();

    auto* addBtn = mkBtn("+", "actionButton", inputRow);
    addBtn->setFixedSize(H, H);
    addBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    il->addWidget(m_funcInput, 4, Qt::AlignVCenter);
    il->addWidget(m_paramLabel, 0, Qt::AlignVCenter);
    il->addWidget(m_funcInputY, 3, Qt::AlignVCenter);
    il->addWidget(addBtn,      0, Qt::AlignVCenter);

    centerLayout->addWidget(m_panel, 3);

    // Add sidebar to root (right side, full height)
    root->addWidget(centerContainer, 1);
    root->addWidget(sidebar);
    setLayout(root);

    // 3D surface is lazy-initialized on first switch to 3D mode
    // (avoids OpenGL context creation at startup — important for VMs / no-GPU)

    // Apply chart theme now that all panel widgets exist
    applyChartTheme();

    // Connections
    connect(addBtn,      &QPushButton::clicked,     this, &GraphingWidget::addFunction);
    connect(m_funcInput, &QLineEdit::returnPressed,  this, &GraphingWidget::addFunction);
    connect(resetBtn,    &QPushButton::clicked,      this, &GraphingWidget::resetView);
    connect(exportBtn,   &QPushButton::clicked,      this, &GraphingWidget::exportGraph);
    connect(m_themeBtn,  &QToolButton::toggled,       this, [this](bool on){
        m_chartDark = !on;
        m_themeBtn->setText(m_chartDark ? "\xe2\x98\x80  Theme" : "\xf0\x9f\x8c\x99  Theme");
        applyChartTheme();
        plotAll();
    });
    connect(m_derivBtn, &QToolButton::toggled, this, [this](bool on){
        m_showDeriv = on;
        plotAll();
    });
    connect(m_intersectBtn, &QToolButton::toggled, this, [this](bool on){
        m_showIntersect = on;
        if (on) findIntersections();
        else {
            for (auto* s : m_intersectMarkers) { m_chart->removeSeries(s); delete s; }
            m_intersectMarkers.clear();
        }
    });

    auto clearShade = [this]{
        for (auto* s : m_shadeSeries) { m_chart->removeSeries(s); delete s; }
        m_shadeSeries.clear();
    };
    connect(m_shadeBtn, &QToolButton::toggled, this, [this, clearShade](bool on){
        if (on) shadeIntegrals(); else clearShade();
    });
    connect(m_shadeA, &QLineEdit::returnPressed, this, [this]{ if(m_shadeBtn->isChecked()) shadeIntegrals(); });
    connect(m_shadeB, &QLineEdit::returnPressed, this, [this]{ if(m_shadeBtn->isChecked()) shadeIntegrals(); });
    connect(m_radio2D, &QRadioButton::toggled, this, [this](bool on){ if (on) switchDimension(false); });
#ifdef HAVE_DATAVISUALIZATION
    connect(m_radio3D, &QRadioButton::toggled, this, [this](bool on){ if (on) switchDimension(true);  });
#endif
    connect(m_plotModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GraphingWidget::onPlotModeChanged);
#ifdef HAVE_DATAVISUALIZATION
    if (m_rotateBtn)
        connect(m_rotateBtn, &QToolButton::toggled, this, &GraphingWidget::toggleAutoRotate);
#endif
    connect(m_xMin, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
    connect(m_xMax, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
    connect(m_yMin, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
    connect(m_yMax, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
#ifdef HAVE_DATAVISUALIZATION
    connect(m_zMin, &QDoubleSpinBox::valueChanged, this, [this](double){ if(m_is3D) plot3D(); });
    connect(m_zMax, &QDoubleSpinBox::valueChanged, this, [this](double){ if(m_is3D) plot3D(); });
#endif

    // No default functions — start with a blank canvas
}

// ── Lazy 3D surface init ──────────────────────────────────────────────────────
#ifdef HAVE_DATAVISUALIZATION
void GraphingWidget::init3DSurface() {
    if (m_surface) return;  // already initialized

    m_surface = new Q3DSurface();

    auto* inputHandler = new Q3DInputHandler(m_surface);
    inputHandler->setRotationEnabled(true);
    inputHandler->setZoomEnabled(true);
    inputHandler->setSelectionEnabled(true);
    m_surface->setActiveInputHandler(inputHandler);

    m_surface3DContainer = QWidget::createWindowContainer(m_surface, m_stackedWidget);
    m_surface3DContainer->setMinimumSize(200, 200);
    m_surface3DContainer->setFocusPolicy(Qt::StrongFocus);
    m_surface3DContainer->setMouseTracking(true);
    m_stackedWidget->addWidget(m_surface3DContainer);  // page 1 = 3D surface

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
#endif

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

    // Re-apply Cartesian axes (theme resets pens)
    QPen axisPen(m_chartDark ? QColor(160,160,160) : QColor(80,80,80));
    axisPen.setWidth(1);
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
#ifdef HAVE_DATAVISUALIZATION
        init3DSurface();  // lazy init — only creates OpenGL context on first use
        sync3DTheme();
        m_stackedWidget->setCurrentIndex(1);  // switch to 3D page
        if (m_rotateBtn) m_rotateBtn->show();
        if (!m_entries.isEmpty()) {
            plot3D();
        }
#endif
    } else {
#ifdef HAVE_DATAVISUALIZATION
        m_stackedWidget->setCurrentIndex(0);  // switch to 2D chart page
#endif
#ifdef HAVE_DATAVISUALIZATION
        if (m_rotateBtn) m_rotateBtn->hide();
        if (m_autoRotate) { m_autoRotate = false; if(m_rotateBtn) m_rotateBtn->setChecked(false); if(m_rotationTimer) m_rotationTimer->stop(); }
#endif
        plotAll();
    }

    updateFunctionList();
}

// ── Overlay positioning ───────────────────────────────────────────────────────
int  GraphingWidget::panelY() const   { return m_panel->y(); }
void GraphingWidget::setPanelY(int y) { m_panel->move(0, y); }

void GraphingWidget::repositionOverlays() {
}

void GraphingWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
}

#ifdef HAVE_DATAVISUALIZATION
void GraphingWidget::adjustFor3DOverlap(bool historyOpen, int historyWidth) {
    // 3D container is in a stacked widget — layout handles sizing automatically.
    // This method is kept for ABI compat but is now a no-op.
    Q_UNUSED(historyOpen); Q_UNUSED(historyWidth);
}
#endif

void GraphingWidget::syncToAppTheme(bool dark) {
    m_chartDark = dark;
    m_themeBtn->setText(dark ? "\xe2\x98\x80  Theme" : "\xf0\x9f\x8c\x99  Theme");
    applyChartTheme();
#ifdef HAVE_DATAVISUALIZATION
    if (m_surface) sync3DTheme();
#endif
    if (!m_is3D) plotAll();
}

void GraphingWidget::togglePanel() {
    // No-op — panel is always visible in layout
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

    auto* chipParent = qobject_cast<QWidget*>(m_3dChipRowLayout->parent());
    for (int i = 0; i < m_entries.size(); ++i) {
        auto& entry = m_entries[i];
        QString chipText;
        QColor chipColor = entry.color;

        if (entry.isParamDef) {
            // Show parameter definition with distinct style
            for (auto it = m_params.constBegin(); it != m_params.constEnd(); ++it) {
                if (entry.expression.contains(it.key() + " =")) {
                    chipText = it.key() + " \xe2\x88\x88 [" +
                               QString::number(it.value().first(), 'g', 6) + "... " +
                               QString::number(it.value().last(), 'g', 6) + "]";
                    break;
                }
            }
            if (chipText.isEmpty()) chipText = entry.expression;
            chipColor = QColor(100, 149, 237); // cornflower blue for params
        } else {
            chipText = entry.expression;
            if (entry.plotMode == PlotMode::Implicit)
                chipText = entry.expression + " = " + entry.implicitRHS;
        }

        auto* chip = makeChip(chipParent, chipText, chipColor);
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

    // Check if this is a parameter definition (e.g. "a = [1...5]" or "a = 1,2,3")
    if (parseParamDef(expr)) {
        m_funcInput->clear();
        updateFunctionList();
        plotAll(); // replot with new parameter values
        return;
    }

    int comboIdx = m_plotModeCombo->currentIndex();
    PlotMode mode;
    if (comboIdx == 0) {
        mode = detectPlotMode(expr);
    } else {
        mode = static_cast<PlotMode>(comboIdx - 1);
    }

    PlotEntry e;
    e.color = nextColor();
    e.visible = true;
    e.plotMode = mode;

    if (mode == PlotMode::Implicit) {
        // Split on '=' to get LHS and RHS
        int eqPos = expr.indexOf('=');
        if (eqPos < 0) {
            // No '=' found — treat as "expr = 0"
            e.expression = expr;
            e.implicitRHS = "0";
        } else {
            e.expression = expr.left(eqPos).trimmed();
            e.implicitRHS = expr.mid(eqPos + 1).trimmed();
        }
    } else if (mode == PlotMode::Parametric) {
        e.expression = expr;
        e.yExpression = m_funcInputY->text().trimmed();
        if (e.yExpression.isEmpty()) return;
        m_funcInputY->clear();
    } else {
        e.expression = expr;
    }

    m_entries.append(e);
    m_funcInput->clear();
    updateFunctionList();
#ifdef HAVE_DATAVISUALIZATION
    if (m_is3D) {
        if (m_surface3DContainer && !m_surface3DContainer->isVisible()) {
            int ph = PANEL_HEIGHT_3D;
            m_surface3DContainer->setGeometry(0, 0, width(), height() - ph);
            m_surface3DContainer->show();
            m_surface3DContainer->setFocus();
            m_panel->raise();
        }
        plot3D();
    } else {
#endif
        plotAll();
#ifdef HAVE_DATAVISUALIZATION
    }
#endif
}

void GraphingWidget::removeFunction(int index) {
    if (index < 0 || index >= m_entries.size()) return;
    if (m_entries[index].series) {
        m_chart->removeSeries(m_entries[index].series);
        delete m_entries[index].series;
    }
    if (m_entries[index].derivSeries) {
        m_chart->removeSeries(m_entries[index].derivSeries);
        delete m_entries[index].derivSeries;
    }
    for (auto* s : m_entries[index].implicitChains) { m_chart->removeSeries(s); delete s; }
    m_entries.removeAt(index);
    updateFunctionList();
#ifdef HAVE_DATAVISUALIZATION
    if (m_is3D) {
        if (m_entries.isEmpty() && m_surface3DContainer)
            m_surface3DContainer->hide();
        else
            plot3D();
    } else {
#endif
        plotAll();
#ifdef HAVE_DATAVISUALIZATION
    }
#endif
}

// ── 2D plotting ───────────────────────────────────────────────────────────────
static QString substituteParams(const QString& expr, const QMap<QString, double>& vals) {
    QString result = expr;
    for (auto it = vals.constBegin(); it != vals.constEnd(); ++it) {
        QRegularExpression re(QString(R"((?<![a-zA-Z0-9_])%1(?![a-zA-Z0-9_]))").arg(it.key()));
        result.replace(re, QString::number(it.value(), 'g', 15));
    }
    return result;
}

void GraphingWidget::plotAll() {
    for (auto& e : m_entries) {
        if (e.series) { m_chart->removeSeries(e.series); delete e.series; e.series = nullptr; }
        if (e.derivSeries) { m_chart->removeSeries(e.derivSeries); delete e.derivSeries; e.derivSeries = nullptr; }
        for (auto* s : e.implicitChains) { m_chart->removeSeries(s); delete s; }
        e.implicitChains.clear();
    }
    // Clear intersection markers
    for (auto* s : m_intersectMarkers) { m_chart->removeSeries(s); delete s; }
    m_intersectMarkers.clear();
    // Clear shade series
    for (auto* s : m_shadeSeries) { m_chart->removeSeries(s); delete s; }
    m_shadeSeries.clear();

    for (auto& e : m_entries) {
        if (e.isParamDef) continue;

        // Check if this entry uses any parameters
        QStringList params = findParamsIn(e.expression);
        if (!e.yExpression.isEmpty()) params += findParamsIn(e.yExpression);
        if (!e.implicitRHS.isEmpty()) params += findParamsIn(e.implicitRHS);
        params.removeDuplicates();

        if (params.isEmpty()) {
            // No parameters — plot normally
            switch (e.plotMode) {
            case PlotMode::Cartesian:   plotEntry(e);      break;
            case PlotMode::Polar:       plotPolar(e);      break;
            case PlotMode::Parametric:  plotParametric(e); break;
            case PlotMode::Implicit:    plotImplicit(e);   break;
            }
        } else {
            // Has parameters — generate all combinations and plot each
            // Build list of value vectors for each parameter
            QList<QString> paramNames;
            QList<QVector<double>> paramValues;
            for (auto& p : params) {
                paramNames.append(p);
                paramValues.append(m_params.value(p));
            }

            // Iterate over all combinations (cartesian product)
            QVector<int> indices(paramNames.size(), 0);
            bool done = false;
            while (!done) {
                // Build substitution map
                QMap<QString, double> subst;
                for (int k = 0; k < paramNames.size(); ++k)
                    subst[paramNames[k]] = paramValues[k][indices[k]];

                // Create temporary entry with substituted expressions
                PlotEntry tmp = e;
                tmp.expression = substituteParams(e.expression, subst);
                if (!e.yExpression.isEmpty())
                    tmp.yExpression = substituteParams(e.yExpression, subst);
                if (!e.implicitRHS.isEmpty())
                    tmp.implicitRHS = substituteParams(e.implicitRHS, subst);

                // Build label showing parameter values
                QStringList parts;
                for (int k = 0; k < paramNames.size(); ++k)
                    parts.append(QString("%1=%2").arg(paramNames[k]).arg(paramValues[k][indices[k]], 0, 'g', 6));
                tmp.expression = parts.join(", ") + ": " + tmp.expression;

                // Plot the temporary entry
                switch (tmp.plotMode) {
                case PlotMode::Cartesian:   plotEntry(tmp);      break;
                case PlotMode::Polar:       plotPolar(tmp);      break;
                case PlotMode::Parametric:  plotParametric(tmp); break;
                case PlotMode::Implicit:    plotImplicit(tmp);   break;
                }

                // Move plotted series to the real entry
                if (tmp.series) { e.implicitChains.append(tmp.series); tmp.series = nullptr; }
                for (auto* s : tmp.implicitChains) { e.implicitChains.append(s); }

                // Advance to next combination
                int k = 0;
                while (k < paramNames.size()) {
                    indices[k]++;
                    if (indices[k] < paramValues[k].size()) break;
                    indices[k] = 0;
                    k++;
                }
                done = (k >= paramNames.size());
            }
        }
    }
    m_axisX->setRange(m_xMin->value(), m_xMax->value());
    m_axisY->setRange(m_yMin->value(), m_yMax->value());

    if (m_showIntersect) findIntersections();
    if (m_shadeBtn && m_shadeBtn->isChecked()) shadeIntegrals();
}

void GraphingWidget::plotEntry(PlotEntry& entry) {
    auto* series = new QLineSeries();
    series->setName(entry.expression);
    const auto& palette = m_chartDark ? COLORS_DARK : COLORS_LIGHT;
    QPen pen(entry.color); pen.setWidth(2); series->setPen(pen);

    // Sample wide range so graph extends endlessly when panning
    static constexpr double PLOT_BOUND = 1000.0;
    static constexpr double STEP = 0.1;
    double prevY = std::numeric_limits<double>::quiet_NaN();
    bool hasError = false;

    for (double x = -PLOT_BOUND; x <= PLOT_BOUND; x += STEP) {
        try {
            double y = m_parser->evaluate(entry.expression, x);
            if (!std::isfinite(y)) { prevY = std::numeric_limits<double>::quiet_NaN(); continue; }
            if (!std::isnan(prevY) && std::abs(y - prevY) > 1e6)
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

    // Derivative overlay — dashed line of same color but lighter
    if (m_showDeriv) {
        auto* dSeries = new QLineSeries();
        dSeries->setName("__deriv__" + entry.expression);
        QPen dPen(entry.color.lighter(150));
        dPen.setWidth(1);
        dPen.setStyle(Qt::DashLine);
        dSeries->setPen(dPen);
        const double h = 1e-5;
        double prevDy = std::numeric_limits<double>::quiet_NaN();
        for (double x = -PLOT_BOUND; x <= PLOT_BOUND; x += STEP) {
            try {
                double dy = (m_parser->evaluate(entry.expression, x+h)
                           - m_parser->evaluate(entry.expression, x-h)) / (2*h);
                if (!std::isfinite(dy)) { prevDy = std::numeric_limits<double>::quiet_NaN(); continue; }
                if (!std::isnan(prevDy) && std::abs(dy - prevDy) > 1e6)
                    dSeries->append(x, std::numeric_limits<double>::quiet_NaN());
                dSeries->append(x, dy);
                prevDy = dy;
            } catch (...) { prevDy = std::numeric_limits<double>::quiet_NaN(); }
        }
        m_chart->addSeries(dSeries);
        dSeries->attachAxis(m_axisX); dSeries->attachAxis(m_axisY);
        dSeries->setVisible(entry.visible);
        entry.derivSeries = dSeries;
    }
}

// ── Intersection Finder ───────────────────────────────────────────────────────
void GraphingWidget::findIntersections() {
    // Clear old markers
    for (auto* s : m_intersectMarkers) { m_chart->removeSeries(s); delete s; }
    m_intersectMarkers.clear();

    if (m_entries.size() < 2) return;

    double xMin = m_xMin->value(), xMax = m_xMax->value();
    static constexpr int STEPS = 2000;
    double step = (xMax - xMin) / STEPS;

    // Compare every pair of visible functions
    for (int i = 0; i < m_entries.size(); i++) {
        for (int j = i+1; j < m_entries.size(); j++) {
            if (!m_entries[i].visible || !m_entries[j].visible) continue;

            double prevDiff = std::numeric_limits<double>::quiet_NaN();
            double prevX = xMin;

            for (int k = 0; k <= STEPS; k++) {
                double x = xMin + k * step;
                double fi, fj;
                try { fi = m_parser->evaluate(m_entries[i].expression, x); } catch(...) { prevDiff = std::numeric_limits<double>::quiet_NaN(); continue; }
                try { fj = m_parser->evaluate(m_entries[j].expression, x); } catch(...) { prevDiff = std::numeric_limits<double>::quiet_NaN(); continue; }
                if (!std::isfinite(fi) || !std::isfinite(fj)) { prevDiff = std::numeric_limits<double>::quiet_NaN(); continue; }

                double diff = fi - fj;

                // Sign change → intersection between prevX and x
                if (!std::isnan(prevDiff) && prevDiff * diff < 0) {
                    // Bisect to find precise crossing
                    double lo = prevX, hi = x;
                    for (int b = 0; b < 30; b++) {
                        double mid = (lo + hi) / 2;
                        double fmid;
                        try {
                            fmid = m_parser->evaluate(m_entries[i].expression, mid)
                                 - m_parser->evaluate(m_entries[j].expression, mid);
                        } catch(...) { break; }
                        if (prevDiff * fmid < 0) hi = mid; else lo = mid;
                    }
                    double ix = (lo + hi) / 2;
                    double iy;
                    try { iy = m_parser->evaluate(m_entries[i].expression, ix); } catch(...) { prevDiff = diff; prevX = x; continue; }

                    // Add scatter marker
                    auto* marker = new QScatterSeries();
                    marker->setName(QString("∩(%1,%2)").arg(ix,0,'g',4).arg(iy,0,'g',4));
                    marker->setMarkerShape(QScatterSeries::MarkerShapeCircle);
                    marker->setMarkerSize(10);
                    marker->setColor(QColor(0xff, 0xd7, 0x00)); // yellow
                    marker->setBorderColor(QColor(0xff, 0x88, 0x00));
                    marker->append(ix, iy);
                    m_chart->addSeries(marker);
                    marker->attachAxis(m_axisX);
                    marker->attachAxis(m_axisY);
                    m_intersectMarkers.append(marker);
                }
                prevDiff = diff;
                prevX = x;
            }
        }
    }
}

// ── Integral Shading ──────────────────────────────────────────────────────────
void GraphingWidget::shadeIntegrals() {
    for (auto* s : m_shadeSeries) { m_chart->removeSeries(s); delete s; }
    m_shadeSeries.clear();

    if (m_entries.isEmpty()) return;
    bool okA, okB;
    double a = m_shadeA->text().toDouble(&okA);
    double b = m_shadeB->text().toDouble(&okB);
    if (!okA || !okB || a >= b) return;

    static constexpr int SHADE_STEPS = 500;
    double step = (b - a) / SHADE_STEPS;

    for (const PlotEntry& entry : m_entries) {
        if (!entry.visible) continue;

        auto* upper = new QLineSeries();
        auto* lower = new QLineSeries();

        for (int i = 0; i <= SHADE_STEPS; i++) {
            double x = a + i * step;
            double y = 0;
            try { y = m_parser->evaluate(entry.expression, x); } catch(...) { y = 0; }
            if (!std::isfinite(y)) y = 0;
            upper->append(x, y);
            lower->append(x, 0);
        }

        auto* area = new QAreaSeries(upper, lower);
        QColor shadeColor = entry.color;
        shadeColor.setAlpha(60);
        QPen pen(entry.color); pen.setWidth(1);
        area->setPen(pen);
        area->setBrush(shadeColor);
        area->setName(QString("∫%1 [%2,%3]").arg(entry.expression).arg(a,0,'g',4).arg(b,0,'g',4));

        m_chart->addSeries(area);
        area->attachAxis(m_axisX);
        area->attachAxis(m_axisY);
        m_shadeSeries.append(area);
    }
}

// ── Plot mode change ───────────────────────────────────────────────────────────
void GraphingWidget::onPlotModeChanged(int index) {
    // index 0=Auto, 1=Cartesian, 2=Polar, 3=Parametric, 4=Implicit
    bool isParametric = (index == 3);
    bool isImplicit = (index == 4);
    m_paramLabel->setVisible(isParametric);
    m_funcInputY->setVisible(isParametric);

    switch (index) {
    case 0: // Auto
        m_funcInput->setPlaceholderText("e.g.  sin(x),  r=sin(3*theta),  (cos(t),sin(t)),  x^2+y^2=1");
        break;
    case 1: // Cartesian
        m_funcInput->setPlaceholderText("e.g.  sin(x)  or  x^2 - 4");
        break;
    case 2: // Polar
        m_funcInput->setPlaceholderText("e.g.  2*sin(3*theta)  or  1 + cos(theta)");
        break;
    case 3: // Parametric
        m_funcInput->setPlaceholderText("x(t) e.g.  cos(t)");
        m_funcInputY->setPlaceholderText("y(t) e.g.  sin(t)");
        break;
    case 4: // Implicit
        m_funcInput->setPlaceholderText("e.g.  x^2 + y^2 = 1  or  (x^2+y^2-1)^3 = x^2*y^3");
        break;
    }
}

// ── Polar plotting ─────────────────────────────────────────────────────────────
void GraphingWidget::plotPolar(PlotEntry& entry) {
    auto* series = new QLineSeries();
    series->setName(entry.expression);
    QPen pen(entry.color); pen.setWidth(2); series->setPen(pen);

    static constexpr int POLAR_STEPS = 2000;
    double thetaMax = 4 * M_PI; // two full rotations
    double step = thetaMax / POLAR_STEPS;
    double prevX = std::numeric_limits<double>::quiet_NaN();

    for (double theta = 0; theta <= thetaMax; theta += step) {
        try {
            double r = m_parser->evaluate(entry.expression, theta);
            if (!std::isfinite(r)) { prevX = std::numeric_limits<double>::quiet_NaN(); continue; }
            double px = r * std::cos(theta);
            double py = r * std::sin(theta);
            // Break line at large jumps (e.g. r goes through 0)
            if (!std::isnan(prevX) && std::abs(px - prevX) > 50)
                series->append(px, std::numeric_limits<double>::quiet_NaN());
            series->append(px, py);
            prevX = px;
        } catch (...) { prevX = std::numeric_limits<double>::quiet_NaN(); }
    }

    m_chart->addSeries(series);
    series->attachAxis(m_axisX); series->attachAxis(m_axisY);
    series->setVisible(entry.visible);
    entry.series = series;
}

// ── Parametric plotting ────────────────────────────────────────────────────────
void GraphingWidget::plotParametric(PlotEntry& entry) {
    if (entry.yExpression.isEmpty()) return;
    auto* series = new QLineSeries();
    series->setName(QString("%1, %2").arg(entry.expression).arg(entry.yExpression));
    QPen pen(entry.color); pen.setWidth(2); series->setPen(pen);

    static constexpr int PARAM_STEPS = 2000;
    double tMin = -10, tMax = 10;
    double step = (tMax - tMin) / PARAM_STEPS;
    double prevX = std::numeric_limits<double>::quiet_NaN();

    for (double t = tMin; t <= tMax; t += step) {
        try {
            double px = m_parser->evaluate(entry.expression, t);
            double py = m_parser->evaluate(entry.yExpression, t);
            if (!std::isfinite(px) || !std::isfinite(py)) {
                prevX = std::numeric_limits<double>::quiet_NaN();
                continue;
            }
            if (!std::isnan(prevX) && std::abs(px - prevX) > 50)
                series->append(px, std::numeric_limits<double>::quiet_NaN());
            series->append(px, py);
            prevX = px;
        } catch (...) { prevX = std::numeric_limits<double>::quiet_NaN(); }
    }

    m_chart->addSeries(series);
    series->attachAxis(m_axisX); series->attachAxis(m_axisY);
    series->setVisible(entry.visible);
    entry.series = series;
}

// ── Implicit plotting (marching squares with contour tracing) ─────────────────
void GraphingWidget::plotImplicit(PlotEntry& entry) {
    QString diffExpr = "(" + entry.expression + ")-(" + entry.implicitRHS + ")";

    const double xLo = m_xMin->value();
    const double xHi = m_xMax->value();
    const double yLo = m_yMin->value();
    const double yHi = m_yMax->value();

    static constexpr int GRID = 500;
    const double dx = (xHi - xLo) / GRID;
    const double dy = (yHi - yLo) / GRID;

    // Evaluate f(x,y) on the grid
    QVector<QVector<double>> grid(GRID + 1, QVector<double>(GRID + 1));
    for (int i = 0; i <= GRID; ++i) {
        double x = xLo + i * dx;
        for (int j = 0; j <= GRID; ++j) {
            double y = yLo + j * dy;
            try { grid[i][j] = m_parser->evaluate(diffExpr, x, y); }
            catch (...) { grid[i][j] = std::numeric_limits<double>::quiet_NaN(); }
        }
    }

    auto lerp = [](double v1, double v2, double a, double b) -> double {
        if (std::abs(v1 - v2) < 1e-30) return (a + b) / 2.0;
        double t = v1 / (v1 - v2);
        return a + t * (b - a);
    };

    // Edge-based marching squares
    // Horizontal edge hEdge[i][j]: bottom edge of cell (i,j), from (i,j) to (i+1,j)
    //   exists for i in [0,GRID), j in [0,GRID]
    // Vertical edge vEdge[i][j]: right edge of cell (i,j), from (i+1,j) to (i+1,j+1)
    //   exists for i in [0,GRID], j in [0,GRID)
    // An edge crossing is stored as an index into a point array (-1 = no crossing)

    QVector<QVector<int>> hEdge(GRID, QVector<int>(GRID + 1, -1));
    QVector<QVector<int>> vEdge(GRID + 1, QVector<int>(GRID, -1));
    QVector<QPointF> pts; // crossing points, indexed by edge crossing index

    auto addCrossing = [&](int i, int j, bool horizontal) -> int {
        double x0, y0, v1, v2;
        if (horizontal) {
            // Bottom edge of cell (i,j): from (xLo+i*dx, yLo+j*dy) to (xLo+(i+1)*dx, yLo+j*dy)
            x0 = xLo + i * dx; y0 = yLo + j * dy;
            v1 = grid[i][j]; v2 = grid[i + 1][j];
        } else {
            // Vertical edge at x-position i: from (xLo+i*dx, yLo+j*dy) to (xLo+i*dx, yLo+(j+1)*dy)
            x0 = xLo + i * dx; y0 = yLo + j * dy;
            v1 = grid[i][j]; v2 = grid[i][j + 1];
        }
        if (!std::isfinite(v1) || !std::isfinite(v2) || v1 * v2 >= 0) return -1;
        int idx = pts.size();
        if (horizontal)
            pts.append(QPointF(lerp(v1, v2, x0, x0 + dx), y0));
        else
            pts.append(QPointF(x0, lerp(v1, v2, y0, y0 + dy)));
        return idx;
    };

    // Compute all edge crossings
    for (int i = 0; i < GRID; ++i)
        for (int j = 0; j <= GRID; ++j)
            hEdge[i][j] = addCrossing(i, j, true);
    for (int i = 0; i <= GRID; ++i)
        for (int j = 0; j < GRID; ++j)
            vEdge[i][j] = addCrossing(i, j, false);

    // Build segments: each segment connects two edge crossing indices
    struct Seg { int a, b; };
    QList<Seg> segs;

    for (int i = 0; i < GRID; ++i) {
        for (int j = 0; j < GRID; ++j) {
            double v00 = grid[i][j], v10 = grid[i+1][j];
            double v11 = grid[i+1][j+1], v01 = grid[i][j+1];
            if (!std::isfinite(v00) || !std::isfinite(v10) ||
                !std::isfinite(v11) || !std::isfinite(v01)) continue;

            int c = 0;
            if (v00 > 0) c |= 1;  if (v10 > 0) c |= 2;
            if (v11 > 0) c |= 4;  if (v01 > 0) c |= 8;
            if (c == 0 || c == 15) continue;

            // Edge indices: B=hEdge[i][j], R=vEdge[i+1][j], T=hEdge[i][j+1], L=vEdge[i][j]
            int B = hEdge[i][j], R = vEdge[i+1][j], T = hEdge[i][j+1], L = vEdge[i][j];

            switch (c) {
            case 1: case 14: if (B >= 0 && L >= 0) segs.append({B, L}); break;
            case 2: case 13: if (B >= 0 && R >= 0) segs.append({B, R}); break;
            case 3: case 12: if (L >= 0 && R >= 0) segs.append({L, R}); break;
            case 4: case 11: if (T >= 0 && R >= 0) segs.append({T, R}); break;
            case 6: case 9:  if (B >= 0 && T >= 0) segs.append({B, T}); break;
            case 7: case 8:  if (L >= 0 && T >= 0) segs.append({L, T}); break;
            case 5: case 10: { // saddle
                double ctr = (v00 + v10 + v11 + v01) / 4.0;
                if ((c == 5) == (ctr > 0)) {
                    if (B >= 0 && R >= 0) segs.append({B, R});
                    if (L >= 0 && T >= 0) segs.append({L, T});
                } else {
                    if (B >= 0 && L >= 0) segs.append({B, L});
                    if (R >= 0 && T >= 0) segs.append({R, T});
                }
                break;
            }
            default: break;
            }
        }
    }

    // Trace chains using edge-crossing indices as node IDs
    // Build adjacency: for each crossing index, which other crossing indices is it connected to?
    QMap<int, QList<int>> adj;
    for (auto& s : segs) {
        adj[s.a].append(s.b);
        adj[s.b].append(s.a);
    }

    QSet<int> visited;
    QList<QList<int>> chains;

    for (auto it = adj.constBegin(); it != adj.constEnd(); ++it) {
        if (visited.contains(it.key())) continue;

        int start = it.key();
        visited.insert(start);
        QList<int> chain;
        chain.append(start);

        // Extend forward
        bool ext = true;
        while (ext) {
            ext = false;
            int back = chain.last();
            for (int nb : adj.value(back)) {
                if (visited.contains(nb)) continue;
                visited.insert(nb);
                chain.append(nb);
                ext = true;
                break;
            }
        }
        // Extend backward
        ext = true;
        while (ext) {
            ext = false;
            int front = chain.first();
            for (int nb : adj.value(front)) {
                if (visited.contains(nb)) continue;
                visited.insert(nb);
                chain.prepend(nb);
                ext = true;
                break;
            }
        }

        if (chain.size() >= 2)
            chains.append(chain);
    }

    // Build one QLineSeries per chain (no NaN needed)
    QPen pen(entry.color); pen.setWidth(2);

    for (auto& chain : chains) {
        auto* s = new QLineSeries();
        s->setName(entry.expression + " = " + entry.implicitRHS);
        s->setPen(pen);
        for (int idx : chain)
            s->append(pts[idx].x(), pts[idx].y());
        m_chart->addSeries(s);
        s->attachAxis(m_axisX); s->attachAxis(m_axisY);
        s->setVisible(entry.visible);
        entry.implicitChains.append(s);
    }
}

// ── 3D plotting ───────────────────────────────────────────────────────────────
#ifdef HAVE_DATAVISUALIZATION
void GraphingWidget::plot3D() {
    if (!m_surface) return;

    // Collect visible Cartesian entries (only Cartesian mode works in 3D)
    QStringList exprs;
    QList<QColor> colors;
    for (auto& e : m_entries) {
        if (e.visible && e.plotMode == PlotMode::Cartesian) {
            exprs << e.expression;
            colors << e.color;
        }
    }

    if (exprs.isEmpty()) {
        m_series3D->dataProxy()->resetArray(new QSurfaceDataArray());
        for (auto* s : m_extra3DSeries) { m_surface->removeSeries(s); delete s; }
        m_extra3DSeries.clear();
        return;
    }

    double xMin = m_xMin->value(), xMax = m_xMax->value();
    double yMin = m_yMin->value(), yMax = m_yMax->value();
    int N = SAMPLES_3D;

    // Helper lambda to evaluate a surface grid
    auto evalSurface = [&](const QString& expr) -> QSurfaceDataArray* {
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
                    z = std::numeric_limits<double>::quiet_NaN();
                }
                zGrid[j * N + i] = z;
            }
        }

        double zFallback = zFiniteCount > 0 ? (zFiniteSum / zFiniteCount) : 0.0;
        if (zMin > zMax) { zMin = -1; zMax = 1; }
        double zRange = zMax - zMin;
        double zClampLo = zMin - zRange * 2;
        double zClampHi = zMax + zRange * 2;
        for (double& z : zGrid) {
            if (!std::isfinite(z)) z = zFallback;
            else z = std::clamp(z, zClampLo, zClampHi);
        }

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
        return dataArray;
    };

    // First surface goes into m_series3D
    m_series3D->dataProxy()->resetArray(evalSurface(exprs[0]));
    m_series3D->setName(exprs[0]);

    // Additional surfaces
    // Remove old extra series
    for (auto* s : m_extra3DSeries) { m_surface->removeSeries(s); delete s; }
    m_extra3DSeries.clear();

    QLinearGradient gradient;
    gradient.setColorAt(0.0,  QColor(0x00, 0x40, 0xff));
    gradient.setColorAt(0.25, QColor(0x00, 0xc8, 0xff));
    gradient.setColorAt(0.5,  QColor(0x00, 0xe0, 0x60));
    gradient.setColorAt(0.75, QColor(0xff, 0xd0, 0x00));
    gradient.setColorAt(1.0,  QColor(0xff, 0x20, 0x00));

    for (int si = 1; si < exprs.size(); ++si) {
        auto* extraSeries = new QSurface3DSeries();
        extraSeries->setDrawMode(QSurface3DSeries::DrawSurface);
        extraSeries->setFlatShadingEnabled(false);
        extraSeries->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
        extraSeries->setBaseGradient(gradient);
        extraSeries->dataProxy()->resetArray(evalSurface(exprs[si]));
        extraSeries->setName(exprs[si]);
        // Offset slightly in Y so surfaces don't z-fight
        m_surface->addSeries(extraSeries);
        m_extra3DSeries.append(extraSeries);
    }

    m_surface->axisX()->setRange(xMin, xMax);
    m_surface->axisZ()->setRange(yMin, yMax);
}
#endif

// ── Range / view ──────────────────────────────────────────────────────────────
void GraphingWidget::onRangeChanged() {
    if (m_xMin->value() >= m_xMax->value() || m_yMin->value() >= m_yMax->value()) return;
#ifdef HAVE_DATAVISUALIZATION
    m_is3D ? plot3D() : plotAll();
#else
    plotAll();
#endif
}

void GraphingWidget::resetView() {
    m_xMin->setValue(-10); m_xMax->setValue(10);
    m_yMin->setValue(-10); m_yMax->setValue(10);
#ifdef HAVE_DATAVISUALIZATION
    m_is3D ? plot3D() : plotAll();
#else
    plotAll();
#endif
}

void GraphingWidget::exportGraph() {
    QString path = QFileDialog::getSaveFileName(this, "Export Graph", "graph.png",
        "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (path.isEmpty()) return;
    if (!m_is3D)
        m_chartView->grab().save(path);
#ifdef HAVE_DATAVISUALIZATION
    else
        m_surface3DContainer->grab().save(path);
#endif
}

QColor GraphingWidget::nextColor() {
    const auto& palette = m_chartDark ? COLORS_DARK : COLORS_LIGHT;
    return palette[m_colorIndex++ % NUM_COLORS];
}

PlotMode GraphingWidget::detectPlotMode(const QString& expr) const {
    QString e = expr.trimmed().toLower();

    // Implicit: contains 'y' and '=' (e.g. "x^2 + y^2 = 1")
    if (e.contains('y') && e.contains('=')) return PlotMode::Implicit;

    // Parametric: starts with '(' and has two comma-separated expressions with 't'
    if (e.startsWith('(')) {
        // Check for pattern (expr, expr)
        int depth = 0;
        for (int i = 0; i < e.size(); ++i) {
            if (e[i] == '(') depth++;
            else if (e[i] == ')') depth--;
            else if (e[i] == ',' && depth == 1) {
                // Found top-level comma — parametric
                return PlotMode::Parametric;
            }
        }
    }

    // Polar: contains 'theta' or 'r' as standalone variable
    if (e.contains("theta")) return PlotMode::Polar;

    // Default: Cartesian
    return PlotMode::Cartesian;
}

bool GraphingWidget::parseParamDef(const QString& expr) {
    // Match: name = [min...max]  or  name = min,max,min,max,...
    static QRegularExpression reRange(R"(^\s*([a-zA-Z_]\w*)\s*=\s*\[([^\]]+)\]\s*$)");
    static QRegularExpression reList(R"(^\s*([a-zA-Z_]\w*)\s*=\s*([0-9eE.,+\- ]+)\s*$)");

    auto match = reRange.match(expr);
    if (match.hasMatch()) {
        QString name = match.captured(1);
        QString range = match.captured(2);
        // Parse min...max or min...max...step
        QStringList parts = range.split("...");
        if (parts.size() >= 2) {
            double lo = parts[0].trimmed().toDouble();
            double hi = parts[1].trimmed().toDouble();
            double step = (parts.size() >= 3) ? parts[2].trimmed().toDouble() : 0;
            QVector<double> values;
            if (step > 0) {
                for (double v = lo; v <= hi + 1e-12; v += step)
                    values.append(v);
            } else {
                // Auto-step: ~20 values
                int n = qMax(2, qMin(50, static_cast<int>((hi - lo) / 0.1 + 1)));
                for (int i = 0; i < n; ++i)
                    values.append(lo + (hi - lo) * i / (n - 1));
            }
            m_params[name] = values;
            return true;
        }
    }

    match = reList.match(expr);
    if (match.hasMatch()) {
        QString name = match.captured(1);
        QString list = match.captured(2);
        QVector<double> values;
        for (auto& s : list.split(',')) {
            bool ok;
            double v = s.trimmed().toDouble(&ok);
            if (ok) values.append(v);
        }
        if (!values.isEmpty()) {
            m_params[name] = values;
            return true;
        }
    }

    return false;
}

QStringList GraphingWidget::findParamsIn(const QString& expr) const {
    QStringList found;
    for (auto it = m_params.constBegin(); it != m_params.constEnd(); ++it) {
        // Check if parameter name appears as a standalone word
        QRegularExpression re(QString(R"((?<![a-zA-Z0-9_])%1(?![a-zA-Z0-9_]))").arg(it.key()));
        if (re.match(expr).hasMatch())
            found.append(it.key());
    }
    return found;
}

// ── 3D theme sync ─────────────────────────────────────────────────────────────
#ifdef HAVE_DATAVISUALIZATION
void GraphingWidget::sync3DTheme() {
    if (!m_surface) return;
    auto* theme = m_surface->activeTheme();
    if (!theme) return;

    if (m_chartDark) {
        theme->setBackgroundColor(QColor(0x0d, 0x0d, 0x0d));
        theme->setWindowColor(QColor(0x1a, 0x1a, 0x1a));
        theme->setGridEnabled(true);
        theme->setBackgroundEnabled(true);
    } else {
        theme->setBackgroundColor(QColor(0xff, 0xff, 0xff));
        theme->setWindowColor(QColor(0xf0, 0xf0, 0xf0));
        theme->setGridEnabled(true);
        theme->setBackgroundEnabled(true);
    }
}

// ── 3D auto-rotation ──────────────────────────────────────────────────────────
void GraphingWidget::toggleAutoRotate() {
    if (!m_rotateBtn) return;
    m_autoRotate = m_rotateBtn->isChecked();
    if (m_autoRotate) {
        if (!m_rotationTimer) {
            m_rotationTimer = new QTimer(this);
            connect(m_rotationTimer, &QTimer::timeout, this, [this]{
                if (!m_surface) return;
                auto* handler = m_surface->activeInputHandler();
                if (!handler) return;
                // Rotate camera around Y axis
                float rotY = m_surface->scene()->activeCamera()->yRotation();
                m_surface->scene()->activeCamera()->setYRotation(rotY + 1.0f);
            });
        }
        m_rotationTimer->start(30); // ~33 fps
    } else {
        if (m_rotationTimer) m_rotationTimer->stop();
    }
}
#endif
