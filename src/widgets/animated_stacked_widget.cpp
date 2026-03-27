#include "animated_stacked_widget.h"
#include <QPainter>
#include <QPaintEvent>

AnimatedStackedWidget::AnimatedStackedWidget(QWidget* parent)
    : QStackedWidget(parent)
{
    m_timeLine = new QTimeLine(m_duration, this);
    m_timeLine->setFrameRange(0, 100);
    connect(m_timeLine, &QTimeLine::frameChanged, this, &AnimatedStackedWidget::onFrameChanged);
    connect(m_timeLine, &QTimeLine::finished, this, &AnimatedStackedWidget::onAnimFinished);
}

void AnimatedStackedWidget::setAnimationDuration(int ms) {
    m_duration = ms;
    m_timeLine->setDuration(ms);
}

void AnimatedStackedWidget::setCurrentIndex(int index) {
    if (m_animating) {
        // Finish immediately if already animating
        onAnimFinished();
    }
    if (index == QStackedWidget::currentIndex() || index < 0 || index >= count())
        return;

    // Capture the current widget as pixmap
    QWidget* prev = currentWidget();
    QWidget* next = widget(index);
    if (!prev || !next) { QStackedWidget::setCurrentIndex(index); return; }

    m_prevPixmap = prev->grab();
    m_nextIndex = index;

    // Switch to the new widget immediately (off-screen) and grab it
    QStackedWidget::setCurrentIndex(index);
    next->resize(prev->size());
    m_nextPixmap = next->grab();

    m_animating = true;
    m_timeLine->start();
}

void AnimatedStackedWidget::setCurrentWidget(QWidget* w) {
    int idx = indexOf(w);
    if (idx >= 0) setCurrentIndex(idx);
}

void AnimatedStackedWidget::paintEvent(QPaintEvent* event) {
    if (!m_animating || m_prevPixmap.isNull() || m_nextPixmap.isNull()) {
        QStackedWidget::paintEvent(event);
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    double progress = m_timeLine->currentFrame() / 100.0;

    // Cross-fade: draw outgoing fading out, incoming fading in
    p.setOpacity(1.0 - progress);
    p.drawPixmap(0, 0, m_prevPixmap);

    p.setOpacity(progress);
    p.drawPixmap(0, 0, m_nextPixmap);

    p.end();
}

void AnimatedStackedWidget::onFrameChanged(int frame) {
    // Force repaint with animation frame
    update();
}

void AnimatedStackedWidget::onAnimFinished() {
    m_animating = false;
    m_prevPixmap = QPixmap();
    m_nextPixmap = QPixmap();
    m_nextIndex = -1;
    update(); // Final repaint with the actual widget
}
