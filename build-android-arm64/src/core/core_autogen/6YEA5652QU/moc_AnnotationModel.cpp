/****************************************************************************
** Meta object code from reading C++ file 'AnnotationModel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/include/AnnotationModel.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AnnotationModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN15AnnotationModelE_t {};
} // unnamed namespace

template <> constexpr inline auto AnnotationModel::qt_create_metaobjectdata<qt_meta_tag_ZN15AnnotationModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AnnotationModel",
        "libraryItemIdChanged",
        "",
        "lastErrorChanged",
        "countChanged",
        "revisionChanged",
        "get",
        "QVariantMap",
        "index",
        "highlightRangesForChapter",
        "QVariantList",
        "chapterIndex",
        "anchorsForPage",
        "pageIndex",
        "exportAnnotations",
        "path",
        "addAnnotation",
        "locator",
        "type",
        "text",
        "color",
        "updateAnnotation",
        "id",
        "deleteAnnotation",
        "attachDatabase",
        "dbPath",
        "attachConnection",
        "connectionName",
        "libraryItemId",
        "lastError",
        "count",
        "revision"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'libraryItemIdChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'countChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'revisionChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'get'
        QtMocHelpers::MethodData<QVariantMap(int) const>(6, 2, QMC::AccessPublic, 0x80000000 | 7, {{
            { QMetaType::Int, 8 },
        }}),
        // Method 'highlightRangesForChapter'
        QtMocHelpers::MethodData<QVariantList(int) const>(9, 2, QMC::AccessPublic, 0x80000000 | 10, {{
            { QMetaType::Int, 11 },
        }}),
        // Method 'anchorsForPage'
        QtMocHelpers::MethodData<QVariantList(int) const>(12, 2, QMC::AccessPublic, 0x80000000 | 10, {{
            { QMetaType::Int, 13 },
        }}),
        // Method 'exportAnnotations'
        QtMocHelpers::MethodData<bool(const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 15 },
        }}),
        // Method 'addAnnotation'
        QtMocHelpers::MethodData<bool(const QString &, const QString &, const QString &, const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 17 }, { QMetaType::QString, 18 }, { QMetaType::QString, 19 }, { QMetaType::QString, 20 },
        }}),
        // Method 'updateAnnotation'
        QtMocHelpers::MethodData<bool(int, const QString &, const QString &, const QString &, const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 22 }, { QMetaType::QString, 17 }, { QMetaType::QString, 18 }, { QMetaType::QString, 19 },
            { QMetaType::QString, 20 },
        }}),
        // Method 'deleteAnnotation'
        QtMocHelpers::MethodData<bool(int)>(23, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 22 },
        }}),
        // Method 'attachDatabase'
        QtMocHelpers::MethodData<bool(const QString &)>(24, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 25 },
        }}),
        // Method 'attachConnection'
        QtMocHelpers::MethodData<bool(const QString &)>(26, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 27 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'libraryItemId'
        QtMocHelpers::PropertyData<int>(28, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(29, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'count'
        QtMocHelpers::PropertyData<int>(30, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
        // property 'revision'
        QtMocHelpers::PropertyData<int>(31, QMetaType::Int, QMC::DefaultPropertyFlags, 3),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AnnotationModel, qt_meta_tag_ZN15AnnotationModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AnnotationModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AnnotationModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AnnotationModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15AnnotationModelE_t>.metaTypes,
    nullptr
} };

void AnnotationModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AnnotationModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->libraryItemIdChanged(); break;
        case 1: _t->lastErrorChanged(); break;
        case 2: _t->countChanged(); break;
        case 3: _t->revisionChanged(); break;
        case 4: { QVariantMap _r = _t->get((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantMap*>(_a[0]) = std::move(_r); }  break;
        case 5: { QVariantList _r = _t->highlightRangesForChapter((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 6: { QVariantList _r = _t->anchorsForPage((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 7: { bool _r = _t->exportAnnotations((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 8: { bool _r = _t->addAnnotation((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 9: { bool _r = _t->updateAnnotation((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[5])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 10: { bool _r = _t->deleteAnnotation((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 11: { bool _r = _t->attachDatabase((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 12: { bool _r = _t->attachConnection((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AnnotationModel::*)()>(_a, &AnnotationModel::libraryItemIdChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnnotationModel::*)()>(_a, &AnnotationModel::lastErrorChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnnotationModel::*)()>(_a, &AnnotationModel::countChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AnnotationModel::*)()>(_a, &AnnotationModel::revisionChanged, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<int*>(_v) = _t->libraryItemId(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->count(); break;
        case 3: *reinterpret_cast<int*>(_v) = _t->revision(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setLibraryItemId(*reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *AnnotationModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AnnotationModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15AnnotationModelE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int AnnotationModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void AnnotationModel::libraryItemIdChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AnnotationModel::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AnnotationModel::countChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void AnnotationModel::revisionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
