#pragma once
#include <QWidget>
#include <QVector>
#include <QVBoxLayout>
#include <QPropertyAnimation>
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
#endif

class QLineEdit;
class QLabel;
class QPushButton;
class QToolButton;
class QRadioButton;
class QDoubleSpinBox;
class QComboBox;
class FunctionParser;
class ZoomChartView;

enum class PlotMode { Cartesian = 0, Polar, Parametric };

struct PlotEntry {
    QString      expression;
    QString      yExpression; // for parametric mode: y(t)
    QColor       color;
    PlotMode     plotMode = PlotMode::Cartesian;
    bool         visible = true;
    QLineSeries* series      = nullptr;
    QLineSeries* derivSeries = nullptr; // f'(x) overlay
};

class GraphingWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int panelY READ panelY WRITE setPanelY)
public:
    explicit GraphingWidget(QWidget* parent = nullptr);

    // Called by MainWindow when history drawer opens/closes in 3D mode
    void adjustFor3DOverlap(bool historyOpen, int historyWidth = 240);

    // Called by MainWindow after applying a new app theme
    void syncToAppTheme(bool dark);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void buildUI();
#ifdef HAVE_DATAVISUALIZATION
    void init3DSurface();  // lazy — called on first switch to 3D
#endif

    // 2D
    void plotAll();
    void plotEntry(PlotEntry& entry);
    void plotPolar(PlotEntry& entry);
    void plotParametric(PlotEntry& entry);
    void applyChartTheme();
    void findIntersections();
    void shadeIntegrals();
    void onPlotModeChanged(int index);

#ifdef HAVE_DATAVISUALIZATION
    // 3D
    void plot3D();
    void sync3DTheme();
    void toggleAutoRotate();
#endif

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
#ifdef HAVE_DATAVISUALIZATION
    Q3DSurface*          m_surface   = nullptr;
    QWidget*             m_surface3DContainer = nullptr;
    QSurface3DSeries*    m_series3D  = nullptr;
    QList<QSurface3DSeries*> m_extra3DSeries; // additional 3D surfaces
    QTimer*              m_rotationTimer = nullptr; // animated rotation
    bool                 m_autoRotate = false;
#endif

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
    QLineEdit*      m_funcInputY = nullptr; // for parametric y(t)
    QComboBox*      m_plotModeCombo = nullptr;
    QLabel*         m_paramLabel = nullptr; // "y(t) =" label for parametric
    QDoubleSpinBox* m_xMin, *m_xMax;
    QDoubleSpinBox* m_yMin, *m_yMax;
    QLabel*         m_statusLabel = nullptr;  // unused, kept for ABI compat
    QRadioButton*   m_radio2D;
    QRadioButton*   m_radio3D;
    QToolButton*    m_themeBtn;   // light/dark chart toggle
    QToolButton*    m_derivBtn;   // f'(x) overlay toggle
    QToolButton*    m_intersectBtn; // intersection finder toggle
    QToolButton*    m_shadeBtn;     // integral shading toggle
    QToolButton*    m_rotateBtn = nullptr; // 3D auto-rotate toggle
    QLineEdit*      m_shadeA;       // shade from x=a
    QLineEdit*      m_shadeB;       // shade to x=b
    QList<QAreaSeries*> m_shadeSeries;

    QVector<PlotEntry> m_entries;
    FunctionParser*    m_parser;

    bool m_is3D = false;
    bool m_showDeriv = false;
    bool m_showIntersect = false;
    QList<QScatterSeries*> m_intersectMarkers;

    static constexpr int SAMPLE_POINTS  = 1000;
    static constexpr int SAMPLES_3D     = 100;
    static constexpr int PANEL_HEIGHT   = 90;
    static constexpr int PANEL_HEIGHT_3D = 130;
    int m_colorIndex = 0;
};
