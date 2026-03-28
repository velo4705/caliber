#pragma once
#include <QTabWidget>
#include <QPoint>

class SwipeableTabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit SwipeableTabWidget(QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QPoint m_swipeStart;
    bool m_swiping = false;
};
