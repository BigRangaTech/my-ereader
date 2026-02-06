/****************************************************************************
** Meta object code from reading C++ file 'SettingsManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/include/SettingsManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SettingsManager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15SettingsManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto SettingsManager::qt_create_metaobjectdata<qt_meta_tag_ZN15SettingsManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SettingsManager",
        "readingFontSizeChanged",
        "",
        "readingLineHeightChanged",
        "ttsRateChanged",
        "ttsPitchChanged",
        "ttsVolumeChanged",
        "ttsVoiceKeyChanged",
        "autoLockEnabledChanged",
        "autoLockMinutesChanged",
        "rememberPassphraseChanged",
        "epubFontSizeChanged",
        "epubLineHeightChanged",
        "epubShowImagesChanged",
        "epubTextAlignChanged",
        "epubParagraphSpacingChanged",
        "epubParagraphIndentChanged",
        "epubImageMaxWidthChanged",
        "epubImageSpacingChanged",
        "fb2FontSizeChanged",
        "fb2LineHeightChanged",
        "fb2ShowImagesChanged",
        "fb2TextAlignChanged",
        "fb2ParagraphSpacingChanged",
        "fb2ParagraphIndentChanged",
        "fb2ImageMaxWidthChanged",
        "fb2ImageSpacingChanged",
        "txtFontSizeChanged",
        "txtLineHeightChanged",
        "txtMonospaceChanged",
        "txtEncodingChanged",
        "txtTabWidthChanged",
        "txtTrimWhitespaceChanged",
        "txtAutoChaptersChanged",
        "mobiFontSizeChanged",
        "mobiLineHeightChanged",
        "mobiShowImagesChanged",
        "mobiTextAlignChanged",
        "mobiParagraphSpacingChanged",
        "mobiParagraphIndentChanged",
        "mobiImageMaxWidthChanged",
        "mobiImageSpacingChanged",
        "pdfDpiChanged",
        "pdfCacheLimitChanged",
        "pdfPrefetchDistanceChanged",
        "pdfPreRenderPagesChanged",
        "pdfPrefetchStrategyChanged",
        "pdfCachePolicyChanged",
        "pdfRenderPresetChanged",
        "pdfColorModeChanged",
        "pdfBackgroundModeChanged",
        "pdfBackgroundColorChanged",
        "pdfMaxWidthChanged",
        "pdfMaxHeightChanged",
        "pdfImageFormatChanged",
        "pdfJpegQualityChanged",
        "pdfExtractTextChanged",
        "pdfTileSizeChanged",
        "pdfProgressiveRenderingChanged",
        "pdfProgressiveDpiChanged",
        "djvuDpiChanged",
        "djvuCacheLimitChanged",
        "djvuPrefetchDistanceChanged",
        "djvuPreRenderPagesChanged",
        "djvuCachePolicyChanged",
        "djvuImageFormatChanged",
        "djvuExtractTextChanged",
        "djvuRotationChanged",
        "comicMinZoomChanged",
        "comicMaxZoomChanged",
        "comicSortModeChanged",
        "comicSortDescendingChanged",
        "comicDefaultFitModeChanged",
        "comicRememberFitModeChanged",
        "comicReadingDirectionChanged",
        "comicResetZoomOnPageChangeChanged",
        "comicZoomStepChanged",
        "comicSmoothScalingChanged",
        "comicTwoPageSpreadChanged",
        "comicSpreadInPortraitChanged",
        "keyBindingsChanged",
        "recentImportFoldersChanged",
        "formatSettingsPath",
        "format",
        "comicFitModeForPath",
        "path",
        "setComicFitModeForPath",
        "mode",
        "keyBinding",
        "action",
        "keyBindingList",
        "setKeyBinding",
        "binding",
        "resetKeyBindings",
        "addRecentImportFolder",
        "clearRecentImportFolders",
        "resetDefaults",
        "resetPdfDefaults",
        "resetEpubDefaults",
        "resetFb2Defaults",
        "resetTxtDefaults",
        "resetMobiDefaults",
        "resetComicDefaults",
        "resetDjvuDefaults",
        "reload",
        "sidebarModeForPath",
        "setSidebarModeForPath",
        "readingFontSize",
        "readingLineHeight",
        "ttsRate",
        "ttsPitch",
        "ttsVolume",
        "ttsVoiceKey",
        "autoLockEnabled",
        "autoLockMinutes",
        "rememberPassphrase",
        "epubFontSize",
        "epubLineHeight",
        "epubShowImages",
        "epubTextAlign",
        "epubParagraphSpacing",
        "epubParagraphIndent",
        "epubImageMaxWidth",
        "epubImageSpacing",
        "fb2FontSize",
        "fb2LineHeight",
        "fb2ShowImages",
        "fb2TextAlign",
        "fb2ParagraphSpacing",
        "fb2ParagraphIndent",
        "fb2ImageMaxWidth",
        "fb2ImageSpacing",
        "txtFontSize",
        "txtLineHeight",
        "txtMonospace",
        "txtEncoding",
        "txtTabWidth",
        "txtTrimWhitespace",
        "txtAutoChapters",
        "mobiFontSize",
        "mobiLineHeight",
        "mobiShowImages",
        "mobiTextAlign",
        "mobiParagraphSpacing",
        "mobiParagraphIndent",
        "mobiImageMaxWidth",
        "mobiImageSpacing",
        "pdfDpi",
        "pdfCacheLimit",
        "pdfPrefetchDistance",
        "pdfPreRenderPages",
        "pdfPrefetchStrategy",
        "pdfCachePolicy",
        "pdfRenderPreset",
        "pdfColorMode",
        "pdfBackgroundMode",
        "pdfBackgroundColor",
        "pdfMaxWidth",
        "pdfMaxHeight",
        "pdfImageFormat",
        "pdfJpegQuality",
        "pdfExtractText",
        "pdfTileSize",
        "pdfProgressiveRendering",
        "pdfProgressiveDpi",
        "djvuDpi",
        "djvuCacheLimit",
        "djvuPrefetchDistance",
        "djvuPreRenderPages",
        "djvuCachePolicy",
        "djvuImageFormat",
        "djvuExtractText",
        "djvuRotation",
        "comicMinZoom",
        "comicMaxZoom",
        "comicSortMode",
        "comicSortDescending",
        "comicDefaultFitMode",
        "comicRememberFitMode",
        "comicReadingDirection",
        "comicResetZoomOnPageChange",
        "comicZoomStep",
        "comicSmoothScaling",
        "comicTwoPageSpread",
        "comicSpreadInPortrait",
        "settingsPath",
        "iconPath",
        "keyBindings",
        "QVariantMap",
        "recentImportFolders"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'readingFontSizeChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'readingLineHeightChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ttsRateChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ttsPitchChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ttsVolumeChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'ttsVoiceKeyChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'autoLockEnabledChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'autoLockMinutesChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'rememberPassphraseChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'epubFontSizeChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'epubLineHeightChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'epubShowImagesChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'epubTextAlignChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'epubParagraphSpacingChanged'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'epubParagraphIndentChanged'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'epubImageMaxWidthChanged'
        QtMocHelpers::SignalData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'epubImageSpacingChanged'
        QtMocHelpers::SignalData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fb2FontSizeChanged'
        QtMocHelpers::SignalData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fb2LineHeightChanged'
        QtMocHelpers::SignalData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fb2ShowImagesChanged'
        QtMocHelpers::SignalData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fb2TextAlignChanged'
        QtMocHelpers::SignalData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fb2ParagraphSpacingChanged'
        QtMocHelpers::SignalData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fb2ParagraphIndentChanged'
        QtMocHelpers::SignalData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fb2ImageMaxWidthChanged'
        QtMocHelpers::SignalData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'fb2ImageSpacingChanged'
        QtMocHelpers::SignalData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'txtFontSizeChanged'
        QtMocHelpers::SignalData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'txtLineHeightChanged'
        QtMocHelpers::SignalData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'txtMonospaceChanged'
        QtMocHelpers::SignalData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'txtEncodingChanged'
        QtMocHelpers::SignalData<void()>(30, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'txtTabWidthChanged'
        QtMocHelpers::SignalData<void()>(31, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'txtTrimWhitespaceChanged'
        QtMocHelpers::SignalData<void()>(32, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'txtAutoChaptersChanged'
        QtMocHelpers::SignalData<void()>(33, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mobiFontSizeChanged'
        QtMocHelpers::SignalData<void()>(34, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mobiLineHeightChanged'
        QtMocHelpers::SignalData<void()>(35, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mobiShowImagesChanged'
        QtMocHelpers::SignalData<void()>(36, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mobiTextAlignChanged'
        QtMocHelpers::SignalData<void()>(37, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mobiParagraphSpacingChanged'
        QtMocHelpers::SignalData<void()>(38, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mobiParagraphIndentChanged'
        QtMocHelpers::SignalData<void()>(39, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mobiImageMaxWidthChanged'
        QtMocHelpers::SignalData<void()>(40, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mobiImageSpacingChanged'
        QtMocHelpers::SignalData<void()>(41, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfDpiChanged'
        QtMocHelpers::SignalData<void()>(42, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfCacheLimitChanged'
        QtMocHelpers::SignalData<void()>(43, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfPrefetchDistanceChanged'
        QtMocHelpers::SignalData<void()>(44, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfPreRenderPagesChanged'
        QtMocHelpers::SignalData<void()>(45, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfPrefetchStrategyChanged'
        QtMocHelpers::SignalData<void()>(46, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfCachePolicyChanged'
        QtMocHelpers::SignalData<void()>(47, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfRenderPresetChanged'
        QtMocHelpers::SignalData<void()>(48, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfColorModeChanged'
        QtMocHelpers::SignalData<void()>(49, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfBackgroundModeChanged'
        QtMocHelpers::SignalData<void()>(50, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfBackgroundColorChanged'
        QtMocHelpers::SignalData<void()>(51, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfMaxWidthChanged'
        QtMocHelpers::SignalData<void()>(52, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfMaxHeightChanged'
        QtMocHelpers::SignalData<void()>(53, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfImageFormatChanged'
        QtMocHelpers::SignalData<void()>(54, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfJpegQualityChanged'
        QtMocHelpers::SignalData<void()>(55, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfExtractTextChanged'
        QtMocHelpers::SignalData<void()>(56, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfTileSizeChanged'
        QtMocHelpers::SignalData<void()>(57, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfProgressiveRenderingChanged'
        QtMocHelpers::SignalData<void()>(58, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pdfProgressiveDpiChanged'
        QtMocHelpers::SignalData<void()>(59, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'djvuDpiChanged'
        QtMocHelpers::SignalData<void()>(60, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'djvuCacheLimitChanged'
        QtMocHelpers::SignalData<void()>(61, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'djvuPrefetchDistanceChanged'
        QtMocHelpers::SignalData<void()>(62, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'djvuPreRenderPagesChanged'
        QtMocHelpers::SignalData<void()>(63, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'djvuCachePolicyChanged'
        QtMocHelpers::SignalData<void()>(64, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'djvuImageFormatChanged'
        QtMocHelpers::SignalData<void()>(65, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'djvuExtractTextChanged'
        QtMocHelpers::SignalData<void()>(66, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'djvuRotationChanged'
        QtMocHelpers::SignalData<void()>(67, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicMinZoomChanged'
        QtMocHelpers::SignalData<void()>(68, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicMaxZoomChanged'
        QtMocHelpers::SignalData<void()>(69, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicSortModeChanged'
        QtMocHelpers::SignalData<void()>(70, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicSortDescendingChanged'
        QtMocHelpers::SignalData<void()>(71, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicDefaultFitModeChanged'
        QtMocHelpers::SignalData<void()>(72, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicRememberFitModeChanged'
        QtMocHelpers::SignalData<void()>(73, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicReadingDirectionChanged'
        QtMocHelpers::SignalData<void()>(74, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicResetZoomOnPageChangeChanged'
        QtMocHelpers::SignalData<void()>(75, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicZoomStepChanged'
        QtMocHelpers::SignalData<void()>(76, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicSmoothScalingChanged'
        QtMocHelpers::SignalData<void()>(77, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicTwoPageSpreadChanged'
        QtMocHelpers::SignalData<void()>(78, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'comicSpreadInPortraitChanged'
        QtMocHelpers::SignalData<void()>(79, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'keyBindingsChanged'
        QtMocHelpers::SignalData<void()>(80, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'recentImportFoldersChanged'
        QtMocHelpers::SignalData<void()>(81, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'formatSettingsPath'
        QtMocHelpers::MethodData<QString(const QString &) const>(82, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 83 },
        }}),
        // Method 'comicFitModeForPath'
        QtMocHelpers::MethodData<QString(const QString &) const>(84, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 85 },
        }}),
        // Method 'setComicFitModeForPath'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(86, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 85 }, { QMetaType::QString, 87 },
        }}),
        // Method 'keyBinding'
        QtMocHelpers::MethodData<QString(const QString &) const>(88, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 89 },
        }}),
        // Method 'keyBindingList'
        QtMocHelpers::MethodData<QStringList(const QString &) const>(90, 2, QMC::AccessPublic, QMetaType::QStringList, {{
            { QMetaType::QString, 89 },
        }}),
        // Method 'setKeyBinding'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(91, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 89 }, { QMetaType::QString, 92 },
        }}),
        // Method 'resetKeyBindings'
        QtMocHelpers::MethodData<void()>(93, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'addRecentImportFolder'
        QtMocHelpers::MethodData<void(const QString &)>(94, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 85 },
        }}),
        // Method 'clearRecentImportFolders'
        QtMocHelpers::MethodData<void()>(95, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetDefaults'
        QtMocHelpers::MethodData<void()>(96, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetPdfDefaults'
        QtMocHelpers::MethodData<void()>(97, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetEpubDefaults'
        QtMocHelpers::MethodData<void()>(98, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetFb2Defaults'
        QtMocHelpers::MethodData<void()>(99, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetTxtDefaults'
        QtMocHelpers::MethodData<void()>(100, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetMobiDefaults'
        QtMocHelpers::MethodData<void()>(101, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetComicDefaults'
        QtMocHelpers::MethodData<void()>(102, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'resetDjvuDefaults'
        QtMocHelpers::MethodData<void()>(103, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'reload'
        QtMocHelpers::MethodData<void()>(104, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'sidebarModeForPath'
        QtMocHelpers::MethodData<QString(const QString &) const>(105, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 85 },
        }}),
        // Method 'setSidebarModeForPath'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(106, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 85 }, { QMetaType::QString, 87 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'readingFontSize'
        QtMocHelpers::PropertyData<int>(107, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'readingLineHeight'
        QtMocHelpers::PropertyData<double>(108, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'ttsRate'
        QtMocHelpers::PropertyData<double>(109, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'ttsPitch'
        QtMocHelpers::PropertyData<double>(110, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'ttsVolume'
        QtMocHelpers::PropertyData<double>(111, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'ttsVoiceKey'
        QtMocHelpers::PropertyData<QString>(112, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
        // property 'autoLockEnabled'
        QtMocHelpers::PropertyData<bool>(113, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 6),
        // property 'autoLockMinutes'
        QtMocHelpers::PropertyData<int>(114, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 7),
        // property 'rememberPassphrase'
        QtMocHelpers::PropertyData<bool>(115, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 8),
        // property 'epubFontSize'
        QtMocHelpers::PropertyData<int>(116, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 9),
        // property 'epubLineHeight'
        QtMocHelpers::PropertyData<double>(117, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 10),
        // property 'epubShowImages'
        QtMocHelpers::PropertyData<bool>(118, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 11),
        // property 'epubTextAlign'
        QtMocHelpers::PropertyData<QString>(119, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 12),
        // property 'epubParagraphSpacing'
        QtMocHelpers::PropertyData<double>(120, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 13),
        // property 'epubParagraphIndent'
        QtMocHelpers::PropertyData<double>(121, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 14),
        // property 'epubImageMaxWidth'
        QtMocHelpers::PropertyData<int>(122, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 15),
        // property 'epubImageSpacing'
        QtMocHelpers::PropertyData<double>(123, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 16),
        // property 'fb2FontSize'
        QtMocHelpers::PropertyData<int>(124, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 17),
        // property 'fb2LineHeight'
        QtMocHelpers::PropertyData<double>(125, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 18),
        // property 'fb2ShowImages'
        QtMocHelpers::PropertyData<bool>(126, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 19),
        // property 'fb2TextAlign'
        QtMocHelpers::PropertyData<QString>(127, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 20),
        // property 'fb2ParagraphSpacing'
        QtMocHelpers::PropertyData<double>(128, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 21),
        // property 'fb2ParagraphIndent'
        QtMocHelpers::PropertyData<double>(129, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 22),
        // property 'fb2ImageMaxWidth'
        QtMocHelpers::PropertyData<int>(130, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 23),
        // property 'fb2ImageSpacing'
        QtMocHelpers::PropertyData<double>(131, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 24),
        // property 'txtFontSize'
        QtMocHelpers::PropertyData<int>(132, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 25),
        // property 'txtLineHeight'
        QtMocHelpers::PropertyData<double>(133, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 26),
        // property 'txtMonospace'
        QtMocHelpers::PropertyData<bool>(134, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 27),
        // property 'txtEncoding'
        QtMocHelpers::PropertyData<QString>(135, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 28),
        // property 'txtTabWidth'
        QtMocHelpers::PropertyData<int>(136, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 29),
        // property 'txtTrimWhitespace'
        QtMocHelpers::PropertyData<bool>(137, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 30),
        // property 'txtAutoChapters'
        QtMocHelpers::PropertyData<bool>(138, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 31),
        // property 'mobiFontSize'
        QtMocHelpers::PropertyData<int>(139, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 32),
        // property 'mobiLineHeight'
        QtMocHelpers::PropertyData<double>(140, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 33),
        // property 'mobiShowImages'
        QtMocHelpers::PropertyData<bool>(141, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 34),
        // property 'mobiTextAlign'
        QtMocHelpers::PropertyData<QString>(142, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 35),
        // property 'mobiParagraphSpacing'
        QtMocHelpers::PropertyData<double>(143, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 36),
        // property 'mobiParagraphIndent'
        QtMocHelpers::PropertyData<double>(144, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 37),
        // property 'mobiImageMaxWidth'
        QtMocHelpers::PropertyData<int>(145, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 38),
        // property 'mobiImageSpacing'
        QtMocHelpers::PropertyData<double>(146, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 39),
        // property 'pdfDpi'
        QtMocHelpers::PropertyData<int>(147, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 40),
        // property 'pdfCacheLimit'
        QtMocHelpers::PropertyData<int>(148, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 41),
        // property 'pdfPrefetchDistance'
        QtMocHelpers::PropertyData<int>(149, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 42),
        // property 'pdfPreRenderPages'
        QtMocHelpers::PropertyData<int>(150, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 43),
        // property 'pdfPrefetchStrategy'
        QtMocHelpers::PropertyData<QString>(151, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 44),
        // property 'pdfCachePolicy'
        QtMocHelpers::PropertyData<QString>(152, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 45),
        // property 'pdfRenderPreset'
        QtMocHelpers::PropertyData<QString>(153, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 46),
        // property 'pdfColorMode'
        QtMocHelpers::PropertyData<QString>(154, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 47),
        // property 'pdfBackgroundMode'
        QtMocHelpers::PropertyData<QString>(155, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 48),
        // property 'pdfBackgroundColor'
        QtMocHelpers::PropertyData<QString>(156, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 49),
        // property 'pdfMaxWidth'
        QtMocHelpers::PropertyData<int>(157, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 50),
        // property 'pdfMaxHeight'
        QtMocHelpers::PropertyData<int>(158, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 51),
        // property 'pdfImageFormat'
        QtMocHelpers::PropertyData<QString>(159, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 52),
        // property 'pdfJpegQuality'
        QtMocHelpers::PropertyData<int>(160, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 53),
        // property 'pdfExtractText'
        QtMocHelpers::PropertyData<bool>(161, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 54),
        // property 'pdfTileSize'
        QtMocHelpers::PropertyData<int>(162, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 55),
        // property 'pdfProgressiveRendering'
        QtMocHelpers::PropertyData<bool>(163, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 56),
        // property 'pdfProgressiveDpi'
        QtMocHelpers::PropertyData<int>(164, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 57),
        // property 'djvuDpi'
        QtMocHelpers::PropertyData<int>(165, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 58),
        // property 'djvuCacheLimit'
        QtMocHelpers::PropertyData<int>(166, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 59),
        // property 'djvuPrefetchDistance'
        QtMocHelpers::PropertyData<int>(167, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 60),
        // property 'djvuPreRenderPages'
        QtMocHelpers::PropertyData<int>(168, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 61),
        // property 'djvuCachePolicy'
        QtMocHelpers::PropertyData<QString>(169, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 62),
        // property 'djvuImageFormat'
        QtMocHelpers::PropertyData<QString>(170, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 63),
        // property 'djvuExtractText'
        QtMocHelpers::PropertyData<bool>(171, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 64),
        // property 'djvuRotation'
        QtMocHelpers::PropertyData<int>(172, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 65),
        // property 'comicMinZoom'
        QtMocHelpers::PropertyData<double>(173, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 66),
        // property 'comicMaxZoom'
        QtMocHelpers::PropertyData<double>(174, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 67),
        // property 'comicSortMode'
        QtMocHelpers::PropertyData<QString>(175, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 68),
        // property 'comicSortDescending'
        QtMocHelpers::PropertyData<bool>(176, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 69),
        // property 'comicDefaultFitMode'
        QtMocHelpers::PropertyData<QString>(177, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 70),
        // property 'comicRememberFitMode'
        QtMocHelpers::PropertyData<bool>(178, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 71),
        // property 'comicReadingDirection'
        QtMocHelpers::PropertyData<QString>(179, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 72),
        // property 'comicResetZoomOnPageChange'
        QtMocHelpers::PropertyData<bool>(180, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 73),
        // property 'comicZoomStep'
        QtMocHelpers::PropertyData<double>(181, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 74),
        // property 'comicSmoothScaling'
        QtMocHelpers::PropertyData<bool>(182, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 75),
        // property 'comicTwoPageSpread'
        QtMocHelpers::PropertyData<bool>(183, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 76),
        // property 'comicSpreadInPortrait'
        QtMocHelpers::PropertyData<bool>(184, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 77),
        // property 'settingsPath'
        QtMocHelpers::PropertyData<QString>(185, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'iconPath'
        QtMocHelpers::PropertyData<QString>(186, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'keyBindings'
        QtMocHelpers::PropertyData<QVariantMap>(187, 0x80000000 | 188, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 78),
        // property 'recentImportFolders'
        QtMocHelpers::PropertyData<QStringList>(189, QMetaType::QStringList, QMC::DefaultPropertyFlags, 79),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SettingsManager, qt_meta_tag_ZN15SettingsManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SettingsManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15SettingsManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15SettingsManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15SettingsManagerE_t>.metaTypes,
    nullptr
} };

void SettingsManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SettingsManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->readingFontSizeChanged(); break;
        case 1: _t->readingLineHeightChanged(); break;
        case 2: _t->ttsRateChanged(); break;
        case 3: _t->ttsPitchChanged(); break;
        case 4: _t->ttsVolumeChanged(); break;
        case 5: _t->ttsVoiceKeyChanged(); break;
        case 6: _t->autoLockEnabledChanged(); break;
        case 7: _t->autoLockMinutesChanged(); break;
        case 8: _t->rememberPassphraseChanged(); break;
        case 9: _t->epubFontSizeChanged(); break;
        case 10: _t->epubLineHeightChanged(); break;
        case 11: _t->epubShowImagesChanged(); break;
        case 12: _t->epubTextAlignChanged(); break;
        case 13: _t->epubParagraphSpacingChanged(); break;
        case 14: _t->epubParagraphIndentChanged(); break;
        case 15: _t->epubImageMaxWidthChanged(); break;
        case 16: _t->epubImageSpacingChanged(); break;
        case 17: _t->fb2FontSizeChanged(); break;
        case 18: _t->fb2LineHeightChanged(); break;
        case 19: _t->fb2ShowImagesChanged(); break;
        case 20: _t->fb2TextAlignChanged(); break;
        case 21: _t->fb2ParagraphSpacingChanged(); break;
        case 22: _t->fb2ParagraphIndentChanged(); break;
        case 23: _t->fb2ImageMaxWidthChanged(); break;
        case 24: _t->fb2ImageSpacingChanged(); break;
        case 25: _t->txtFontSizeChanged(); break;
        case 26: _t->txtLineHeightChanged(); break;
        case 27: _t->txtMonospaceChanged(); break;
        case 28: _t->txtEncodingChanged(); break;
        case 29: _t->txtTabWidthChanged(); break;
        case 30: _t->txtTrimWhitespaceChanged(); break;
        case 31: _t->txtAutoChaptersChanged(); break;
        case 32: _t->mobiFontSizeChanged(); break;
        case 33: _t->mobiLineHeightChanged(); break;
        case 34: _t->mobiShowImagesChanged(); break;
        case 35: _t->mobiTextAlignChanged(); break;
        case 36: _t->mobiParagraphSpacingChanged(); break;
        case 37: _t->mobiParagraphIndentChanged(); break;
        case 38: _t->mobiImageMaxWidthChanged(); break;
        case 39: _t->mobiImageSpacingChanged(); break;
        case 40: _t->pdfDpiChanged(); break;
        case 41: _t->pdfCacheLimitChanged(); break;
        case 42: _t->pdfPrefetchDistanceChanged(); break;
        case 43: _t->pdfPreRenderPagesChanged(); break;
        case 44: _t->pdfPrefetchStrategyChanged(); break;
        case 45: _t->pdfCachePolicyChanged(); break;
        case 46: _t->pdfRenderPresetChanged(); break;
        case 47: _t->pdfColorModeChanged(); break;
        case 48: _t->pdfBackgroundModeChanged(); break;
        case 49: _t->pdfBackgroundColorChanged(); break;
        case 50: _t->pdfMaxWidthChanged(); break;
        case 51: _t->pdfMaxHeightChanged(); break;
        case 52: _t->pdfImageFormatChanged(); break;
        case 53: _t->pdfJpegQualityChanged(); break;
        case 54: _t->pdfExtractTextChanged(); break;
        case 55: _t->pdfTileSizeChanged(); break;
        case 56: _t->pdfProgressiveRenderingChanged(); break;
        case 57: _t->pdfProgressiveDpiChanged(); break;
        case 58: _t->djvuDpiChanged(); break;
        case 59: _t->djvuCacheLimitChanged(); break;
        case 60: _t->djvuPrefetchDistanceChanged(); break;
        case 61: _t->djvuPreRenderPagesChanged(); break;
        case 62: _t->djvuCachePolicyChanged(); break;
        case 63: _t->djvuImageFormatChanged(); break;
        case 64: _t->djvuExtractTextChanged(); break;
        case 65: _t->djvuRotationChanged(); break;
        case 66: _t->comicMinZoomChanged(); break;
        case 67: _t->comicMaxZoomChanged(); break;
        case 68: _t->comicSortModeChanged(); break;
        case 69: _t->comicSortDescendingChanged(); break;
        case 70: _t->comicDefaultFitModeChanged(); break;
        case 71: _t->comicRememberFitModeChanged(); break;
        case 72: _t->comicReadingDirectionChanged(); break;
        case 73: _t->comicResetZoomOnPageChangeChanged(); break;
        case 74: _t->comicZoomStepChanged(); break;
        case 75: _t->comicSmoothScalingChanged(); break;
        case 76: _t->comicTwoPageSpreadChanged(); break;
        case 77: _t->comicSpreadInPortraitChanged(); break;
        case 78: _t->keyBindingsChanged(); break;
        case 79: _t->recentImportFoldersChanged(); break;
        case 80: { QString _r = _t->formatSettingsPath((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 81: { QString _r = _t->comicFitModeForPath((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 82: _t->setComicFitModeForPath((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 83: { QString _r = _t->keyBinding((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 84: { QStringList _r = _t->keyBindingList((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        case 85: _t->setKeyBinding((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 86: _t->resetKeyBindings(); break;
        case 87: _t->addRecentImportFolder((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 88: _t->clearRecentImportFolders(); break;
        case 89: _t->resetDefaults(); break;
        case 90: _t->resetPdfDefaults(); break;
        case 91: _t->resetEpubDefaults(); break;
        case 92: _t->resetFb2Defaults(); break;
        case 93: _t->resetTxtDefaults(); break;
        case 94: _t->resetMobiDefaults(); break;
        case 95: _t->resetComicDefaults(); break;
        case 96: _t->resetDjvuDefaults(); break;
        case 97: _t->reload(); break;
        case 98: { QString _r = _t->sidebarModeForPath((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 99: _t->setSidebarModeForPath((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::readingFontSizeChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::readingLineHeightChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::ttsRateChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::ttsPitchChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::ttsVolumeChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::ttsVoiceKeyChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::autoLockEnabledChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::autoLockMinutesChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::rememberPassphraseChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::epubFontSizeChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::epubLineHeightChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::epubShowImagesChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::epubTextAlignChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::epubParagraphSpacingChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::epubParagraphIndentChanged, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::epubImageMaxWidthChanged, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::epubImageSpacingChanged, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::fb2FontSizeChanged, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::fb2LineHeightChanged, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::fb2ShowImagesChanged, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::fb2TextAlignChanged, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::fb2ParagraphSpacingChanged, 21))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::fb2ParagraphIndentChanged, 22))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::fb2ImageMaxWidthChanged, 23))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::fb2ImageSpacingChanged, 24))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::txtFontSizeChanged, 25))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::txtLineHeightChanged, 26))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::txtMonospaceChanged, 27))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::txtEncodingChanged, 28))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::txtTabWidthChanged, 29))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::txtTrimWhitespaceChanged, 30))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::txtAutoChaptersChanged, 31))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::mobiFontSizeChanged, 32))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::mobiLineHeightChanged, 33))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::mobiShowImagesChanged, 34))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::mobiTextAlignChanged, 35))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::mobiParagraphSpacingChanged, 36))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::mobiParagraphIndentChanged, 37))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::mobiImageMaxWidthChanged, 38))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::mobiImageSpacingChanged, 39))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfDpiChanged, 40))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfCacheLimitChanged, 41))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfPrefetchDistanceChanged, 42))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfPreRenderPagesChanged, 43))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfPrefetchStrategyChanged, 44))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfCachePolicyChanged, 45))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfRenderPresetChanged, 46))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfColorModeChanged, 47))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfBackgroundModeChanged, 48))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfBackgroundColorChanged, 49))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfMaxWidthChanged, 50))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfMaxHeightChanged, 51))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfImageFormatChanged, 52))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfJpegQualityChanged, 53))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfExtractTextChanged, 54))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfTileSizeChanged, 55))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfProgressiveRenderingChanged, 56))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::pdfProgressiveDpiChanged, 57))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::djvuDpiChanged, 58))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::djvuCacheLimitChanged, 59))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::djvuPrefetchDistanceChanged, 60))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::djvuPreRenderPagesChanged, 61))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::djvuCachePolicyChanged, 62))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::djvuImageFormatChanged, 63))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::djvuExtractTextChanged, 64))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::djvuRotationChanged, 65))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicMinZoomChanged, 66))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicMaxZoomChanged, 67))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicSortModeChanged, 68))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicSortDescendingChanged, 69))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicDefaultFitModeChanged, 70))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicRememberFitModeChanged, 71))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicReadingDirectionChanged, 72))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicResetZoomOnPageChangeChanged, 73))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicZoomStepChanged, 74))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicSmoothScalingChanged, 75))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicTwoPageSpreadChanged, 76))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::comicSpreadInPortraitChanged, 77))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::keyBindingsChanged, 78))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsManager::*)()>(_a, &SettingsManager::recentImportFoldersChanged, 79))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<int*>(_v) = _t->readingFontSize(); break;
        case 1: *reinterpret_cast<double*>(_v) = _t->readingLineHeight(); break;
        case 2: *reinterpret_cast<double*>(_v) = _t->ttsRate(); break;
        case 3: *reinterpret_cast<double*>(_v) = _t->ttsPitch(); break;
        case 4: *reinterpret_cast<double*>(_v) = _t->ttsVolume(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->ttsVoiceKey(); break;
        case 6: *reinterpret_cast<bool*>(_v) = _t->autoLockEnabled(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->autoLockMinutes(); break;
        case 8: *reinterpret_cast<bool*>(_v) = _t->rememberPassphrase(); break;
        case 9: *reinterpret_cast<int*>(_v) = _t->epubFontSize(); break;
        case 10: *reinterpret_cast<double*>(_v) = _t->epubLineHeight(); break;
        case 11: *reinterpret_cast<bool*>(_v) = _t->epubShowImages(); break;
        case 12: *reinterpret_cast<QString*>(_v) = _t->epubTextAlign(); break;
        case 13: *reinterpret_cast<double*>(_v) = _t->epubParagraphSpacing(); break;
        case 14: *reinterpret_cast<double*>(_v) = _t->epubParagraphIndent(); break;
        case 15: *reinterpret_cast<int*>(_v) = _t->epubImageMaxWidth(); break;
        case 16: *reinterpret_cast<double*>(_v) = _t->epubImageSpacing(); break;
        case 17: *reinterpret_cast<int*>(_v) = _t->fb2FontSize(); break;
        case 18: *reinterpret_cast<double*>(_v) = _t->fb2LineHeight(); break;
        case 19: *reinterpret_cast<bool*>(_v) = _t->fb2ShowImages(); break;
        case 20: *reinterpret_cast<QString*>(_v) = _t->fb2TextAlign(); break;
        case 21: *reinterpret_cast<double*>(_v) = _t->fb2ParagraphSpacing(); break;
        case 22: *reinterpret_cast<double*>(_v) = _t->fb2ParagraphIndent(); break;
        case 23: *reinterpret_cast<int*>(_v) = _t->fb2ImageMaxWidth(); break;
        case 24: *reinterpret_cast<double*>(_v) = _t->fb2ImageSpacing(); break;
        case 25: *reinterpret_cast<int*>(_v) = _t->txtFontSize(); break;
        case 26: *reinterpret_cast<double*>(_v) = _t->txtLineHeight(); break;
        case 27: *reinterpret_cast<bool*>(_v) = _t->txtMonospace(); break;
        case 28: *reinterpret_cast<QString*>(_v) = _t->txtEncoding(); break;
        case 29: *reinterpret_cast<int*>(_v) = _t->txtTabWidth(); break;
        case 30: *reinterpret_cast<bool*>(_v) = _t->txtTrimWhitespace(); break;
        case 31: *reinterpret_cast<bool*>(_v) = _t->txtAutoChapters(); break;
        case 32: *reinterpret_cast<int*>(_v) = _t->mobiFontSize(); break;
        case 33: *reinterpret_cast<double*>(_v) = _t->mobiLineHeight(); break;
        case 34: *reinterpret_cast<bool*>(_v) = _t->mobiShowImages(); break;
        case 35: *reinterpret_cast<QString*>(_v) = _t->mobiTextAlign(); break;
        case 36: *reinterpret_cast<double*>(_v) = _t->mobiParagraphSpacing(); break;
        case 37: *reinterpret_cast<double*>(_v) = _t->mobiParagraphIndent(); break;
        case 38: *reinterpret_cast<int*>(_v) = _t->mobiImageMaxWidth(); break;
        case 39: *reinterpret_cast<double*>(_v) = _t->mobiImageSpacing(); break;
        case 40: *reinterpret_cast<int*>(_v) = _t->pdfDpi(); break;
        case 41: *reinterpret_cast<int*>(_v) = _t->pdfCacheLimit(); break;
        case 42: *reinterpret_cast<int*>(_v) = _t->pdfPrefetchDistance(); break;
        case 43: *reinterpret_cast<int*>(_v) = _t->pdfPreRenderPages(); break;
        case 44: *reinterpret_cast<QString*>(_v) = _t->pdfPrefetchStrategy(); break;
        case 45: *reinterpret_cast<QString*>(_v) = _t->pdfCachePolicy(); break;
        case 46: *reinterpret_cast<QString*>(_v) = _t->pdfRenderPreset(); break;
        case 47: *reinterpret_cast<QString*>(_v) = _t->pdfColorMode(); break;
        case 48: *reinterpret_cast<QString*>(_v) = _t->pdfBackgroundMode(); break;
        case 49: *reinterpret_cast<QString*>(_v) = _t->pdfBackgroundColor(); break;
        case 50: *reinterpret_cast<int*>(_v) = _t->pdfMaxWidth(); break;
        case 51: *reinterpret_cast<int*>(_v) = _t->pdfMaxHeight(); break;
        case 52: *reinterpret_cast<QString*>(_v) = _t->pdfImageFormat(); break;
        case 53: *reinterpret_cast<int*>(_v) = _t->pdfJpegQuality(); break;
        case 54: *reinterpret_cast<bool*>(_v) = _t->pdfExtractText(); break;
        case 55: *reinterpret_cast<int*>(_v) = _t->pdfTileSize(); break;
        case 56: *reinterpret_cast<bool*>(_v) = _t->pdfProgressiveRendering(); break;
        case 57: *reinterpret_cast<int*>(_v) = _t->pdfProgressiveDpi(); break;
        case 58: *reinterpret_cast<int*>(_v) = _t->djvuDpi(); break;
        case 59: *reinterpret_cast<int*>(_v) = _t->djvuCacheLimit(); break;
        case 60: *reinterpret_cast<int*>(_v) = _t->djvuPrefetchDistance(); break;
        case 61: *reinterpret_cast<int*>(_v) = _t->djvuPreRenderPages(); break;
        case 62: *reinterpret_cast<QString*>(_v) = _t->djvuCachePolicy(); break;
        case 63: *reinterpret_cast<QString*>(_v) = _t->djvuImageFormat(); break;
        case 64: *reinterpret_cast<bool*>(_v) = _t->djvuExtractText(); break;
        case 65: *reinterpret_cast<int*>(_v) = _t->djvuRotation(); break;
        case 66: *reinterpret_cast<double*>(_v) = _t->comicMinZoom(); break;
        case 67: *reinterpret_cast<double*>(_v) = _t->comicMaxZoom(); break;
        case 68: *reinterpret_cast<QString*>(_v) = _t->comicSortMode(); break;
        case 69: *reinterpret_cast<bool*>(_v) = _t->comicSortDescending(); break;
        case 70: *reinterpret_cast<QString*>(_v) = _t->comicDefaultFitMode(); break;
        case 71: *reinterpret_cast<bool*>(_v) = _t->comicRememberFitMode(); break;
        case 72: *reinterpret_cast<QString*>(_v) = _t->comicReadingDirection(); break;
        case 73: *reinterpret_cast<bool*>(_v) = _t->comicResetZoomOnPageChange(); break;
        case 74: *reinterpret_cast<double*>(_v) = _t->comicZoomStep(); break;
        case 75: *reinterpret_cast<bool*>(_v) = _t->comicSmoothScaling(); break;
        case 76: *reinterpret_cast<bool*>(_v) = _t->comicTwoPageSpread(); break;
        case 77: *reinterpret_cast<bool*>(_v) = _t->comicSpreadInPortrait(); break;
        case 78: *reinterpret_cast<QString*>(_v) = _t->settingsPath(); break;
        case 79: *reinterpret_cast<QString*>(_v) = _t->iconPath(); break;
        case 80: *reinterpret_cast<QVariantMap*>(_v) = _t->keyBindings(); break;
        case 81: *reinterpret_cast<QStringList*>(_v) = _t->recentImportFolders(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setReadingFontSize(*reinterpret_cast<int*>(_v)); break;
        case 1: _t->setReadingLineHeight(*reinterpret_cast<double*>(_v)); break;
        case 2: _t->setTtsRate(*reinterpret_cast<double*>(_v)); break;
        case 3: _t->setTtsPitch(*reinterpret_cast<double*>(_v)); break;
        case 4: _t->setTtsVolume(*reinterpret_cast<double*>(_v)); break;
        case 5: _t->setTtsVoiceKey(*reinterpret_cast<QString*>(_v)); break;
        case 6: _t->setAutoLockEnabled(*reinterpret_cast<bool*>(_v)); break;
        case 7: _t->setAutoLockMinutes(*reinterpret_cast<int*>(_v)); break;
        case 8: _t->setRememberPassphrase(*reinterpret_cast<bool*>(_v)); break;
        case 9: _t->setEpubFontSize(*reinterpret_cast<int*>(_v)); break;
        case 10: _t->setEpubLineHeight(*reinterpret_cast<double*>(_v)); break;
        case 11: _t->setEpubShowImages(*reinterpret_cast<bool*>(_v)); break;
        case 12: _t->setEpubTextAlign(*reinterpret_cast<QString*>(_v)); break;
        case 13: _t->setEpubParagraphSpacing(*reinterpret_cast<double*>(_v)); break;
        case 14: _t->setEpubParagraphIndent(*reinterpret_cast<double*>(_v)); break;
        case 15: _t->setEpubImageMaxWidth(*reinterpret_cast<int*>(_v)); break;
        case 16: _t->setEpubImageSpacing(*reinterpret_cast<double*>(_v)); break;
        case 17: _t->setFb2FontSize(*reinterpret_cast<int*>(_v)); break;
        case 18: _t->setFb2LineHeight(*reinterpret_cast<double*>(_v)); break;
        case 19: _t->setFb2ShowImages(*reinterpret_cast<bool*>(_v)); break;
        case 20: _t->setFb2TextAlign(*reinterpret_cast<QString*>(_v)); break;
        case 21: _t->setFb2ParagraphSpacing(*reinterpret_cast<double*>(_v)); break;
        case 22: _t->setFb2ParagraphIndent(*reinterpret_cast<double*>(_v)); break;
        case 23: _t->setFb2ImageMaxWidth(*reinterpret_cast<int*>(_v)); break;
        case 24: _t->setFb2ImageSpacing(*reinterpret_cast<double*>(_v)); break;
        case 25: _t->setTxtFontSize(*reinterpret_cast<int*>(_v)); break;
        case 26: _t->setTxtLineHeight(*reinterpret_cast<double*>(_v)); break;
        case 27: _t->setTxtMonospace(*reinterpret_cast<bool*>(_v)); break;
        case 28: _t->setTxtEncoding(*reinterpret_cast<QString*>(_v)); break;
        case 29: _t->setTxtTabWidth(*reinterpret_cast<int*>(_v)); break;
        case 30: _t->setTxtTrimWhitespace(*reinterpret_cast<bool*>(_v)); break;
        case 31: _t->setTxtAutoChapters(*reinterpret_cast<bool*>(_v)); break;
        case 32: _t->setMobiFontSize(*reinterpret_cast<int*>(_v)); break;
        case 33: _t->setMobiLineHeight(*reinterpret_cast<double*>(_v)); break;
        case 34: _t->setMobiShowImages(*reinterpret_cast<bool*>(_v)); break;
        case 35: _t->setMobiTextAlign(*reinterpret_cast<QString*>(_v)); break;
        case 36: _t->setMobiParagraphSpacing(*reinterpret_cast<double*>(_v)); break;
        case 37: _t->setMobiParagraphIndent(*reinterpret_cast<double*>(_v)); break;
        case 38: _t->setMobiImageMaxWidth(*reinterpret_cast<int*>(_v)); break;
        case 39: _t->setMobiImageSpacing(*reinterpret_cast<double*>(_v)); break;
        case 40: _t->setPdfDpi(*reinterpret_cast<int*>(_v)); break;
        case 41: _t->setPdfCacheLimit(*reinterpret_cast<int*>(_v)); break;
        case 42: _t->setPdfPrefetchDistance(*reinterpret_cast<int*>(_v)); break;
        case 43: _t->setPdfPreRenderPages(*reinterpret_cast<int*>(_v)); break;
        case 44: _t->setPdfPrefetchStrategy(*reinterpret_cast<QString*>(_v)); break;
        case 45: _t->setPdfCachePolicy(*reinterpret_cast<QString*>(_v)); break;
        case 46: _t->setPdfRenderPreset(*reinterpret_cast<QString*>(_v)); break;
        case 47: _t->setPdfColorMode(*reinterpret_cast<QString*>(_v)); break;
        case 48: _t->setPdfBackgroundMode(*reinterpret_cast<QString*>(_v)); break;
        case 49: _t->setPdfBackgroundColor(*reinterpret_cast<QString*>(_v)); break;
        case 50: _t->setPdfMaxWidth(*reinterpret_cast<int*>(_v)); break;
        case 51: _t->setPdfMaxHeight(*reinterpret_cast<int*>(_v)); break;
        case 52: _t->setPdfImageFormat(*reinterpret_cast<QString*>(_v)); break;
        case 53: _t->setPdfJpegQuality(*reinterpret_cast<int*>(_v)); break;
        case 54: _t->setPdfExtractText(*reinterpret_cast<bool*>(_v)); break;
        case 55: _t->setPdfTileSize(*reinterpret_cast<int*>(_v)); break;
        case 56: _t->setPdfProgressiveRendering(*reinterpret_cast<bool*>(_v)); break;
        case 57: _t->setPdfProgressiveDpi(*reinterpret_cast<int*>(_v)); break;
        case 58: _t->setDjvuDpi(*reinterpret_cast<int*>(_v)); break;
        case 59: _t->setDjvuCacheLimit(*reinterpret_cast<int*>(_v)); break;
        case 60: _t->setDjvuPrefetchDistance(*reinterpret_cast<int*>(_v)); break;
        case 61: _t->setDjvuPreRenderPages(*reinterpret_cast<int*>(_v)); break;
        case 62: _t->setDjvuCachePolicy(*reinterpret_cast<QString*>(_v)); break;
        case 63: _t->setDjvuImageFormat(*reinterpret_cast<QString*>(_v)); break;
        case 64: _t->setDjvuExtractText(*reinterpret_cast<bool*>(_v)); break;
        case 65: _t->setDjvuRotation(*reinterpret_cast<int*>(_v)); break;
        case 66: _t->setComicMinZoom(*reinterpret_cast<double*>(_v)); break;
        case 67: _t->setComicMaxZoom(*reinterpret_cast<double*>(_v)); break;
        case 68: _t->setComicSortMode(*reinterpret_cast<QString*>(_v)); break;
        case 69: _t->setComicSortDescending(*reinterpret_cast<bool*>(_v)); break;
        case 70: _t->setComicDefaultFitMode(*reinterpret_cast<QString*>(_v)); break;
        case 71: _t->setComicRememberFitMode(*reinterpret_cast<bool*>(_v)); break;
        case 72: _t->setComicReadingDirection(*reinterpret_cast<QString*>(_v)); break;
        case 73: _t->setComicResetZoomOnPageChange(*reinterpret_cast<bool*>(_v)); break;
        case 74: _t->setComicZoomStep(*reinterpret_cast<double*>(_v)); break;
        case 75: _t->setComicSmoothScaling(*reinterpret_cast<bool*>(_v)); break;
        case 76: _t->setComicTwoPageSpread(*reinterpret_cast<bool*>(_v)); break;
        case 77: _t->setComicSpreadInPortrait(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *SettingsManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SettingsManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15SettingsManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SettingsManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 100)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 100;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 100)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 100;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 82;
    }
    return _id;
}

// SIGNAL 0
void SettingsManager::readingFontSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SettingsManager::readingLineHeightChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SettingsManager::ttsRateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SettingsManager::ttsPitchChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SettingsManager::ttsVolumeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SettingsManager::ttsVoiceKeyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void SettingsManager::autoLockEnabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void SettingsManager::autoLockMinutesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void SettingsManager::rememberPassphraseChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void SettingsManager::epubFontSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void SettingsManager::epubLineHeightChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void SettingsManager::epubShowImagesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void SettingsManager::epubTextAlignChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void SettingsManager::epubParagraphSpacingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void SettingsManager::epubParagraphIndentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void SettingsManager::epubImageMaxWidthChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}

// SIGNAL 16
void SettingsManager::epubImageSpacingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 16, nullptr);
}

// SIGNAL 17
void SettingsManager::fb2FontSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 17, nullptr);
}

// SIGNAL 18
void SettingsManager::fb2LineHeightChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 18, nullptr);
}

// SIGNAL 19
void SettingsManager::fb2ShowImagesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 19, nullptr);
}

// SIGNAL 20
void SettingsManager::fb2TextAlignChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 20, nullptr);
}

// SIGNAL 21
void SettingsManager::fb2ParagraphSpacingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 21, nullptr);
}

// SIGNAL 22
void SettingsManager::fb2ParagraphIndentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 22, nullptr);
}

// SIGNAL 23
void SettingsManager::fb2ImageMaxWidthChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 23, nullptr);
}

// SIGNAL 24
void SettingsManager::fb2ImageSpacingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 24, nullptr);
}

// SIGNAL 25
void SettingsManager::txtFontSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 25, nullptr);
}

// SIGNAL 26
void SettingsManager::txtLineHeightChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 26, nullptr);
}

// SIGNAL 27
void SettingsManager::txtMonospaceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 27, nullptr);
}

// SIGNAL 28
void SettingsManager::txtEncodingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 28, nullptr);
}

// SIGNAL 29
void SettingsManager::txtTabWidthChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 29, nullptr);
}

// SIGNAL 30
void SettingsManager::txtTrimWhitespaceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 30, nullptr);
}

// SIGNAL 31
void SettingsManager::txtAutoChaptersChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 31, nullptr);
}

// SIGNAL 32
void SettingsManager::mobiFontSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 32, nullptr);
}

// SIGNAL 33
void SettingsManager::mobiLineHeightChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 33, nullptr);
}

// SIGNAL 34
void SettingsManager::mobiShowImagesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 34, nullptr);
}

// SIGNAL 35
void SettingsManager::mobiTextAlignChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 35, nullptr);
}

// SIGNAL 36
void SettingsManager::mobiParagraphSpacingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 36, nullptr);
}

// SIGNAL 37
void SettingsManager::mobiParagraphIndentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 37, nullptr);
}

// SIGNAL 38
void SettingsManager::mobiImageMaxWidthChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 38, nullptr);
}

// SIGNAL 39
void SettingsManager::mobiImageSpacingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 39, nullptr);
}

// SIGNAL 40
void SettingsManager::pdfDpiChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 40, nullptr);
}

// SIGNAL 41
void SettingsManager::pdfCacheLimitChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 41, nullptr);
}

// SIGNAL 42
void SettingsManager::pdfPrefetchDistanceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 42, nullptr);
}

// SIGNAL 43
void SettingsManager::pdfPreRenderPagesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 43, nullptr);
}

// SIGNAL 44
void SettingsManager::pdfPrefetchStrategyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 44, nullptr);
}

// SIGNAL 45
void SettingsManager::pdfCachePolicyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 45, nullptr);
}

// SIGNAL 46
void SettingsManager::pdfRenderPresetChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 46, nullptr);
}

// SIGNAL 47
void SettingsManager::pdfColorModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 47, nullptr);
}

// SIGNAL 48
void SettingsManager::pdfBackgroundModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 48, nullptr);
}

// SIGNAL 49
void SettingsManager::pdfBackgroundColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 49, nullptr);
}

// SIGNAL 50
void SettingsManager::pdfMaxWidthChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 50, nullptr);
}

// SIGNAL 51
void SettingsManager::pdfMaxHeightChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 51, nullptr);
}

// SIGNAL 52
void SettingsManager::pdfImageFormatChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 52, nullptr);
}

// SIGNAL 53
void SettingsManager::pdfJpegQualityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 53, nullptr);
}

// SIGNAL 54
void SettingsManager::pdfExtractTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 54, nullptr);
}

// SIGNAL 55
void SettingsManager::pdfTileSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 55, nullptr);
}

// SIGNAL 56
void SettingsManager::pdfProgressiveRenderingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 56, nullptr);
}

// SIGNAL 57
void SettingsManager::pdfProgressiveDpiChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 57, nullptr);
}

// SIGNAL 58
void SettingsManager::djvuDpiChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 58, nullptr);
}

// SIGNAL 59
void SettingsManager::djvuCacheLimitChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 59, nullptr);
}

// SIGNAL 60
void SettingsManager::djvuPrefetchDistanceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 60, nullptr);
}

// SIGNAL 61
void SettingsManager::djvuPreRenderPagesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 61, nullptr);
}

// SIGNAL 62
void SettingsManager::djvuCachePolicyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 62, nullptr);
}

// SIGNAL 63
void SettingsManager::djvuImageFormatChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 63, nullptr);
}

// SIGNAL 64
void SettingsManager::djvuExtractTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 64, nullptr);
}

// SIGNAL 65
void SettingsManager::djvuRotationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 65, nullptr);
}

// SIGNAL 66
void SettingsManager::comicMinZoomChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 66, nullptr);
}

// SIGNAL 67
void SettingsManager::comicMaxZoomChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 67, nullptr);
}

// SIGNAL 68
void SettingsManager::comicSortModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 68, nullptr);
}

// SIGNAL 69
void SettingsManager::comicSortDescendingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 69, nullptr);
}

// SIGNAL 70
void SettingsManager::comicDefaultFitModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 70, nullptr);
}

// SIGNAL 71
void SettingsManager::comicRememberFitModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 71, nullptr);
}

// SIGNAL 72
void SettingsManager::comicReadingDirectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 72, nullptr);
}

// SIGNAL 73
void SettingsManager::comicResetZoomOnPageChangeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 73, nullptr);
}

// SIGNAL 74
void SettingsManager::comicZoomStepChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 74, nullptr);
}

// SIGNAL 75
void SettingsManager::comicSmoothScalingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 75, nullptr);
}

// SIGNAL 76
void SettingsManager::comicTwoPageSpreadChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 76, nullptr);
}

// SIGNAL 77
void SettingsManager::comicSpreadInPortraitChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 77, nullptr);
}

// SIGNAL 78
void SettingsManager::keyBindingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 78, nullptr);
}

// SIGNAL 79
void SettingsManager::recentImportFoldersChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 79, nullptr);
}
QT_WARNING_POP
