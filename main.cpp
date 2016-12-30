#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator myappTranslator;
    // testing:
    // myappTranslator.load("l10n/wasta-backup_" + QLocale::system().name());

    QDir localPath("l10n");

    if ( !localPath.exists() ) {
        myappTranslator.load("/usr/share/wasta-backup/l10n/wasta-backup_" + QLocale::system().name());
    } else {
        myappTranslator.load("l10n/wasta-backup_" + QLocale::system().name());
    }
    app.installTranslator(&myappTranslator);

    MainWindow win(app.arguments());
    win.show();
    
    return app.exec();
}
