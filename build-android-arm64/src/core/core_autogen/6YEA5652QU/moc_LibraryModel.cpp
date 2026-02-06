/****************************************************************************
** Meta object code from reading C++ file 'LibraryModel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/include/LibraryModel.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LibraryModel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12LibraryModelE_t {};
} // unnamed namespace

template <> constexpr inline auto LibraryModel::qt_create_metaobjectdata<qt_meta_tag_ZN12LibraryModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "LibraryModel",
        "readyChanged",
        "",
        "countChanged",
        "totalCountChanged",
        "pageSizeChanged",
        "pageIndexChanged",
        "pageCountChanged",
        "availableCollectionsChanged",
        "availableTagsChanged",
        "lastErrorChanged",
        "searchQueryChanged",
        "sortKeyChanged",
        "sortDescendingChanged",
        "filterTagChanged",
        "filterCollectionChanged",
        "bulkImportChanged",
        "openDefault",
        "openAt",
        "dbPath",
        "addBook",
        "filePath",
        "addBooks",
        "filePaths",
        "addFolder",
        "folderPath",
        "recursive",
        "updateMetadata",
        "id",
        "title",
        "authors",
        "series",
        "publisher",
        "description",
        "tags",
        "collection",
        "removeBook",
        "updateTagsCollection",
        "QVariantList",
        "ids",
        "updateTags",
        "updateCollection",
        "removeBooks",
        "cancelBulkImport",
        "get",
        "QVariantMap",
        "index",
        "exportAnnotationSync",
        "importAnnotationSync",
        "payload",
        "exportLibrarySync",
        "importLibrarySync",
        "conflictPolicy",
        "hasFileHash",
        "fileHash",
        "pathForHash",
        "nextPage",
        "prevPage",
        "goToPage",
        "close",
        "openEncryptedVault",
        "vaultPath",
        "passphrase",
        "saveEncryptedVault",
        "connectionName",
        "ready",
        "count",
        "totalCount",
        "pageSize",
        "pageIndex",
        "pageCount",
        "availableCollections",
        "availableTags",
        "lastError",
        "searchQuery",
        "sortKey",
        "sortDescending",
        "filterTag",
        "filterCollection",
        "bulkImportActive",
        "bulkImportTotal",
        "bulkImportDone"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'readyChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'countChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'totalCountChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pageSizeChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pageIndexChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pageCountChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'availableCollectionsChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'availableTagsChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastErrorChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'searchQueryChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sortKeyChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sortDescendingChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'filterTagChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'filterCollectionChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'bulkImportChanged'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openDefault'
        QtMocHelpers::MethodData<bool()>(17, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'openAt'
        QtMocHelpers::MethodData<bool(const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 19 },
        }}),
        // Method 'addBook'
        QtMocHelpers::MethodData<bool(const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 21 },
        }}),
        // Method 'addBooks'
        QtMocHelpers::MethodData<bool(const QStringList &)>(22, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QStringList, 23 },
        }}),
        // Method 'addFolder'
        QtMocHelpers::MethodData<bool(const QString &, bool)>(24, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 25 }, { QMetaType::Bool, 26 },
        }}),
        // Method 'updateMetadata'
        QtMocHelpers::MethodData<bool(int, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &)>(27, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 28 }, { QMetaType::QString, 29 }, { QMetaType::QString, 30 }, { QMetaType::QString, 31 },
            { QMetaType::QString, 32 }, { QMetaType::QString, 33 }, { QMetaType::QString, 34 }, { QMetaType::QString, 35 },
        }}),
        // Method 'removeBook'
        QtMocHelpers::MethodData<bool(int)>(36, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 28 },
        }}),
        // Method 'updateTagsCollection'
        QtMocHelpers::MethodData<bool(const QVariantList &, const QString &, const QString &, bool, bool)>(37, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { 0x80000000 | 38, 39 }, { QMetaType::QString, 34 }, { QMetaType::QString, 35 }, { QMetaType::Bool, 40 },
            { QMetaType::Bool, 41 },
        }}),
        // Method 'removeBooks'
        QtMocHelpers::MethodData<bool(const QVariantList &)>(42, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { 0x80000000 | 38, 39 },
        }}),
        // Method 'cancelBulkImport'
        QtMocHelpers::MethodData<void()>(43, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'get'
        QtMocHelpers::MethodData<QVariantMap(int) const>(44, 2, QMC::AccessPublic, 0x80000000 | 45, {{
            { QMetaType::Int, 46 },
        }}),
        // Method 'exportAnnotationSync'
        QtMocHelpers::MethodData<QVariantList() const>(47, 2, QMC::AccessPublic, 0x80000000 | 38),
        // Method 'importAnnotationSync'
        QtMocHelpers::MethodData<int(const QVariantList &)>(48, 2, QMC::AccessPublic, QMetaType::Int, {{
            { 0x80000000 | 38, 49 },
        }}),
        // Method 'exportLibrarySync'
        QtMocHelpers::MethodData<QVariantList() const>(50, 2, QMC::AccessPublic, 0x80000000 | 38),
        // Method 'importLibrarySync'
        QtMocHelpers::MethodData<int(const QVariantList &, const QString &)>(51, 2, QMC::AccessPublic, QMetaType::Int, {{
            { 0x80000000 | 38, 49 }, { QMetaType::QString, 52 },
        }}),
        // Method 'hasFileHash'
        QtMocHelpers::MethodData<bool(const QString &) const>(53, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 54 },
        }}),
        // Method 'pathForHash'
        QtMocHelpers::MethodData<QString(const QString &) const>(55, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 54 },
        }}),
        // Method 'nextPage'
        QtMocHelpers::MethodData<void()>(56, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'prevPage'
        QtMocHelpers::MethodData<void()>(57, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'goToPage'
        QtMocHelpers::MethodData<void(int)>(58, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 46 },
        }}),
        // Method 'close'
        QtMocHelpers::MethodData<void()>(59, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openEncryptedVault'
        QtMocHelpers::MethodData<bool(const QString &, const QString &)>(60, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 61 }, { QMetaType::QString, 62 },
        }}),
        // Method 'saveEncryptedVault'
        QtMocHelpers::MethodData<bool(const QString &, const QString &)>(63, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 61 }, { QMetaType::QString, 62 },
        }}),
        // Method 'connectionName'
        QtMocHelpers::MethodData<QString() const>(64, 2, QMC::AccessPublic, QMetaType::QString),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'ready'
        QtMocHelpers::PropertyData<bool>(65, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'count'
        QtMocHelpers::PropertyData<int>(66, QMetaType::Int, QMC::DefaultPropertyFlags, 1),
        // property 'totalCount'
        QtMocHelpers::PropertyData<int>(67, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
        // property 'pageSize'
        QtMocHelpers::PropertyData<int>(68, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'pageIndex'
        QtMocHelpers::PropertyData<int>(69, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'pageCount'
        QtMocHelpers::PropertyData<int>(70, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'availableCollections'
        QtMocHelpers::PropertyData<QStringList>(71, QMetaType::QStringList, QMC::DefaultPropertyFlags, 6),
        // property 'availableTags'
        QtMocHelpers::PropertyData<QStringList>(72, QMetaType::QStringList, QMC::DefaultPropertyFlags, 7),
        // property 'lastError'
        QtMocHelpers::PropertyData<QString>(73, QMetaType::QString, QMC::DefaultPropertyFlags, 8),
        // property 'searchQuery'
        QtMocHelpers::PropertyData<QString>(74, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 9),
        // property 'sortKey'
        QtMocHelpers::PropertyData<QString>(75, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 10),
        // property 'sortDescending'
        QtMocHelpers::PropertyData<bool>(76, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 11),
        // property 'filterTag'
        QtMocHelpers::PropertyData<QString>(77, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 12),
        // property 'filterCollection'
        QtMocHelpers::PropertyData<QString>(78, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 13),
        // property 'bulkImportActive'
        QtMocHelpers::PropertyData<bool>(79, QMetaType::Bool, QMC::DefaultPropertyFlags, 14),
        // property 'bulkImportTotal'
        QtMocHelpers::PropertyData<int>(80, QMetaType::Int, QMC::DefaultPropertyFlags, 14),
        // property 'bulkImportDone'
        QtMocHelpers::PropertyData<int>(81, QMetaType::Int, QMC::DefaultPropertyFlags, 14),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LibraryModel, qt_meta_tag_ZN12LibraryModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject LibraryModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LibraryModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LibraryModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12LibraryModelE_t>.metaTypes,
    nullptr
} };

void LibraryModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LibraryModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->readyChanged(); break;
        case 1: _t->countChanged(); break;
        case 2: _t->totalCountChanged(); break;
        case 3: _t->pageSizeChanged(); break;
        case 4: _t->pageIndexChanged(); break;
        case 5: _t->pageCountChanged(); break;
        case 6: _t->availableCollectionsChanged(); break;
        case 7: _t->availableTagsChanged(); break;
        case 8: _t->lastErrorChanged(); break;
        case 9: _t->searchQueryChanged(); break;
        case 10: _t->sortKeyChanged(); break;
        case 11: _t->sortDescendingChanged(); break;
        case 12: _t->filterTagChanged(); break;
        case 13: _t->filterCollectionChanged(); break;
        case 14: _t->bulkImportChanged(); break;
        case 15: { bool _r = _t->openDefault();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 16: { bool _r = _t->openAt((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 17: { bool _r = _t->addBook((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 18: { bool _r = _t->addBooks((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 19: { bool _r = _t->addFolder((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 20: { bool _r = _t->updateMetadata((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[6])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[7])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[8])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 21: { bool _r = _t->removeBook((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 22: { bool _r = _t->updateTagsCollection((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[5])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 23: { bool _r = _t->removeBooks((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 24: _t->cancelBulkImport(); break;
        case 25: { QVariantMap _r = _t->get((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantMap*>(_a[0]) = std::move(_r); }  break;
        case 26: { QVariantList _r = _t->exportAnnotationSync();
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 27: { int _r = _t->importAnnotationSync((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        case 28: { QVariantList _r = _t->exportLibrarySync();
            if (_a[0]) *reinterpret_cast<QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 29: { int _r = _t->importLibrarySync((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        case 30: { bool _r = _t->hasFileHash((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 31: { QString _r = _t->pathForHash((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 32: _t->nextPage(); break;
        case 33: _t->prevPage(); break;
        case 34: _t->goToPage((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 35: _t->close(); break;
        case 36: { bool _r = _t->openEncryptedVault((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 37: { bool _r = _t->saveEncryptedVault((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 38: { QString _r = _t->connectionName();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::readyChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::countChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::totalCountChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::pageSizeChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::pageIndexChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::pageCountChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::availableCollectionsChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::availableTagsChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::lastErrorChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::searchQueryChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::sortKeyChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::sortDescendingChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::filterTagChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::filterCollectionChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (LibraryModel::*)()>(_a, &LibraryModel::bulkImportChanged, 14))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->ready(); break;
        case 1: *reinterpret_cast<int*>(_v) = _t->count(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->totalCount(); break;
        case 3: *reinterpret_cast<int*>(_v) = _t->pageSize(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->pageIndex(); break;
        case 5: *reinterpret_cast<int*>(_v) = _t->pageCount(); break;
        case 6: *reinterpret_cast<QStringList*>(_v) = _t->availableCollections(); break;
        case 7: *reinterpret_cast<QStringList*>(_v) = _t->availableTags(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->lastError(); break;
        case 9: *reinterpret_cast<QString*>(_v) = _t->searchQuery(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->sortKey(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->sortDescending(); break;
        case 12: *reinterpret_cast<QString*>(_v) = _t->filterTag(); break;
        case 13: *reinterpret_cast<QString*>(_v) = _t->filterCollection(); break;
        case 14: *reinterpret_cast<bool*>(_v) = _t->bulkImportActive(); break;
        case 15: *reinterpret_cast<int*>(_v) = _t->bulkImportTotal(); break;
        case 16: *reinterpret_cast<int*>(_v) = _t->bulkImportDone(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 3: _t->setPageSize(*reinterpret_cast<int*>(_v)); break;
        case 4: _t->setPageIndex(*reinterpret_cast<int*>(_v)); break;
        case 9: _t->setSearchQuery(*reinterpret_cast<QString*>(_v)); break;
        case 10: _t->setSortKey(*reinterpret_cast<QString*>(_v)); break;
        case 11: _t->setSortDescending(*reinterpret_cast<bool*>(_v)); break;
        case 12: _t->setFilterTag(*reinterpret_cast<QString*>(_v)); break;
        case 13: _t->setFilterCollection(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *LibraryModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LibraryModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LibraryModelE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int LibraryModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 39)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 39;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 39)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 39;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void LibraryModel::readyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LibraryModel::countChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LibraryModel::totalCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LibraryModel::pageSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void LibraryModel::pageIndexChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void LibraryModel::pageCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void LibraryModel::availableCollectionsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void LibraryModel::availableTagsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void LibraryModel::lastErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void LibraryModel::searchQueryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void LibraryModel::sortKeyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void LibraryModel::sortDescendingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void LibraryModel::filterTagChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void LibraryModel::filterCollectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void LibraryModel::bulkImportChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}
QT_WARNING_POP
