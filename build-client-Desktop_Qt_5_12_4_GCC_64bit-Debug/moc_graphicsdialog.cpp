/****************************************************************************
** Meta object code from reading C++ file 'graphicsdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../client/graphicsdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'graphicsdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GraphicsDialog_t {
    QByteArrayData data[11];
    char stringdata0[130];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GraphicsDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GraphicsDialog_t qt_meta_stringdata_GraphicsDialog = {
    {
QT_MOC_LITERAL(0, 0, 14), // "GraphicsDialog"
QT_MOC_LITERAL(1, 15, 12), // "requestClose"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 15), // "addActivePlayer"
QT_MOC_LITERAL(4, 45, 8), // "clientId"
QT_MOC_LITERAL(5, 54, 5), // "color"
QT_MOC_LITERAL(6, 60, 9), // "addPlayer"
QT_MOC_LITERAL(7, 70, 12), // "removePlayer"
QT_MOC_LITERAL(8, 83, 21), // "updatePlayerPositions"
QT_MOC_LITERAL(9, 105, 11), // "QJsonArray&"
QT_MOC_LITERAL(10, 117, 12) // "playersArray"

    },
    "GraphicsDialog\0requestClose\0\0"
    "addActivePlayer\0clientId\0color\0addPlayer\0"
    "removePlayer\0updatePlayerPositions\0"
    "QJsonArray&\0playersArray"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GraphicsDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    2,   40,    2, 0x0a /* Public */,
       6,    2,   45,    2, 0x0a /* Public */,
       7,    1,   50,    2, 0x0a /* Public */,
       8,    1,   53,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::QColor,    4,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::QColor,    4,    5,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, 0x80000000 | 9,   10,

       0        // eod
};

void GraphicsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GraphicsDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->requestClose(); break;
        case 1: _t->addActivePlayer((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 2: _t->addPlayer((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 3: _t->removePlayer((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->updatePlayerPositions((*reinterpret_cast< QJsonArray(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GraphicsDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GraphicsDialog::requestClose)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject GraphicsDialog::staticMetaObject = { {
    &QDialog::staticMetaObject,
    qt_meta_stringdata_GraphicsDialog.data,
    qt_meta_data_GraphicsDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GraphicsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GraphicsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GraphicsDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int GraphicsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void GraphicsDialog::requestClose()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
