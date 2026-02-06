/****************************************************************************
** Meta object code from reading C++ file 'ReaderController.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/include/ReaderController.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ReaderController.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16ReaderControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto ReaderController::qt_create_metaobjectdata<qt_meta_tag_ZN16ReaderControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ReaderController",
        "currentChanged",
        "",
        "imageReloadTokenChanged",
        "busyChanged",
        "lastErrorChanged",
        "openFile",
        "path",
        "openFileAsync",
        "close",
        "jumpToLocator",
        "locator",
        "nextChapter",
        "prevChapter",
        "goToChapter",
        "index",
        "chapterTitle",
        "tocTitle",
        "tocChapterIndex",
        "nextImage",
        "prevImage",
        "goToImage",
        "imageUrlAt",
        "QUrl",
        "currentTitle",
        "currentText",
        "currentPlainText",
        "currentTextIsRich",
        "currentPath",
        "currentFormat",
        "isOpen",
        "currentChapterIndex",
        "currentChapterTitle",
        "chapterCount",
        "tocCount",
        "hasImages",
        "currentImageIndex",
        "imageCount",
        "currentImagePath",
        "currentImageUrl",
        "imageReloadToken",
        "currentCoverPath",
        "currentCoverUrl",
        "busy",
        "lastError",
        "ttsAllowed"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'currentChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'imageReloadTokenChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'busyChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openFile'
        QtMocHelpers::MethodData<bool(const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 7 },
        }}),
        // Method 'openFileAsync'
        QtMocHelpers::MethodData<void(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Method 'close'
        QtMocHelpers::MethodData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'jumpToLocator'
        QtMocHelpers::MethodData<bool(const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 11 },
        }}),
        // Method 'nextChapter'
        QtMocHelpers::MethodData<bool()>(12, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'prevChapter'
        QtMocHelpers::MethodData<bool()>(13, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'goToChapter'
        QtMocHelpers::MethodData<bool(int)>(14, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'chapterTitle'
        QtMocHelpers::MethodData<QString(int) const>(16, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'tocTitle'
        QtMocHelpers::MethodData<QString(int) const>(17, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'tocChapterIndex'
        QtMocHelpers::MethodData<int(int) const>(18, 2, QMC::AccessPublic, QMetaType::Int, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'nextImage'
        QtMocHelpers::MethodData<bool()>(19, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'prevImage'
        QtMocHelpers::MethodData<bool()>(20, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'goToImage'
        QtMocHelpers::MethodData<bool(int)>(21, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 15 },
        }}),
        // Method 'imageUrlAt'
        QtMocHelpers::MethodData<QUrl(int) const>(22, 2, QMC::AccessPublic, 0x80000000 | 23, {{
            { QMetaType::Int, 15 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'currentTitle'
        QtMocHelpers::PropertyData<QString>(24, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentText'
        QtMocHelpers::PropertyData<QString>(25, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentPlainText'
        QtMocHelpers::PropertyData<QString>(26, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentTextIsRich'
        QtMocHelpers::PropertyData<bool>(27, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'currentPath'
        QtMocHelpers::PropertyData<QString>(28, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentFormat'
        QtMocHelpers::PropertyData<QString>(29, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'isOpen'
        QtMocHelpers::PropertyData<bool>(30, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'currentChapterIndex'
        QtMocHelpers::PropertyData<int>(31, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'currentChapterTitle'
        QtMocHelpers::PropertyData<QString>(32, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'chapterCount'
        QtMocHelpers::PropertyData<int>(33, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'tocCount'
        QtMocHelpers::PropertyData<int>(34, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'hasImages'
        QtMocHelpers::PropertyData<bool>(35, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'currentImageIndex'
        QtMocHelpers::PropertyData<int>(36, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'imageCount'
        QtMocHelpers::PropertyData<int>(37, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'currentImagePath'
        QtMocHelpers::PropertyData<QString>(38, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentImageUrl'
        QtMocHelpers::PropertyData<QUrl>(39, 0x80000000 | 23, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'imageReloadToken'
        QtMocHelpers::PropertyData<int>(40, QMetaType::Int, QMC::DefaultPropertyFlags, 1),
        // property 'currentCoverPath'
        QtMocHelpers::PropertyData<QString>(41, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'currentCoverUrl'
        QtMocHelpers::PropertyData<QUrl>(42, 0x80000000 | 23, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'busy'
        QtMocHelpers::PropertyData<bool>(43, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(44, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'ttsAllowed'
        QtMocHelpers::PropertyData<bool>(45, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ReaderController, qt_meta_tag_ZN16ReaderControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ReaderController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ReaderControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ReaderControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16ReaderControllerE_t>.metaTypes,
    nullptr
} };

void ReaderController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ReaderController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->currentChanged(); break;
        case 1: _t->imageReloadTokenChanged(); break;
        case 2: _t->busyChanged(); break;
        case 3: _t->lastErrorChanged(); break;
        case 4: { bool _r = _t->openFile((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 5: _t->openFileAsync((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->close(); break;
        case 7: { bool _r = _t->jumpToLocator((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 8: { bool _r = _t->nextChapter();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 9: { bool _r = _t->prevChapter();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 10: { bool _r = _t->goToChapter((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 11: { QString _r = _t->chapterTitle((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 12: { QString _r = _t->tocTitle((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 13: { int _r = _t->tocChapterIndex((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        case 14: { bool _r = _t->nextImage();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 15: { bool _r = _t->prevImage();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 16: { bool _r = _t->goToImage((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 17: { QUrl _r = _t->imageUrlAt((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QUrl*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ReaderController::*)()>(_a, &ReaderController::currentChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReaderController::*)()>(_a, &ReaderController::imageReloadTokenChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReaderController::*)()>(_a, &ReaderController::busyChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReaderController::*)()>(_a, &ReaderController::lastErrorChanged, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->currentTitle(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->currentText(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->currentPlainText(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->currentTextIsRich(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->currentPath(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->currentFormat(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->isOpen(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->currentChapterIndex(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->currentChapterTitle(); break;
        case 9: *reinterpret_cast<int*>(_v) = _t->chapterCount(); break;
        case 10: *reinterpret_cast<int*>(_v) = _t->tocCount(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->hasImages(); break;
        case 12: *reinterpret_cast<int*>(_v) = _t->currentImageIndex(); break;
        case 13: *reinterpret_cast<int*>(_v) = _t->imageCount(); break;
        case 14: *reinterpret_cast<QString*>(_v) = _t->currentImagePath(); break;
        case 15: *reinterpret_cast<QUrl*>(_v) = _t->currentImageUrl(); break;
        case 16: *reinterpret_cast<int*>(_v) = _t->imageReloadToken(); break;
        case 17: *reinterpret_cast<QString*>(_v) = _t->currentCoverPath(); break;
        case 18: *reinterpret_cast<QUrl*>(_v) = _t->currentCoverUrl(); break;
        case 19: *reinterpret_cast<bool*>(_v) = _t->busy(); break;
        case 20: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        case 21: *reinterpret_cast<bool*>(_v) = _t->ttsAllowed(); break;
        default: break;
        }
    }
}

const QMetaObject *ReaderController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ReaderController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ReaderControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ReaderController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    return _id;
}

// SIGNAL 0
void ReaderController::currentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ReaderController::imageReloadTokenChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ReaderController::busyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ReaderController::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
