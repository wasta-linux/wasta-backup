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
QFile useBackupIncludeFilterFile;
QString prevBackupDevice;
QString prevBackupDate;
QFile prevBackupDevFile;
QFile prevBackupDateFile;
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

    QDir configPath(configDir);

    if ( !configPath.exists() ) {
        configPath.mkpath(configDir);
    }

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

    // Check for backupInclude file
    if ( !QFile::exists(configDir + "backupInclude.txt") ) {
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
                "- **\n";

        // open it
        QFile file(configDir + "backupInclude.txt");

        file.open(QIODevice::ReadWrite);

        writeLog("No " + configDir + "backupInclude.txt: creating it.");

        QTextStream stream(&file);
        stream << include;
        stream.flush();
        file.close();
    }

    // Check for backupDirs file
    backupDirFile.setFileName(configDir + "backupDirs.txt");

    if ( !backupDirFile.exists() ) {
        // create it
        QString include =
                "#Folder Display Name,Folder Location,useInclude,Additional rdiff-backup Parms,Remove Older Than\n"
                "#  useInclude = YES ==> only include specified filetypes in backupInclude.txt\n"
                "#  useInclude = NO  ==> don't limit backup to specified filetypes in backupInclude.txt\n"
                "#  Remove Older Than options:\n"
                "#          30B means keep versions from the past 30 backup sessions, delete older ones\n"
                "#          3M to remove versions older than 3 months\n"
                "#          1Y to remove versions older than 1 year\n"
                "#          1Y3M to remove versions older than 1 year 3 months\n"
                "\n"
                "Bloom,$HOME/Bloom,NO,,1Y\n"
                "Paratext,$HOME/ParatextProjects,NO,,1Y\n"
                "Paratext8,$HOME/Paratext8Projects,NO,,1Y\n"
                "Fieldworks,$HOME/.local/share/fieldworks/Projects,NO,,1Y\n"
                "WeSay,$HOME/WeSay,NO,,1Y\n"
                "Adapt It,$HOME/Adapt It Unicode Work,NO,--exclude ignorecase:**/.temp,1Y\n"
                "Publications,$HOME/Publications,NO,,1Y\n"
                "Thunderbird,$HOME/.thunderbird,NO,--exclude ignorecase:**/Cache,1Y\n"
                "Documents,$HOME/Documents,YES,,1Y\n"
                "Desktop,$HOME/Desktop,YES,,1Y";

        // open it

        backupDirFile.open(QIODevice::ReadWrite);
        writeLog("No " + configDir + "backupDirs.txt: creating it.");

        QTextStream stream(&backupDirFile);
        stream << include;
        stream.flush();
        backupDirFile.close();
    }

    // Check for useFilter file
    useBackupIncludeFilterFile.setFileName(configDir + "useBackupIncludeFilter.txt");

    if ( !useBackupIncludeFilterFile.exists() ) {
        // create it
        useBackupIncludeFilterFile.open(QIODevice::ReadWrite);
        writeLog("No " + useBackupIncludeFilterFile.fileName() + ": creating it.");

        QTextStream stream(&useBackupIncludeFilterFile);
        stream << "YES";
        stream.flush();
        useBackupIncludeFilterFile.close();
    }

    // Check for prevBackupDev file
    prevBackupDevFile.setFileName(configDir + "prevBackupDevice.txt");

    if ( !prevBackupDevFile.exists() ) {
        // create it ... but leave empty
        prevBackupDevFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
        writeLog("No " + prevBackupDevFile.fileName() + ": creating it (empty).");

        QTextStream stream(&prevBackupDevFile);
        stream << "";
        stream.flush();
        prevBackupDevFile.close();
    }

    // Check for prevBackupDate file
    prevBackupDateFile.setFileName(configDir + "prevBackupDate.txt");

    if ( !prevBackupDateFile.exists() ) {
        // create it ... but leave empty
        prevBackupDateFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
        writeLog("No " + prevBackupDateFile.fileName() + ": creating it (empty).");

        QTextStream stream(&prevBackupDateFile);
        stream << "";
        stream.flush();
        prevBackupDateFile.close();
    }

    // Launch Main Window
    ui->setupUi(this);

    // set app icon (used in 'About')
    QIcon wastaIcon;
    wastaIcon.addFile("/usr/share/icons/hicolor/512x512/apps/wasta-backup.png");
    this->setWindowIcon(wastaIcon);

    // set location to center of primary screen
    QRect screenGeometry = QApplication::desktop()->availableGeometry(QApplication::desktop()->primaryScreen());
    int x = (screenGeometry.width()  - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;

    this->move(x, y);
    this->setWindowTitle(tr("Wasta [Backup]"));
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
            ui->messageOutput->append(tr("USB device attached") + ": " + arguments.value(1).mid(arguments.value(1).lastIndexOf("/") + 1) + "\n");
            setTargetDevice(arguments.value(1));
        }
    }

    if ( targetDevice == "" ) {
        // if argument exists above, but not writable, will get to here also
        // check if prev backup writable, if so, set as target device
        QFileInfo prevBackupInfo(prevBackupDevice + "/wasta-backup");
        if ( prevBackupInfo.isWritable() ) {
            //use it
            writeLog("Previous backup location found: " + prevBackupDevice + " setting as target device.");
            //ui->messageOutput->append(tr("Previous backup device found") + ": " + prevBackupDevice.mid(prevBackupDevice.lastIndexOf("/") + 1) + "\n");
            setTargetDevice(prevBackupDevice);
        } else {
            // use logic to determine preferred destination
            writeLog("No argument or previous backup location found.  Calling setPreferredDestination to set target device.");
            ui->messageOutput->append(tr("Previous backup device not found") + ": " +
                                      prevBackupDevice.mid(prevBackupDevice.lastIndexOf("/") + 1) + "\n");
            setPreferredDestination();
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ##########################################################################
// #### MAIN INTERFACE PROCEDURES                                        ####
// ##########################################################################

void MainWindow::on_actionBackupOnlyImportant_changed()
{
    QString fileText;

    if ( ui->actionBackupOnlyImportant->isChecked() ) {
        fileText = "YES";
        ui->backupIncludeLabel->setText(tr("Picture, Music, and Video files will <b>NOT</b> be included in the Backup.  <b>You must backup those files yourself!</b>"));
    } else {
        fileText = "NO";
        ui->backupIncludeLabel->setText(tr("<b>ALL Files</b> (including Pictures, Music, and Videos) will be included in the Backup."));
    }
    // write to file for next time;
    useBackupIncludeFilterFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    writeLog("Updating " + useBackupIncludeFilterFile.fileName() + ": value: " + fileText);

    QTextStream stream(&useBackupIncludeFilterFile);
    stream << fileText;
    stream.flush();
    useBackupIncludeFilterFile.close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,tr("About Wasta-Backup"),"<h3>" + tr("Wasta-Backup") + "</h3>" +
                       "<p>" + tr("Wasta-Backup is a simple backup GUI using rdiff-backup for version backups of data to an external USB device.") +
                       "<p>" + tr("Wasta-Backup will auto-launch when a USB device with a previous Wasta-Backup on it is inserted.") +
                       "<p>" + tr("Restore possibilities include restoring previous versions of existing files or folders as well as restoring deleted files or folders from the backup device") + ". " +
                                  tr("In the case of restoring previous versions of existing items, the current item is first renamed using the current date and time.") +
                       "<p>" + tr("Additionally, a 'Restore ALL' option is available that will replace all data on the computer from the backup device.") +
                       "<p>" + tr("The following configurable settings are stored in a user's ~/.config/wasta-backup/ directory:") +
                       "<ul>" +
                       "<li><p><b>backupDirs.txt:</b> " + tr("specifies directories to backup and other parameters such as number of versions to keep") + "</li>" +
                       "<li><p><b>backupInclude.txt:</b> " +tr("specifies file extensions to backup (so files with media extensions, etc., will be politely ignored)") + "</li>" +
                       "<a href=\"https://sites.google.com/site/wastalinux/wasta-applications/wasta-backup\">https://sites.google.com/site/wastalinux/wasta-applications/wasta-backup</a>");
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
                // If only 1 wasta-backup folder found, choose it (regardless of space).
                //If more than 1 wasta-backup folder, pick one with most remaining space.
                //if no wasta-backup folders found, pick largest device

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
            QMessageBox::information(this, tr("No Device Found"), tr("No Writable USB device found to backup to!  Please insert USB device and click 'Change'!"));
            writeLog("A target device found, but not writeable.");
        }

    } else {
        // no devices found
        QTest::qWait(1);
        QMessageBox::information(this, tr("No Device Found"), tr("No device found to backup to!  Please insert a USB device and click 'Change'."));
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
                ui->messageOutput->append(tr("Existing backup found on device") + ": " +
                                          ui->targetDeviceDisp->text() + "\n");
                ui->messageOutput->append(tr("Ready for backup"));
                ui->messageOutput->append("\n\n\n");
                ui->messageOutput->moveCursor(QTextCursor::End);
                writeLog(targetDevice + " has existing backup and ready for backup.");

            } else {
                //new backup: display message
                ui->messageOutput->append(tr("No existing backup found on device") + ": " +
                                          ui->targetDeviceDisp->text() + "\n");
                ui->messageOutput->append(tr("Ready for first backup (may take some time to complete)"));
                ui->messageOutput->append("\n\n\n");
                ui->messageOutput->moveCursor(QTextCursor::End);
                writeLog(targetDevice + " doesn't have existing backup but ready for first backup.");
            }

        } else {
            // newTarget not writable, give error
            QTest::qWait(1);
            QMessageBox::warning(this, tr("Choose Again"), tr("Chosen directory") + " " + newTarget + " " +
                                 tr("is not writable.") + " " + tr("Backup Device not changed."));
            writeLog("User can't write to newTarget: " + newTarget);
            // targetDevice and ui.targetDevice.Disp remain unchanged
        }

    } else {
        // no newTarget found to be used.
        QTest::qWait(1);
        QMessageBox::warning(this, tr("Choose Again"), inputDir + " " + tr("not a valid backup device. Try again."));
        writeLog(inputDir + " not a valid backup device.");
    }
}

void MainWindow::on_changeDeviceButton_clicked()
{
    ui->progressBar->setVisible(0);
    QString startTarget;

    // need to fix!!!! to handle /media/akiverson plus /media!

    // if current device has user ID in it, then newer
    if ( targetDevice.indexOf(userID) > 0 ) {
        // on 12.10 +: USB mounted to /media/userid/mount-name
        startTarget = "/media/" + userID;
    } else {
        // on 12.04: USB mounted to /media/mount-name
        startTarget = "/media";
    }
    // file utility to select device.... but trim to just be /media/xxxxx
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Choose Backup Device"),
                                                        startTarget,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    ui->messageOutput->append("\n\n\n\n");
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
    ui->messageOutput->append("\nCanceling!");
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
    QDir path;
    QString output;

    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(1);

    ui->changeDeviceButton->setEnabled(0);
    ui->backupButton->setEnabled(0);
    ui->restoreTab->setEnabled(0);

    ui->cancelBackupButton->setEnabled(1);

    processCanceled = false;

    // clear visible parts of message output window
    ui->messageOutput->append("\n\n\n\n\n\n\n\n\n");
    ui->messageOutput->append(tr("Starting backup to") + " " + ui->targetDeviceDisp->text() + " " + tr("device") + "...\n");
    ui->messageOutput->moveCursor(QTextCursor::End);
    writeLog("Starting backup to " + ui->targetDeviceDisp->text());
    QTest::qWait(2000);

    QString targetDir = targetDevice + "/wasta-backup/" + machine;

    //update targetDevice with config files (to use for Restore ALL instead of an existing machines config files)
    QString backupConfigDir = targetDir + "/wasta-backup-config-" + userID + "/";

    // Ensure backupConfigDir exists
    QDir backupConfigPath(backupConfigDir);
    if ( !backupConfigPath.exists() ) {
        //Legacy: clean out old backupConfigDir location that will cause any rdiff-backup of ~/.config to fail
        //   since before 2013-10-13 backupConfigDir was targetDir + userHome + /.config/wasta-backup
        //   Since this was NOT a "rdiff-backup folder" then rdiff-backup will warn folder exists and will not process
        //   So, need to manually remove .config folder so will not conflict with anyone attempting to backup home
        output = shellRun("rm -rf \"" + targetDir + userHome + "/.config\"", false);

        backupConfigPath.mkpath(backupConfigDir);
    }

    //use rsync to do configDir syncing to targetDevice
    output = shellRun("rsync -rlt --delete \"" + configDir + "\" \"" + backupConfigDir + "\"", false);

    //now proceed with backups
    int progress = 10;
    ui->progressBar->setValue(progress);

    QString rdiffReturn;

    for ( int i = 0; i< backupDirList.count(); i++) {

        // value 3: additional parameters
        parms = stdParms + " " + backupDirList[i].value(3);

        // to use Filter, need backup directory to specify using it PLUS program set to use it.
        if ( (backupDirList[i].value(2) == "YES") & (ui->actionBackupOnlyImportant->isChecked()) ) {
            // value 2=YES: include filetype filter
            parms = parms + " --include-globbing-filelist " + configDir + "backupInclude.txt";
        }

        source = backupDirList[i].value(1).replace("$HOME",getenv("HOME"));
        dest = targetDir + backupDirList[i].value(1).replace("$HOME",getenv("HOME"));

        if ( path.exists(source) ) {
            ui->messageOutput->append(tr("Backing up") + " " + backupDirList[i].value(0) + "....\n");

            //ensure dest path exists
            if ( !path.exists(dest) ) {
                path.mkpath(dest);
            }

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
        ui->messageOutput->append("\n" + tr("Backup Complete"));
        writeLog("Backup Complete");
        ui->messageOutput->moveCursor(QTextCursor::End);

        //update prevBackupDevFile info
        QString fileText = targetDevice;
        prevBackupDevFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
        writeLog("Updating " + prevBackupDevFile.fileName() + ": value: " + fileText);

        QTextStream devStream(&prevBackupDevFile);
        devStream << fileText;
        devStream.flush();
        prevBackupDevFile.close();

        //update prevBackupDateFile info
        fileText = QDate::currentDate().toString("yyyy-MM-dd");
        prevBackupDateFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
        writeLog("Updating " + prevBackupDateFile.fileName() + ": value: " + fileText);

        QTextStream dateStream(&prevBackupDateFile);
        dateStream << fileText;
        dateStream.flush();
        prevBackupDateFile.close();

    } else {
        ui->messageOutput->append("\nBackup Canceled!");
        writeLog("Backup Canceled!");
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
        restItemName = QFileDialog::getOpenFileName(this, tr("Select File or Folder"),
                                                    "/home/" + userID);
        restoreFolder = restItemName.mid(0,restItemName.lastIndexOf("/"));
    } else {
        // open directory dialog
        // add trailing "/"
        restItemName = QFileDialog::getExistingDirectory(this, tr("Select File or Folder"),
                                                         "/home/" + userID);
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
        QMessageBox::information(this, "Not Found", restItemName + "\n\nNot part of backups on " +
                                 ui->targetDeviceDisp->text() + " device.");
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
        startIncDate = incList.value(i).lastIndexOf(QRegExp("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}"));
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
        QMessageBox::information(this, tr("No Items Found"), tr("No previous versions of item") + ":\n\n" + restItemName +
                                 + "\n\n" + tr("found in backups on") + " " + ui->targetDeviceDisp->text() + " " + tr("device."));
        writeLog("No previous versions of item: " + restItemName + " found in backups on " +
                 ui->targetDeviceDisp->text() + " " + tr("device."));
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

    missingDir = QFileDialog::getExistingDirectory(this, tr("Select File or Folder"),
                                                   "/home/" + userID);

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
        QMessageBox::information(this, tr("Not Found"), tr("Backup of") + " " + missingDir +
                                 + " " + tr("not found on device") + " " + targetDevice);
        writeLog("Backup of " + missingDir + " not found on device " + targetDevice);
        return;
    }

    ui->delFolder->setText(missingDir);
    restoreFolder = missingDir;
    ui->openRestoreFolderButton->setEnabled(1);

    int restItemCount = 0;

    QString shellCommand;
    QString shellReturn;

    // first, check CURRENT backup as compared to directory for missing items.

    shellCommand = "rdiff-backup --compare-at-time now \"" + missingDir + "\" \"" + targetDevice +
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
            compItem.replace(tr("deleted") + ": ","");

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

        int startTimeStamp = incItem.lastIndexOf(QRegExp("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}"));
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

                    if ( incItem.endsWith(".dir") ) {
                        //missing item is folder
                        folderDesc = " (" + tr("folder") + ")";
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
        QMessageBox::information(this, tr("None Found"), tr("No deleted items found on") + " " +
                                 ui->targetDeviceDisp->text() + " " + tr("in backup folder for restore"));
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
    ui->messageOutput->append("\n\n\n\n\n\n\n\n\n");
    ui->messageOutput->moveCursor(QTextCursor::End);
    ui->messageOutput->append(tr("Starting restore from") + " " + ui->targetDeviceDisp->text() + " " + tr("device") + "...\n");
    writeLog("Starting restore from " + ui->targetDeviceDisp->text());
    QTest::qWait(2000);

    processCanceled = false;

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
            QMessageBox::warning(this, tr("No Items Selected"), tr("No deleted items selected for Restore."));
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
                    QMessageBox::warning(this, tr("Error"), tr("Item") + ": " + restItemList[row].value(0) +
                                         " " + tr("already exists. NO RESTORE DONE!"));
                    writeLog("Item: " + restItemList[row].value(0) + " already exists.  NO RESTORE DONE!");
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
            QMessageBox::warning(this, tr("Error"), tr("Error in renaming item") + ": " + configDir +
                                 " " + tr("to") + " " + configSave + ": " + tr("is the item opened?"));
            writeLog("Error in renaming item: " + configDir + " to " + configSave +
                     ": is the item opened?");
            return;
        }
        writeLog("Existing Configuration Files backed up to: " + configSave);

        // copy down config files from backup
        ui->messageOutput->append(tr("Restoring Configuration Files from") + " " + targetDevice + " " + tr("for user") + " " + restUser + "\n");
        writeLog("Restoring Configuration Files from Backup Directory: " + userBackupConfigDir);

        //use rsync to copy down backupConfigDir to configDir
        QString output;

        output = shellRun("rsync -rlt --delete \"" + userBackupConfigDir + "\" \"" + configDir + "\"", false);

        // reload config files
        ui->messageOutput->append(tr("Loading Restored Configuration Files") + "\n");
        loadConfigFiles();

        // reset restItemList
        restItemList.resize(0);
        restItemList.resize(backupDirList.count());

        // load up restItemList from backupDirs folder
        for (row = 0; row < backupDirList.count(); row++) {
            // need to user restUser for backup name, but current user for dest name
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
        ui->messageOutput->append("\n" + tr("Restore Complete"));
        writeLog("Restore complete.");
        ui->messageOutput->moveCursor(QTextCursor::End);
    } else {
        ui->messageOutput->append("\n" + tr("Restore Canceled!"));
        writeLog("Restore Canceled!");
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
            QMessageBox::warning(this, tr("Error"), tr("Error in renaming item") + ": " + originalItem +
                                 " " + tr("to") + " " + newItem + ": " + tr("is the item opened?"));
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

        // confirm backup device has item;
        if ( QFile::exists(backupItem)) {
            // do restore
            ui->messageOutput->append(tr("Restoring") + " " + originalItem + "....\n");
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
                //    ~/ParatextProjects may have never existed but is part of default backup list
                // So, don't give warning message for Restore ALL.
            } else {
                QTest::qWait(1);
                QMessageBox::warning(this, tr("Warning"), tr("Item") + ": " + originalItem + " " + tr("not found on") + " " +
                                     targetDevice + ". " + tr("NO RESTORE DONE!"));
                writeLog("Item: " + originalItem + " not found on " + targetDevice + ".  NO RESTORE DONE!");
            }
            return;
        }
    } else {
        //Item Exists: no restore
        QTest::qWait(1);
        QMessageBox::warning(this, tr("Error"), tr("Item") + ": " + originalItem + " " + tr("already exists. NO RESTORE DONE!"));
        writeLog("Item: " + originalItem + " already exists.  NO RESTORE DONE!");
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
    ui->messageOutput->append("\n\n\n\n\n\n\n\n\n");
    ui->messageOutput->append(tr("Starting undo last restore from") + " " +
                              ui->targetDeviceDisp->text() + " " + tr("device") + "...\n");
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

            ui->messageOutput->append(tr("Undoing restore of") + " " + origItemName + "....\n");
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
                            ui->messageOutput->append("...." + tr("Done") + ".\n");
                            ui->messageOutput->moveCursor(QTextCursor::End);
                            writeLog("File " + origItemName + " replaced from saved version " + itemName + ".");
                            progress = progress + 10;
                            ui->progressBar->setValue(progress);
                        } else {
                            // rename unsuccessful
                            ui->messageOutput->append("\n !!!" + tr("ERROR") + "!!!\n");
                            writeLog("ERROR: File " + origItemName + " removed but " + itemName +
                                     " unable to be renamed to " + origItemName + "!!!");
                            QTest::qWait(1);
                            QMessageBox::warning(this,tr("Error"),tr("ERROR: File") + " " + origItemName + " " + tr("removed but") + " " +
                                                 itemName + " " + tr("unable to be renamed to") + " " + origItemName + "!!!");
                            break; // foreach
                        }
                    } else {
                        // remove unsuccessful
                        ui->messageOutput->append("\n !!!ERROR!!!\n");
                        writeLog("ERROR: File " + origItemName + " unable to be removed. " + itemName +
                                 " not attempted to be renamed to " + origItemName + "!!!");
                        QTest::qWait(1);
                        QMessageBox::warning(this,tr("Error"),tr("ERROR: File") + " " + origItemName + " " + tr("unable to be removed.") + " " +
                                             itemName + " " + tr("not attempted to be renamed to") + " " + origItemName + "!!!");
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
                            ui->messageOutput->append("...." + tr("Done") + ".\n");
                            ui->messageOutput->moveCursor(QTextCursor::End);
                            writeLog("Folder " + origItemName + " replaced from saved version " + itemName + ".");
                            progress = progress + 10;
                            ui->progressBar->setValue(progress);
                        } else {
                            //rename unsucessful
                            ui->messageOutput->append("\n !!!ERROR!!!\n");
                            writeLog("ERROR: Folder " + origItemName + " removed but " + itemName +
                                     " unable to be renamed to " + origItemName + "!!!");
                            QTest::qWait(1);
                            QMessageBox::warning(this,tr("Error"),tr("ERROR: Folder") + " " + origItemName +
                                                 + " " + tr("removed but") + " " + itemName + " " + tr("unable to be renamed to") + " " +
                                                 origItemName + "!!!");
                            break; // foreach
                        }
                    } else {
                        // remove unsucessful (no rename attempted)
                        ui->messageOutput->append("\n !!!ERROR!!!\n");
                        writeLog("ERROR: Folder " + origItemName + " unable to be removed. " + itemName +
                                 " not attempted to be renamed to " + origItemName + "!!!");
                        QTest::qWait(1);
                        QMessageBox::warning(this,tr("Error"),tr("ERROR: Folder") + " " + origItemName +
                                             " " + tr("unable to be removed.") + " " + itemName +
                                             " " + tr("not attempted to be renamed to") + " " + origItemName + "!!!");
                        break; // foreach
                    }
                }
            } else {
                // file doesn't exist: can't remove and rename for undo
                ui->messageOutput->append("\n !!!ERROR!!!\n");
                writeLog("ERROR: Item " + itemName + " doesn't exist so no undo restore possible!");
                QTest::qWait(1);
                QMessageBox::warning(this,tr("Error"),tr("ERROR: Item") + " " + itemName +
                                     " " + tr("doesn't exist so no undo restore possible!"));
                break; // foreach
            }
        } else {
            // else (no renameText at end of item) then was just a restored 'delete'
            // so undo just deletes current.
            ui->messageOutput->append(tr("Undoing restore of") + " " + itemName + "....\n");
            QTest::qWait(2000);
            ui->messageOutput->moveCursor(QTextCursor::End);

            if ( QFile::exists(itemName)) {
                QFileInfo item;
                item.setFile(itemName);

                if ( item.isFile() ) {
                    // file delete
                    if ( QFile::remove(itemName) ) {
                        // success deleting
                        ui->messageOutput->append("...." + tr("Done") + ".\n");
                        writeLog("File " + itemName + " deleted.");
                        ui->messageOutput->moveCursor(QTextCursor::End);
                        progress = progress + 10;
                        ui->progressBar->setValue(progress);
                    } else {
                        // remove FILE failed
                        ui->messageOutput->append("\n!!!" + tr("ERROR") + "!!!\n");
                        writeLog("ERROR: File " + itemName + " unable to be removed!!!");
                        QTest::qWait(1);
                        QMessageBox::warning(this,tr("Error"),tr("ERROR: File") + " " + itemName +
                                             " " + tr("unable to be removed!!!"));
                        break; // foreach
                    }
                } else {
                    // folder delete
                    if ( removeDir(itemName) ) {
                        // success deleting
                        ui->messageOutput->append("...." + tr("Done") + ".\n");
                        ui->messageOutput->moveCursor(QTextCursor::End);
                        writeLog("Folder " + itemName + " deleted.");
                        progress = progress + 10;
                        ui->progressBar->setValue(progress);
                    } else {
                        // remove FOLDER failed
                        ui->messageOutput->append("\n !!!" + tr("ERROR") + "!!!\n");
                        writeLog("ERROR: Folder " + itemName + " unable to be removed!!!");
                        QTest::qWait(1);
                        QMessageBox::warning(this,tr("Error"),tr("ERROR: Folder") + " " + itemName +
                                             " " + tr("unable to be removed!!!"));
                        break; // foreach
                    }
                }
            } else {
                // item doesn't exist can't delete
                ui->messageOutput->append("\n !!!" + tr("ERROR") + "!!!\n");
                writeLog("ERROR: Item " + itemName + " doesn't exist so no need to undo restore!");
                QTest::qWait(1);
                QMessageBox::warning(this,tr("Error"),tr("ERROR: Item") + " " + itemName +
                                     " " + tr("doesn't exist so no need to undo restore!"));
                break; // foreach
            }
        } // end else (no renameText at end of item)

    } // foreach


    //Last, check if configSave found (configDir + renameText).  If so, that is last thing to undo.
    if ( !configSave.isEmpty()) {

        QDir configSaveDir(configSave);
        if ( configSaveDir.exists() ) {
            ui->messageOutput->append(tr("Undoing restore of Configuration Files") + "\n");
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
                    QMessageBox::warning(this, tr("Error"), tr("Error in renaming item") + ": " + configSave +
                                         " " + tr("to") + " " + configDir + ": " + tr("is the item opened?"));
                    writeLog("Error in renaming item: " + configSave + " to " + configDir +
                             ": is the item opened?");
                    return;
                }
                //reload Config Files
                ui->messageOutput->append(tr("Loading Original Configuration Files") + "\n");
                loadConfigFiles();
            } else {
                //error: remove of configDir didn't work
                writeLog("ERROR: Folder " + configDir + " unable to be removed!!!");
                QTest::qWait(1);
                QMessageBox::warning(this,tr("Error"),tr("ERROR: Folder") + " " + configDir +
                                     " " + tr("unable to be removed") + "!!!");
                return;
            }
        }

    }

    ui->messageOutput->append("\n" + tr("Undo Last Restore Complete"));
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

    shellReturn = shellProcess->readAll();

    writeLog("shellReturn:\n" + shellReturn);

    if (processCanceled) {
        ui->messageOutput->append(tr("CANCELED") + "!!\n");
        writeLog (tr("Process CANCELED!"));
    } else if ( shellProcess->exitCode() == 0 ) {
        ui->messageOutput->moveCursor(QTextCursor::End);
        if (giveFeedback) {
            ui->messageOutput->insertPlainText(". " + tr("Done") + "\n");
        }
    } else {
        QTest::qWait(1);

        // Handle certain errors, else give generic output

        if ( shellReturn.indexOf("No space left on device") > -1) {
            QMessageBox::critical(this, tr("Backup Device Full"), tr("ERROR!! Backup device is FULL") + ": " +
                                  ui->targetDeviceDisp->text() + "\n\n " + tr("Backup not complete!"));
            ui->messageOutput->append("\n !!!" + tr("ERROR!!! Backup device is FULL") + ": " +
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
            QMessageBox::critical(this, tr("ERROR"), tr("Error during shell process") + "!!\n\n" + tr("Error") + ": " +
                                  QString::number(shellProcess->exitCode()) + "\n\n" + tr("Command") + ": " +
                                  command + "\n\n" + tr("Message") + ": " + shellReturn);
            ui->messageOutput->append("\n !!!" + tr("ERROR") + "!!!\n");

            writeLog("Error during shell process!!\n\nError: " + QString::number(shellProcess->exitCode()) +
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

    // Load up prevBackupDate file
    prevBackupDateFile.open(QIODevice::ReadOnly);
    QTextStream prevBackupDateStream(&prevBackupDateFile);
    prevBackupDate = prevBackupDateStream.readLine();
    prevBackupDateStream.flush();
    prevBackupDateFile.close();

    // load up useInclude file
    useBackupIncludeFilterFile.open(QIODevice::ReadOnly);
    QTextStream filterStream(&useBackupIncludeFilterFile);
    line = filterStream.readLine();
    filterStream.flush();
    useBackupIncludeFilterFile.close();

    if ( line.mid(0,2) != "NO" ) {
        ui->actionBackupOnlyImportant->setChecked(1);
        ui->backupIncludeLabel->setText(tr("Picture, Music, and Video files will") + " <b>" + tr("NOT") + "</b> " + tr("be included in the Backup.") + " <b>" + tr("You must backup those files yourself") + "!</b>");
    } else {
        ui->actionBackupOnlyImportant->setChecked(0);
        ui->backupIncludeLabel->setText("<b>" + tr("ALL Files") + "</b> (" + tr("including Pictures, Music, and Videos) will be included in the Backup."));
    }

    //load backupDir file
    //resize for safety
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
                // if starts with # or is blank, then comment line: throw away
                backupDirList[i] = line.split(",");
                if ( backupDirList[i].value(1).endsWith("/") ) {
                    // throw away trailing "/" so compares work easier later
                    QString text = backupDirList[i].value(1);
                    text.chop(1);
                    backupDirList[i].replace(1, text);
                }
                i++;
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
