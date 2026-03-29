#include <QApplication>
#include <QIcon>
#include <cstdlib>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    // Force software OpenGL on systems without a GPU (e.g. VMs).
#if defined(Q_OS_WIN)
    qputenv("QT_OPENGL", "software");
#endif

    QApplication app(argc, argv);
    app.setApplicationName("Caliber");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("Caliber");
    app.setWindowIcon(QIcon(":/icons/caliber.svg"));

    MainWindow window;
    window.show();

    return app.exec();
}
