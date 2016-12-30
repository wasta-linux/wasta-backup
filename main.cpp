#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator myappTranslator;
    myappTranslator.load("/usr/share/wasta-backup/l10n/wasta-backup_" + QLocale::system().name());
    myappTranslator.load("l10n/wasta-backup_" + QLocale::system().name());
    app.installTranslator(&myappTranslator);

    MainWindow win(app.arguments());
    win.show();
    
    return app.exec();
}
