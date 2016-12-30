/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      28,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      38,   11,   11,   11, 0x08,
      93,   72,   64,   11, 0x08,
     121,  116,   11,   11, 0x08,
     139,   11,   11,   11, 0x08,
     157,   11,   11,   11, 0x08,
     187,   11,   11,   11, 0x08,
     216,   11,   11,   11, 0x08,
     245,   11,   11,   11, 0x08,
     272,   11,   11,   11, 0x08,
     304,   11,   11,   11, 0x08,
     338,   11,   11,   11, 0x08,
     365,   11,   11,   11, 0x08,
     400,  394,   11,   11, 0x08,
     443,   11,   11,   11, 0x08,
     478,   11,   11,   11, 0x08,
     505,   11,   11,   11, 0x08,
     537,   11,   11,   11, 0x08,
     570,   11,   11,   11, 0x08,
     586,   11,   11,   11, 0x08,
     628,  623,   11,   11, 0x08,
     696,  665,   11,   11, 0x08,
     739,   11,   11,   11, 0x08,
     778,   11,   11,   11, 0x08,
     805,   11,   11,   11, 0x08,
     853,  845,  840,   11, 0x08,
     881,  872,   11,   11, 0x08,
     919,  906,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0setPreferredDestination()\0"
    "on_backupButton_clicked()\0QString\0"
    "command,giveFeedback\0shellRun(QString,bool)\0"
    "data\0writeLog(QString)\0loadConfigFiles()\0"
    "on_restorePrevRadio_clicked()\0"
    "on_restoreDelRadio_clicked()\0"
    "on_restoreAllRadio_clicked()\0"
    "on_restoreButton_clicked()\0"
    "on_changeDeviceButton_clicked()\0"
    "on_selectPrevItemButton_clicked()\0"
    "on_prevFileRadio_clicked()\0"
    "on_prevFolderRadio_clicked()\0index\0"
    "on_backupRestoreWidget_currentChanged(int)\0"
    "on_selectDelFolderButton_clicked()\0"
    "on_viewLogButton_clicked()\0"
    "on_cancelBackupButton_clicked()\0"
    "on_cancelRestoreButton_clicked()\0"
    "cancelProcess()\0on_openRestoreFolderButton_clicked()\0"
    "arg1\0on_restoreAllCheck_stateChanged(int)\0"
    "renameItem,targetItem,restUser\0"
    "renameRestoreItem(QString,QString,QString)\0"
    "on_actionBackupOnlyImportant_changed()\0"
    "on_actionAbout_triggered()\0"
    "on_undoLastRestoreButton_clicked()\0"
    "bool\0dirName\0removeDir(QString)\0"
    "inputDir\0setTargetDevice(QString)\0"
    "machineValue\0on_machineCombo_currentIndexChanged(QString)\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->setPreferredDestination(); break;
        case 1: _t->on_backupButton_clicked(); break;
        case 2: { QString _r = _t->shellRun((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 3: _t->writeLog((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->loadConfigFiles(); break;
        case 5: _t->on_restorePrevRadio_clicked(); break;
        case 6: _t->on_restoreDelRadio_clicked(); break;
        case 7: _t->on_restoreAllRadio_clicked(); break;
        case 8: _t->on_restoreButton_clicked(); break;
        case 9: _t->on_changeDeviceButton_clicked(); break;
        case 10: _t->on_selectPrevItemButton_clicked(); break;
        case 11: _t->on_prevFileRadio_clicked(); break;
        case 12: _t->on_prevFolderRadio_clicked(); break;
        case 13: _t->on_backupRestoreWidget_currentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->on_selectDelFolderButton_clicked(); break;
        case 15: _t->on_viewLogButton_clicked(); break;
        case 16: _t->on_cancelBackupButton_clicked(); break;
        case 17: _t->on_cancelRestoreButton_clicked(); break;
        case 18: _t->cancelProcess(); break;
        case 19: _t->on_openRestoreFolderButton_clicked(); break;
        case 20: _t->on_restoreAllCheck_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: _t->renameRestoreItem((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 22: _t->on_actionBackupOnlyImportant_changed(); break;
        case 23: _t->on_actionAbout_triggered(); break;
        case 24: _t->on_undoLastRestoreButton_clicked(); break;
        case 25: { bool _r = _t->removeDir((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 26: _t->setTargetDevice((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 27: _t->on_machineCombo_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
