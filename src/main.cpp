#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("kore"));
    QApplication::setApplicationName(QStringLiteral("kore"));

    MainWindow window;
    window.show();

    return QApplication::exec();
}
