#pragma once
#include <QWidget>
#include <QVector>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DInputHandler>

class QLineEdit;
class QLabel;
class QPushButton;
class QToolButton;
class QRadioButton;
class QDoubleSpinBox;
class FunctionParser;
class ZoomChartView;

struct PlotEntry {
    QString      expression;
    QColor       color;
    bool         visible = true;
    QLineSeries* series  = nullptr;
};

class GraphingWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int panelY READ panelY WRITE setPanelY)
public:
    explicit GraphingWidget(QWidget* parent = nullptr);

    // Called by MainWindow when history drawer opens/closes in 3D mode
    void adjustFor3DOverlap(bool historyOpen, int historyWidth = 240);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void buildUI();

    // 2D
    void plotAll();
    void plotEntry(PlotEntry& entry);
    void applyChartTheme();

    // 3D
    void plot3D();

    // shared
    void addFunction();
    void removeFunction(int index);
    void updateFunctionList();
    void resetView();
    void exportGraph();
    void onRangeChanged();
    void togglePanel();
    void repositionOverlays();
    void switchDimension(bool is3D);
    QColor nextColor();

    int  panelY() const;
    void setPanelY(int y);

    // ── 2D ───────────────────────────────────────────────────────────────────
    QChart*        m_chart;
    ZoomChartView* m_chartView;
    QValueAxis*    m_axisX;
    QValueAxis*    m_axisY;
    bool           m_chartDark = true;

    // ── 3D ───────────────────────────────────────────────────────────────────
    Q3DSurface*          m_surface   = nullptr;
    QWidget*             m_surface3DContainer = nullptr;
    QSurface3DSeries*    m_series3D  = nullptr;

    // ── Stacked view (2D / 3D) ────────────────────────────────────────────────
    // (no stacked widget — 3D container is a manual overlay)

    // ── Chip overlay (top-left) — REMOVED, chips now live in panel chip row ──

    // ── Bottom panel ──────────────────────────────────────────────────────────
    QWidget*            m_panel;
    QWidget*            m_3dChipRow = nullptr;   // extra row shown only in 3D
    QHBoxLayout*        m_3dChipRowLayout = nullptr;
    QToolButton*        m_toggleBtn;
    QPropertyAnimation* m_anim;
    bool                m_panelOpen = true;

    QLineEdit*      m_funcInput;
    QDoubleSpinBox* m_xMin, *m_xMax;
    QDoubleSpinBox* m_yMin, *m_yMax;
    QLabel*         m_statusLabel = nullptr;  // unused, kept for ABI compat
    QRadioButton*   m_radio2D;
    QRadioButton*   m_radio3D;
    QToolButton*    m_themeBtn;   // light/dark chart toggle

    QVector<PlotEntry> m_entries;
    FunctionParser*    m_parser;

    bool m_is3D = false;

    static constexpr int SAMPLE_POINTS  = 1000;
    static constexpr int SAMPLES_3D     = 100;
    static constexpr int PANEL_HEIGHT   = 90;
    static constexpr int PANEL_HEIGHT_3D = 130;
    int m_colorIndex = 0;
};
