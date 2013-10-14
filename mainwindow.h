#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QStringList arguments, QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void setPreferredDestination();

    void on_backupButton_clicked();

    QString shellRun(QString command, bool giveFeedback);

    void writeLog(QString data);

    void loadConfigFiles();

    void on_restorePrevRadio_clicked();

    void on_restoreDelRadio_clicked();

    void on_restoreAllRadio_clicked();

    void on_restoreButton_clicked();

    void on_changeDeviceButton_clicked();

    void on_selectPrevItemButton_clicked();

    void on_prevFileRadio_clicked();

    void on_prevFolderRadio_clicked();

    void on_backupRestoreWidget_currentChanged(int index);

    void on_selectDelFolderButton_clicked();

    void on_viewLogButton_clicked();

    void on_cancelBackupButton_clicked();

    void on_cancelRestoreButton_clicked();

    void cancelProcess();

    void on_openRestoreFolderButton_clicked();

    void on_restoreAllCheck_stateChanged(int arg1);

    void renameRestoreItem(QString renameItem, QString targetItem, QString restUser);

    void on_actionBackupOnlyImportant_changed();

    void on_actionAbout_triggered();

    void on_undoLastRestoreButton_clicked();

    bool removeDir(const QString & dirName);

    void setTargetDevice(QString inputDir);

    void on_machineCombo_currentIndexChanged(const QString machineValue);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
