#include "MainWindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("kore"));
    QApplication::setApplicationName(QStringLiteral("kore"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icon.svg")));

    MainWindow window;
    window.show();

    return QApplication::exec();
}
