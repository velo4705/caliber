#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

// Shows the current expression (top, smaller) and result (bottom, large)
class DisplayWidget : public QWidget {
    Q_OBJECT
public:
    explicit DisplayWidget(QWidget* parent = nullptr);

    void setExpression(const QString& expr);
    void setResult(const QString& result, bool isError = false);
    void clear();

private:
    QLabel* m_expressionLabel;
    QLabel* m_resultLabel;
};
