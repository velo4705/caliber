#include <QApplication>
#include <QIcon>
#include <QFont>
#include <cstdlib>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    // Force software OpenGL on systems without a GPU (e.g. VMs).
#if defined(Q_OS_WIN)
    qputenv("QT_OPENGL", "software");
#endif

#if defined(Q_OS_ANDROID)
    // Disable touch event compression so every tap registers immediately
    qputenv("QT_NO_TOUCH_COMPRESSION", "1");
#endif

    QApplication app(argc, argv);
#if defined(Q_OS_ANDROID)
    // Synthesize mouse events from touch for proper button click handling
    app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);
    app.setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, true);
#endif
    app.setApplicationName("Caliber");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("Caliber");
    app.setWindowIcon(QIcon(":/icons/caliber.svg"));

#if defined(Q_OS_ANDROID)
    // Increase font size and touch target spacing for mobile
    QFont appFont = app.font();
    appFont.setPixelSize(16);
    app.setFont(appFont);
#endif

    MainWindow window;
    window.show();

    return app.exec();
}
