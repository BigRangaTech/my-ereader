/****************************************************************************
** Meta object code from reading C++ file 'UpdateManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/include/UpdateManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UpdateManager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13UpdateManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto UpdateManager::qt_create_metaobjectdata<qt_meta_tag_ZN13UpdateManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "UpdateManager",
        "stateChanged",
        "",
        "statusChanged",
        "summaryChanged",
        "detailsChanged",
        "canUpdateChanged",
        "checkForUpdates",
        "applyUpdate",
        "restartApp",
        "state",
        "State",
        "status",
        "summary",
        "details",
        "canUpdate",
        "Idle",
        "Checking",
        "Applying",
        "UpToDate",
        "UpdateAvailable",
        "Unavailable",
        "Error"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stateChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'summaryChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'detailsChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'canUpdateChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'checkForUpdates'
        QtMocHelpers::MethodData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'applyUpdate'
        QtMocHelpers::MethodData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'restartApp'
        QtMocHelpers::MethodData<bool()>(9, 2, QMC::AccessPublic, QMetaType::Bool),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'state'
        QtMocHelpers::PropertyData<enum State>(10, 0x80000000 | 11, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'status'
        QtMocHelpers::PropertyData<QString>(12, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'summary'
        QtMocHelpers::PropertyData<QString>(13, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'details'
        QtMocHelpers::PropertyData<QString>(14, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'canUpdate'
        QtMocHelpers::PropertyData<bool>(15, QMetaType::Bool, QMC::DefaultPropertyFlags, 4),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'State'
        QtMocHelpers::EnumData<enum State>(11, 11, QMC::EnumIsScoped).add({
            {   16, State::Idle },
            {   17, State::Checking },
            {   18, State::Applying },
            {   19, State::UpToDate },
            {   20, State::UpdateAvailable },
            {   21, State::Unavailable },
            {   22, State::Error },
        }),
    };
    return QtMocHelpers::metaObjectData<UpdateManager, qt_meta_tag_ZN13UpdateManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject UpdateManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13UpdateManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13UpdateManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13UpdateManagerE_t>.metaTypes,
    nullptr
} };

void UpdateManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<UpdateManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stateChanged(); break;
        case 1: _t->statusChanged(); break;
        case 2: _t->summaryChanged(); break;
        case 3: _t->detailsChanged(); break;
        case 4: _t->canUpdateChanged(); break;
        case 5: _t->checkForUpdates(); break;
        case 6: _t->applyUpdate(); break;
        case 7: { bool _r = _t->restartApp();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (UpdateManager::*)()>(_a, &UpdateManager::stateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (UpdateManager::*)()>(_a, &UpdateManager::statusChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (UpdateManager::*)()>(_a, &UpdateManager::summaryChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (UpdateManager::*)()>(_a, &UpdateManager::detailsChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (UpdateManager::*)()>(_a, &UpdateManager::canUpdateChanged, 4))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<enum State*>(_v) = _t->state(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->status(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->summary(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->details(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->canUpdate(); break;
        default: break;
        }
    }
}

const QMetaObject *UpdateManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UpdateManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13UpdateManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int UpdateManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void UpdateManager::stateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void UpdateManager::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void UpdateManager::summaryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void UpdateManager::detailsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void UpdateManager::canUpdateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
