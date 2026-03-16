#pragma once
#include <QWidget>
#include <QVector>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class QLineEdit;
class QLabel;
class QPushButton;
class QDoubleSpinBox;
class QColorDialog;
class FunctionParser;

struct PlotEntry {
    QString     expression;
    QColor      color;
    bool        visible = true;
    QLineSeries* series = nullptr;
};

// Graphing mode: 2D function plotter using Qt Charts
class GraphingWidget : public QWidget {
    Q_OBJECT
public:
    explicit GraphingWidget(QWidget* parent = nullptr);

private:
    void buildUI();
    void plotAll();
    void plotEntry(PlotEntry& entry);
    void addFunction();
    void removeFunction(int index);
    void updateFunctionList();
    void resetView();
    void exportGraph();
    void onRangeChanged();
    QColor nextColor();

    QChart*     m_chart;
    QChartView* m_chartView;
    QValueAxis* m_axisX;
    QValueAxis* m_axisY;

    QWidget*    m_funcListWidget;
    QVBoxLayout* m_funcListLayout;

    QLineEdit*      m_funcInput;
    QDoubleSpinBox* m_xMin, *m_xMax;
    QDoubleSpinBox* m_yMin, *m_yMax;
    QLabel*         m_statusLabel;

    QVector<PlotEntry> m_entries;
    FunctionParser*    m_parser;

    static constexpr int SAMPLE_POINTS = 1000;
    int m_colorIndex = 0;
};
