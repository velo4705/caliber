#pragma once
#include <QWidget>
#include <QString>

class DisplayWidget;
class MathEngine;
class HistoryManager;
class HistoryPanel;

enum class AngleMode { Degrees, Radians, Gradians };

// Scientific calculator mode: trig, log, power, constants, angle modes
class ScientificWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScientificWidget(MathEngine* engine,
                              HistoryManager* history,
                              HistoryPanel* historyPanel,
                              QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onButtonClicked(const QString& value);
    void onEquals();
    void onClear();
    void onBackspace();
    void onAngleModeChanged(int index);

private:
    void buildUI();
    void appendToExpression(const QString& text);
    void calculate();
    double applyAngleConversion(double val) const; // converts to radians if needed

    DisplayWidget*  m_display;
    MathEngine*     m_engine;
    HistoryManager* m_history;
    HistoryPanel*   m_historyPanel;

    QString    m_expression;
    AngleMode  m_angleMode  = AngleMode::Degrees;
    bool       m_resultShown = false;
    bool       m_invertMode  = false; // INV toggle for arc functions
};
