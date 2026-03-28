#pragma once
// Android mobile support — includes and helpers for touch-friendly UI

#if defined(Q_OS_ANDROID)

#include "widgets/swipeable_tab_widget.h"
// On Android, use SwipeableTabWidget for swipe navigation between solver tabs
using MobileTabWidget = SwipeableTabWidget;

#else

#include <QTabWidget>
// On desktop, use standard QTabWidget
using MobileTabWidget = QTabWidget;

#endif
