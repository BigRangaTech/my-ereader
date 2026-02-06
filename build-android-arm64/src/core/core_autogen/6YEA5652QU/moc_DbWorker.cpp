/****************************************************************************
** Meta object code from reading C++ file 'DbWorker.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/include/DbWorker.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DbWorker.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8DbWorkerE_t {};
} // unnamed namespace

template <> constexpr inline auto DbWorker::qt_create_metaobjectdata<qt_meta_tag_ZN8DbWorkerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DbWorker",
        "openFinished",
        "",
        "ok",
        "error",
        "saveFinished",
        "libraryLoaded",
        "QList<LibraryItem>",
        "items",
        "totalCount",
        "facetsLoaded",
        "collections",
        "tags",
        "annotationCountChanged",
        "libraryItemId",
        "count",
        "annotationsLoaded",
        "QList<AnnotationItem>",
        "addBookFinished",
        "updateBookFinished",
        "deleteBookFinished",
        "addAnnotationFinished",
        "updateAnnotationFinished",
        "deleteAnnotationFinished",
        "openAt",
        "dbPath",
        "openEncryptedVault",
        "vaultPath",
        "passphrase",
        "saveEncryptedVault",
        "closeDatabase",
        "addBook",
        "filePath",
        "updateLibraryItem",
        "id",
        "title",
        "authors",
        "series",
        "publisher",
        "description",
        "collection",
        "deleteLibraryItem",
        "bulkUpdateTagsCollection",
        "QList<int>",
        "ids",
        "updateTags",
        "updateCollection",
        "deleteLibraryItems",
        "loadLibrary",
        "loadLibraryFiltered",
        "searchQuery",
        "sortKey",
        "sortDescending",
        "filterTag",
        "filterCollection",
        "pageSize",
        "pageIndex",
        "loadAnnotations",
        "addAnnotation",
        "locator",
        "type",
        "text",
        "color",
        "updateAnnotation",
        "annotationId",
        "deleteAnnotation",
        "exportAnnotationSync",
        "QVariantList",
        "importAnnotationSync",
        "payload",
        "exportLibrarySync",
        "importLibrarySync",
        "conflictPolicy",
        "hasFileHash",
        "fileHash",
        "pathForHash"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'openFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'saveFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'libraryLoaded'
        QtMocHelpers::SignalData<void(const QVector<LibraryItem> &, int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 }, { QMetaType::Int, 9 },
        }}),
        // Signal 'facetsLoaded'
        QtMocHelpers::SignalData<void(const QStringList &, const QStringList &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QStringList, 11 }, { QMetaType::QStringList, 12 },
        }}),
        // Signal 'annotationCountChanged'
        QtMocHelpers::SignalData<void(int, int)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 }, { QMetaType::Int, 15 },
        }}),
        // Signal 'annotationsLoaded'
        QtMocHelpers::SignalData<void(int, const QVector<AnnotationItem> &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 }, { 0x80000000 | 17, 8 },
        }}),
        // Signal 'addBookFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'updateBookFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'deleteBookFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'addAnnotationFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'updateAnnotationFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'deleteAnnotationFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Slot 'openAt'
        QtMocHelpers::SlotData<void(const QString &)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 25 },
        }}),
        // Slot 'openEncryptedVault'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 27 }, { QMetaType::QString, 28 },
        }}),
        // Slot 'saveEncryptedVault'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 27 }, { QMetaType::QString, 28 },
        }}),
        // Slot 'closeDatabase'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'addBook'
        QtMocHelpers::SlotData<void(const QString &)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 32 },
        }}),
        // Slot 'updateLibraryItem'
        QtMocHelpers::SlotData<void(int, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 34 }, { QMetaType::QString, 35 }, { QMetaType::QString, 36 }, { QMetaType::QString, 37 },
            { QMetaType::QString, 38 }, { QMetaType::QString, 39 }, { QMetaType::QString, 12 }, { QMetaType::QString, 40 },
        }}),
        // Slot 'deleteLibraryItem'
        QtMocHelpers::SlotData<void(int)>(41, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'bulkUpdateTagsCollection'
        QtMocHelpers::SlotData<void(const QVector<int> &, const QString &, const QString &, bool, bool)>(42, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 43, 44 }, { QMetaType::QString, 12 }, { QMetaType::QString, 40 }, { QMetaType::Bool, 45 },
            { QMetaType::Bool, 46 },
        }}),
        // Slot 'deleteLibraryItems'
        QtMocHelpers::SlotData<void(const QVector<int> &)>(47, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 43, 44 },
        }}),
        // Slot 'loadLibrary'
        QtMocHelpers::SlotData<void()>(48, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'loadLibraryFiltered'
        QtMocHelpers::SlotData<void(const QString &, const QString &, bool, const QString &, const QString &, int, int)>(49, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 50 }, { QMetaType::QString, 51 }, { QMetaType::Bool, 52 }, { QMetaType::QString, 53 },
            { QMetaType::QString, 54 }, { QMetaType::Int, 55 }, { QMetaType::Int, 56 },
        }}),
        // Slot 'loadAnnotations'
        QtMocHelpers::SlotData<void(int)>(57, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 },
        }}),
        // Slot 'addAnnotation'
        QtMocHelpers::SlotData<void(int, const QString &, const QString &, const QString &, const QString &)>(58, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 }, { QMetaType::QString, 59 }, { QMetaType::QString, 60 }, { QMetaType::QString, 61 },
            { QMetaType::QString, 62 },
        }}),
        // Slot 'updateAnnotation'
        QtMocHelpers::SlotData<void(int, int, const QString &, const QString &, const QString &, const QString &)>(63, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 64 }, { QMetaType::Int, 14 }, { QMetaType::QString, 59 }, { QMetaType::QString, 60 },
            { QMetaType::QString, 61 }, { QMetaType::QString, 62 },
        }}),
        // Slot 'deleteAnnotation'
        QtMocHelpers::SlotData<void(int, int)>(65, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 64 }, { QMetaType::Int, 14 },
        }}),
        // Slot 'exportAnnotationSync'
        QtMocHelpers::SlotData<QVariantList()>(66, 2, QMC::AccessPublic, 0x80000000 | 67),
        // Slot 'importAnnotationSync'
        QtMocHelpers::SlotData<int(const QVariantList &)>(68, 2, QMC::AccessPublic, QMetaType::Int, {{
            { 0x80000000 | 67, 69 },
        }}),
        // Slot 'exportLibrarySync'
        QtMocHelpers::SlotData<QVariantList()>(70, 2, QMC::AccessPublic, 0x80000000 | 67),
        // Slot 'importLibrarySync'
        QtMocHelpers::SlotData<int(const QVariantList &, const QString &)>(71, 2, QMC::AccessPublic, QMetaType::Int, {{
            { 0x80000000 | 67, 69 }, { QMetaType::QString, 72 },
        }}),
        // Slot 'hasFileHash'
        QtMocHelpers::SlotData<bool(const QString &)>(73, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 74 },
        }}),
        // Slot 'pathForHash'
        QtMocHelpers::SlotData<QString(const QString &)>(75, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 74 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DbWorker, qt_meta_tag_ZN8DbWorkerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DbWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8DbWorkerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8DbWorkerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8DbWorkerE_t>.metaTypes,
    nullptr
} };

void DbWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DbWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->openFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->saveFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: _t->libraryLoaded((*reinterpret_cast<std::add_pointer_t<QList<LibraryItem>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->facetsLoaded((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[2]))); break;
        case 4: _t->annotationCountChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 5: _t->annotationsLoaded((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<AnnotationItem>>>(_a[2]))); break;
        case 6: _t->addBookFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 7: _t->updateBookFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 8: _t->deleteBookFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->addAnnotationFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->updateAnnotationFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 11: _t->deleteAnnotationFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 12: _t->openAt((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->openEncryptedVault((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 14: _t->saveEncryptedVault((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 15: _t->closeDatabase(); break;
        case 16: _t->addBook((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->updateLibraryItem((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[6])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[7])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[8]))); break;
        case 18: _t->deleteLibraryItem((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->bulkUpdateTagsCollection((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[5]))); break;
        case 20: _t->deleteLibraryItems((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 21: _t->loadLibrary(); break;
        case 22: _t->loadLibraryFiltered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[6])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[7]))); break;
        case 23: _t->loadAnnotations((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 24: _t->addAnnotation((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[5]))); break;
        case 25: _t->updateAnnotation((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[6]))); break;
        case 26: _t->deleteAnnotation((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 27: { QVariantList _r = _t->exportAnnotationSync();
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 28: { int _r = _t->importAnnotationSync((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        case 29: { QVariantList _r = _t->exportLibrarySync();
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 30: { int _r = _t->importLibrarySync((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        case 31: { bool _r = _t->hasFileHash((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 32: { QString _r = _t->pathForHash((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<LibraryItem> >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<AnnotationItem> >(); break;
            }
            break;
        case 19:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 20:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(bool , const QString & )>(_a, &DbWorker::openFinished, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(bool , const QString & )>(_a, &DbWorker::saveFinished, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(const QVector<LibraryItem> & , int )>(_a, &DbWorker::libraryLoaded, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(const QStringList & , const QStringList & )>(_a, &DbWorker::facetsLoaded, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(int , int )>(_a, &DbWorker::annotationCountChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(int , const QVector<AnnotationItem> & )>(_a, &DbWorker::annotationsLoaded, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(bool , const QString & )>(_a, &DbWorker::addBookFinished, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(bool , const QString & )>(_a, &DbWorker::updateBookFinished, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(bool , const QString & )>(_a, &DbWorker::deleteBookFinished, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(bool , const QString & )>(_a, &DbWorker::addAnnotationFinished, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(bool , const QString & )>(_a, &DbWorker::updateAnnotationFinished, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (DbWorker::*)(bool , const QString & )>(_a, &DbWorker::deleteAnnotationFinished, 11))
            return;
    }
}

const QMetaObject *DbWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DbWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8DbWorkerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DbWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 33)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 33;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 33)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 33;
    }
    return _id;
}

// SIGNAL 0
void DbWorker::openFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void DbWorker::saveFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void DbWorker::libraryLoaded(const QVector<LibraryItem> & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void DbWorker::facetsLoaded(const QStringList & _t1, const QStringList & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void DbWorker::annotationCountChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void DbWorker::annotationsLoaded(int _t1, const QVector<AnnotationItem> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2);
}

// SIGNAL 6
void DbWorker::addBookFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2);
}

// SIGNAL 7
void DbWorker::updateBookFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2);
}

// SIGNAL 8
void DbWorker::deleteBookFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2);
}

// SIGNAL 9
void DbWorker::addAnnotationFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2);
}

// SIGNAL 10
void DbWorker::updateAnnotationFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1, _t2);
}

// SIGNAL 11
void DbWorker::deleteAnnotationFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1, _t2);
}
QT_WARNING_POP
