#include <QApplication>
#include <QDir>
#include "ShoinFrame.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Shoin");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Shoin");

    ShoinFrame frame;
    frame.show();

    return app.exec();
}