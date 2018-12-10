#include <QSqlDatabase>
#include <QApplication>
#include "BuildInfo.h"
#include "MainWindow.h"

int main(int num_args, char** args)
{
    QApplication app(num_args, args);
    app.setOrganizationName("vmartinlac");
    app.setApplicationVersion(BuildInfo::getVersionString().c_str());

    QSqlDatabase::addDatabase("QSQLITE");

    MainWindow* w = new MainWindow();
    w->show();

    const int ret = app.exec();

    return ret;
}

