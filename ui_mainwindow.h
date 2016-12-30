/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QTextBrowser>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionAbout;
    QAction *actionBackupOnlyImportant;
    QWidget *centralWidget;
    QPushButton *changeDeviceButton;
    QLineEdit *targetDeviceDisp;
    QLabel *label;
    QLabel *label_4;
    QTabWidget *backupRestoreWidget;
    QWidget *backupTab;
    QPushButton *backupButton;
    QLabel *label_11;
    QLabel *backupIncludeLabel;
    QPushButton *cancelBackupButton;
    QWidget *restoreTab;
    QFrame *restTypeFrame;
    QRadioButton *restoreAllRadio;
    QRadioButton *restoreDelRadio;
    QLabel *restoreDelRadioLabel;
    QRadioButton *restorePrevRadio;
    QLabel *restorePrevRadioLabel;
    QLabel *label_7;
    QLabel *restoreAllRadioLabel;
    QStackedWidget *restorePageWidget;
    QWidget *restorePrevPage;
    QLabel *prevDateTimeLabel;
    QLabel *prevItemLabel;
    QLineEdit *prevItem;
    QPushButton *selectPrevItemButton;
    QComboBox *prevListCombo;
    QFrame *line_2;
    QRadioButton *prevFileRadio;
    QRadioButton *prevFolderRadio;
    QWidget *restoreDelPage;
    QListWidget *delList;
    QLabel *delListLabel;
    QLineEdit *delFolder;
    QPushButton *selectDelFolderButton;
    QLabel *delFolderLabel;
    QFrame *line_5;
    QWidget *restoreAllPage;
    QLabel *restoreAllLabel;
    QCheckBox *restoreAllCheck;
    QFrame *line_4;
    QComboBox *machineCombo;
    QComboBox *restUserCombo;
    QLabel *machineLabel;
    QLabel *restUserLabel;
    QPushButton *cancelRestoreButton;
    QPushButton *openRestoreFolderButton;
    QPushButton *undoLastRestoreButton;
    QPushButton *restoreButton;
    QProgressBar *progressBar;
    QTextBrowser *messageOutput;
    QLabel *label_10;
    QPushButton *viewLogButton;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuAdvanced;
    QMenu *menuFile_Selection;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(900, 515);
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionBackupOnlyImportant = new QAction(MainWindow);
        actionBackupOnlyImportant->setObjectName(QString::fromUtf8("actionBackupOnlyImportant"));
        actionBackupOnlyImportant->setCheckable(true);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        changeDeviceButton = new QPushButton(centralWidget);
        changeDeviceButton->setObjectName(QString::fromUtf8("changeDeviceButton"));
        changeDeviceButton->setGeometry(QRect(400, 10, 121, 27));
        targetDeviceDisp = new QLineEdit(centralWidget);
        targetDeviceDisp->setObjectName(QString::fromUtf8("targetDeviceDisp"));
        targetDeviceDisp->setGeometry(QRect(180, 10, 211, 27));
        targetDeviceDisp->setReadOnly(true);
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 5, 161, 41));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label->setWordWrap(true);
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(530, 10, 291, 61));
        QFont font;
        font.setPointSize(10);
        font.setItalic(true);
        label_4->setFont(font);
        label_4->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        label_4->setWordWrap(true);
        backupRestoreWidget = new QTabWidget(centralWidget);
        backupRestoreWidget->setObjectName(QString::fromUtf8("backupRestoreWidget"));
        backupRestoreWidget->setEnabled(true);
        backupRestoreWidget->setGeometry(QRect(10, 50, 881, 291));
        backupRestoreWidget->setTabPosition(QTabWidget::North);
        backupRestoreWidget->setDocumentMode(false);
        backupRestoreWidget->setTabsClosable(false);
        backupTab = new QWidget();
        backupTab->setObjectName(QString::fromUtf8("backupTab"));
        backupButton = new QPushButton(backupTab);
        backupButton->setObjectName(QString::fromUtf8("backupButton"));
        backupButton->setGeometry(QRect(310, 110, 191, 41));
        backupButton->setDefault(true);
        label_11 = new QLabel(backupTab);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(120, 10, 721, 21));
        backupIncludeLabel = new QLabel(backupTab);
        backupIncludeLabel->setObjectName(QString::fromUtf8("backupIncludeLabel"));
        backupIncludeLabel->setGeometry(QRect(120, 50, 611, 51));
        backupIncludeLabel->setTextFormat(Qt::RichText);
        backupIncludeLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        backupIncludeLabel->setWordWrap(true);
        cancelBackupButton = new QPushButton(backupTab);
        cancelBackupButton->setObjectName(QString::fromUtf8("cancelBackupButton"));
        cancelBackupButton->setGeometry(QRect(310, 160, 191, 27));
        backupRestoreWidget->addTab(backupTab, QString());
        restoreTab = new QWidget();
        restoreTab->setObjectName(QString::fromUtf8("restoreTab"));
        restTypeFrame = new QFrame(restoreTab);
        restTypeFrame->setObjectName(QString::fromUtf8("restTypeFrame"));
        restTypeFrame->setGeometry(QRect(10, 10, 221, 231));
        restTypeFrame->setFrameShape(QFrame::StyledPanel);
        restTypeFrame->setFrameShadow(QFrame::Raised);
        restoreAllRadio = new QRadioButton(restTypeFrame);
        restoreAllRadio->setObjectName(QString::fromUtf8("restoreAllRadio"));
        restoreAllRadio->setGeometry(QRect(10, 170, 21, 21));
        restoreDelRadio = new QRadioButton(restTypeFrame);
        restoreDelRadio->setObjectName(QString::fromUtf8("restoreDelRadio"));
        restoreDelRadio->setGeometry(QRect(10, 110, 21, 22));
        restoreDelRadioLabel = new QLabel(restTypeFrame);
        restoreDelRadioLabel->setObjectName(QString::fromUtf8("restoreDelRadioLabel"));
        restoreDelRadioLabel->setGeometry(QRect(30, 110, 171, 51));
        restoreDelRadioLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        restoreDelRadioLabel->setWordWrap(true);
        restorePrevRadio = new QRadioButton(restTypeFrame);
        restorePrevRadio->setObjectName(QString::fromUtf8("restorePrevRadio"));
        restorePrevRadio->setGeometry(QRect(10, 50, 21, 22));
        restorePrevRadio->setChecked(true);
        restorePrevRadio->setAutoRepeat(false);
        restorePrevRadioLabel = new QLabel(restTypeFrame);
        restorePrevRadioLabel->setObjectName(QString::fromUtf8("restorePrevRadioLabel"));
        restorePrevRadioLabel->setGeometry(QRect(30, 50, 171, 51));
        restorePrevRadioLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        restorePrevRadioLabel->setWordWrap(true);
        label_7 = new QLabel(restTypeFrame);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 10, 201, 31));
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        label_7->setFont(font1);
        label_7->setWordWrap(true);
        restoreAllRadioLabel = new QLabel(restTypeFrame);
        restoreAllRadioLabel->setObjectName(QString::fromUtf8("restoreAllRadioLabel"));
        restoreAllRadioLabel->setGeometry(QRect(30, 170, 171, 51));
        restoreAllRadioLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        restoreAllRadioLabel->setWordWrap(true);
        restorePageWidget = new QStackedWidget(restoreTab);
        restorePageWidget->setObjectName(QString::fromUtf8("restorePageWidget"));
        restorePageWidget->setGeometry(QRect(240, 0, 631, 251));
        restorePrevPage = new QWidget();
        restorePrevPage->setObjectName(QString::fromUtf8("restorePrevPage"));
        prevDateTimeLabel = new QLabel(restorePrevPage);
        prevDateTimeLabel->setObjectName(QString::fromUtf8("prevDateTimeLabel"));
        prevDateTimeLabel->setGeometry(QRect(0, 70, 131, 81));
        prevDateTimeLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        prevDateTimeLabel->setWordWrap(true);
        prevItemLabel = new QLabel(restorePrevPage);
        prevItemLabel->setObjectName(QString::fromUtf8("prevItemLabel"));
        prevItemLabel->setGeometry(QRect(-10, 15, 131, 51));
        prevItemLabel->setAlignment(Qt::AlignRight|Qt::AlignTop|Qt::AlignTrailing);
        prevItemLabel->setWordWrap(true);
        prevItem = new QLineEdit(restorePrevPage);
        prevItem->setObjectName(QString::fromUtf8("prevItem"));
        prevItem->setGeometry(QRect(130, 10, 391, 27));
        prevItem->setReadOnly(true);
        selectPrevItemButton = new QPushButton(restorePrevPage);
        selectPrevItemButton->setObjectName(QString::fromUtf8("selectPrevItemButton"));
        selectPrevItemButton->setGeometry(QRect(530, 10, 101, 27));
        prevListCombo = new QComboBox(restorePrevPage);
        prevListCombo->setObjectName(QString::fromUtf8("prevListCombo"));
        prevListCombo->setGeometry(QRect(130, 80, 501, 27));
        line_2 = new QFrame(restorePrevPage);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(10, 60, 611, 16));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        prevFileRadio = new QRadioButton(restorePrevPage);
        prevFileRadio->setObjectName(QString::fromUtf8("prevFileRadio"));
        prevFileRadio->setGeometry(QRect(130, 40, 101, 22));
        prevFileRadio->setChecked(true);
        prevFolderRadio = new QRadioButton(restorePrevPage);
        prevFolderRadio->setObjectName(QString::fromUtf8("prevFolderRadio"));
        prevFolderRadio->setGeometry(QRect(240, 40, 111, 22));
        restorePageWidget->addWidget(restorePrevPage);
        restoreDelPage = new QWidget();
        restoreDelPage->setObjectName(QString::fromUtf8("restoreDelPage"));
        delList = new QListWidget(restoreDelPage);
        delList->setObjectName(QString::fromUtf8("delList"));
        delList->setGeometry(QRect(0, 120, 341, 131));
        delListLabel = new QLabel(restoreDelPage);
        delListLabel->setObjectName(QString::fromUtf8("delListLabel"));
        delListLabel->setGeometry(QRect(0, 50, 631, 68));
        delListLabel->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);
        delListLabel->setWordWrap(true);
        delFolder = new QLineEdit(restoreDelPage);
        delFolder->setObjectName(QString::fromUtf8("delFolder"));
        delFolder->setGeometry(QRect(170, 10, 351, 27));
        delFolder->setReadOnly(true);
        selectDelFolderButton = new QPushButton(restoreDelPage);
        selectDelFolderButton->setObjectName(QString::fromUtf8("selectDelFolderButton"));
        selectDelFolderButton->setGeometry(QRect(530, 10, 101, 27));
        delFolderLabel = new QLabel(restoreDelPage);
        delFolderLabel->setObjectName(QString::fromUtf8("delFolderLabel"));
        delFolderLabel->setGeometry(QRect(0, -5, 161, 51));
        delFolderLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        delFolderLabel->setWordWrap(true);
        line_5 = new QFrame(restoreDelPage);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setGeometry(QRect(10, 40, 611, 16));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);
        restorePageWidget->addWidget(restoreDelPage);
        restoreAllPage = new QWidget();
        restoreAllPage->setObjectName(QString::fromUtf8("restoreAllPage"));
        restoreAllLabel = new QLabel(restoreAllPage);
        restoreAllLabel->setObjectName(QString::fromUtf8("restoreAllLabel"));
        restoreAllLabel->setEnabled(true);
        restoreAllLabel->setGeometry(QRect(10, 10, 611, 101));
        QFont font2;
        font2.setItalic(true);
        restoreAllLabel->setFont(font2);
        restoreAllLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        restoreAllLabel->setWordWrap(true);
        restoreAllCheck = new QCheckBox(restoreAllPage);
        restoreAllCheck->setObjectName(QString::fromUtf8("restoreAllCheck"));
        restoreAllCheck->setGeometry(QRect(10, 110, 331, 27));
        line_4 = new QFrame(restoreAllPage);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setGeometry(QRect(10, 100, 611, 16));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        machineCombo = new QComboBox(restoreAllPage);
        machineCombo->setObjectName(QString::fromUtf8("machineCombo"));
        machineCombo->setGeometry(QRect(190, 140, 151, 27));
        restUserCombo = new QComboBox(restoreAllPage);
        restUserCombo->setObjectName(QString::fromUtf8("restUserCombo"));
        restUserCombo->setGeometry(QRect(190, 200, 151, 27));
        machineLabel = new QLabel(restoreAllPage);
        machineLabel->setObjectName(QString::fromUtf8("machineLabel"));
        machineLabel->setGeometry(QRect(10, 140, 181, 51));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(machineLabel->sizePolicy().hasHeightForWidth());
        machineLabel->setSizePolicy(sizePolicy);
        machineLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        machineLabel->setWordWrap(true);
        restUserLabel = new QLabel(restoreAllPage);
        restUserLabel->setObjectName(QString::fromUtf8("restUserLabel"));
        restUserLabel->setGeometry(QRect(10, 200, 181, 51));
        restUserLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        restUserLabel->setWordWrap(true);
        restorePageWidget->addWidget(restoreAllPage);
        cancelRestoreButton = new QPushButton(restoreTab);
        cancelRestoreButton->setObjectName(QString::fromUtf8("cancelRestoreButton"));
        cancelRestoreButton->setGeometry(QRect(590, 170, 281, 21));
        openRestoreFolderButton = new QPushButton(restoreTab);
        openRestoreFolderButton->setObjectName(QString::fromUtf8("openRestoreFolderButton"));
        openRestoreFolderButton->setGeometry(QRect(590, 200, 281, 21));
        undoLastRestoreButton = new QPushButton(restoreTab);
        undoLastRestoreButton->setObjectName(QString::fromUtf8("undoLastRestoreButton"));
        undoLastRestoreButton->setGeometry(QRect(590, 230, 281, 21));
        restoreButton = new QPushButton(restoreTab);
        restoreButton->setObjectName(QString::fromUtf8("restoreButton"));
        restoreButton->setGeometry(QRect(590, 120, 281, 41));
        restoreButton->setDefault(true);
        backupRestoreWidget->addTab(restoreTab, QString());
        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(10, 350, 251, 27));
        progressBar->setValue(24);
        messageOutput = new QTextBrowser(centralWidget);
        messageOutput->setObjectName(QString::fromUtf8("messageOutput"));
        messageOutput->setGeometry(QRect(270, 350, 621, 131));
        label_10 = new QLabel(centralWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(70, 405, 191, 21));
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        viewLogButton = new QPushButton(centralWidget);
        viewLogButton->setObjectName(QString::fromUtf8("viewLogButton"));
        viewLogButton->setGeometry(QRect(10, 446, 251, 31));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 900, 23));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuAdvanced = new QMenu(menuFile);
        menuAdvanced->setObjectName(QString::fromUtf8("menuAdvanced"));
        menuFile_Selection = new QMenu(menuAdvanced);
        menuFile_Selection->setObjectName(QString::fromUtf8("menuFile_Selection"));
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionAbout);
        menuFile->addAction(menuAdvanced->menuAction());
        menuAdvanced->addAction(menuFile_Selection->menuAction());
        menuFile_Selection->addAction(actionBackupOnlyImportant);

        retranslateUi(MainWindow);

        backupRestoreWidget->setCurrentIndex(1);
        restorePageWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Wasta [Backup]", 0, QApplication::UnicodeUTF8));
        actionAbout->setText(QApplication::translate("MainWindow", "&About", 0, QApplication::UnicodeUTF8));
        actionBackupOnlyImportant->setText(QApplication::translate("MainWindow", "&Only Backup Important Files (Default)", 0, QApplication::UnicodeUTF8));
        actionBackupOnlyImportant->setIconText(QApplication::translate("MainWindow", "Only Backup Important Files (Default)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionBackupOnlyImportant->setToolTip(QApplication::translate("MainWindow", "if unchecked, ALL files will be included in backup (be careful of available space on backup device!)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        actionBackupOnlyImportant->setStatusTip(QApplication::translate("MainWindow", "if unchecked, ALL files will be included in backup (be careful of available space on backup device!)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        actionBackupOnlyImportant->setWhatsThis(QApplication::translate("MainWindow", "if unchecked, ALL files will be included in backup (be careful of available space on backup device!)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        changeDeviceButton->setText(QApplication::translate("MainWindow", "Change", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Backup Device:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "To choose a different device, make sure it is connected and click \"Change\".", 0, QApplication::UnicodeUTF8));
        backupButton->setText(QApplication::translate("MainWindow", "Backup", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:600;\">Wasta-Backup</span> makes version backups of data to an external USB device.</p></body></html>", 0, QApplication::UnicodeUTF8));
        backupIncludeLabel->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:600;\">Picture, Music, and Video files will NOT be backed up! Those files must be backed up in a different way!</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        cancelBackupButton->setText(QApplication::translate("MainWindow", "Cancel Backup", 0, QApplication::UnicodeUTF8));
        backupRestoreWidget->setTabText(backupRestoreWidget->indexOf(backupTab), QApplication::translate("MainWindow", "Backup", 0, QApplication::UnicodeUTF8));
        restoreAllRadio->setText(QString());
        restoreDelRadio->setText(QString());
        restoreDelRadioLabel->setText(QApplication::translate("MainWindow", "Restore Deleted File or Folder", 0, QApplication::UnicodeUTF8));
        restorePrevRadio->setText(QString());
        restorePrevRadioLabel->setText(QApplication::translate("MainWindow", "Restore Previous Version of File or Folder", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("MainWindow", "Restore what?", 0, QApplication::UnicodeUTF8));
        restoreAllRadioLabel->setText(QApplication::translate("MainWindow", "Restore ALL Backup Data", 0, QApplication::UnicodeUTF8));
        prevDateTimeLabel->setText(QApplication::translate("MainWindow", "Restore previous version from:", 0, QApplication::UnicodeUTF8));
        prevItemLabel->setText(QApplication::translate("MainWindow", "File to Restore:", 0, QApplication::UnicodeUTF8));
        selectPrevItemButton->setText(QApplication::translate("MainWindow", "Select", 0, QApplication::UnicodeUTF8));
        prevFileRadio->setText(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        prevFolderRadio->setText(QApplication::translate("MainWindow", "Folder", 0, QApplication::UnicodeUTF8));
        delListLabel->setText(QApplication::translate("MainWindow", "<html><head/><body><p>Select Deleted Files or Folders to Restore:</p><p><span style=\" font-size:10pt; font-style:italic;\">(use ctrl + click to select more than 1 item to restore)</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        selectDelFolderButton->setText(QApplication::translate("MainWindow", "Select", 0, QApplication::UnicodeUTF8));
        delFolderLabel->setText(QApplication::translate("MainWindow", "Folder of Deleted Item:", 0, QApplication::UnicodeUTF8));
        restoreAllLabel->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:600;\">WARNING!</span></p><p>Restore ALL Backup Data is normally done <span style=\" font-weight:600;\">only when transferring ALL Backup Data to a new computer.</span></p><p>Backup Data will replace data that is currently on the computer. </p></body></html>", 0, QApplication::UnicodeUTF8));
        restoreAllCheck->setText(QApplication::translate("MainWindow", "enable Restore ALL", 0, QApplication::UnicodeUTF8));
        machineLabel->setText(QApplication::translate("MainWindow", "Machine to Restore Data From:", 0, QApplication::UnicodeUTF8));
        restUserLabel->setText(QApplication::translate("MainWindow", "User to Restore Data From:", 0, QApplication::UnicodeUTF8));
        cancelRestoreButton->setText(QApplication::translate("MainWindow", "Cancel Restore", 0, QApplication::UnicodeUTF8));
        openRestoreFolderButton->setText(QApplication::translate("MainWindow", "Open Restore Folder", 0, QApplication::UnicodeUTF8));
        undoLastRestoreButton->setText(QApplication::translate("MainWindow", "Undo Last Restore", 0, QApplication::UnicodeUTF8));
        restoreButton->setText(QApplication::translate("MainWindow", "Restore", 0, QApplication::UnicodeUTF8));
        backupRestoreWidget->setTabText(backupRestoreWidget->indexOf(restoreTab), QApplication::translate("MainWindow", "Restore", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("MainWindow", "Messages:", 0, QApplication::UnicodeUTF8));
        viewLogButton->setText(QApplication::translate("MainWindow", "View Log File", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "Fi&le", 0, QApplication::UnicodeUTF8));
        menuAdvanced->setTitle(QApplication::translate("MainWindow", "A&dvanced...", 0, QApplication::UnicodeUTF8));
        menuFile_Selection->setTitle(QApplication::translate("MainWindow", "&Backup File Selection", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
