#include <QApplication>
#include <QIcon>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Caliber");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Caliber");
    app.setWindowIcon(QIcon(":/icons/caliber.svg"));

    MainWindow window;
    window.show();

    return app.exec();
}
