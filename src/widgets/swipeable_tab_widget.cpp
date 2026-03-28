#include "swipeable_tab_widget.h"
#include <QEvent>
#include <QMouseEvent>
#include <QTouchEvent>

SwipeableTabWidget::SwipeableTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    // Install event filter on the tab widget to capture swipe gestures
    installEventFilter(this);
    // Also filter events on child widgets
    setAttribute(Qt::WA_AcceptTouchEvents, true);
}

bool SwipeableTabWidget::eventFilter(QObject* obj, QEvent* event) {
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->source() == Qt::MouseEventNotSynthesized) {
            m_swipeStart = me->pos();
            m_swiping = true;
        }
        break;
    }
    case QEvent::MouseMove: {
        if (!m_swiping) break;
        auto* me = static_cast<QMouseEvent*>(event);
        int dx = me->pos().x() - m_swipeStart.x();
        // Only consume horizontal swipes that exceed threshold
        if (qAbs(dx) > 10 && qAbs(dx) > qAbs(me->pos().y() - m_swipeStart.y())) {
            return true; // consume to prevent button interference
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        if (!m_swiping) break;
        auto* me = static_cast<QMouseEvent*>(event);
        int dx = me->pos().x() - m_swipeStart.x();
        int dy = me->pos().y() - m_swipeStart.y();
        m_swiping = false;

        // Horizontal swipe detected (min 60px, more horizontal than vertical)
        if (qAbs(dx) > 60 && qAbs(dx) > qAbs(dy) * 2) {
            int current = currentIndex();
            if (dx < 0 && current < count() - 1) {
                // Swipe left → next tab
                setCurrentIndex(current + 1);
                return true;
            } else if (dx > 0 && current > 0) {
                // Swipe right → previous tab
                setCurrentIndex(current - 1);
                return true;
            }
        }
        break;
    }
    default:
        break;
    }
    return QTabWidget::eventFilter(obj, event);
}
