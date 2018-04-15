/****************************************************************************
** Meta object code from reading C++ file 'Gofer.h'
**
** Created: Thu Jun 9 19:39:22 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "Gofer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Gofer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Gofer[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      32,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,    7,    6,    6, 0x05,

 // slots: signature, parameters, type, tag, flags
      30,    6,    6,    6, 0x08,
      51,    6,    6,    6, 0x08,
      75,    6,    6,    6, 0x08,
     114,  102,    6,    6, 0x08,
     157,    6,    6,    6, 0x08,
     165,    6,    6,    6, 0x08,
     177,    6,    6,    6, 0x08,
     191,    6,    6,    6, 0x08,
     202,    7,    6,    6, 0x08,
     222,  219,    6,    6, 0x08,
     250,    6,    6,    6, 0x08,
     280,  271,    6,    6, 0x08,
     304,    7,    6,    6, 0x08,
     325,  322,    6,    6, 0x08,
     342,    6,    6,    6, 0x08,
     363,    6,  359,    6, 0x08,
     375,    6,    6,    6, 0x08,
     388,    6,    6,    6, 0x08,
     399,    6,    6,    6, 0x08,
     414,    6,    6,    6, 0x08,
     425,    6,    6,    6, 0x08,
     440,    6,    6,    6, 0x08,
     452,    6,    6,    6, 0x08,
     465,    6,    6,    6, 0x08,
     488,  483,    6,    6, 0x08,
     514,    6,    6,    6, 0x08,
     544,  534,    6,    6, 0x08,
     569,    7,    6,    6, 0x08,
     598,  592,    6,    6, 0x08,
     626,    6,    6,    6, 0x08,
     633,    6,    6,    6, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Gofer[] = {
    "Gofer\0\0index\0setTabIndex(int)\0"
    "addSearchDirectory()\0removeSearchDirectory()\0"
    "duplicateSearchDirectory()\0item,column\0"
    "searchDirItemChanged(QTreeWidgetItem*,int)\0"
    "about()\0newWindow()\0closeWindow()\0"
    "closeAll()\0newCombiner(int)\0cs\0"
    "setCombiner(st_expr_type_t)\0"
    "newCombineDistance()\0distance\0"
    "setCombineDistance(int)\0newExactness(int)\0"
    "id\0textEntered(int)\0startSearching()\0"
    "int\0keepGoing()\0stopSearch()\0findPrev()\0"
    "findPrevFile()\0findNext()\0findNextFile()\0"
    "newSearch()\0saveSearch()\0loadSavedSearch()\0"
    "next\0setNewState(SearchState*)\0"
    "deleteSavedSearch()\0stateName\0"
    "savedSearchAdd(QString*)\0"
    "savedSearchRemove(int)\0event\0"
    "keyReleaseEvent(QKeyEvent*)\0copy()\0"
    "paste()\0"
};

const QMetaObject Gofer::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_Gofer,
      qt_meta_data_Gofer, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Gofer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Gofer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Gofer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Gofer))
        return static_cast<void*>(const_cast< Gofer*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int Gofer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setTabIndex((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: addSearchDirectory(); break;
        case 2: removeSearchDirectory(); break;
        case 3: duplicateSearchDirectory(); break;
        case 4: searchDirItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: about(); break;
        case 6: newWindow(); break;
        case 7: closeWindow(); break;
        case 8: closeAll(); break;
        case 9: newCombiner((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: setCombiner((*reinterpret_cast< st_expr_type_t(*)>(_a[1]))); break;
        case 11: newCombineDistance(); break;
        case 12: setCombineDistance((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: newExactness((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: textEntered((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: startSearching(); break;
        case 16: { int _r = keepGoing();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 17: stopSearch(); break;
        case 18: findPrev(); break;
        case 19: findPrevFile(); break;
        case 20: findNext(); break;
        case 21: findNextFile(); break;
        case 22: newSearch(); break;
        case 23: saveSearch(); break;
        case 24: loadSavedSearch(); break;
        case 25: setNewState((*reinterpret_cast< SearchState*(*)>(_a[1]))); break;
        case 26: deleteSavedSearch(); break;
        case 27: savedSearchAdd((*reinterpret_cast< QString*(*)>(_a[1]))); break;
        case 28: savedSearchRemove((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 29: keyReleaseEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 30: copy(); break;
        case 31: paste(); break;
        default: ;
        }
        _id -= 32;
    }
    return _id;
}

// SIGNAL 0
void Gofer::setTabIndex(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
