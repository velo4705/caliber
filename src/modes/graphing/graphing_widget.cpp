#include "graphing_widget.h"
#include "function_parser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QScrollArea>
#include <QFrame>
#include <QPixmap>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cmath>

// Palette of distinct plot colors
static const QColor PLOT_COLORS[] = {
    {0x42, 0x9e, 0xf5}, // blue
    {0xf5, 0x6c, 0x42}, // orange
    {0x4c, 0xd9, 0x7a}, // green
    {0xf5, 0xd0, 0x42}, // yellow
    {0xc0, 0x42, 0xf5}, // purple
    {0xf5, 0x42, 0x8d}, // pink
    {0x42, 0xf5, 0xe3}, // cyan
    {0xf5, 0x42, 0x42}, // red
};
static constexpr int NUM_COLORS = 8;

static QPushButton* mkBtn(const QString& t, const QString& cls, QWidget* p) {
    auto* b = new QPushButton(t, p);
    b->setProperty("class", cls);
    b->setMinimumHeight(34);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}

// ── constructor ───────────────────────────────────────────────────────────────
GraphingWidget::GraphingWidget(QWidget* parent)
    : QWidget(parent)
    , m_parser(new FunctionParser())
{
    buildUI();
}

// ── UI ────────────────────────────────────────────────────────────────────────
void GraphingWidget::buildUI() {
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── Left panel ────────────────────────────────────────────────────────────
    auto* leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(260);
    leftPanel->setObjectName("modeSidebar");
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(10,12,10,12);
    leftLayout->setSpacing(8);

    // Title
    auto* title = new QLabel("Graphing", leftPanel);
    title->setStyleSheet("font-size:16px; font-weight:bold;");
    leftLayout->addWidget(title);

    // Function input
    auto* inputRow = new QHBoxLayout();
    m_funcInput = new QLineEdit(leftPanel);
    m_funcInput->setPlaceholderText("e.g.  sin(x)  or  x^2-4");
    m_funcInput->setMinimumHeight(34);
    auto* addBtn = mkBtn("+", "actionButton", leftPanel);
    addBtn->setFixedWidth(34);
    inputRow->addWidget(m_funcInput);
    inputRow->addWidget(addBtn);
    leftLayout->addLayout(inputRow);

    // Function list (scrollable)
    auto* scroll = new QScrollArea(leftPanel);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    m_funcListWidget = new QWidget(scroll);
    m_funcListLayout = new QVBoxLayout(m_funcListWidget);
    m_funcListLayout->setContentsMargins(0,0,0,0);
    m_funcListLayout->setSpacing(4);
    m_funcListLayout->addStretch();
    scroll->setWidget(m_funcListWidget);
    leftLayout->addWidget(scroll, 1);

    // X range
    auto* xRangeLabel = new QLabel("X range:", leftPanel);
    leftLayout->addWidget(xRangeLabel);
    auto* xRow = new QHBoxLayout();
    m_xMin = new QDoubleSpinBox(leftPanel); m_xMin->setRange(-1e6,1e6); m_xMin->setValue(-10); m_xMin->setDecimals(2);
    m_xMax = new QDoubleSpinBox(leftPanel); m_xMax->setRange(-1e6,1e6); m_xMax->setValue(10);  m_xMax->setDecimals(2);
    xRow->addWidget(m_xMin); xRow->addWidget(new QLabel("to",leftPanel)); xRow->addWidget(m_xMax);
    leftLayout->addLayout(xRow);

    // Y range
    auto* yRangeLabel = new QLabel("Y range:", leftPanel);
    leftLayout->addWidget(yRangeLabel);
    auto* yRow = new QHBoxLayout();
    m_yMin = new QDoubleSpinBox(leftPanel); m_yMin->setRange(-1e6,1e6); m_yMin->setValue(-10); m_yMin->setDecimals(2);
    m_yMax = new QDoubleSpinBox(leftPanel); m_yMax->setRange(-1e6,1e6); m_yMax->setValue(10);  m_yMax->setDecimals(2);
    yRow->addWidget(m_yMin); yRow->addWidget(new QLabel("to",leftPanel)); yRow->addWidget(m_yMax);
    leftLayout->addLayout(yRow);

    // Action buttons
    auto* resetBtn  = mkBtn("Reset View",    "operatorButton", leftPanel);
    auto* exportBtn = mkBtn("Export Image",  "calcButton",     leftPanel);
    leftLayout->addWidget(resetBtn);
    leftLayout->addWidget(exportBtn);

    m_statusLabel = new QLabel("", leftPanel);
    m_statusLabel->setStyleSheet("color:#f56c42; font-size:11px;");
    m_statusLabel->setWordWrap(true);
    leftLayout->addWidget(m_statusLabel);

    root->addWidget(leftPanel);

    // ── Chart view ────────────────────────────────────────────────────────────
    m_chart = new QChart();
    m_chart->setTitle("");
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setAnimationOptions(QChart::NoAnimation); // performance

    m_axisX = new QValueAxis();
    m_axisX->setRange(-10, 10);
    m_axisX->setTickCount(11);
    m_axisX->setLabelFormat("%.1f");
    m_axisX->setTitleText("x");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY = new QValueAxis();
    m_axisY->setRange(-10, 10);
    m_axisY->setTickCount(11);
    m_axisY->setLabelFormat("%.1f");
    m_axisY->setTitleText("y");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setRubberBand(QChartView::RectangleRubberBand); // zoom by drag
    root->addWidget(m_chartView, 1);

    setLayout(root);

    // Connections
    connect(addBtn,   &QPushButton::clicked, this, &GraphingWidget::addFunction);
    connect(m_funcInput, &QLineEdit::returnPressed, this, &GraphingWidget::addFunction);
    connect(resetBtn, &QPushButton::clicked, this, &GraphingWidget::resetView);
    connect(exportBtn,&QPushButton::clicked, this, &GraphingWidget::exportGraph);
    connect(m_xMin, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
    connect(m_xMax, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
    connect(m_yMin, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });
    connect(m_yMax, &QDoubleSpinBox::valueChanged, this, [this](double){ onRangeChanged(); });

    // Add a default function to start with
    m_funcInput->setText("sin(x)");
    addFunction();
    m_funcInput->setText("x^2/4");
    addFunction();
    m_funcInput->clear();
}

// ── Add / remove functions ────────────────────────────────────────────────────
void GraphingWidget::addFunction() {
    QString expr = m_funcInput->text().trimmed();
    if (expr.isEmpty()) return;

    PlotEntry entry;
    entry.expression = expr;
    entry.color      = nextColor();
    entry.visible    = true;
    entry.series     = nullptr;

    m_entries.append(entry);
    m_funcInput->clear();
    updateFunctionList();
    plotAll();
}

void GraphingWidget::removeFunction(int index) {
    if (index < 0 || index >= m_entries.size()) return;
    if (m_entries[index].series) {
        m_chart->removeSeries(m_entries[index].series);
        delete m_entries[index].series;
    }
    m_entries.removeAt(index);
    updateFunctionList();
    plotAll();
}

void GraphingWidget::updateFunctionList() {
    // Clear existing rows (except the stretch at end)
    while (m_funcListLayout->count() > 1) {
        auto* item = m_funcListLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    for (int i = 0; i < m_entries.size(); ++i) {
        auto& entry = m_entries[i];

        auto* row = new QWidget(m_funcListWidget);
        auto* rl  = new QHBoxLayout(row);
        rl->setContentsMargins(0,0,0,0);
        rl->setSpacing(4);

        // Color swatch button
        auto* colorBtn = new QPushButton(row);
        colorBtn->setFixedSize(20, 20);
        colorBtn->setStyleSheet(QString("background:%1; border-radius:4px; border:none;")
                                    .arg(entry.color.name()));
        colorBtn->setFocusPolicy(Qt::NoFocus);

        // Visibility checkbox
        auto* vis = new QCheckBox(row);
        vis->setChecked(entry.visible);
        vis->setFixedWidth(20);

        // Expression label
        auto* lbl = new QLabel(entry.expression, row);
        lbl->setStyleSheet("font-size:12px;");
        lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        // Remove button
        auto* rmBtn = new QPushButton("✕", row);
        rmBtn->setFixedSize(22, 22);
        rmBtn->setProperty("class", "clearButton");
        rmBtn->setFocusPolicy(Qt::NoFocus);

        rl->addWidget(colorBtn);
        rl->addWidget(vis);
        rl->addWidget(lbl);
        rl->addWidget(rmBtn);

        m_funcListLayout->insertWidget(m_funcListLayout->count()-1, row);

        // Connections
        int idx = i;
        connect(rmBtn, &QPushButton::clicked, this, [this, idx]{ removeFunction(idx); });
        connect(vis, &QCheckBox::toggled, this, [this, idx](bool checked){
            m_entries[idx].visible = checked;
            if (m_entries[idx].series)
                m_entries[idx].series->setVisible(checked);
        });
        connect(colorBtn, &QPushButton::clicked, this, [this, idx, colorBtn]{
            QColor c = QColorDialog::getColor(m_entries[idx].color, this, "Pick color");
            if (c.isValid()) {
                m_entries[idx].color = c;
                colorBtn->setStyleSheet(QString("background:%1; border-radius:4px; border:none;").arg(c.name()));
                if (m_entries[idx].series)
                    m_entries[idx].series->setColor(c);
            }
        });
    }
}

// ── Plotting ──────────────────────────────────────────────────────────────────
void GraphingWidget::plotAll() {
    // Remove all existing series from chart
    for (auto& entry : m_entries) {
        if (entry.series) {
            m_chart->removeSeries(entry.series);
            delete entry.series;
            entry.series = nullptr;
        }
    }

    m_statusLabel->clear();
    for (auto& entry : m_entries)
        plotEntry(entry);

    m_axisX->setRange(m_xMin->value(), m_xMax->value());
    m_axisY->setRange(m_yMin->value(), m_yMax->value());
}

void GraphingWidget::plotEntry(PlotEntry& entry) {
    auto* series = new QLineSeries();
    series->setName(entry.expression);
    series->setColor(entry.color);
    QPen pen(entry.color);
    pen.setWidth(2);
    series->setPen(pen);

    double xMin = m_xMin->value();
    double xMax = m_xMax->value();
    double step = (xMax - xMin) / SAMPLE_POINTS;

    bool hasError = false;
    double prevY  = std::numeric_limits<double>::quiet_NaN();

    for (int i = 0; i <= SAMPLE_POINTS; ++i) {
        double x = xMin + i * step;
        try {
            double y = m_parser->evaluate(entry.expression, x);

            // Skip discontinuities (large jumps like tan asymptotes)
            if (!std::isfinite(y)) { prevY = std::numeric_limits<double>::quiet_NaN(); continue; }
            if (!std::isnan(prevY) && std::abs(y - prevY) > (m_yMax->value() - m_yMin->value()) * 10)
                series->append(x, std::numeric_limits<double>::quiet_NaN()); // break line

            series->append(x, y);
            prevY = y;
        } catch (...) {
            hasError = true;
            prevY = std::numeric_limits<double>::quiet_NaN();
        }
    }

    if (hasError)
        m_statusLabel->setText(m_statusLabel->text() +
            QString("⚠ '%1' has evaluation errors\n").arg(entry.expression));

    m_chart->addSeries(series);
    series->attachAxis(m_axisX);
    series->attachAxis(m_axisY);
    series->setVisible(entry.visible);
    entry.series = series;
}

// ── Range / view ──────────────────────────────────────────────────────────────
void GraphingWidget::onRangeChanged() {
    if (m_xMin->value() >= m_xMax->value() || m_yMin->value() >= m_yMax->value()) return;
    plotAll();
}

void GraphingWidget::resetView() {
    m_xMin->setValue(-10); m_xMax->setValue(10);
    m_yMin->setValue(-10); m_yMax->setValue(10);
    plotAll();
}

// ── Export ────────────────────────────────────────────────────────────────────
void GraphingWidget::exportGraph() {
    QString path = QFileDialog::getSaveFileName(this, "Export Graph",
        "graph.png", "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (path.isEmpty()) return;
    QPixmap px = m_chartView->grab();
    px.save(path);
    m_statusLabel->setText("Saved to " + path);
}

// ── Helpers ───────────────────────────────────────────────────────────────────
QColor GraphingWidget::nextColor() {
    return PLOT_COLORS[m_colorIndex++ % NUM_COLORS];
}
