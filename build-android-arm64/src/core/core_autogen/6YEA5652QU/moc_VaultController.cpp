/****************************************************************************
** Meta object code from reading C++ file 'VaultController.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/include/VaultController.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VaultController.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15VaultControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto VaultController::qt_create_metaobjectdata<qt_meta_tag_ZN15VaultControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "VaultController",
        "stateChanged",
        "",
        "lastErrorChanged",
        "libraryModelChanged",
        "initialize",
        "unlock",
        "passphrase",
        "setupNew",
        "lock",
        "loadStoredPassphrase",
        "storePassphrase",
        "clearStoredPassphrase",
        "state",
        "State",
        "lastError",
        "vaultPath",
        "dbPath",
        "keychainAvailable",
        "libraryModel",
        "Locked",
        "Unlocked",
        "NeedsSetup",
        "Error"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stateChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'libraryModelChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'initialize'
        QtMocHelpers::MethodData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'unlock'
        QtMocHelpers::MethodData<bool(const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 7 },
        }}),
        // Method 'setupNew'
        QtMocHelpers::MethodData<bool(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 7 },
        }}),
        // Method 'lock'
        QtMocHelpers::MethodData<bool(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 7 },
        }}),
        // Method 'loadStoredPassphrase'
        QtMocHelpers::MethodData<QString()>(10, 2, QMC::AccessPublic, QMetaType::QString),
        // Method 'storePassphrase'
        QtMocHelpers::MethodData<bool(const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 7 },
        }}),
        // Method 'clearStoredPassphrase'
        QtMocHelpers::MethodData<bool()>(12, 2, QMC::AccessPublic, QMetaType::Bool),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'state'
        QtMocHelpers::PropertyData<enum State>(13, 0x80000000 | 14, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(15, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'vaultPath'
        QtMocHelpers::PropertyData<QString>(16, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'dbPath'
        QtMocHelpers::PropertyData<QString>(17, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'keychainAvailable'
        QtMocHelpers::PropertyData<bool>(18, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'libraryModel'
        QtMocHelpers::PropertyData<QObject*>(19, QMetaType::QObjectStar, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'State'
        QtMocHelpers::EnumData<enum State>(14, 14, QMC::EnumFlags{}).add({
            {   20, State::Locked },
            {   21, State::Unlocked },
            {   22, State::NeedsSetup },
            {   23, State::Error },
        }),
    };
    return QtMocHelpers::metaObjectData<VaultController, qt_meta_tag_ZN15VaultControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject VaultController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15VaultControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15VaultControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15VaultControllerE_t>.metaTypes,
    nullptr
} };

void VaultController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<VaultController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stateChanged(); break;
        case 1: _t->lastErrorChanged(); break;
        case 2: _t->libraryModelChanged(); break;
        case 3: _t->initialize(); break;
        case 4: { bool _r = _t->unlock((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 5: { bool _r = _t->setupNew((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 6: { bool _r = _t->lock((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 7: { QString _r = _t->loadStoredPassphrase();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 8: { bool _r = _t->storePassphrase((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 9: { bool _r = _t->clearStoredPassphrase();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (VaultController::*)()>(_a, &VaultController::stateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (VaultController::*)()>(_a, &VaultController::lastErrorChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (VaultController::*)()>(_a, &VaultController::libraryModelChanged, 2))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<enum State*>(_v) = _t->state(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->vaultPath(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->dbPath(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->keychainAvailable(); break;
        case 5: *reinterpret_cast<QObject**>(_v) = _t->libraryModel(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 5: _t->setLibraryModel(*reinterpret_cast<QObject**>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *VaultController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VaultController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15VaultControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int VaultController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void VaultController::stateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void VaultController::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void VaultController::libraryModelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
