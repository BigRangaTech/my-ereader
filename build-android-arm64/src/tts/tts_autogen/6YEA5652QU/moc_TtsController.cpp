/****************************************************************************
** Meta object code from reading C++ file 'TtsController.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/tts/include/TtsController.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TtsController.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13TtsControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto TtsController::qt_create_metaobjectdata<qt_meta_tag_ZN13TtsControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TtsController",
        "availabilityChanged",
        "",
        "speakingChanged",
        "rateChanged",
        "pitchChanged",
        "volumeChanged",
        "voiceKeyChanged",
        "voicesChanged",
        "queueLengthChanged",
        "speak",
        "text",
        "enqueue",
        "speakQueue",
        "texts",
        "stop",
        "clearQueue",
        "available",
        "speaking",
        "rate",
        "pitch",
        "volume",
        "voiceKey",
        "voiceKeys",
        "voiceLabels",
        "queueLength"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'availabilityChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'speakingChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'rateChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pitchChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'volumeChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'voiceKeyChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'voicesChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'queueLengthChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'speak'
        QtMocHelpers::MethodData<bool(const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 11 },
        }}),
        // Method 'enqueue'
        QtMocHelpers::MethodData<void(const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Method 'speakQueue'
        QtMocHelpers::MethodData<void(const QStringList &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QStringList, 14 },
        }}),
        // Method 'stop'
        QtMocHelpers::MethodData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'clearQueue'
        QtMocHelpers::MethodData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'available'
        QtMocHelpers::PropertyData<bool>(17, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'speaking'
        QtMocHelpers::PropertyData<bool>(18, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'rate'
        QtMocHelpers::PropertyData<double>(19, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'pitch'
        QtMocHelpers::PropertyData<double>(20, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'volume'
        QtMocHelpers::PropertyData<double>(21, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'voiceKey'
        QtMocHelpers::PropertyData<QString>(22, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
        // property 'voiceKeys'
        QtMocHelpers::PropertyData<QStringList>(23, QMetaType::QStringList, QMC::DefaultPropertyFlags, 6),
        // property 'voiceLabels'
        QtMocHelpers::PropertyData<QStringList>(24, QMetaType::QStringList, QMC::DefaultPropertyFlags, 6),
        // property 'queueLength'
        QtMocHelpers::PropertyData<int>(25, QMetaType::Int, QMC::DefaultPropertyFlags, 7),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TtsController, qt_meta_tag_ZN13TtsControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject TtsController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13TtsControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13TtsControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13TtsControllerE_t>.metaTypes,
    nullptr
} };

void TtsController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TtsController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->availabilityChanged(); break;
        case 1: _t->speakingChanged(); break;
        case 2: _t->rateChanged(); break;
        case 3: _t->pitchChanged(); break;
        case 4: _t->volumeChanged(); break;
        case 5: _t->voiceKeyChanged(); break;
        case 6: _t->voicesChanged(); break;
        case 7: _t->queueLengthChanged(); break;
        case 8: { bool _r = _t->speak((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 9: _t->enqueue((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->speakQueue((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1]))); break;
        case 11: _t->stop(); break;
        case 12: _t->clearQueue(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TtsController::*)()>(_a, &TtsController::availabilityChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TtsController::*)()>(_a, &TtsController::speakingChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TtsController::*)()>(_a, &TtsController::rateChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TtsController::*)()>(_a, &TtsController::pitchChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TtsController::*)()>(_a, &TtsController::volumeChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TtsController::*)()>(_a, &TtsController::voiceKeyChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (TtsController::*)()>(_a, &TtsController::voicesChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (TtsController::*)()>(_a, &TtsController::queueLengthChanged, 7))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->available(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->speaking(); break;
        case 2: *reinterpret_cast<double*>(_v) = _t->rate(); break;
        case 3: *reinterpret_cast<double*>(_v) = _t->pitch(); break;
        case 4: *reinterpret_cast<double*>(_v) = _t->volume(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->voiceKey(); break;
        case 6: *reinterpret_cast<QStringList*>(_v) = _t->voiceKeys(); break;
        case 7: *reinterpret_cast<QStringList*>(_v) = _t->voiceLabels(); break;
        case 8: *reinterpret_cast<int*>(_v) = _t->queueLength(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 2: _t->setRate(*reinterpret_cast<double*>(_v)); break;
        case 3: _t->setPitch(*reinterpret_cast<double*>(_v)); break;
        case 4: _t->setVolume(*reinterpret_cast<double*>(_v)); break;
        case 5: _t->setVoiceKey(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *TtsController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TtsController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13TtsControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TtsController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void TtsController::availabilityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void TtsController::speakingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TtsController::rateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void TtsController::pitchChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void TtsController::volumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void TtsController::voiceKeyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void TtsController::voicesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void TtsController::queueLengthChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
