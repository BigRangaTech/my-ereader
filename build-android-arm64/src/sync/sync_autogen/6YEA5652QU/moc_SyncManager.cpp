/****************************************************************************
** Meta object code from reading C++ file 'SyncManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/sync/include/SyncManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SyncManager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11SyncManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto SyncManager::qt_create_metaobjectdata<qt_meta_tag_ZN11SyncManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SyncManager",
        "enabledChanged",
        "",
        "statusChanged",
        "deviceNameChanged",
        "pinChanged",
        "discoveryPortChanged",
        "listenPortChanged",
        "discoveringChanged",
        "devicesChanged",
        "libraryModelChanged",
        "conflictPolicyChanged",
        "transferEnabledChanged",
        "transferMaxMbChanged",
        "transferProgressChanged",
        "uploadProgressChanged",
        "startDiscovery",
        "stopDiscovery",
        "requestPairing",
        "deviceId",
        "unpair",
        "syncNow",
        "enabled",
        "status",
        "deviceName",
        "pin",
        "discoveryPort",
        "listenPort",
        "discovering",
        "devices",
        "QVariantList",
        "libraryModel",
        "conflictPolicy",
        "transferEnabled",
        "transferMaxMb",
        "transferActive",
        "transferTotal",
        "transferDone",
        "uploadActive",
        "uploadTotal",
        "uploadDone"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'enabledChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'deviceNameChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pinChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'discoveryPortChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'listenPortChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'discoveringChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'devicesChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'libraryModelChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'conflictPolicyChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'transferEnabledChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'transferMaxMbChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'transferProgressChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'uploadProgressChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'startDiscovery'
        QtMocHelpers::MethodData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'stopDiscovery'
        QtMocHelpers::MethodData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'requestPairing'
        QtMocHelpers::MethodData<void(const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Method 'unpair'
        QtMocHelpers::MethodData<void(const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Method 'syncNow'
        QtMocHelpers::MethodData<void(const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'enabled'
        QtMocHelpers::PropertyData<bool>(22, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'status'
        QtMocHelpers::PropertyData<QString>(23, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'deviceName'
        QtMocHelpers::PropertyData<QString>(24, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'deviceId'
        QtMocHelpers::PropertyData<QString>(19, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'pin'
        QtMocHelpers::PropertyData<QString>(25, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'discoveryPort'
        QtMocHelpers::PropertyData<int>(26, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'listenPort'
        QtMocHelpers::PropertyData<int>(27, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
        // property 'discovering'
        QtMocHelpers::PropertyData<bool>(28, QMetaType::Bool, QMC::DefaultPropertyFlags, 6),
        // property 'devices'
        QtMocHelpers::PropertyData<QVariantList>(29, 0x80000000 | 30, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 7),
        // property 'libraryModel'
        QtMocHelpers::PropertyData<QObject*>(31, QMetaType::QObjectStar, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 8),
        // property 'conflictPolicy'
        QtMocHelpers::PropertyData<QString>(32, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 9),
        // property 'transferEnabled'
        QtMocHelpers::PropertyData<bool>(33, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 10),
        // property 'transferMaxMb'
        QtMocHelpers::PropertyData<int>(34, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 11),
        // property 'transferActive'
        QtMocHelpers::PropertyData<bool>(35, QMetaType::Bool, QMC::DefaultPropertyFlags, 12),
        // property 'transferTotal'
        QtMocHelpers::PropertyData<int>(36, QMetaType::Int, QMC::DefaultPropertyFlags, 12),
        // property 'transferDone'
        QtMocHelpers::PropertyData<int>(37, QMetaType::Int, QMC::DefaultPropertyFlags, 12),
        // property 'uploadActive'
        QtMocHelpers::PropertyData<bool>(38, QMetaType::Bool, QMC::DefaultPropertyFlags, 13),
        // property 'uploadTotal'
        QtMocHelpers::PropertyData<int>(39, QMetaType::Int, QMC::DefaultPropertyFlags, 13),
        // property 'uploadDone'
        QtMocHelpers::PropertyData<int>(40, QMetaType::Int, QMC::DefaultPropertyFlags, 13),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SyncManager, qt_meta_tag_ZN11SyncManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SyncManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11SyncManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11SyncManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11SyncManagerE_t>.metaTypes,
    nullptr
} };

void SyncManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SyncManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->enabledChanged(); break;
        case 1: _t->statusChanged(); break;
        case 2: _t->deviceNameChanged(); break;
        case 3: _t->pinChanged(); break;
        case 4: _t->discoveryPortChanged(); break;
        case 5: _t->listenPortChanged(); break;
        case 6: _t->discoveringChanged(); break;
        case 7: _t->devicesChanged(); break;
        case 8: _t->libraryModelChanged(); break;
        case 9: _t->conflictPolicyChanged(); break;
        case 10: _t->transferEnabledChanged(); break;
        case 11: _t->transferMaxMbChanged(); break;
        case 12: _t->transferProgressChanged(); break;
        case 13: _t->uploadProgressChanged(); break;
        case 14: _t->startDiscovery(); break;
        case 15: _t->stopDiscovery(); break;
        case 16: _t->requestPairing((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->unpair((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 18: _t->syncNow((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::enabledChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::statusChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::deviceNameChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::pinChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::discoveryPortChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::listenPortChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::discoveringChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::devicesChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::libraryModelChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::conflictPolicyChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::transferEnabledChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::transferMaxMbChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::transferProgressChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (SyncManager::*)()>(_a, &SyncManager::uploadProgressChanged, 13))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->enabled(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->status(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->deviceName(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->deviceId(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->pin(); break;
        case 5: *reinterpret_cast<int*>(_v) = _t->discoveryPort(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->listenPort(); break;
        case 7: *reinterpret_cast<bool*>(_v) = _t->discovering(); break;
        case 8: *reinterpret_cast<QVariantList*>(_v) = _t->devices(); break;
        case 9: *reinterpret_cast<QObject**>(_v) = _t->libraryModel(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->conflictPolicy(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->transferEnabled(); break;
        case 12: *reinterpret_cast<int*>(_v) = _t->transferMaxMb(); break;
        case 13: *reinterpret_cast<bool*>(_v) = _t->transferActive(); break;
        case 14: *reinterpret_cast<int*>(_v) = _t->transferTotal(); break;
        case 15: *reinterpret_cast<int*>(_v) = _t->transferDone(); break;
        case 16: *reinterpret_cast<bool*>(_v) = _t->uploadActive(); break;
        case 17: *reinterpret_cast<int*>(_v) = _t->uploadTotal(); break;
        case 18: *reinterpret_cast<int*>(_v) = _t->uploadDone(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setEnabled(*reinterpret_cast<bool*>(_v)); break;
        case 2: _t->setDeviceName(*reinterpret_cast<QString*>(_v)); break;
        case 4: _t->setPin(*reinterpret_cast<QString*>(_v)); break;
        case 5: _t->setDiscoveryPort(*reinterpret_cast<int*>(_v)); break;
        case 6: _t->setListenPort(*reinterpret_cast<int*>(_v)); break;
        case 9: _t->setLibraryModel(*reinterpret_cast<QObject**>(_v)); break;
        case 10: _t->setConflictPolicy(*reinterpret_cast<QString*>(_v)); break;
        case 11: _t->setTransferEnabled(*reinterpret_cast<bool*>(_v)); break;
        case 12: _t->setTransferMaxMb(*reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *SyncManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SyncManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11SyncManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SyncManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 19;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void SyncManager::enabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SyncManager::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SyncManager::deviceNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SyncManager::pinChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SyncManager::discoveryPortChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SyncManager::listenPortChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void SyncManager::discoveringChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void SyncManager::devicesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void SyncManager::libraryModelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void SyncManager::conflictPolicyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void SyncManager::transferEnabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void SyncManager::transferMaxMbChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void SyncManager::transferProgressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void SyncManager::uploadProgressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}
QT_WARNING_POP
