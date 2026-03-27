#pragma once
#include <QStackedWidget>
#include <QTimeLine>
#include <QPixmap>

class AnimatedStackedWidget : public QStackedWidget {
    Q_OBJECT
public:
    explicit AnimatedStackedWidget(QWidget* parent = nullptr);

    void setAnimationDuration(int ms);
    int animationDuration() const { return m_duration; }

public slots:
    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget* w);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onFrameChanged(int frame);
    void onAnimFinished();

private:
    QTimeLine* m_timeLine;
    int m_duration = 200;
    int m_nextIndex = -1;
    QPixmap m_prevPixmap;
    QPixmap m_nextPixmap;
    bool m_animating = false;
};
