/****************************************************************************
** Meta object code from reading C++ file 'AScopeReader.h'
**
** Created: Fri Nov 5 14:45:56 2010
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "AScopeReader.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AScopeReader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AScopeReader[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      20,   14,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,   14,   13,   13, 0x0a,
      83,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_AScopeReader[] = {
    "AScopeReader\0\0pItem\0newItem(AScope::TimeSeries)\0"
    "returnItemSlot(AScope::TimeSeries)\0"
    "readFromServer()\0"
};

const QMetaObject AScopeReader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_AScopeReader,
      qt_meta_data_AScopeReader, 0 }
};

const QMetaObject *AScopeReader::metaObject() const
{
    return &staticMetaObject;
}

void *AScopeReader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AScopeReader))
        return static_cast<void*>(const_cast< AScopeReader*>(this));
    return QObject::qt_metacast(_clname);
}

int AScopeReader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newItem((*reinterpret_cast< AScope::TimeSeries(*)>(_a[1]))); break;
        case 1: returnItemSlot((*reinterpret_cast< AScope::TimeSeries(*)>(_a[1]))); break;
        case 2: readFromServer(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void AScopeReader::newItem(AScope::TimeSeries _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
