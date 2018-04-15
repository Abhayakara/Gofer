/****************************************************************************
** Meta object code from reading C++ file 'TextViewer.h'
**
** Created: Thu Jun 9 19:39:24 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "TextViewer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TextViewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TextViewer[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   12,   11,   11, 0x08,
      49,   42,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_TextViewer[] = {
    "TextViewer\0\0value\0scrollValueChanged(int)\0"
    "action\0scrollActionTriggered(int)\0"
};

const QMetaObject TextViewer::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_TextViewer,
      qt_meta_data_TextViewer, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TextViewer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TextViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TextViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TextViewer))
        return static_cast<void*>(const_cast< TextViewer*>(this));
    return QWidget::qt_metacast(_clname);
}

int TextViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: scrollValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: scrollActionTriggered((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
