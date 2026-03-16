#pragma once
#include <QWidget>
#include <QString>

class DisplayWidget;
class MathEngine;
class HistoryManager;
class HistoryPanel;

// Basic calculator mode: number pad + arithmetic operators
class BasicWidget : public QWidget {
    Q_OBJECT
public:
    explicit BasicWidget(MathEngine* engine,
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

private:
    void buildUI();
    void appendToExpression(const QString& text);
    void calculate();

    DisplayWidget*  m_display;
    MathEngine*     m_engine;
    HistoryManager* m_history;
    HistoryPanel*   m_historyPanel;

    QString m_expression;
    bool    m_resultShown = false; // after '=' is pressed
};
