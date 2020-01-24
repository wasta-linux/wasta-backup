// =============================================================================
//
// wasta-backup: mainwindow.cpp
//
// 2015-02-23 rik: increase backupDirs array from 10 to 20.  I thought it would
//      auto-expand, but it doesn't seem to.
// 2015-09-19 rik: Restore ALL needs to "mkdir -p" on the restore folder, as
//      it may several layers (of non-existing subfolders) deep.  Normal restore
//      won't need this because am starting from the immediate parent folder.
//    - Added '2>&1' to shellRun commands so that warnings and errors both
//      returned and sent to logfile.
//    - Added quotes to restore all rsync command to restore .config/wasta-backup
//    - Added 'sync' commands to restore processes
// 2016-12-20 rik: Adding in tr() to display strings
//    - Adding .ramp and .bloompack to default extensions
//    - Adding $HOME/Publications to default folders (Pathway exports)
// 2017-03-31 rik: replacing ' with \" for shell commands to account for single
//      quote in directory or file names (such as "Rik's Files" for example.
// 2018-05-22 rik: adding Paratext8Projects to default folders
// 2018-11-16 rik: when backing up, if source is symlink then set it to the
//      REAL path
//    - Attempt restore if EITHER of the following:
//      1. if restoreTime == now: if it is in the current backup (don't try
//         rdiff-backup --list-at-time now since if not found it will error)
//      2. if restoreTime <> now: if rdiff-backup --list-at-time restoreTime
//         exists since may not be in current backup but only in previous backups
// 2018-11-21 rik: restore deleted item: if parent folder is a symlink then
//      follow to the REAL path
// 2018-11-30 rik: cleaning up tr() strings to make more simple to translate
//    - Using xdg-user-dir to retrieve user's DESKTOP and DOCUMENTS locations
//      to account for locale differences in folder names.
// 2020-01-24 jcl: remove useBackupFilter.txt, relying on backup target
//      to define if filename extension filter should be used (default)
//      or not. This allows the same source to have full backups for a large
//      disk and also have filtered backups for a different smaller disk.
//
// =============================================================================

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
#include <QTextStream>
#include <QFileDialog>
#include <QFile>
#include <QHostInfo>
#include <QTest>
#include <QRegExp>
#include <QDesktopServices>
#include <QApplication>
#include <QDesktopWidget>

QString userID;
QString userHome;
QString restoreFolder;
bool processCanceled;
QString configDir;
QString configSave;
QString logDir;
QVector<QStringList> backupDirList(20); //initialize with 20 entries: a "few" extra can be added, later trimmed down to correct size
QVector<QStringList> restItemList(20);
QStringList restItems;

QString targetDevice = "";
QString machine = QHostInfo::localHostName();
QFile logFile;
QString prevBackupDevice;
QFile prevBackupDevFile;
QFile backupIncludeFile;
QFile backupDirFile;
QString renameText = "-SAVE-YYY-MM-DD";

MainWindow::MainWindow(QStringList arguments, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    userID = getenv("USER");
    userHome = getenv("HOME");
    // Ensure configDir exists
    configDir = userHome + "/.config/wasta-backup/";
    QDir().mkpath(configDir);

    //Ensure logDir exists
    logDir = userHome + "/.cache/wasta-backup/logs/";

    QDir logPath(logDir);

    if ( !logPath.exists() ) {
        logPath.mkpath(logDir);
    }
    // set current log
    logFile.setFileName(logPath.absolutePath() + "/" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") +
                        "-wasta-backup.log");
    // legacy: remove .config/wasta-backup/logs/ directory (until version 0.9.2, but don't want going forward)
    removeDir(configDir + "logs/");
    writeLog("===========================================\n"
             "== wasta-backup started                  ==\n"
             "===========================================");
    // clean out old logs (greater than 1 year old)
    QStringList logList = logPath.entryList(QDir::Files);
    QString fileName;

    foreach (fileName, logList) {
        QFileInfo info(logDir + fileName);

        // check if created more than 1 year ago
        if (  info.created().operator <( QDateTime::currentDateTime().addMonths(-12) )) {
            // if so, delete
            QFile::remove(logDir + fileName);
        }
    }

    //Legacy file cleanup
    if ( QFile::exists(configDir + "useBackupIncludeFilter.txt") )
        QFile::remove(configDir + "useBackupIncludeFilter.txt");
    if ( QFile::exists(configDir + "prevBackupDate.txt") )
        QFile::remove(configDir + "prevBackupDate.txt");

    // Check for backupInclude file
    backupIncludeFile.setFileName(configDir + "backupInclude.txt");
    if ( !backupIncludeFile.exists() ) {
        // create it
        QString include =
                "+ ignorecase:**.od*\n"
                "+ ignorecase:**.ot*\n"
                "\n"
                "+ ignorecase:**.doc*\n"
                "+ ignorecase:**.dot*\n"
                "+ ignorecase:**.xls*\n"
                "+ ignorecase:**.xlt*\n"
                "+ ignorecase:**.ppt*\n"
                "\n"
                "+ ignorecase:**.abw\n"
                "+ ignorecase:**.zabw\n"
                "+ ignorecase:**.awt\n"
                "\n"
                "+ ignorecase:**.rtf\n"
                "+ ignorecase:**.txt\n"
                "+ ignorecase:**.csv\n"
                "+ ignorecase:**.pdf\n"
                "+ ignorecase:**.xml\n"
                "+ ignorecase:**.htm*\n"
                "\n"
                "+ ignorecase:**.ramp\n"
                "+ ignorecase:**.bloompack\n"
                "+ ignorecase:**.fwbackup"
                "\n"
                "- **\n";

        // open it
        backupIncludeFile.open(QIODevice::ReadWrite);

        writeLog("No " + configDir + "backupInclude.txt: creating it.");

        QTextStream stream(&backupIncludeFile);
        stream << include;
        stream.flush();
        backupIncludeFile.close();
    }

    // Check for backupDirs file
    backupDirFile.setFileName(configDir + "backupDirs.txt");

    if ( !backupDirFile.exists() ) {
        // create it

        QString include =
                "# Backup Entries follow this syntax:\n"
                "#   Folder Display Name,Folder Location,useInclude,Additional rdiff-backup Parameters,Remove Older Than\n"
                "#\n"
                "# Notes:\n"
                "#   Folder Location: if includes xdg-user-dir NAME then will be set at runtime\n"
                "#     to account for locale changes to folder names. In this case,\n"
                "#     Folder Display Name will also be set at runtime.\n"
                "#     If NAME is not a valid xdg-user-dir folder, this backup folder will be IGNORED.\n"
                "#     Use the command 'man xdg-user-dir' to see a list of available folders.\n"
                "#\n"
                "#   useInclude = YES ==> only include specified filetypes in backupInclude.txt\n"
                "#   useInclude = NO  ==> don't limit backup to specified filetypes in backupInclude.txt\n"
                "#\n"
                "#   Remove Older Than examples:\n"
                "#     30B  - keep versions from the past 30 backup sessions, remove older ones\n"
                "#     3M   - remove versions older than 3 months\n"
                "#     1Y   - remove versions older than 1 year\n"
                "#     1Y3M - remove versions older than 1 year 3 months\n"
                "\n"
                "Bloom,$HOME/Bloom,NO,,1Y\n"
                "Paratext 9,$HOME/Paratext9Projects,NO,,1Y\n"
                "Paratext 8,$HOME/Paratext8Projects,NO,,1Y\n"
                "Fieldworks Projects,$HOME/.local/share/fieldworks/Projects,NO,,1Y\n"
                "WeSay,$HOME/WeSay,NO,,1Y\n"
                "Adapt It,$HOME/Adapt It Unicode Work,NO,--exclude ignorecase:**/.temp,1Y\n"
                "Publications,$HOME/Publications,NO,,1Y\n"
                "Thunderbird,$HOME/.thunderbird,NO,--exclude ignorecase:**/Cache,1Y\n"
                "display-name,xdg-user-dir DOCUMENTS,YES,,1Y\n"
                "display-name,xdg-user-dir DESKTOP,YES,,1Y";
        // open it
        backupDirFile.open(QIODevice::ReadWrite);
        writeLog("No " + configDir + "backupDirs.txt: creating it.");

        QTextStream stream(&backupDirFile);
        stream << include;
        stream.flush();
        backupDirFile.close();
    }

    // Check for prevBackupDev file
    prevBackupDevFile.setFileName(configDir + "prevBackupDevice.txt");

    if ( !prevBackupDevFile.exists() ) {
        // create it: but leave empty
        prevBackupDevFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
        writeLog("No " + prevBackupDevFile.fileName() + ": creating it (empty).");

        QTextStream stream(&prevBackupDevFile);
        stream << "";
        stream.flush();
        prevBackupDevFile.close();
    }
    // Launch Main Window
    ui->setupUi(this);

    // set app icon (used in 'About')
    QIcon wastaIcon = QIcon::fromTheme("wasta-backup");
    this->setWindowIcon(wastaIcon);

    // set location to center of primary screen
    QRect screenGeometry = QApplication::desktop()->availableGeometry(QApplication::desktop()->primaryScreen());
    int x = (screenGeometry.width()  - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;

    this->move(x, y);
    this->setWindowTitle(tr("Wasta-Backup"));
    this->show();

    // Setup GUI Defaults
    ui->progressBar->setVisible(0);
    ui->backupButton->setEnabled(0);
    ui->cancelBackupButton->setEnabled(0);
    ui->restoreTab->setEnabled(0);
    ui->backupRestoreWidget->setCurrentIndex(0);

    // Default restore is for Previous Version: set defaults
    //Disable Buttons
    ui->restoreButton->setEnabled(0);
    ui->cancelRestoreButton->setEnabled(0);
    ui->undoLastRestoreButton->setEnabled(0);
    ui->openRestoreFolderButton->setEnabled(0);

    // Setup details for Previous Version
    ui->prevDateTimeLabel->setEnabled(0);
    ui->prevListCombo->clear();
    ui->prevListCombo->setEnabled(0);
    ui->restorePageWidget->setCurrentIndex(0);

    // load up config files
    loadConfigFiles();

    // check for passed argument: if found, means wasta-backup launched by udev
    //   in this case, set passed argument as target device
    if ( arguments.value(1) != "" ) {
        QFileInfo argumentInfo(arguments.value(1) + "/wasta-backup");
        if ( argumentInfo.isWritable() ) {
            writeLog("wasta-backup started with argument: " + arguments.value(1) + " setting as target device.");
            ui->messageOutput->append("<b>" + tr("Backup:") + "</b> " + arguments.value(1).mid(arguments.value(1).lastIndexOf("/") + 1));
            (arguments.value(1));
        }
    }

    if ( targetDevice == "" ) {
        // if argument exists above, but not writable, will get to here also
        // check if prev backup writable, if so, set as target device
        QFileInfo prevBackupInfo(prevBackupDevice + "/wasta-backup");
        if ( prevBackupInfo.isWritable() ) {
            //use it
            writeLog("Previous backup found: " + prevBackupDevice + " setting as target device.");
            //ui->messageOutput->append("<b>" + tr("Previous backup found:") + "</b> " + prevBackupDevice.mid(prevBackupDevice.lastIndexOf("/") + 1));
            setTargetDevice(prevBackupDevice);
        } else {
            // use logic to determine preferred destination
            writeLog("No argument or previous backup found.  Calling setPreferredDestination to set target device.");
            //ui->messageOutput->append("<b>" + tr("Previous backup not found:") + "</b> " +
            //                          prevBackupDevice.mid(prevBackupDevice.lastIndexOf("/") + 1));
            setPreferredDestination();
        }
    }
    //ui->messageOutput->append("");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ##########################################################################
// #### MAIN INTERFACE PROCEDURES                                        ####
// ##########################################################################

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,tr("About Wasta-Backup"),"<h3>" + tr("Wasta-Backup") + "</h3>" +
                       "<p>" + tr("Wasta-Backup is a simple backup GUI using rdiff-backup for version backups of data.") +
                       "<p>" + tr("Wasta-Backup will auto-launch when a USB device with a previous Wasta-Backup on it is inserted.") +
                       "<p>" + tr("Restore possibilities include restoring previous versions of existing files or folders as well as restoring deleted files or folders from the backup") + ". " +
                       tr("In the case of restoring previous versions of existing items, the current item is first renamed using the current date and time.") +
                       "<p>" + tr("Additionally, a 'Restore ALL' option is available that will replace all data on the computer from the backup.") +
                       "<p>" + tr("The following configurable settings are stored in a user's ~/.config/wasta-backup/ directory:") +
                       "<ul>" +
                       "<li><p><b>backupDirs.txt:</b> " + tr("This file specifies directories to backup and other parameters such as number of versions to keep") + "</li>" +
                       "<li><p><b>backupInclude.txt:</b> " + tr("This file specifies file extensions to backup (so files with media extensions, etc., will be ignored)") +
                       "</li></ul>" +
                       "<p><b>" + tr("Wasta-Backup website:") + "</b>"
                       "<p><a href=\"https://www.wastalinux.org/wasta-apps/wasta-backup\">https://www.wastalinux.org/wasta-apps/wasta-backup</a>");
}

void MainWindow::setPreferredDestination()
{
    QString shellCommand;
    QString shellReturn;

    // wipe targetDevice
    ui->targetDeviceDisp->setText("");
    ui->backupButton->setEnabled(0);
    ui->restoreTab->setEnabled(0);

    // this will account for filesystems under /media (so will include /media/username for ubuntu 12.10 and newer)
    shellCommand = "df -P -x iso9660 | grep /media/ | awk '{print substr($0, index($0, $6))}'";
    shellReturn = shellRun(shellCommand, false);

    QStringList deviceList = shellReturn.split("\n");

    if ( deviceList.count() > 1 ) {
        int biggestSize = 0;
        int wastaBiggestSize = 0;
        int currentSize = 0;
        int bestDevice = -1;
        int wastaFound = 0;

        // subtract 1 because the ls command returned a trailing \n
        for (int i=0; i<(deviceList.count() - 1); i++)
        {
            //check if writable
            QFileInfo mediaDir(deviceList.value(i));
            if ( mediaDir.isWritable() ) {
                writeLog(deviceList.value(i) + " is writable.");

                // check for wasta-backup folder.  if found, keep track (and count of found wasta-backup folders).
                // if only 1 wasta-backup folder found, choose it (regardless of space).
                // if more than 1 wasta-backup folder, pick one with most remaining space.
                // if no wasta-backup folders found, pick largest device

                //get free space
                shellCommand = "df -P \"" + deviceList.value(i) + "/\" | tail -1 | awk '{print $4}'";
                QString temp2 = shellRun(shellCommand, false);
                currentSize = temp2.toInt();

                QFileInfo wastaDir(deviceList.value(i) + "/wasta-backup");
                if ( wastaDir.exists() ) {
                    wastaFound ++;
                    writeLog(deviceList.value(i) + " has existing wasta-backup folder.");
                    if (currentSize >= wastaBiggestSize) {
                        bestDevice = i;
                        wastaBiggestSize= currentSize;
                    }
                }

                // keep track which is biggestSize in case no wastaFound
                if (( wastaFound == 0 ) & ( currentSize >= biggestSize )) {
                    bestDevice = i;
                    biggestSize = currentSize;
                }
            } else {
                writeLog(deviceList.value(i) + " is NOT writable.");
            }
        }
        if ( bestDevice >= 0 ) {
            // valid device found, set it
            setTargetDevice(deviceList.value(bestDevice));
        } else {
            // no writable devices found
            QTest::qWait(1);
            QMessageBox::information(this, tr("No Device Found"), tr("No backup location found. Please insert a USB device and click 'Change'."));
            writeLog("Target device found, but not writeable.");
        }
    } else {
        // no devices found
        QTest::qWait(1);
        QMessageBox::information(this, tr("No Device Found"), tr("No backup location found. Please insert a USB device and click 'Change'."));
        writeLog("No target device found.");
    }
}

void MainWindow::setTargetDevice(QString inputDir)
{
    // ok, has to have /media at first level OR have a wasta-backup folder (maybe have one more subfolder be wasta-backup)?
    // if no existing wasta-backup folder, then trim it back to mount directory.
    // if wasta-backup folder found, trim it back to the folder directly preceding wasta-backup directory.

    QString shellCommand;
    QString shellReturn;
    QString newTarget = "";

    int wastaLocation = inputDir.indexOf("/wasta-backup",0);
    if (wastaLocation >=0 ) {
        // we found an existing backup location, trim string to it (in case user selected a deeper level)
        newTarget = inputDir.mid(0,wastaLocation);
        writeLog("Existing wasta-backup found in " + inputDir + ".  NewTarget " + newTarget);
    } else {
        // see if inputDir + "/wasta-backup" exists, use it if so
        QFileInfo wastaInfo(inputDir + "/wasta-backup");
        if (wastaInfo.exists()) {
            // we found existing backup location, so set newTarget
            newTarget = inputDir;
            writeLog("Existing wasta-backup found in subfolder of " + inputDir + ".  NewTarget " + newTarget);
        } else if ( inputDir.indexOf("/media",0) == 0 ) {
            // no existing backup location found (give message), but valid "new" location since has "/media" at beginning.
            // trim folder to mount point (so wasta-backup folder created at root of drive).
            shellCommand = "df -P | grep /media/ | awk '{print substr($0, index($0, $6))}'";
            shellReturn = shellRun(shellCommand, false);

            QStringList mountList = shellReturn.split("\n");
            QString mountItem;

            foreach (mountItem, mountList) {
                if ( (!mountItem.isEmpty()) & (inputDir.indexOf(mountItem) == 0) ) {
                    // mount found for new backup: trim string to use it (set new backup to root of drive)
                    newTarget = mountItem;
                    writeLog("No existing wasta-backup found in " + inputDir +
                             ".  But valid new target " + mountItem);
                    break; // foreach loop
                }
            } // foreach
        }  // else if
    } // else

    if ( ! newTarget.isEmpty() ) {
        // confirm writable, then set
        QFileInfo newTargetInfo(newTarget);
        if ( newTargetInfo.isWritable() ) {
            targetDevice = newTarget;

            ui->targetDeviceDisp->setText(newTarget.mid(newTarget.lastIndexOf("/") + 1));

            writeLog("Updating targetDevice: " + targetDevice);
            ui->backupButton->setEnabled(1);
            ui->restoreTab->setEnabled(1);

            QFileInfo existingBackupInfo(newTarget + "/wasta-backup");
            if ( existingBackupInfo.exists() ) {
                //existing backup: display message
                ui->messageOutput->append("<b>" + tr("Existing backup found:") + "</b> " +
                                          ui->targetDeviceDisp->text());
                ui->messageOutput->append("");
                ui->messageOutput->append("<b>" + tr("Ready for backup") + "</b>");
                //ui->messageOutput->moveCursor(QTextCursor::End);
                writeLog(targetDevice + " has existing backup and ready for backup.");

                // load per-backup config from target instead of defaults from the user home-directory
                QString backupConfigDir = newTarget + "/wasta-backup/" + machine + "/wasta-backup-config-" + userID + "/";

                if ( QDir().exists(backupConfigDir) ) {
                    if ( QFile::exists( backupConfigDir + "backupDirs.txt" ) )
                        backupDirFile.setFileName(backupConfigDir + "backupDirs.txt");
                    if ( QFile::exists( backupConfigDir + "backupInclude.txt" ) )
                        backupIncludeFile.setFileName(backupConfigDir + "backupInclude.txt");
                    writeLog("Switching to config on backup device: " +backupDirFile.fileName()+ " " +backupIncludeFile.fileName());

                    //Legacy file cleanup
                    if ( QFile::exists(backupConfigDir + "useBackupIncludeFilter.txt") )
                        QFile::remove(backupConfigDir + "useBackupIncludeFilter.txt");
                    if ( QFile::exists(backupConfigDir + "prevBackupDate.txt") )
                        QFile::remove(backupConfigDir + "prevBackupDate.txt");

                    loadConfigFiles();
                }

            } else {
                // new backup: display message
                ui->messageOutput->append("<b>" + tr("No existing backup found:") + "</b> " +
                                          ui->targetDeviceDisp->text());
                ui->messageOutput->append("");
                ui->messageOutput->append("<b>" + tr("Ready for first backup (may take some time to complete)") + "</b>");
                //ui->messageOutput->moveCursor(QTextCursor::End);
                writeLog(targetDevice + " doesn't have existing backup but ready for first backup.");
            }
        } else {
            // newTarget not writable, give error
            QTest::qWait(1);
            QMessageBox::warning(this, tr("Choose Again"), "<b>" + tr("Chosen directory is not a valid backup location:") + "</b><p>" + newTarget);
            writeLog("User can't write to newTarget: " + newTarget);
            // targetDevice and ui.targetDevice.Disp remain unchanged
        }
    } else {
        // no newTarget found to be used.
        QTest::qWait(1);
        QMessageBox::warning(this, tr("Choose Again"), "<b>" + tr("Chosen directory is not a valid backup location:") + "</b><p>" + inputDir);
        writeLog(inputDir + " not a valid backup location.");
    }
}

void MainWindow::on_changeDeviceButton_clicked()
{
    ui->progressBar->setVisible(0);
    QString startTarget;

    // need to fix! to handle /media/akiverson plus /media!

    // if current device has user ID in it, then newer
    if ( targetDevice.indexOf(userID) > 0 ) {
        // on 12.10 +: USB mounted to /media/userid/mount-name
        startTarget = "/media/" + userID;
    } else {
        // on 12.04: USB mounted to /media/mount-name
        startTarget = "/media";
    }
    // file utility to select device: but trim to just be /media/xxxxx
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Choose Backup Location"),
                                                        startTarget,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    ui->messageOutput->append("");
    setTargetDevice(dirName);
}

void MainWindow::on_backupRestoreWidget_currentChanged(int index)
{
    if ( index == 1 ) {
        // Restore Tab selected: default to Restore Prev
        ui->restorePrevRadio->setChecked(1);
        on_restorePrevRadio_clicked();
    }
    ui->progressBar->setVisible(0);
}

void MainWindow::on_cancelBackupButton_clicked()
{
    cancelProcess();
}

void MainWindow::on_cancelRestoreButton_clicked()
{
    cancelProcess();
}

void MainWindow::cancelProcess()
{
    int systemReturn;
    systemReturn = system("pkill rdiff-backup");

    processCanceled = true;
    ui->cancelRestoreButton->setEnabled(0);
    ui->cancelBackupButton->setEnabled(0);
    ui->messageOutput->append("");
    ui->messageOutput->append("<b>" + tr("Canceling") + "</b>");
    writeLog("Canceling! Return from pkill: " + QString::number(systemReturn));
    ui->messageOutput->moveCursor(QTextCursor::End);
}

void MainWindow::on_viewLogButton_clicked()
{
    QDesktopServices::openUrl(QUrl("file://" + logFile.fileName()));
}

// ##########################################################################
// #### BACKUP TAB PROCEDURES                                            ####
// ##########################################################################

void MainWindow::on_backupButton_clicked()
{
    QString source;
    QString dest;
    QString parms;
    QString stdParms = "--exclude-symbolic-links --override-chars-to-quote '\"*/:<>?\\\\|'";
    QString output;

    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(1);

    ui->changeDeviceButton->setEnabled(0);
    ui->backupButton->setEnabled(0);
    ui->restoreTab->setEnabled(0);

    ui->cancelBackupButton->setEnabled(1);

    processCanceled = false;

    // clear visible parts of message output window
    ui->messageOutput->append("");
    ui->messageOutput->append("<b>" + tr("Starting backup to:") + "</b> " + ui->targetDeviceDisp->text());
    //ui->messageOutput->append("");
    ui->messageOutput->moveCursor(QTextCursor::End);
    writeLog("Starting backup to " + ui->targetDeviceDisp->text());
    QTest::qWait(2000);

    QString targetDir = targetDevice + "/wasta-backup/" + machine;

    //update targetDevice with config files (to use for Restore ALL instead of an existing machines config files)
    QString backupConfigDir = targetDir + "/wasta-backup-config-" + userID + "/";

    // Ensure backupConfigDir exists
    if ( !QDir().exists(backupConfigDir) ) {
        //Legacy: clean out old backupConfigDir location that will cause any rdiff-backup of ~/.config to fail
        //   since before 2013-10-13 backupConfigDir was targetDir + userHome + /.config/wasta-backup
        //   Since this was NOT a "rdiff-backup folder" then rdiff-backup will warn folder exists and will not process
        //   So, need to manually remove .config folder so will not conflict with anyone attempting to backup home
        output = shellRun("rm -rf \"" + targetDir + userHome + "/.config\"", false);

        QDir().mkpath(backupConfigDir);
    }

    //use rsync to fill in missing config files on backup drive from the user's $HOME/.config/
    QString rSyncCmd = "rsync -rlt --ignore-existing --exclude prevBackupDevice.txt ";
    output = shellRun(rSyncCmd + " \"" + configDir + "\" \"" + backupConfigDir + "\"", false);

    // sync back the config used. The timestamp will show the last time the backup was done to that device
    // as well as easily allowing a comparision of the various backup configuration files used by the different devices.
    // It is also an easy way to re-create a backup config in case the backup device is lost.
    QString lastConfigsDir=configDir + "lastUsedConfigs/";
    if ( QDir().mkpath(lastConfigsDir) ) {
        output = shellRun("rsync " +backupIncludeFile.fileName()+ "  " +lastConfigsDir+ "backupInclude.txt_" +ui->targetDeviceDisp->text(), false);
        output = shellRun("rsync " +backupDirFile.fileName()+     "  " +lastConfigsDir+ "backupDirs.txt_" +ui->targetDeviceDisp->text(), false);
    }

    //now proceed with backups
    int progress = 10;
    ui->progressBar->setValue(progress);

    QString rdiffReturn;

    for ( int i = 0; i< backupDirList.count(); i++) {

        // value 3: additional parameters
        parms = stdParms + " " + backupDirList[i].value(3);

        // to use Filter, need backup directory to specify using it.
        QString notifyLimitedBackup="";
        if ( QString::compare(backupDirList[i].value(2), "YES", Qt::CaseInsensitive) == 0 ) {
            // value 2=YES: include filetype filter
            parms = parms + " --include-globbing-filelist " +backupIncludeFile.fileName();
            notifyLimitedBackup = tr(" [config limits backup to specified file-types]");
        }

        source = backupDirList[i].value(1).replace("$HOME",getenv("HOME"));
        dest = targetDir + backupDirList[i].value(1).replace("$HOME",getenv("HOME"));

        if ( QDir().exists(source) ) {

            //Need to check if source is a symlink or else backup will fail
            QFileInfo sourceInfo(source);

            if (sourceInfo.isSymLink()) {
                //Set source to REAL Path (but do NOT change dest: no symlinks in backups but want to
                //  match folder names from backupDirs.txt)
                writeLog(source + " contains a symlink so re-directing to REAL path");
                source = sourceInfo.symLinkTarget();
                writeLog(source + " NOW set as source");
            }
            ui->messageOutput->append("");
            ui->messageOutput->append("<b>" + tr("Backing up:") + "</b> " + backupDirList[i].value(0) +notifyLimitedBackup);

            //ensure dest path exists
            QDir().mkpath(dest);

            // Adding extra check to ensure user didn't cancel BEFORE rdiff called
            if (processCanceled) {
                break; // break out of backup loop
            }
            // Backup
            rdiffReturn = shellRun("rdiff-backup " + parms + " \"" + source +
                                   "\" \"" + dest + "\" 2>&1",true);
            if (processCanceled) {
                break; // break out of backup loop
            }
            // Remove old backups
            QString olderThan = backupDirList[i].value(4);
            if (olderThan.trimmed() != "") {
                QString rdiffCommand = "rdiff-backup " + stdParms + " --remove-older-than " +
                        olderThan + " --force \"" + dest + "\" 2>&1";
                rdiffReturn = shellRun(rdiffCommand,false);
            }
            if (processCanceled) {
                break; // break out of backup loop
            }
            progress = progress + 10;
            ui->progressBar->setValue(progress);
        }
    }

    if ( !processCanceled ) {
        ui->messageOutput->append("");
        ui->messageOutput->append("<b>" + tr("Backup complete") + "</b>");
        writeLog("Backup complete");
        ui->messageOutput->moveCursor(QTextCursor::End);

        //update prevBackupDevFile info
        QString fileText = targetDevice;
        prevBackupDevFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
        writeLog("Updating " + prevBackupDevFile.fileName() + ": value: " + fileText);

        QTextStream devStream(&prevBackupDevFile);
        devStream << fileText;
        devStream.flush();
        prevBackupDevFile.close();
    } else {
        ui->messageOutput->append("");
        ui->messageOutput->append("<b>" + tr("Backup canceled") + "</b>");
        writeLog("Backup canceled");
        ui->messageOutput->moveCursor(QTextCursor::End);
    }

    // reset processCanceled so won't report as canceled for sync commands
    processCanceled = false;

    // regardless of canceled or not, need to sync hdd (this ensures all written to disk: thanks to jl)
    rdiffReturn = shellRun("sync",false);

    // recommended to do it twice (also thanks to jl :)
    rdiffReturn = shellRun("sync",false);

    ui->progressBar->setValue(100);

    ui->cancelBackupButton->setEnabled(0);

    ui->changeDeviceButton->setEnabled(1);
    ui->backupButton->setEnabled(1);
    ui->restoreTab->setEnabled(1);
}

// ##########################################################################
// #### RESTORE TAB PROCEDURES                                           ####
// ##########################################################################

void MainWindow::on_restorePrevRadio_clicked()
{
    // restore Previous always only from current machine
    machine = QHostInfo::localHostName();

    //Disable Buttons
    ui->restoreButton->setEnabled(0);
    ui->undoLastRestoreButton->setEnabled(0);
    ui->openRestoreFolderButton->setEnabled(0);

    // Setup details for Previous Version
    // default to file search
    ui->prevFileRadio->setChecked(1);
    ui->prevDateTimeLabel->setEnabled(0);
    ui->prevItem->clear();
    ui->prevListCombo->clear();
    ui->prevListCombo->setEnabled(0);
    ui->restorePageWidget->setCurrentIndex(0);
}

void MainWindow::on_restoreDelRadio_clicked()
{
    // restore Deleted always only from current machine
    machine = QHostInfo::localHostName();

    //Disable Buttons
    ui->restoreButton->setEnabled(0);
    ui->undoLastRestoreButton->setEnabled(0);
    ui->openRestoreFolderButton->setEnabled(0);

    // Setup details for Missing Item
    ui->delListLabel->setEnabled(0);
    ui->delList->setEnabled(0);
    ui->delList->clear();
    ui->delFolder->clear();
    ui->restorePageWidget->setCurrentIndex(1);
}

void MainWindow::on_restoreAllRadio_clicked()
{    
    // later will set machine based on combobox;
    machine = "";

    //Disable Buttons
    ui->restoreButton->setEnabled(0);
    ui->undoLastRestoreButton->setEnabled(0);
    ui->openRestoreFolderButton->setEnabled(0);

    // Setup details for Restore All
    ui->restoreAllCheck->setChecked(0);
    ui->machineCombo->clear();
    ui->restUserCombo->clear();
    ui->restorePageWidget->setCurrentIndex(2);

    ui->machineCombo->setEnabled(0);
    ui->machineLabel->setEnabled(0);
    ui->restUserCombo->setEnabled(0);
    ui->restUserLabel->setEnabled(0);
}

void MainWindow::on_prevFileRadio_clicked()
{
    ui->prevItemLabel->setText(tr("File to Restore:"));
}

void MainWindow::on_prevFolderRadio_clicked()
{
    ui->prevItemLabel->setText(tr("Folder to Restore:"));
}

void MainWindow::on_selectPrevItemButton_clicked()
{
    ui->prevListCombo->clear();
    ui->prevListCombo->setEnabled(0);
    ui->prevDateTimeLabel->setEnabled(0);

    ui->restoreButton->setEnabled(0);
    ui->openRestoreFolderButton->setEnabled(0);

    QString restItemName;
    QString rdiffCommand;
    QString rdiffReturn;
    int incCount;

    if ( ui->prevFileRadio->isChecked() ) {
        // open file dialog
        restItemName = QFileDialog::getOpenFileName(this, tr("Select File"),
                                                    userHome);
        restoreFolder = restItemName.mid(0,restItemName.lastIndexOf("/"));
    } else {
        // open directory dialog
        // add trailing "/"
        restItemName = QFileDialog::getExistingDirectory(this, tr("Select Folder"),
                                                        userHome);
        restoreFolder = restItemName;
    }
    ui->prevItem->setText(restItemName);

    if (!QFile::exists(restItemName)) {
        restoreFolder = "";
        writeLog(restItemName + " doesn't exist (user probably canceled dialog?)");
        return;
    }

    ui->openRestoreFolderButton->setEnabled(1);

    // first, find if backup dir exists.

    bool backupFound = false;

    // this loop checks if restItemName is part of backups, based on backupDirs
    //  (needed or else rdiff-backup will return error if file not part of a backup)
    QString backupDir = "";

    for ( int i = 0; i < backupDirList.count(); i++ ) {
        backupDir = backupDirList[i].value(1).replace("$HOME",getenv("HOME"));

        if ( restItemName.indexOf(backupDir) == 0) {
            // backup dir found! cut apart target to add in rdiff-backup-data folder to search.
            backupFound = true;
            writeLog("Backup directory for " + restItemName + " found on " + ui->targetDeviceDisp->text());
            break;  // break out of for loop;
        }
    }

    if ( !backupFound ) {
        QTest::qWait(1);
        QMessageBox::information(this, "Not Found", "<b>" + tr("Backup not found:") + "</b><p>" + restItemName);
        writeLog("Backup of " + restItemName + " not found on device " + targetDevice);
        return;
    }

    QString targetItem = targetDevice + "/wasta-backup/" + machine + restItemName;

    // now lets process!

    // get increments

    rdiffCommand = "rdiff-backup -l \"" + targetItem + "\" 2>&1";
    rdiffReturn = shellRun(rdiffCommand,false);

    QStringList incList = rdiffReturn.split("\n");

    //first line format:  "Found N increments:".  So, count of increments in second position.

    QStringList lineSplit = incList.value(0).split(" ");

    incCount = lineSplit.value(1).toInt();

    //re-initialize restItemList
    restItemList.resize(0);
    restItemList.resize(incCount);

    //Now, load up increment listings

    int startIncDate;
    QString restItemTime;
    int prevItemCount = 0;

    for ( int i = 1; i <= incCount; i++) {
        // throw away items indicating missing
        startIncDate = incList.value(i).lastIndexOf(QRegExp("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}[:-][0-9]{2}"));
        restItemTime = incList.value(i).mid(startIncDate,25);

        if ( incList.value(i).mid(startIncDate + 25,8) != ".missing" ) {
            // increment listing lines format: "   FileName.ext.yyyy-MM-ddTHH:mm:ss-HH:mm.TYPE  Display Date
            // TYPE=dir means directory, TYPE=diff.gz means file
            // Second HH:mm are timezone adjustments.

            restItemList[prevItemCount].insert(0, restItemName);
            restItemList[prevItemCount].insert(1, restItemTime);
            ui->prevListCombo->insertItem(prevItemCount,"Date: " + restItemTime.mid(0,10) +
                                          "   Time: " + restItemTime.mid(11,5));
            prevItemCount++;
        } else {
            writeLog("missing increment thrown away: " + incList.value(i));
        }
    }

    // last, check CURRENT backup as compared to directory for previous version (so will be last entry if found)

    // traditional "diff" to compare the two.
    //  -q will make diff only output filenames, not file content differences
    //  -r will make recursive
    //  || true; needed so no error returned if diff found

    rdiffCommand = "diff -qr \"" + restItemName + "\" \"" + targetItem + "\" || true;";
    rdiffReturn = shellRun(rdiffCommand, false);

    if ( rdiffReturn.trimmed() != "" ) {
        //if any diff return found (list of different files), then this folder should be listed for restore

        // add one more to restItemList
        restItemList.resize(incCount + 1);

        // some difference, so list current item from backup as a previous item: would be most current so list last
        restItemList[prevItemCount].insert(0, restItemName);
        restItemList[prevItemCount].insert(1, "now");

        // last line of incList contains list of last backup in this format:
        //     "Current mirror: Sat Dec 14 09:07:18 2013"
        // this would be needed to display date and time
        QString currentMirror = incList.value(incCount + 1);

        // trim off "Current mirror: " so can process date info
        currentMirror.replace("Current mirror: ","");

        QDateTime mirrorDate = QDateTime::fromString(currentMirror, "ddd MMM dd hh:mm:ss yyyy");

        QString modifiedDate = mirrorDate.toString("yyyy-MM-dd");
        QString modifiedTime = mirrorDate.toString("hh:mm");
        ui->prevListCombo->insertItem(prevItemCount, "Date: " + modifiedDate + "   Time: " + modifiedTime);
        prevItemCount++;
    }

    if ( prevItemCount > 0 ) {
        ui->prevListCombo->setCurrentIndex(prevItemCount -1);
        ui->prevListCombo->setEnabled(1);
        ui->prevDateTimeLabel->setEnabled(1);
        ui->restoreButton->setEnabled(1);

    } else {
        QTest::qWait(1);
        QMessageBox::information(this, tr("No Items Found"), "<b>" + tr("No previous versions found in backup:") + "</b><p>" + restItemName);
        writeLog("No previous versions found in backup: " + restItemName);
    }
}

void MainWindow::on_selectDelFolderButton_clicked()
{
    ui->delList->clear();
    ui->delList->setEnabled(0);
    ui->delListLabel->setEnabled(0);
    ui->restoreButton->setEnabled(0);

    ui->openRestoreFolderButton->setEnabled(0);

    // open directory dialog
    QString missingDir;

    missingDir = QFileDialog::getExistingDirectory(this, tr("Select Parent Folder of Deleted Item"),
                                                   userHome);

    if (!QFile::exists(missingDir)) {
        restoreFolder = "";
        writeLog(missingDir + " doesn't exist (user probably canceled dialog?)");
        return;
    }

    QString incDir = "";

    // this loop sets up search Target dir, based on backupDirs
    //  (needed so know where to place in the "rdiff-backup-data/increments" folder.)
    for ( int i = 0; i < backupDirList.count(); i++ ) {
        QString backupDir = backupDirList[i].value(1).replace("$HOME",getenv("HOME"));

        if ( missingDir.indexOf(backupDir) == 0) {
            // backup dir found! cut apart target to add in rdiff-backup-data folder to search.
            incDir = targetDevice + "/wasta-backup/" + machine + backupDir +
                    "/rdiff-backup-data/increments" + missingDir.mid(backupDir.length(),-1);
            writeLog("Set increment Dir to: " + incDir);
            break;  // break out of for loop;
        }
    }

    if ( incDir == "" ) {
        QTest::qWait(1);
        QMessageBox::information(this, tr("Not Found"), "<b>" + tr("Backup not found:") + "</b><p>" + missingDir);
        writeLog("Backup of " + missingDir + " not found on device " + targetDevice);
        return;
    }

    ui->delFolder->setText(missingDir);
    restoreFolder = missingDir;

    //Need to check if folder is a symlink or else all items will show as deleted
    QFileInfo restoreFolderInfo(restoreFolder);

    if (restoreFolderInfo.isSymLink()) {
        //Set to REAL Path so restore will be the correct
        //  location not the symlink location
        writeLog(restoreFolder + " contains a symlink so re-directing to REAL path");
        restoreFolder = restoreFolderInfo.symLinkTarget();
        writeLog(restoreFolder + " NOW set as restoreFolder");
    }

    ui->openRestoreFolderButton->setEnabled(1);

    int restItemCount = 0;

    QString shellCommand;
    QString shellReturn;

    // first, check CURRENT backup as compared to directory for missing items.

    shellCommand = "rdiff-backup --compare-at-time now \"" + restoreFolder + "\" \"" + targetDevice +
            "/wasta-backup/" + machine + missingDir + "\" 2>&1 | grep 'deleted: ' | { grep -v '/' || true; }";
    shellReturn = shellRun(shellCommand,false);

    // loop through results of compare-at-time now
    QStringList compList = shellReturn.split("\n");

    //re-initialize restItemList
    restItemList.resize(0);
    restItemList.resize(compList.count());

    QString compItem;

    QString folderDesc;
    QString restItemNameDisp;
    QString restItemName;
    QString restItemTime;
    QDir itemPath;
    QString modifiedTime;
    QFileInfo compInfo;

    foreach (compItem, compList) {
        // load up item, but first need to know if it is a folder or file.
        // throw away the Thumbs.db: it will crash when attempting to get date, etc.
        if ( compItem != "" ) {
            // may be empty line return
            // first, trim off "deleted: "
            compItem.replace("deleted: ","");

            //check if folder or not
            itemPath.setPath(targetDevice + "/wasta-backup/" + machine + missingDir+ "/" + compItem);
            if ( itemPath.exists()) {
                //missing item is folder
                folderDesc = " (folder)";
            } else {
                folderDesc = "";
            }
            compInfo.setFile(targetDevice + "/wasta-backup/" + machine + missingDir+ "/" + compItem);

            // get date and time for display
            modifiedTime = compInfo.lastModified().toString("yyyy-MM-dd  hh:mm  -  ");
            restItemList[restItemCount].insert(0, missingDir + "/" + compItem);
            restItemList[restItemCount].insert(1, "now");

            ui->delList->insertItem(restItemCount, modifiedTime + compItem + folderDesc);
            restItemCount++;
        }
    }

    // now, search through increments directory for additional missing items

    // get listing of files that don't contain ".missing": this will be all available increments (but many will be existing)
    // sh needs the {} escaped with a \.  Qt needs \ escaped with \\.
    shellCommand = "ls -r \"" + incDir + "\" | grep '[0-9]\\{4\\}-[0-9]\\{2\\}-[0-9]\\{2\\}T[0-9]\\{2\\}.*\\(snapshot\\|dir\\)' | { grep -v 'missing' || true; }";
    shellReturn = shellRun(shellCommand,false);

    shellReturn.replace(incDir + "/","");
    // get colons back in datetime
    shellReturn.replace(";058",":");

    writeLog("Shell return cleaned up: " + shellReturn);

    QStringList incList = shellReturn.split("\n");

    // increase size of restItemList to be safe
    restItemList.resize(compList.count() + incList.count());

    QString incItem;
    QString prevFileName = "";

    ui->delList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    foreach (incItem , incList) {

        // parse up this long string to fill missing item, date info so can retrieve later.
        // just take first entry for each file/folder (likely multiple, but sorted so "newest first" due to -r on the ls listing)

        int startTimeStamp = incItem.lastIndexOf(QRegExp("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}[:-][0-9]{2}"));
        restItemNameDisp = incItem.mid(0,startTimeStamp - 1);
        restItemName = ui->delFolder->text() + "/" + restItemNameDisp;
        restItemTime = incItem.mid(startTimeStamp,25);

        if ( (restItemName != prevFileName) & (restItemName != "") ) {
            // check if doesn't exist (increment could be a change not a delete).  If not, add to list!
            if ( !QFile::exists(restItemName)) {

                bool restItemExists = false;

                // Confirm not already in restItemList first
                int row;
                for (row = 0; row < restItemCount; row++) {
                    if ( restItemList[row].value(0) == restItemName ) {
                        // flag it: shouldn't add to list because already exists
                        restItemExists = true;
                    }
                }

                if ( restItemExists == false ) {

                    restItemList[restItemCount].insert(0, restItemName);
                    restItemList[restItemCount].insert(1, restItemTime);

                    if ( incItem.endsWith(".dir")) {
                        //missing item is folder
                        folderDesc = " (" + tr("Folder") + ")";
                    } else {
                        folderDesc = "";
                    }

                    ui->delList->insertItem(restItemCount, restItemTime.mid(0,10) + "  " + restItemTime.mid(11,5) +
                                            "  -  " + restItemNameDisp + folderDesc);
                    restItemCount++;
                }
            }
            prevFileName = restItemName;
        }
    }

    if ( restItemCount > 0 ) {
        ui->delList->setEnabled(1);
        ui->delListLabel->setEnabled(1);
        ui->restoreButton->setEnabled(1);

    } else {
        QTest::qWait(1);
        QMessageBox::information(this, tr("None Found"), "<b>" + tr("No deleted items found in backup:") + "</b><p>" +
                                 ui->delFolder->text());
        writeLog("No deleted items found on " + ui->targetDeviceDisp->text() + " in backup folder for restore");
    }
}

void MainWindow::on_restoreButton_clicked()
{
    QString rdiffCommand;
    QString rdiffReturn;
    int row;

    //clear restItem list;
    restItems.clear();

    //clear configSave
    configSave = "";

    ui->restoreButton->setEnabled(0);
    ui->changeDeviceButton->setEnabled(0);
    ui->backupTab->setEnabled(0);
    ui->restTypeFrame->setEnabled(0);
    ui->restorePageWidget->setEnabled(0);
    ui->cancelRestoreButton->setEnabled(1);

    int progress = 10;
    ui->progressBar->setValue(progress);
    ui->progressBar->setVisible(1);

    // clear visibal parts of message output window
    ui->messageOutput->append("");
    ui->messageOutput->moveCursor(QTextCursor::End);
    ui->messageOutput->append("<b>" + tr("Starting restore from:") + "</b><p>" + ui->targetDeviceDisp->text());
    writeLog("Starting restore from " + ui->targetDeviceDisp->text());
    QTest::qWait(2000);

    processCanceled = false;

    //ui->messageOutput->append("");
    switch (ui->restorePageWidget->currentIndex())
    {
    case 0: {
        // restore PREV
        row = ui->prevListCombo->currentIndex();

        renameText = "-SAVE-" + QDate::currentDate().toString("yyyy-MM-dd") +
                "-" + QTime::currentTime().toString("HH:mm:ss");

        // renameRestore will take care of adding to restItems list for undo
        renameRestoreItem(restItemList[row].value(0) , restItemList[row].value(1) , "");

        progress = progress + 10;
        ui->progressBar->setValue(progress);
    }
        break; // break case 0

    case 1: {
        // restore DELETED
        if ( ui->delList->selectedItems().count() == 0 ) {
            QTest::qWait(1);
            QMessageBox::warning(this, tr("No Items Selected"), "<b>" + tr("No deleted items selected for restore") + "</b>");
            writeLog("No deleted items selected for Restore.");
            break; // break out of switch;
        }
        for (row = 0; row < ui->delList->count(); row++) {
            if ( ui->delList->item(row)->isSelected() ) {
                // we want to restore this item

                // double check item DOESN't exist before restore
                if ( !QFile::exists(restItemList[row].value(0)) ) {

                    // Adding extra check to ensure user didn't cancel BEFORE rdiff called
                    if (processCanceled) {
                        break; // break out of backup loop
                    }
                    ui->messageOutput->append("");
                    ui->messageOutput->append("<b>" + tr("Restoring item:") + "</b> " + restItemList[row].value(0));
                    writeLog("Restoring " + restItemList[row].value(0));
                    rdiffCommand = "rdiff-backup --restore-as-of " + restItemList[row].value(1) +
                            " \"" + targetDevice + "/wasta-backup/" + machine + restItemList[row].value(0) +
                            "\" \"" + restItemList[row].value(0) + "\" 2>&1";
                    rdiffReturn = shellRun(rdiffCommand,true);
                    if (processCanceled) {
                        break; // break out of for loop;
                    }
                    // if NOT canceled, add to restItems list for undo
                    restItems.append(restItemList[row].value(0));

                    progress = progress + 10;
                    ui->progressBar->setValue(progress);

                } else {
                    //Item Exists: no restore
                    QTest::qWait(1);
                    QMessageBox::warning(this, tr("Error"), "<b>" + tr("Item already exists:") + "</b><p>" + restItemList[row].value(0) +
                                         "<p><b>" + tr("No restore done") + "</b>");
                    writeLog("Item already exists: " + restItemList[row].value(0) + "No restore done!");
                    processCanceled = true;
                    break; // break out of for loop;
                }
            }
        }
        if (processCanceled) {
            break; // break out of switch
        }
    }
        break; // break case 1

    case 2: {
        // restore ALL
        machine = ui->machineCombo->currentText();
        QString restUser = ui->restUserCombo->currentText();

        // set userBackupConfigDir
        QString userBackupConfigDir = targetDevice + "/wasta-backup/" + machine + "/wasta-backup-config-" + restUser + "/";

        renameText = "-SAVE-" + QDate::currentDate().toString("yyyy-MM-dd") + "-" +
                QTime::currentTime().toString("HH:mm:ss");

        // Restore config files (will also need to undo this later)

        // trim off trailing "/" and add renameText
        configSave = configDir.mid(0,configDir.length()-1) + renameText;

        // Backup exising config files.  Undo last restore will have to undo this also.
        QDir path(configDir);
        // rename it
        bool checkRename = path.rename(configDir, configSave);
        if ( !checkRename ) {
            //failed to rename
            QTest::qWait(1);
            QMessageBox::warning(this, tr("Error"), "<b>" + tr("Error Renaming Item") + "<p>   " + tr("Original Name:") + "</b> " + configDir +
                                 "<p>   <b>" + tr("New Name:") + "</b> " + configSave + "<p><b>" + tr("Is the item opened?") + "</b>");
            writeLog("Error in renaming item: " + configDir + " to " + configSave +
                     ": is the item opened?");
            return;
        }
        writeLog("Existing Configuration Files backed up to: " + configSave);

        // copy down config files from backup
        ui->messageOutput->append(tr("Restoring configuration files from backup:") + " " + userBackupConfigDir);
        writeLog("Restoring Configuration Files from Backup Directory: " + userBackupConfigDir);

        //use rsync to copy down backupConfigDir to configDir
        QString output;

        output = shellRun("rsync -rlt --delete \"" + userBackupConfigDir + "\" \"" + configDir + "\"", false);

        // reload config files
        ui->messageOutput->append(tr("Loading restored configuration files"));
        loadConfigFiles();

        // reset restItemList
        restItemList.resize(0);
        restItemList.resize(backupDirList.count());

        // load up restItemList from backupDirs folder
        for (row = 0; row < backupDirList.count(); row++) {
            // need to use restUser for backup name, but current user for dest name
            QString destDir = backupDirList[row].value(1).replace("$HOME",getenv("HOME"));

            // load up restItemList (for later undo)
            restItemList[row].insert(0, destDir);
            restItemList[row].insert(1, "now");

            // renameRestore will take care of adding to restItems list for undo
            renameRestoreItem(destDir, "now", restUser);

            if (processCanceled) {
                break; // break out of for loop
            }
            progress = progress + 10;
            ui->progressBar->setValue(progress);
        }
    }
        break; // break case 2
    } // end case

    if ( !processCanceled ) {
        ui->messageOutput->append("");
        ui->messageOutput->append("<b>" + tr("Restore complete") + "</b>");
        writeLog("Restore complete.");
        ui->messageOutput->moveCursor(QTextCursor::End);
    } else {
        ui->messageOutput->append("");
        ui->messageOutput->append("<b>" + tr("Restore canceled") + "<b/>");
        writeLog("Restore Canceled");
        ui->messageOutput->moveCursor(QTextCursor::End);
    }

    // reset processCanceled so won't report as canceled for sync commands
    processCanceled = false;

    // regardless of canceled or not, need to sync hdd (this ensures all written to disk: thanks to jl)
    rdiffReturn = shellRun("sync",false);

    // recommended to do it twice (also thanks to jl :)
    rdiffReturn = shellRun("sync",false);

    ui->progressBar->setValue(100);

    ui->cancelRestoreButton->setEnabled(0);
    ui->restoreButton->setEnabled(1);
    ui->changeDeviceButton->setEnabled(1);
    ui->backupTab->setEnabled(1);
    ui->undoLastRestoreButton->setEnabled(1);
    ui->restTypeFrame->setEnabled(1);
    ui->restorePageWidget->setEnabled(1);
}

void MainWindow::renameRestoreItem(QString originalItem, QString restoreTime, QString restUser)
{
    QString newItem = "";

    if ( QFile::exists(originalItem) ) {

        if ( originalItem.mid(originalItem.length()-1,1) == "/" ) {
            // rename current item (trim off trailing "/" from destDir)
            newItem = originalItem.mid(0,originalItem.length()-1) + renameText;
        } else {
            newItem = originalItem + renameText;
        }

        QDir path(originalItem);
        // rename it
        bool checkRename = path.rename(originalItem, newItem);
        if ( !checkRename ) {
            //failed to rename
            QTest::qWait(1);
            QMessageBox::warning(this, tr("Error"), "<b>" + tr("Error Renaming Item") + "<p>   " + tr("Original Name:") + "</b> " + configDir +
                                 "<p>   <b>" + tr("New Name:") + "</b> " + configSave + "<p><b>" + tr("Is the item opened?") + "</b>");
            writeLog("Error in renaming item: " + originalItem + " to " + newItem +
                     ": is the item opened?");
            return;
        }
    }
    //now, shouldn't exist before rdiff restore

    // double check item DOESN'T exist before restore
    if ( !QFile::exists(originalItem) ) {

        QString backupItem;

        if ( restUser.isEmpty() ) {
            backupItem = targetDevice + "/wasta-backup/" + machine + originalItem;
            writeLog("restUser not entered. backupItem to restore: " + backupItem);
        } else {
            // need to duplicate originalItem or else originalItem modified by replace
            QString backupOriginalItem = originalItem;
            backupItem = targetDevice + "/wasta-backup/" + machine + backupOriginalItem.replace(userID,restUser);
            writeLog("restUser exists. backupItem to restore: " + backupItem);
        }

        // CONFIRM item to restore on backup: COULD exist only in "previous time" in the case
        // that the item was first restored from deletion THEN before a new backup was made
        // user requested to restore a previous version.
        //
        //  Attempt restore if EITHER of the following:
        //      1. if restoreTime == now: if it is in the current backup (don't try
        //         rdiff-backup --list-at-time now since if not found it will error)
        //      2. if restoreTime <> now: if rdiff-backup --list-at-time restoreTime
        //         exists since may not be in current backup but only in previous backups

        QString willRestore;

        if (restoreTime == "now") {
            // check if backup item exists
            if (QFile::exists(backupItem)) {
                willRestore = "true";
            } else {
                willRestore = "false";
            }
        } else {
            QString rdiffCommand = "rdiff-backup --list-at-time " + restoreTime + " \"" +
                    backupItem + "\" 2>&1";
            //!!!test
            QString shellReturn = shellRun(rdiffCommand,false);

            if (processCanceled) {
                return;
            }

            if (shellReturn.isEmpty()) {
                willRestore = "false";
            } else {
                willRestore = "true";
            }
        }

        if ( willRestore == "true") {
            // TODO: item may not exist in root of backup but COULD exist in rdiff-backup-data
            // EXAMPLE: person restored a DELETED file, then wants to restore a PREVIOUS
            //          VERSION of the file, BEFORE a new backup is done of the file (so
            //          the "current root of the backup" still would NOT have the file.
            // 2018-11-16 TESTING if don't check for item existing is OK

            // do restore
            ui->messageOutput->append("");
            ui->messageOutput->append("<b>" + tr("Restoring item:") + "</b> " + originalItem);
            writeLog("Restoring " + originalItem);

            // Adding extra check to ensure user didn't cancel BEFORE rdiff called
            if (processCanceled) {
                return;
            }

            // Ensure path exists: ridff-backup will fail if path not existing already
            shellRun("mkdir -p \"" + originalItem + "\"",false);

            // Do Restore
            QString rdiffCommand = "rdiff-backup --restore-as-of " + restoreTime + " \"" +
                    backupItem + "\" \"" + originalItem + "\" 2>&1";
            QString rdiffReturn = shellRun(rdiffCommand,true);

            if (processCanceled) {
                return;
            }
            // don't add to restItems if canceled, otherwise add for later undo

            if ( !newItem.isEmpty() ) {
                //adding newItem (has renameText appended) to restItems so know what to get rid of later
                restItems.append(newItem);
            } else {
                //originalItem added to restItems w/o renameText at end will mean just delete on "undo last restore"
                restItems.append(originalItem);
            }
        } else {
            //backup not found on backup device: no restore done
            if ( ! restUser.isEmpty() ) {
                // For "Restore ALL", possible (likely) that item may not exist: because something like
                //    ~/Paratext9Projects may have never existed but is part of default backup list
                // So, don't give warning message for Restore ALL.
            } else {
                QTest::qWait(1);
                QMessageBox::warning(this, tr("Warning"), "<b>" + tr("Item not found:") + "</b><p>" + originalItem + "<p><b>" + tr("No restore done") + "</b>");
                writeLog("Item: " + originalItem + " not found on " + targetDevice + ".  No restore done");
            }
            return;
        }
    } else {
        //Item Exists: no restore
        QTest::qWait(1);
        QMessageBox::warning(this, tr("Error"), "<b>" + tr("Item already exists:") + "</b><p>" + originalItem + "<p><b>" + tr("No restore done") + "</b>");
        writeLog("Item: " + originalItem + " already exists.  No restore done!");
        return;
    }
}

void MainWindow::on_openRestoreFolderButton_clicked()
{
    QDesktopServices::openUrl(QUrl("file://" + restoreFolder));
}

void MainWindow::on_restoreAllCheck_stateChanged(int arg1)
{

    // Start with GUI elements disabled
    ui->restoreButton->setEnabled(0);
    ui->machineCombo->setEnabled(0);
    ui->machineLabel->setEnabled(0);
    ui->restUserCombo->setEnabled(0);
    ui->restUserLabel->setEnabled(0);
    ui->machineCombo->clear();
    ui->restUserCombo->clear();
    ui->openRestoreFolderButton->setEnabled(0);

    if (arg1 == 2) {
        // restoreALLCheck is checked: allow restore ALL;

        // populate machineCombo
        if ( QFile::exists(targetDevice + "/wasta-backup/") ) {

            QString shellCommand = "ls -1 \"" + targetDevice + "/wasta-backup/\"";
            QString shellReturn = shellRun(shellCommand,false);

            QStringList machList = shellReturn.split("\n");
            QString machItem;

            foreach (machItem , machList) {
                if ( machItem != "") {
                    //load it up
                    ui->machineCombo->addItem(machItem);
                }
            }

            if ( ui->machineCombo->count() == 0) {
                // no backup machines on device.
                ui->restUserCombo->setEnabled(0);
                ui->machineCombo->setEnabled(0);
            }
        }
    }
}

void MainWindow::on_undoLastRestoreButton_clicked()
{
    ui->changeDeviceButton->setEnabled(0);
    ui->backupTab->setEnabled(0);
    ui->restoreTab->setEnabled(0);
    ui->undoLastRestoreButton->setEnabled(0);

    // clear visibal parts of message output window
    ui->messageOutput->append("");
    ui->messageOutput->append("<b>" + tr("Starting undo") + "</b>");
    ui->messageOutput->moveCursor(QTextCursor::End);
    writeLog("Starting Undo Last Restore");
    QTest::qWait(2000);

    ui->progressBar->setVisible(1);
    ui->progressBar->setValue(0);

    int progress = 10;
    ui->progressBar->setValue(progress);

    QString itemName;
    QString origItemName;

    foreach (itemName, restItems) {
        // check if end of item matches renameText (indicating need to delete current plus restore renamed item)
        if ( itemName.endsWith(renameText)) {
            origItemName = itemName;
            origItemName.chop(renameText.length());
            ui->messageOutput->append("");
            ui->messageOutput->append("<b>" + tr("Undoing restore:") + "</b> " + origItemName);
            ui->messageOutput->moveCursor(QTextCursor::End);
            writeLog("Undoing restore of " + origItemName);
            QTest::qWait(2000);

            // check that item exists first (could have been deleted?),
            // then delete origItem, then rename item to origItem
            if ( QFile::exists(itemName)) {

                QFileInfo item;
                item.setFile(itemName);

                if ( item.isFile() ) {
                    // file processing
                    if ( QFile::remove(origItemName) ) {
                        if ( QFile::rename(itemName, origItemName) ) {
                            // remove and rename successful
                            ui->messageOutput->insertPlainText(". . " + tr("Done"));
                            ui->messageOutput->moveCursor(QTextCursor::End);
                            writeLog("File " + origItemName + " replaced from saved version " + itemName + ".");
                            progress = progress + 10;
                            ui->progressBar->setValue(progress);
                        } else {
                            // rename unsuccessful
                            ui->messageOutput->append("<b>" + tr("Error:") + "</b>");
                            writeLog("Error: File " + origItemName + " removed but " + itemName +
                                     " unable to be renamed to " + origItemName + "!");
                            QTest::qWait(1);
                            QMessageBox::warning(this,tr("Error"),"<b>" + tr("Error") + " - " +tr("Original item removed:") + "</b><p>" +
                                                 origItemName + "<p><b>" + tr("New item unable to be renamed:")
                                                 + "</b><p>" + itemName);
                            break; // foreach
                        }
                    } else {
                        // remove unsuccessful
                        ui->messageOutput->append("");
                        ui->messageOutput->append("<b>" + tr("Error") + "!</b>");
                        writeLog("Error: File " + origItemName + " unable to be removed. " + itemName +
                                 " not attempted to be renamed to " + origItemName + "!");
                        QTest::qWait(1);
                        QMessageBox::warning(this,tr("Error"),"<b>" + tr("Original item unable to be removed:") + "</b><p>" + origItemName);
                        break; // foreach
                    }

                } else {
                    // item exists and is folder
                    if ( removeDir(origItemName) ) {
                        QDir directory;
                        directory.setPath(itemName);
                        // now rename
                        if ( directory.rename(itemName, origItemName) ) {
                            // rename successful
                            ui->messageOutput->insertPlainText(". . " + tr("Done"));
                            ui->messageOutput->moveCursor(QTextCursor::End);
                            writeLog("Folder " + origItemName + " replaced from saved version " + itemName + ".");
                            progress = progress + 10;
                            ui->progressBar->setValue(progress);
                        } else {
                            //rename unsucessful
                            ui->messageOutput->append("");
                            ui->messageOutput->append("<b>" + tr("Error") + "</b>");
                            writeLog("Error: Folder " + origItemName + " removed but " + itemName +
                                     " unable to be renamed to " + origItemName + "!");
                            QTest::qWait(1);
                            QMessageBox::warning(this,tr("Error"),"<b>" + tr("Original item removed:") + "</b><p>" + origItemName + "<p><b>" + tr("New item unable to be renamed:")
                                                 + "</b><p>" + itemName);
                            break; // foreach
                        }
                    } else {
                        // remove unsucessful (no rename attempted)
                        ui->messageOutput->append("");
                        ui->messageOutput->append("<b>" + tr("Error") + "!</b>");
                        writeLog("Error: Folder " + origItemName + " unable to be removed. " + itemName +
                                 " not attempted to be renamed to " + origItemName + "!");
                        QTest::qWait(1);
                        QMessageBox::warning(this, tr("Error"),"<b>" + tr("Original item unable to be removed:") + "</b><p>" + origItemName);
                        break; // foreach
                    }
                }
            } else {
                // file doesn't exist: can't remove and rename for undo
                ui->messageOutput->append("");
                ui->messageOutput->append("<b>" + tr("Error") + "</b>");
                writeLog("Error: Item " + itemName + " doesn't exist so no undo restore possible!");
                QTest::qWait(1);
                QMessageBox::warning(this,tr("Error"),"<b>" + tr("Item doesn't exist so undo is not possible:") + "</b><p>" + itemName);
                break; // foreach
            }
        } else {
            // else (no renameText at end of item) then was just a restored 'delete'
            // so undo just deletes current.
            ui->messageOutput->append("");
            ui->messageOutput->append(tr("Undoing restore:") + " " + itemName);
            QTest::qWait(2000);
            ui->messageOutput->moveCursor(QTextCursor::End);

            if ( QFile::exists(itemName)) {
                QFileInfo item;
                item.setFile(itemName);

                if ( item.isFile() ) {
                    // file delete
                    if ( QFile::remove(itemName) ) {
                        // success deleting
                        ui->messageOutput->insertPlainText(". . " + tr("Done"));
                        writeLog("File " + itemName + " deleted.");
                        ui->messageOutput->moveCursor(QTextCursor::End);
                        progress = progress + 10;
                        ui->progressBar->setValue(progress);
                    } else {
                        // remove FILE failed
                        ui->messageOutput->append("");
                        ui->messageOutput->append("<b>" + tr("Error") + "</b>");
                        writeLog("ERROR: File " + itemName + " unable to be removed!");
                        QTest::qWait(1);
                        QMessageBox::warning(this,tr("Error"),"<b>" + tr("Item unable to be removed:") + "</b><p>" + itemName);
                        break; // foreach
                    }
                } else {
                    // folder delete
                    if ( removeDir(itemName) ) {
                        // success deleting
                        ui->messageOutput->insertPlainText(". . " + tr("Done"));
                        ui->messageOutput->moveCursor(QTextCursor::End);
                        writeLog("Folder " + itemName + " deleted.");
                        progress = progress + 10;
                        ui->progressBar->setValue(progress);
                    } else {
                        // remove FOLDER failed
                        ui->messageOutput->append("");
                        ui->messageOutput->append("<b>" + tr("Error") + "</b>");
                        writeLog("Error: Folder " + itemName + " unable to be removed!");
                        QTest::qWait(1);
                        QMessageBox::warning(this,tr("Error"),"<b>" + tr("Item unable to be removed:") + "</b><p>" + itemName);
                        break; // foreach
                    }
                }
            } else {
                // item doesn't exist can't delete
                ui->messageOutput->append("");
                ui->messageOutput->append("<b>" + tr("Error") + "</b>");
                writeLog("Error: Item " + itemName + " doesn't exist so no need to undo restore!");
                QTest::qWait(1);
                QMessageBox::warning(this,tr("Error"),"<b>" + tr("Item doesn't exist so undo is not possible:") + "</b><p>" + itemName);
                break; // foreach
            }
        } // end else (no renameText at end of item)
    } // foreach


    //Last, check if configSave found (configDir + renameText).  If so, that is last thing to undo.
    if ( !configSave.isEmpty()) {

        QDir configSaveDir(configSave);
        if ( configSaveDir.exists() ) {
            ui->messageOutput->append("");
            ui->messageOutput->append(tr("Undoing restore of configuration files"));
            ui->messageOutput->moveCursor(QTextCursor::End);
            writeLog("Undoing restore of Config Files.  Will replace from: " + configSave);

            // delete current configDir, restore renameConfigDir to configDir and reload backupDirList
            bool deleted = removeDir(configDir);
            if (deleted) {
                //rename configSave to configDir
                bool checkRename = configSaveDir.rename(configSave, configDir);
                if ( !checkRename ) {
                    //failed to rename
                    QTest::qWait(1);
                    QMessageBox::warning(this, tr("Error"), "<b>" + tr("Error Renaming Item") + "<p>   " + tr("Original Name:") + "</b> " + configSave +
                                         "<p>   <b>" + tr("New Name:") + "</b> " + configDir + "<p><b>" + tr("Is the item opened?") + "</b>");
                    writeLog("Error in renaming item: " + configSave + " to " + configDir +
                             ": is the item opened?");
                    return;
                }
                //reload Config Files
                ui->messageOutput->append("");
                ui->messageOutput->append(tr("Loading original configuration files"));
                loadConfigFiles();
            } else {
                //error: remove of configDir didn't work
                writeLog("Error: Folder " + configDir + " unable to be removed!");
                QTest::qWait(1);
                QMessageBox::warning(this,tr("Error"),"<b>" + tr("Item unable to be removed:") + "</b><p>" + configDir);
                return;
            }
        }
    }
    ui->messageOutput->append("");
    ui->messageOutput->append("<b>" + tr("Undo Complete") + "</b>");
    ui->messageOutput->moveCursor(QTextCursor::End);
    writeLog("Undo Last Restore Complete");
    ui->progressBar->setValue(100);

    ui->changeDeviceButton->setEnabled(1);
    ui->backupTab->setEnabled(1);
    ui->restoreTab->setEnabled(1);
}

// ##########################################################################
// #### MISCELLANEOUS PROCEDURES                                         ####
// ##########################################################################

void MainWindow::writeLog(QString data)
{
    logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    QTextStream stream(&logFile);
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n" + data + "\n\n";
    stream.flush();
    logFile.close();
}

QString MainWindow::shellRun(QString command, bool giveFeedback)
{
    //QString output;

    QString shellReturn;

    writeLog("shellCommand:\n" + command);

    QProcess *shellProcess = new QProcess();
    shellProcess->start("sh", QStringList() << "-c" << command + " 2>&1");

    if (giveFeedback) {
        while (shellProcess->pid() > 0 ) {
            ui->messageOutput->moveCursor(QTextCursor::End);
            ui->messageOutput->insertPlainText(". ");

            QTest::qWait(2000);
        }
    }

    shellProcess->waitForFinished(-1);
    QFileInfo item;
    shellReturn = shellProcess->readAll();

    writeLog("shellReturn:\n" + shellReturn);

    if (processCanceled) {
        //ui->messageOutput->append("<b>" + tr("Canceled!") + "<b/>");
        writeLog ("Process Canceled!");
    } else if ( shellProcess->exitCode() == 0 ) {
        ui->messageOutput->moveCursor(QTextCursor::End);
        if (giveFeedback) {
            ui->messageOutput->insertPlainText(". " + tr("Done"));
        }
    } else {
        QTest::qWait(1);

        // Handle certain errors, else give generic output

        if ( shellReturn.indexOf("No space left on device") > -1) {
            QMessageBox::critical(this, tr("Backup Location Full"),"<b>" + tr("Backup location full:") + "</b><p>" +
                                  ui->targetDeviceDisp->text() + "<p></b>" + tr("Backup not complete") + "</b>");
            ui->messageOutput->append("<b>" + tr("Backup location full:") + "</b> " +
                                      ui->targetDeviceDisp->text());
        } else if ( shellReturn.indexOf("hard linking not supported by filesystem") > -1 ) {
            // Hard Links are not able to be used on FAT or NTFS drives.  In this case, the
            //   files are fully duplicated instead of linked.  Not an error.
            shellReturn = "";
            ui->messageOutput->moveCursor(QTextCursor::End);
            if (giveFeedback) {
                ui->messageOutput->insertPlainText(". " + tr("Done") + "\n");
            }
        } else {
            // Standard message if no other error handled
            QMessageBox::critical(this, tr("Error"), "<b>" + tr("Error during shell process") + "<p>" + tr("Error:") + "</b><p>" +
                                  QString::number(shellProcess->exitCode()) + "<p><b>" + tr("Command:") + "</b><p>" +
                                  command + "<p><b>" + tr("Message:") + "</b><p>" + shellReturn);
            ui->messageOutput->append("<b>" + tr("Error") + "</b>");

            writeLog("Error during shell process!\n\nError: " + QString::number(shellProcess->exitCode()) +
                     "\n\nCommand: " + command + "\n\nMessage: " + shellReturn);

            // set processCanceled so any remaining processes will be canceled also.
            cancelProcess();
        }
    }
    ui->messageOutput->moveCursor(QTextCursor::End);

    return shellReturn;
}

void MainWindow::loadConfigFiles() {

    writeLog("Loading Configuration Files from: " + configDir);

    QString line;

    // Load up prevBackupDev file
    prevBackupDevFile.open(QIODevice::ReadOnly);
    QTextStream prevBackupDevStream(&prevBackupDevFile);
    prevBackupDevice = prevBackupDevStream.readLine();
    prevBackupDevStream.flush();
    prevBackupDevFile.close();

    //load backupDir file
    //resize for safety
    QString shellCommand;
    QString backupDirText;
    QString backupDir;
    QString backupDirDisplay;

    backupDirList.resize(0);
    backupDirList.resize(20);

    int i = 0;

    backupDirFile.open(QIODevice::ReadOnly);
    QTextStream textStream(&backupDirFile);
    while (true)
    {
        line = textStream.readLine();
        if (line.isNull()) {
            break;
        } else {
            if ( ( !line.startsWith("#")) & (line.trimmed() != "") ) {
                // if starts with # or is blank, then comment line: don't evaluate
                backupDirList[i] = line.split(",");
                backupDirText = backupDirList[i].value(1);
                if ( backupDirText.endsWith("/") ) {
                    // throw away trailing "/" so compares work easier later
                    backupDirText.chop(1);
                    backupDirList[i].replace(1, backupDirText);
                }
                if (backupDirText.contains("xdg-user-dir") ) {
                    // dynamically generate backup directory
                    shellCommand = backupDirText;
                    backupDir = shellRun(shellCommand, false).trimmed();
                    if ( backupDir == getenv("HOME") ) {
                        // don't use backup: not found since returned $HOME
                        backupDirList.remove(i);
                        QMessageBox::information(this, tr("Warning"), "<b>" + tr("Backup configuration file error") +
                                                 "<p>" + tr("Backup item:") + "</b><p>" + backupDirText + "<p><b>" +
                                                 tr("Check configuration file:") + "</b><p>" + backupDirFile.fileName() +
                                                 "<p><b>" + tr("This location will NOT be backed up!") + "</b>");
                        writeLog("backupDir: " + backupDir + " REMOVED since matches user's $HOME");
                    } else {
                        backupDirDisplay = backupDir.mid(backupDir.lastIndexOf("/") + 1);
                        backupDirList[i].replace(1, backupDir);
                        backupDirList[i].replace(0, backupDirDisplay);
                        writeLog("backupDir: " + backupDir + " configured from " + backupDirText);
                    }
                }
                i++;
                if ( i == backupDirList.count() )
                    backupDirList.resize(i+20);
            }
        }
    }
    textStream.flush();
    backupDirFile.close();

    // trim down vector
    backupDirList.resize(i);
}

bool MainWindow::removeDir(const QString & dirName)
{
    // taken from web
    bool result = false;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden |
                                                    QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

void MainWindow::on_machineCombo_currentIndexChanged(const QString machineValue)
{
    QString shellCommand;
    QString shellReturn;

    ui->restUserCombo->clear();
    if ( ! machineValue.isEmpty()) {
        // populate restUserCombo
        shellCommand = "ls -1a \"" + targetDevice + "/wasta-backup/" + machineValue + "\" | grep wasta-backup-config-";
        shellReturn = shellRun(shellCommand,false);
        QStringList userList = shellReturn.split("\n");
        QString userItem;

        foreach (userItem , userList) {
            if ( userItem != "") {
                //load it up
                //20 chars long ==> wasta-backup-config-, get username after it in item name
                ui->restUserCombo->addItem(userItem.mid(20));
            }
        }

        if ( ui->restUserCombo->count() == 0 ) {
            // no backup user config found, can't restore ALL.
        } else {
            // enable GUI elements
            ui->machineCombo->setEnabled(1);
            ui->machineLabel->setEnabled(1);
            ui->restUserCombo->setEnabled(1);
            ui->restUserLabel->setEnabled(1);

            ui->restoreButton->setEnabled(1);
            restoreFolder = getenv("HOME");
            ui->openRestoreFolderButton->setEnabled(1);
        }
    }
}
