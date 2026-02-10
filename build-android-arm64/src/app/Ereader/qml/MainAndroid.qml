import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 6.10
import QtQuick.Layouts 1.15
import QtQuick 2.15 as QQ
import QtQuick.Window 2.15

import Ereader 1.0

ApplicationWindow {
  id: root
  width: 1200
  height: 800
  visible: true
  title: "My Ereader " + Qt.application.version

  readonly property string uiFont: "Space Grotesk"
  readonly property string readingFont: "Literata"
  readonly property string monoFont: "JetBrains Mono"
  property var imageReaderItem: null
  readonly property bool isAndroid: Qt.platform.os === "android"
  readonly property bool isPortrait: height >= width
  readonly property var screenGeom: Screen.geometry
  readonly property var availGeom: Screen.availableGeometry
  readonly property real safeLeft: (screenGeom && availGeom)
                                  ? Math.max(0, availGeom.x - screenGeom.x)
                                  : 0
  readonly property real safeTop: (screenGeom && availGeom)
                                 ? Math.max(0, availGeom.y - screenGeom.y)
                                 : 0
  readonly property real safeRight: (screenGeom && availGeom)
                                   ? Math.max(0, (screenGeom.x + screenGeom.width) - (availGeom.x + availGeom.width))
                                   : 0
  readonly property real safeBottom: (screenGeom && availGeom)
                                    ? Math.max(0, (screenGeom.y + screenGeom.height) - (availGeom.y + availGeom.height))
                                    : 0
  function computeSafeBottomPadding() {
    if (!root.isAndroid) return safeBottom
    var fallback = Math.round(16 * Screen.devicePixelRatio)
    return Math.max(safeBottom, fallback)
  }
  readonly property real safeBottomPadding: computeSafeBottomPadding()
  readonly property real safeTopPadding: root.isAndroid
                                         ? Math.max(safeTop, Math.round(8 * Screen.devicePixelRatio))
                                         : safeTop
  readonly property real uiBaseWidth: 1920
  readonly property real uiBaseHeight: 1080
  readonly property real uiMinWidth: 854
  readonly property real uiMinHeight: 480
  readonly property real uiMaxWidth: 3840
  readonly property real uiMaxHeight: 2160
  property bool androidDebugOverlay: false
  readonly property real uiScale: isAndroid ? 1.0 : Math.min(
    Math.max(uiMinWidth, Math.min(uiMaxWidth, width)) / uiBaseWidth,
    Math.max(uiMinHeight, Math.min(uiMaxHeight, height)) / uiBaseHeight
  )
  property int currentTab: 0
  property int settingsSubPage: 0
  property int formatSettingsTabIndex: 0
  property bool readerChromeVisible: true

  function goToLibrary() { currentTab = 0 }
  function openReader(force) {
    if (force || (reader.currentPath && reader.currentPath.length > 0)) {
      currentTab = 1
    } else {
      currentTab = 0
    }
  }
  function openNotes() { currentTab = 2 }
  function openSettings() { currentTab = 3; settingsSubPage = 0 }
  function openFormatSettings() { currentTab = 3; settingsSubPage = 1 }

  onCurrentTabChanged: {
    if (currentTab !== 1) {
      readerChromeVisible = true
    }
    if (currentTab !== 3) {
      settingsSubPage = 0
    }
  }

  Binding {
    target: root.contentItem
    property: "scale"
    value: root.uiScale
    when: !root.isAndroid
  }

  Binding {
    target: root.contentItem
    property: "transformOrigin"
    value: Item.TopLeft
    when: !root.isAndroid
  }

  Binding {
    target: root.contentItem
    property: "width"
    value: root.width / root.uiScale
    when: !root.isAndroid
  }

  Binding {
    target: root.contentItem
    property: "height"
    value: root.height / root.uiScale
    when: !root.isAndroid
  }

  Binding {
    target: root.overlay ? root.overlay : null
    property: "scale"
    value: root.uiScale
    when: !root.isAndroid && (root.overlay ? (root.overlay.parent !== root.contentItem) : false)
  }

  Binding {
    target: root.overlay ? root.overlay : null
    property: "transformOrigin"
    value: Item.TopLeft
    when: !root.isAndroid && (root.overlay ? (root.overlay.parent !== root.contentItem) : false)
  }

  Binding {
    target: root.overlay ? root.overlay : null
    property: "width"
    value: root.width / root.uiScale
    when: !root.isAndroid && (root.overlay ? (root.overlay.parent !== root.contentItem) : false)
  }

  Binding {
    target: root.overlay ? root.overlay : null
    property: "height"
    value: root.height / root.uiScale
    when: !root.isAndroid && (root.overlay ? (root.overlay.parent !== root.contentItem) : false)
  }

  footer: TabBar {
    id: bottomTabs
    currentIndex: root.currentTab
    visible: !(root.currentTab === 1 && !root.readerChromeVisible)
    onCurrentIndexChanged: root.currentTab = currentIndex
    leftPadding: root.safeLeft
    rightPadding: root.safeRight
    bottomPadding: root.safeBottomPadding
    height: implicitHeight + root.safeBottomPadding

    background: Rectangle {
      color: theme.panel
    }

    TabButton { text: "Library"; font.family: root.uiFont }
    TabButton { text: "Reader"; font.family: root.uiFont }
    TabButton { text: "Notes"; font.family: root.uiFont }
    TabButton { text: "Settings"; font.family: root.uiFont }
  }

  Drawer {
    id: navDrawer
    width: Math.min(root.width * 0.82, 320)
    height: root.height
    modal: true

    Rectangle {
      anchors.fill: parent
      color: theme.panel

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 12

        Text {
          text: "My Ereader"
          color: theme.textPrimary
          font.pixelSize: 20
          font.family: root.uiFont
        }

        Rectangle { height: 1; Layout.fillWidth: true; color: theme.panelHighlight }

        Button { text: "Library"; font.family: root.uiFont; onClicked: { navDrawer.close(); root.goToLibrary() } }
        Button { text: "Reader"; font.family: root.uiFont; onClicked: { navDrawer.close(); root.openReader() } }
        Button { text: "Notes"; font.family: root.uiFont; onClicked: { navDrawer.close(); root.openNotes() } }
        Button { text: "Settings"; font.family: root.uiFont; onClicked: { navDrawer.close(); root.openSettings() } }
        Button { text: "Format settings"; font.family: root.uiFont; onClicked: { navDrawer.close(); root.openFormatSettings() } }
        Button { text: "Updates"; font.family: root.uiFont; onClicked: { navDrawer.close(); updateDialog.open() } }

        Item { Layout.fillHeight: true }
      }
    }
  }

  ToolButton {
    id: drawerButton
    text: "☰"
    font.pixelSize: 18
    visible: root.currentTab !== 1 || root.readerChromeVisible
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.leftMargin: 16 + root.safeLeft
    anchors.topMargin: 16 + root.safeTopPadding
    z: 10
    onClicked: navDrawer.open()
  }
  function fileUrl(path) {
    if (!path || path.length === 0) return ""
    if (path.startsWith("file:") || path.startsWith("qrc:")) return path
    if (path.startsWith("/")) return "file://" + path
    return path
  }
  function localPathFromUrl(value) {
    if (!value) return ""
    if (value.toLocalFile) {
      const local = value.toLocalFile()
      if (local && local.length > 0) return local
    }
    const text = value.toString ? value.toString() : String(value)
    if (text.startsWith("file://")) return decodeURIComponent(text.replace("file://", ""))
    return decodeURIComponent(text)
  }
  function dirFromPath(value) {
    var path = value ? String(value) : ""
    if (path.length === 0) return ""
    if (path.endsWith("/")) path = path.slice(0, -1)
    var idx = path.lastIndexOf("/")
    if (idx <= 0) return path
    return path.slice(0, idx)
  }

  function textFontSizeFor(format) {
    const f = (format || "").toLowerCase()
    if (f === "epub") return settings.epubFontSize
    if (f === "fb2") return settings.fb2FontSize
    if (f === "txt") return settings.txtFontSize
    if (f === "mobi" || f === "azw" || f === "azw3" || f === "azw4" || f === "prc") return settings.mobiFontSize
    return settings.readingFontSize
  }

  function textFontFamilyFor(format) {
    const f = (format || "").toLowerCase()
    if (f === "txt" && settings.txtMonospace) return monoFont
    return readingFont
  }

  function textLineHeightFor(format) {
    const f = (format || "").toLowerCase()
    if (f === "epub") return settings.epubLineHeight
    if (f === "fb2") return settings.fb2LineHeight
    if (f === "txt") return settings.txtLineHeight
    if (f === "mobi" || f === "azw" || f === "azw3" || f === "azw4" || f === "prc") return settings.mobiLineHeight
    return settings.readingLineHeight
  }

  readonly property var keyBindingsMap: settings.keyBindings
  readonly property var keyBindingLabels: ({
    open_book: "Open book",
    import_folder: "Import folder",
    focus_search: "Focus search",
    toggle_tts: "Toggle speak / stop",
    close_book: "Close current book",
    next: "Next page or chapter",
    prev: "Previous page or chapter",
    jump_start: "Jump to start",
    jump_end: "Jump to end",
    zoom_in: "Increase text size or zoom in images",
    zoom_out: "Decrease text size or zoom out images",
    zoom_reset: "Reset image zoom to page fit"
  })

  function bindingList(action) {
    var map = keyBindingsMap
    if (!map) return []
    var list = map[action]
    return list ? list : []
  }

  function bindingText(action) {
    var map = keyBindingsMap
    if (!map) return ""
    var list = map[action]
    if (!list) return ""
    if (Array.isArray(list)) {
      return list.join(", ")
    }
    return String(list)
  }

  function shortcutEnabled(allowWhenTextInput) {
    if (isModalOpen()) return false
    if (!allowWhenTextInput && isTextInputFocused()) return false
    return true
  }

  function actionLabel(action) {
    var labels = keyBindingLabels || {}
    return labels[action] ? labels[action] : action
  }

  function normalizeKeySequence(sequence) {
    return String(sequence).trim().toLowerCase()
  }

  function bindingConflicts(action) {
    var map = keyBindingsMap
    if (!map) return []
    var seqToActions = {}
    for (var key in map) {
      var list = map[key]
      if (!list) continue
      if (!Array.isArray(list)) list = String(list).split(",")
      for (var i = 0; i < list.length; i++) {
        var seqNorm = normalizeKeySequence(list[i])
        if (!seqNorm) continue
        if (!seqToActions[seqNorm]) seqToActions[seqNorm] = []
        if (seqToActions[seqNorm].indexOf(key) === -1) {
          seqToActions[seqNorm].push(key)
        }
      }
    }
    var conflicts = []
    var targetList = map[action]
    if (!targetList) return conflicts
    if (!Array.isArray(targetList)) targetList = String(targetList).split(",")
    for (var j = 0; j < targetList.length; j++) {
      var norm = normalizeKeySequence(targetList[j])
      if (!norm) continue
      var actions = seqToActions[norm]
      if (actions && actions.length > 1) {
        conflicts.push({ sequence: String(targetList[j]).trim(), actions: actions })
      }
    }
    return conflicts
  }

  function bindingConflictText(action) {
    var conflicts = bindingConflicts(action)
    if (conflicts.length === 0) return ""
    var parts = []
    for (var i = 0; i < conflicts.length; i++) {
      var conflict = conflicts[i]
      var others = []
      for (var j = 0; j < conflict.actions.length; j++) {
        var other = conflict.actions[j]
        if (other === action) continue
        others.push(actionLabel(other))
      }
      if (others.length > 0) {
        parts.push(conflict.sequence + " with " + others.join(", "))
      }
    }
    if (parts.length === 0) return ""
    return "Conflicts: " + parts.join(" | ")
  }

  function hasBindingConflict(action) {
    return bindingConflicts(action).length > 0
  }

  onClosing: {
    if (vault.state === VaultController.Unlocked && sessionPassphrase.length > 0) {
      vault.lock(sessionPassphrase)
    }
  }
  onActiveFocusItemChanged: markActivity()
  onActiveChanged: {
    if (active) {
      markActivity()
    }
  }
  property bool nativeFolderDialogOpened: false

  function openFolderImport() {
    if (libraryModel.bulkImportActive) {
      return
    }
    nativeFolderDialogOpened = false
    manualFolderDialog.errorText = ""
    if (folderPathField) {
      folderPathField.text = ""
    }
    if (nativeFolderDialog) {
      nativeFolderDialog.open()
      folderDialogFallbackTimer.restart()
    } else {
      manualFolderDialog.open()
    }
  }

  Item {
    id: keyActivitySink
    anchors.fill: parent
    focus: true
    Keys.onPressed: (event) => {
      markActivity()
      event.accepted = false
    }
    Keys.onReleased: (event) => {
      markActivity()
      event.accepted = false
    }
  }

  Item {
    id: shortcutHost
    visible: false

    Instantiator {
      model: bindingList("open_book")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(true)
        onActivated: fileDialog.open()
      }
    }

    Instantiator {
      model: bindingList("import_folder")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(true)
        onActivated: openFolderImport()
      }
    }

    Instantiator {
      model: bindingList("focus_search")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(true)
        onActivated: {
          if (librarySearch.visible) {
            librarySearch.forceActiveFocus()
            librarySearch.selectAll()
          }
        }
      }
    }

    Instantiator {
      model: bindingList("toggle_tts")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(true)
        onActivated: {
          if (tts.speaking) {
            tts.stop()
          } else if (tts.available && reader.ttsAllowed) {
            tts.speak(reader.currentPlainText)
          }
        }
      }
    }

    Instantiator {
      model: bindingList("close_book")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(true)
        onActivated: {
          if (reader.currentPath && reader.currentPath.length > 0) {
            reader.close()
          }
        }
      }
    }

    Instantiator {
      model: bindingList("next")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(false)
        onActivated: advanceReader(1)
      }
    }

    Instantiator {
      model: bindingList("prev")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(false)
        onActivated: advanceReader(-1)
      }
    }

    Instantiator {
      model: bindingList("jump_start")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(false)
        onActivated: jumpToEdge(-1)
      }
    }

    Instantiator {
      model: bindingList("jump_end")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(false)
        onActivated: jumpToEdge(1)
      }
    }

    Instantiator {
      model: bindingList("zoom_in")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(false)
        onActivated: {
          if (reader.hasImages) {
            if (imageReaderItem) {
              imageReaderItem.zoom = imageReaderItem.clampZoom(imageReaderItem.zoom + imageReaderItem.zoomStep)
            }
          } else {
            adjustTextFont(1)
          }
        }
      }
    }

    Instantiator {
      model: bindingList("zoom_out")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(false)
        onActivated: {
          if (reader.hasImages) {
            if (imageReaderItem) {
              imageReaderItem.zoom = imageReaderItem.clampZoom(imageReaderItem.zoom - imageReaderItem.zoomStep)
            }
          } else {
            adjustTextFont(-1)
          }
        }
      }
    }

    Instantiator {
      model: bindingList("zoom_reset")
      delegate: Shortcut {
        sequence: modelData
        context: Qt.ApplicationShortcut
        enabled: shortcutEnabled(false)
        onActivated: {
          if (reader.hasImages && imageReaderItem) {
            imageReaderItem.applyFitMode("page")
          }
        }
      }
    }
  }

  QtObject {
    id: theme
    property color bgTop: "#111318"
    property color bgBottom: "#1b1f29"
    property color accent: "#ffb347"
    property color accentAlt: "#7bdff2"
    property color textPrimary: "#f1f1f1"
    property color textMuted: "#a9b0bd"
    property color panel: "#202633"
    property color panelHighlight: "#2a3242"
  }

  property bool pickAnnotationAnchor: false
  property var pendingAnnotationDraft: null
  property int textSelectionStart: -1
  property int textSelectionEnd: -1
  property string textSelectionText: ""
  property double lastActivityMs: Date.now()
  property bool autoUnlockArmed: false

  function markActivity() {
    lastActivityMs = Date.now()
    if (autoUnlockArmed && vault.state === VaultController.Locked &&
        sessionPassphrase.length > 0 && settings.rememberPassphrase) {
      if (vault.unlock(sessionPassphrase)) {
        autoUnlockArmed = false
      }
    }
  }

  function isModalOpen() {
    return fileDialog.visible || nativeFolderDialog.visible || manualFolderDialog.visible ||
           exportAnnotationsDialog.visible || annotationDialog.visible ||
           deleteAnnotationDialog.visible || editBookDialog.visible ||
           deleteBookDialog.visible || bulkTagsDialog.visible ||
           deleteBooksDialog.visible || formatWarningDialog.visible ||
           setupDialog.visible || unlockDialog.visible || lockDialog.visible ||
           updateDialog.visible || restartDialog.visible ||
           keyboardDialog.visible || settingsSubPage === 1 ||
           settingsDialog.visible || aboutDialog.visible
  }

  function isTextInputFocused() {
    var item = root.activeFocusItem
    if (!item) return false
    var isInput = item.hasOwnProperty("cursorPosition") ||
           item.hasOwnProperty("inputMethodComposing") ||
           item.hasOwnProperty("selectionStart")
    if (!isInput) return false
    if (item.hasOwnProperty("readOnly") && item.readOnly === true) return false
    if (item.hasOwnProperty("enabled") && item.enabled === false) return false
    return true
  }

  function setTextFontSizeFor(format, size) {
    var value = Math.max(8, Math.min(48, size))
    if (format === "epub") settings.epubFontSize = value
    else if (format === "fb2") settings.fb2FontSize = value
    else if (format === "txt") settings.txtFontSize = value
    else if (format === "mobi" || format === "azw" || format === "azw3" || format === "azw4" || format === "prc") settings.mobiFontSize = value
    else settings.readingFontSize = value
  }

  function adjustTextFont(delta) {
    var current = textFontSizeFor(reader.currentFormat)
    setTextFontSizeFor(reader.currentFormat, current + delta)
  }

  function comicIsRtl() {
    return settings.comicReadingDirection === "rtl"
  }

  function comicSpreadStep() {
    if (imageReaderItem && imageReaderItem.spreadActive === true) {
      return 2
    }
    return 1
  }

  function advanceComicImage(direction) {
    if (reader.imageCount <= 0) return false
    const step = comicSpreadStep()
    const delta = direction * (comicIsRtl() ? -1 : 1) * step
    const target = Math.max(0, Math.min(reader.imageCount - 1, reader.currentImageIndex + delta))
    reader.goToImage(target)
    return true
  }

  function jumpComicImageToEdge(direction) {
    if (reader.imageCount <= 0) return false
    const target = direction > 0
          ? (reader.imageCount - 1)
          : 0
    reader.goToImage(target)
    return true
  }

  function advanceReader(direction) {
    if (!reader.currentPath || reader.currentPath.length === 0) return false
    if (reader.hasImages && reader.imageCount > 0) {
      return advanceComicImage(direction)
    }
    if (reader.chapterCount > 0) {
      if (direction > 0) reader.nextChapter()
      else reader.prevChapter()
      return true
    }
    return false
  }

  function jumpToEdge(direction) {
    if (!reader.currentPath || reader.currentPath.length === 0) return false
    if (reader.hasImages && reader.imageCount > 0) {
      return jumpComicImageToEdge(direction)
    }
    if (reader.chapterCount > 0) {
      reader.goToChapter(direction > 0 ? (reader.chapterCount - 1) : 0)
      return true
    }
    return false
  }

  function readerPaginationCount() {
    if (reader.hasImages) {
      return reader.imageCount
    }
    if (reader.chapterCount > 0) {
      return reader.chapterCount
    }
    return 0
  }

  function readerPaginationIndex() {
    if (reader.hasImages) {
      return reader.currentImageIndex
    }
    if (reader.chapterCount > 0) {
      return reader.currentChapterIndex
    }
    return -1
  }

  function readerPaginationLabel() {
    if (reader.hasImages) {
      if (reader.imageCount <= 0) return ""
      if ((imageReaderItem && imageReaderItem.spreadActive === true) &&
          reader.currentImageIndex + 1 < reader.imageCount) {
        return qsTr("Page %1-%2 / %3")
              .arg(reader.currentImageIndex + 1)
              .arg(reader.currentImageIndex + 2)
              .arg(reader.imageCount)
      }
      return qsTr("Page %1 / %2")
            .arg(reader.currentImageIndex + 1)
            .arg(reader.imageCount)
    }
    if (reader.chapterCount > 0) {
      return qsTr("Chapter %1 / %2")
            .arg(reader.currentChapterIndex + 1)
            .arg(reader.chapterCount)
    }
    return ""
  }

  function goToReaderPagination(index) {
    const target = Math.max(0, index)
    if (reader.hasImages && reader.imageCount > 0) {
      reader.goToImage(Math.min(target, reader.imageCount - 1))
      return true
    }
    if (reader.chapterCount > 0) {
      reader.goToChapter(Math.min(target, reader.chapterCount - 1))
      return true
    }
    return false
  }

  function handleGlobalKey(event) {
    if (isModalOpen()) return false
    const ctrl = (event.modifiers & Qt.ControlModifier) !== 0
    const shift = (event.modifiers & Qt.ShiftModifier) !== 0
    const alt = (event.modifiers & Qt.AltModifier) !== 0

    if (!ctrl && !shift && !alt && isTextInputFocused()) {
      return false
    }

    if (ctrl && !alt && !shift && event.key === Qt.Key_O) {
      fileDialog.open()
      return true
    }
    if (ctrl && alt && event.key === Qt.Key_O) {
      openFolderImport()
      return true
    }
    if (ctrl && event.key === Qt.Key_F) {
      if (librarySearch.visible) {
        librarySearch.forceActiveFocus()
        librarySearch.selectAll()
        return true
      }
    }
    if (ctrl && shift && event.key === Qt.Key_S) {
      if (tts.speaking) {
        tts.stop()
      } else if (tts.available && reader.ttsAllowed) {
        tts.speak(reader.currentPlainText)
      }
      return true
    }

    switch (event.key) {
      case Qt.Key_Escape:
        if (reader.currentPath && reader.currentPath.length > 0) {
          reader.close()
          return true
        }
        return false
      case Qt.Key_Right:
      case Qt.Key_PageDown:
      case Qt.Key_Space:
        return advanceReader(1)
      case Qt.Key_Left:
      case Qt.Key_PageUp:
      case Qt.Key_Backspace:
        return advanceReader(-1)
      case Qt.Key_Home:
        return jumpToEdge(-1)
      case Qt.Key_End:
        return jumpToEdge(1)
      case Qt.Key_Plus:
      case Qt.Key_Equal:
        if (reader.hasImages) {
          if (imageReaderItem) {
            imageReaderItem.zoom = imageReaderItem.clampZoom(imageReaderItem.zoom + imageReaderItem.zoomStep)
          }
        } else {
          adjustTextFont(1)
        }
        return true
      case Qt.Key_Minus:
        if (reader.hasImages) {
          if (imageReaderItem) {
            imageReaderItem.zoom = imageReaderItem.clampZoom(imageReaderItem.zoom - imageReaderItem.zoomStep)
          }
        } else {
          adjustTextFont(-1)
        }
        return true
      case Qt.Key_0:
        if (reader.hasImages) {
          if (imageReaderItem) {
            imageReaderItem.applyFitMode("page")
          }
          return true
        }
        return false
      default:
        return false
    }
  }

  function passphraseScore(value) {
    if (!value) return 0
    var score = 0
    if (value.length >= 10) score += 1
    if (value.length >= 14) score += 1
    if (/[a-z]/.test(value)) score += 1
    if (/[A-Z]/.test(value)) score += 1
    if (/[0-9]/.test(value)) score += 1
    if (/[^A-Za-z0-9]/.test(value)) score += 1
    return score
  }

  function passphraseStrengthLabel(value) {
    const score = passphraseScore(value)
    if (score <= 2) return "Weak"
    if (score <= 4) return "Ok"
    return "Strong"
  }

  function passphraseStrengthColor(value) {
    const score = passphraseScore(value)
    if (score <= 2) return "#f26d6d"
    if (score <= 4) return "#f3c969"
    return "#7fe39a"
  }

  function selectionRange() {
    if (textSelectionStart < 0 || textSelectionEnd < 0 || textSelectionStart === textSelectionEnd) {
      return null
    }
    const start = Math.min(textSelectionStart, textSelectionEnd)
    const end = Math.max(textSelectionStart, textSelectionEnd)
    return { start: start, end: end }
  }

  function makeHighlightLocator(start, end) {
    const chapterIndex = Math.max(0, reader.currentChapterIndex)
    return "hl:c=" + (chapterIndex + 1) + ";s=" + start + ";e=" + end
  }

  function makeAnchorLocator(pageIndex, x, y, w, h) {
    var locator = "pos:p=" + (pageIndex + 1)
    locator += ";x=" + x.toFixed(4) + ";y=" + y.toFixed(4)
    if (w !== undefined && h !== undefined) {
      locator += ";w=" + w.toFixed(4) + ";h=" + h.toFixed(4)
    }
    return locator
  }

  function pinStyleForType(type) {
    if (type === "bookmark") return { radius: 0, rotation: 45 }
    if (type === "highlight") return { radius: 2, rotation: 0 }
    return { radius: 6, rotation: 0 }
  }

  function pinLabelForType(type) {
    if (type === "bookmark") return "Bookmark"
    if (type === "highlight") return "Highlight"
    return "Note"
  }

  function escapeHtml(text) {
    return text.replace(/&/g, "&amp;")
               .replace(/</g, "&lt;")
               .replace(/>/g, "&gt;")
  }

  function toHtmlFromPlain(text) {
    return escapeHtml(text).replace(/\\n/g, "<br/>")
  }

  function wrapHtmlWithLineHeight(html) {
    var lh = textLineHeightFor(reader.currentFormat)
    if (!lh || lh <= 0) {
      return html
    }
    return "<div style=\"line-height:" + lh + ";\">" + html + "</div>"
  }

  MouseArea {
    anchors.fill: parent
    hoverEnabled: true
    acceptedButtons: Qt.AllButtons
    propagateComposedEvents: true
    onPressed: function(mouse) {
      root.markActivity()
      mouse.accepted = false
    }
    onReleased: function(mouse) {
      root.markActivity()
      mouse.accepted = false
    }
    onPositionChanged: function(mouse) {
      root.markActivity()
      mouse.accepted = false
    }
  }

  WheelHandler {
    onWheel: root.markActivity()
  }

  Timer {
    id: autoLockTimer
    interval: 15000
    repeat: true
    running: true
    onTriggered: {
      if (!settings.autoLockEnabled) return
      if (vault.state !== VaultController.Unlocked) return
      if (sessionPassphrase.length === 0) return
      const limitMs = settings.autoLockMinutes * 60 * 1000
      if (limitMs <= 0) return
      if (Date.now() - lastActivityMs >= limitMs) {
        if (vault.lock(sessionPassphrase)) {
          autoUnlockArmed = settings.rememberPassphrase
          if (!settings.rememberPassphrase) {
            sessionPassphrase = ""
          }
        }
      }
    }
  }

  function applyHighlightsToHtml(html, ranges) {
    if (!ranges || ranges.length === 0) return html
    var sorted = ranges.slice().sort(function(a, b) { return a.start - b.start })
    var out = ""
    var pos = 0
    var rangeIndex = 0
    var current = sorted[rangeIndex]
    var inSpan = false
    function closeSpan() {
      if (inSpan) {
        out += "</span>"
        inSpan = false
      }
    }
    function openSpan(color) {
      out += "<span style=\"background-color:" + color + ";\">"
      inSpan = true
    }
    function updateSpan() {
      while (current && pos >= current.end) {
        closeSpan()
        rangeIndex++
        current = sorted[rangeIndex]
      }
      if (current && pos === current.start && !inSpan) {
        openSpan(current.color || "#ffb347")
      }
    }
    for (var i = 0; i < html.length; ) {
      var ch = html[i]
      if (ch === "<") {
        var endTag = html.indexOf(">", i)
        if (endTag === -1) {
          out += html.slice(i)
          break
        }
        out += html.slice(i, endTag + 1)
        i = endTag + 1
        continue
      }
      if (ch === "&") {
        var endEntity = html.indexOf(";", i)
        var entity = endEntity === -1 ? ch : html.slice(i, endEntity + 1)
        updateSpan()
        out += entity
        pos += 1
        i = endEntity === -1 ? i + 1 : endEntity + 1
        continue
      }
      updateSpan()
      out += ch
      pos += 1
      i += 1
    }
    closeSpan()
    return out
  }

  function currentHighlightRanges() {
    var _rev = annotationModel.revision
    return annotationModel.highlightRangesForChapter(reader.currentChapterIndex)
  }

  function anchorsForCurrentPage() {
    var _rev = annotationModel.revision
    return annotationModel.anchorsForPage(reader.currentImageIndex)
  }

  function displayTextForReader() {
    var ranges = currentHighlightRanges()
    var base = reader.currentTextIsRich ? reader.currentText : toHtmlFromPlain(reader.currentText)
    if (ranges && ranges.length > 0) {
      base = applyHighlightsToHtml(base, ranges)
    }
    return wrapHtmlWithLineHeight(base)
  }

  function locatorDisplay(locator) {
    if (!locator) return ""
    var m = locator.match(/^hl:c=(\\d+);s=(\\d+);e=(\\d+)/)
    if (m) {
      return "Chapter " + m[1] + " (" + m[2] + "-" + m[3] + ")"
    }
    m = locator.match(/^pos:p=(\\d+);x=([0-9.]+);y=([0-9.]+)/)
    if (m) {
      return "Page " + m[1] + " @ " + Math.round(parseFloat(m[2]) * 100) + "%, " +
             Math.round(parseFloat(m[3]) * 100) + "%"
    }
    return locator
  }

  LibraryModel {
    id: libraryModel
  }

  AnnotationModel {
    id: annotationModel
  }

  ListModel {
    id: filteredAnnotations
  }

  property string annotationFilterText: ""
  property string annotationFilterType: "all"
  property string annotationFilterColor: "all"

  function rebuildAnnotationFilter() {
    filteredAnnotations.clear()
    const textQuery = annotationFilterText.trim().toLowerCase()
    for (var i = 0; i < annotationModel.count; ++i) {
      const item = annotationModel.get(i)
      if (!item) continue
      if (annotationFilterType !== "all" && item.type !== annotationFilterType) {
        continue
      }
      if (annotationFilterColor !== "all" && item.color !== annotationFilterColor) {
        continue
      }
      if (textQuery.length > 0) {
        const hay = (item.text + " " + item.locator).toLowerCase()
        if (hay.indexOf(textQuery) === -1) {
          continue
        }
      }
      filteredAnnotations.append(item)
    }
  }

  Connections {
    target: annotationModel
    function onRevisionChanged() {
      rebuildAnnotationFilter()
    }
  }

  ReaderController {
    id: reader
  }

  SettingsManager {
    id: settings
  }

  UpdateManager {
    id: updateManager
    onStatusChanged: {
      if (status === "Update applied") {
        restartDialog.open()
      }
    }
  }

  SyncManager {
    id: syncManager
    libraryModel: libraryModel
  }

  TtsController {
    id: ttsBackend
  }

  QtObject {
    id: tts
    readonly property bool available: ttsBackend.available
    readonly property bool speaking: ttsBackend.speaking
    readonly property int queueLength: ttsBackend.queueLength
    readonly property var voiceKeys: ttsBackend.voiceKeys
    readonly property var voiceLabels: ttsBackend.voiceLabels
    readonly property string voiceKey: ttsBackend.voiceKey

    function speak(text) {
      if (!text || text.trim().length === 0) return false
      return ttsBackend.speak(text)
    }

    function enqueue(text) {
      if (!text || text.trim().length === 0) return
      ttsBackend.enqueue(text)
    }

    function stop() {
      ttsBackend.stop()
    }

    function clearQueue() {
      ttsBackend.clearQueue()
    }
  }

  LicenseManager {
    id: licenseManager
  }

  VaultController {
    id: vault
    libraryModel: libraryModel
  }

  property string sessionPassphrase: ""

  Component.onCompleted: {
    vault.initialize()
    if (!vault.keychainAvailable && settings.rememberPassphrase) {
      settings.rememberPassphrase = false
    }
    if (settings.rememberPassphrase && vault.keychainAvailable) {
      const stored = vault.loadStoredPassphrase()
      if (stored && stored.length > 0) {
        sessionPassphrase = stored
        autoUnlockArmed = true
        markActivity()
      }
    }
    ttsBackend.rate = settings.ttsRate
    ttsBackend.pitch = settings.ttsPitch
    ttsBackend.volume = settings.ttsVolume
    if (settings.ttsVoiceKey.length > 0) {
      ttsBackend.voiceKey = settings.ttsVoiceKey
    }
  }

  Connections {
    target: settings
    function onTtsRateChanged() { ttsBackend.rate = settings.ttsRate }
    function onTtsPitchChanged() { ttsBackend.pitch = settings.ttsPitch }
    function onTtsVolumeChanged() { ttsBackend.volume = settings.ttsVolume }
    function onTtsVoiceKeyChanged() {
      if (settings.ttsVoiceKey.length > 0) {
        ttsBackend.voiceKey = settings.ttsVoiceKey
      }
    }
    function onRememberPassphraseChanged() {
      if (!vault.keychainAvailable) {
        settings.rememberPassphrase = false
        return
      }
      if (settings.rememberPassphrase) {
        if (sessionPassphrase.length > 0) {
          vault.storePassphrase(sessionPassphrase)
        }
      } else {
        vault.clearStoredPassphrase()
      }
    }
  }

  Connections {
    target: vault
    function onStateChanged() {
      if (vault.state === VaultController.Locked) {
        annotationModel.attachDatabase("")
        if (autoUnlockArmed) {
          return
        }
        if (!unlockDialog.visible) {
          unlockDialog.open()
        }
      } else if (vault.state === VaultController.NeedsSetup) {
        if (!setupDialog.visible) {
          setupDialog.open()
        }
      }
    }
  }

  Connections {
    target: libraryModel
    function onReadyChanged() {
      if (libraryModel.ready) {
        annotationModel.attachConnection("")
      }
    }
  }

  FileDialog {
    id: fileDialog
    title: "Add book"
    fileMode: FileDialog.OpenFile
    nameFilters: root.isAndroid
                 ? ["All files (*)"]
                 : ["Books (*.epub *.pdf *.mobi *.azw *.azw3 *.fb2 *.cbz *.cbr *.djvu *.djv *.txt)"]
    onAccepted: {
      const path = localPathFromUrl(selectedFile)
      if (path.length > 0) {
        libraryModel.addBook(path)
      }
    }
  }

  FolderDialog {
    id: nativeFolderDialog
    title: "Add folder"
    onAccepted: {
      nativeFolderDialogOpened = true
      const folder = localPathFromUrl(nativeFolderDialog.selectedFolder)
      if (folder.length > 0) {
        settings.addRecentImportFolder(folder)
        libraryModel.addFolder(folder, true)
      }
    }
    onRejected: {
      nativeFolderDialogOpened = true
    }
    onVisibleChanged: {
      if (visible) {
        nativeFolderDialogOpened = true
      }
    }
  }

  Timer {
    id: folderDialogFallbackTimer
    interval: 200
    repeat: false
    onTriggered: {
      if (!nativeFolderDialogOpened) {
        manualFolderDialog.open()
      }
    }
  }

  Dialog {
    id: manualFolderDialog
    title: "Add folder"
    modal: true
    standardButtons: Dialog.NoButton
    width: Math.min(520, root.width - 80)

    property string errorText: ""

    contentItem: Rectangle {
      color: theme.panel
      radius: 16

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
          text: "Choose a folder to import. You can paste a path or pick a file inside it."
          color: theme.textPrimary
          font.pixelSize: 13
          font.family: root.uiFont
          wrapMode: Text.Wrap
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 8

          TextField {
            id: folderPathField
            Layout.fillWidth: true
            placeholderText: "/home/user/Books"
            onTextChanged: manualFolderDialog.errorText = ""
          }

          Button {
            text: "Browse"
            font.family: root.uiFont
            onClicked: folderFilePicker.open()
          }
        }

        ColumnLayout {
          Layout.fillWidth: true
          spacing: 6
          visible: settings.recentImportFolders && settings.recentImportFolders.length > 0

          RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
              text: "Recent folders"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }

            Item { Layout.fillWidth: true }

            Button {
              text: "Clear"
              font.family: root.uiFont
              onClicked: settings.clearRecentImportFolders()
            }
          }

          Repeater {
            model: settings.recentImportFolders || []
            delegate: Button {
              text: modelData
              font.family: root.uiFont
              Layout.fillWidth: true
              onClicked: folderPathField.text = modelData
            }
          }
        }

        Text {
          text: manualFolderDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
          visible: manualFolderDialog.errorText.length > 0
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 10

          Button {
            text: "Cancel"
            font.family: root.uiFont
            onClicked: manualFolderDialog.close()
          }

          Item { Layout.fillWidth: true }

          Button {
            text: "Import"
            font.family: root.uiFont
            onClicked: {
              const folder = folderPathField.text.trim()
              if (folder.length === 0) {
                manualFolderDialog.errorText = "Please enter a folder path."
                return
              }
              manualFolderDialog.errorText = ""
              manualFolderDialog.close()
              settings.addRecentImportFolder(folder)
              libraryModel.addFolder(folder, true)
            }
          }
        }
      }
    }
  }

  FileDialog {
    id: folderFilePicker
    title: "Pick a file in the folder"
    fileMode: FileDialog.OpenFile
    onAccepted: {
      const picked = localPathFromUrl(selectedFile)
      const folder = dirFromPath(picked)
      if (folder.length > 0) {
        folderPathField.text = folder
      }
    }
  }

  Menu {
    id: addMenu
    MenuItem {
      text: "Add file"
      onTriggered: fileDialog.open()
    }
    MenuItem {
      text: "Add folder"
      onTriggered: openFolderImport()
    }
  }

  FileDialog {
    id: exportAnnotationsDialog
    title: "Export annotations"
    fileMode: FileDialog.SaveFile
    nameFilters: ["JSON (*.json)", "CSV (*.csv)", "Markdown (*.md)"]
    onAccepted: {
      var path = ""
      if (selectedFile && selectedFile.toLocalFile) {
        path = selectedFile.toLocalFile()
      } else if (selectedFile) {
        path = selectedFile.toString().replace("file://", "")
        path = decodeURIComponent(path)
      }
      if (path.length > 0) {
        annotationModel.exportAnnotations(path)
      }
    }
  }

  Dialog {
    id: annotationDialog
    title: "Add Annotation"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 520
    height: 360

    property string errorText: ""
    property string locatorText: ""
    property string noteText: ""
    property string selectedType: "note"
    property string selectedColor: "#ffb347"
    property int editId: 0
    property bool editing: false

    function currentLocator() {
      if (reader.hasImages && reader.imageCount > 0) {
        return "Page " + (reader.currentImageIndex + 1)
      }
      if (reader.chapterCount > 0) {
        return "Chapter " + (reader.currentChapterIndex + 1)
      }
      return ""
    }

    onOpened: {
      if (!editing) {
        errorText = ""
        locatorField.text = currentLocator()
        noteField.text = ""
        locatorText = ""
        noteText = ""
        selectedType = "note"
        selectedColor = "#ffb347"
        editId = 0
      }
      editing = false
    }

    onAccepted: {
      if (locatorField.text.trim().length === 0) {
        errorText = "Locator required (page/chapter)"
        return
      }
      if (editId > 0) {
        if (!annotationModel.updateAnnotation(editId,
                                              locatorField.text,
                                              selectedType,
                                              noteField.text,
                                              selectedColor)) {
          errorText = annotationModel.lastError
          return
        }
      } else {
        if (!annotationModel.addAnnotation(locatorField.text, selectedType, noteField.text, selectedColor)) {
          errorText = annotationModel.lastError
          return
        }
      }
      annotationDialog.close()
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        RowLayout {
          Layout.fillWidth: true
          spacing: 8

          TextField {
            id: locatorField
            Layout.fillWidth: true
            placeholderText: "Locator (page/chapter id)"
          }

          Button {
            text: "Use Current"
            onClicked: locatorField.text = annotationDialog.currentLocator()
            font.family: root.uiFont
          }
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 8

          Button {
            text: "Use Selection"
            enabled: selectionRange() !== null && !reader.hasImages
            visible: !reader.hasImages
            onClicked: {
              const range = selectionRange()
              if (!range) return
              selectedType = "highlight"
              selectedColor = selectedColor || "#ffb347"
              if (noteField.text.trim().length === 0) {
                noteField.text = textSelectionText
              }
              locatorField.text = makeHighlightLocator(range.start, range.end)
            }
            font.family: root.uiFont
          }

          Button {
            text: "Pick on page"
            enabled: reader.hasImages
            visible: reader.hasImages
            onClicked: annotationDialog.startPickOnPage()
            font.family: root.uiFont
          }
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 12

          Text {
            text: "Type"
            color: theme.textMuted
            font.pixelSize: 12
            font.family: root.uiFont
            Layout.preferredWidth: 60
          }

          ComboBox {
            Layout.fillWidth: true
            model: ["note", "highlight", "bookmark"]
            currentIndex: model.indexOf(annotationDialog.selectedType)
            onActivated: annotationDialog.selectedType = model[currentIndex]
          }
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 8

          Text {
            text: "Color"
            color: theme.textMuted
            font.pixelSize: 12
            font.family: root.uiFont
            Layout.preferredWidth: 60
          }

          Repeater {
            model: ["#ffb347", "#7bdff2", "#c3f584", "#f4a7d3", "#f07167", "#ffd166", "#bdb2ff"]
            delegate: Rectangle {
              width: 26
              height: 26
              radius: 13
              color: modelData
              border.width: annotationDialog.selectedColor === modelData ? 2 : 1
              border.color: annotationDialog.selectedColor === modelData ? "#f1f1f1" : "#2a3242"

              MouseArea {
                anchors.fill: parent
                onClicked: annotationDialog.selectedColor = modelData
              }
            }
          }
        }

        TextArea {
          id: noteField
          placeholderText: "Note"
          wrapMode: TextArea.Wrap
          Layout.fillWidth: true
          Layout.fillHeight: true
        }

        Text {
          text: annotationDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }
      }
    }

    function openForEdit(item) {
      if (!item) return
      editing = true
      editId = item.id
      errorText = ""
      locatorField.text = item.locator
      noteField.text = item.text
      selectedType = item.type
      selectedColor = item.color
      open()
    }

    function startPickOnPage() {
      pendingAnnotationDraft = {
        id: editId,
        locator: locatorField.text,
        text: noteField.text,
        type: selectedType,
        color: selectedColor
      }
      pickAnnotationAnchor = true
      close()
    }

    function openForAnchor(locator) {
      if (pendingAnnotationDraft) {
        editId = pendingAnnotationDraft.id || 0
        selectedType = pendingAnnotationDraft.type || "note"
        selectedColor = pendingAnnotationDraft.color || "#ffb347"
        noteField.text = pendingAnnotationDraft.text || ""
      } else {
        editId = 0
        selectedType = "note"
        selectedColor = "#ffb347"
        noteField.text = ""
      }
      locatorField.text = locator
      errorText = ""
      open()
      pendingAnnotationDraft = null
    }
  }

  Dialog {
    id: deleteAnnotationDialog
    title: "Delete annotation?"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 420
    height: 200

    property int annotationId: 0
    property string annotationLocator: ""
    property string annotationPreview: ""

    onAccepted: {
      if (annotationId > 0) {
        annotationModel.deleteAnnotation(annotationId)
      }
      annotationId = 0
      annotationLocator = ""
      annotationPreview = ""
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
          text: "This will permanently remove the annotation."
          color: theme.textPrimary
          font.pixelSize: 13
          font.family: root.uiFont
          wrapMode: Text.WordWrap
        }

        Text {
          text: deleteAnnotationDialog.annotationLocator
          color: theme.textMuted
          font.pixelSize: 12
          font.family: root.uiFont
          elide: Text.ElideRight
        }

        Text {
          text: deleteAnnotationDialog.annotationPreview
          color: theme.textMuted
          font.pixelSize: 12
          font.family: root.uiFont
          wrapMode: Text.WordWrap
          elide: Text.ElideRight
        }
      }
    }

    function openForAnnotation(item) {
      if (!item) return
      annotationId = item.id
      annotationLocator = item.locator
      annotationPreview = item.text
      open()
    }
  }

  Dialog {
    id: editBookDialog
    title: "Edit metadata"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 560
    height: 460

    property int bookId: 0
    property string bookPath: ""
    property string errorText: ""

    onOpened: {
      errorText = ""
    }

    onAccepted: {
      if (bookId <= 0) {
        errorText = "Invalid book selection"
        return
      }
      if (!libraryModel.updateMetadata(bookId,
                                       titleField.text,
                                       authorField.text,
                                       seriesField.text,
                                       publisherField.text,
                                       descriptionField.text,
                                       tagsField.text,
                                       collectionField.text)) {
        errorText = libraryModel.lastError
        return
      }
      editBookDialog.close()
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
          text: "File path"
          color: theme.textMuted
          font.pixelSize: 11
          font.family: root.uiFont
        }

        TextArea {
          text: editBookDialog.bookPath
          readOnly: true
          wrapMode: TextArea.Wrap
          selectByMouse: true
          color: theme.textMuted
          font.pixelSize: 11
          font.family: root.uiFont
          background: Rectangle { color: "transparent" }
        }

        Text { text: "Title"; color: theme.textMuted; font.pixelSize: 11; font.family: root.uiFont }
        TextField { id: titleField; placeholderText: "Title" }

        Text { text: "Author(s)"; color: theme.textMuted; font.pixelSize: 11; font.family: root.uiFont }
        TextField { id: authorField; placeholderText: "Author(s)" }

        Text { text: "Series"; color: theme.textMuted; font.pixelSize: 11; font.family: root.uiFont }
        TextField { id: seriesField; placeholderText: "Series" }

        Text { text: "Publisher"; color: theme.textMuted; font.pixelSize: 11; font.family: root.uiFont }
        TextField { id: publisherField; placeholderText: "Publisher" }

        Text { text: "Collection"; color: theme.textMuted; font.pixelSize: 11; font.family: root.uiFont }
        TextField { id: collectionField; placeholderText: "Collection" }

        Text { text: "Tags (comma-separated)"; color: theme.textMuted; font.pixelSize: 11; font.family: root.uiFont }
        TextField { id: tagsField; placeholderText: "Tags (comma-separated)" }

        Text { text: "Description"; color: theme.textMuted; font.pixelSize: 11; font.family: root.uiFont }
        TextArea {
          id: descriptionField
          placeholderText: "Description"
          wrapMode: TextArea.Wrap
          Layout.fillWidth: true
          Layout.fillHeight: true
        }

        Text {
          text: editBookDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }
      }
    }

    function openForBook(item) {
      if (!item) return
      bookId = item.id
      bookPath = item.path
      titleField.text = item.title
      authorField.text = item.authors
      seriesField.text = item.series
      publisherField.text = item.publisher
      collectionField.text = item.collection || ""
      tagsField.text = item.tags || ""
      descriptionField.text = item.description
      errorText = ""
      open()
    }
  }

  Dialog {
    id: deleteBookDialog
    title: "Remove book?"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 420
    height: 200

    property int bookId: 0
    property string bookTitle: ""
    property string bookPath: ""

    onAccepted: {
      if (bookId > 0) {
        libraryModel.removeBook(bookId)
      }
      bookId = 0
      bookTitle = ""
      bookPath = ""
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
          text: "This will remove the book from your library."
          color: theme.textPrimary
          font.pixelSize: 13
          font.family: root.uiFont
          wrapMode: Text.WordWrap
        }

        Text {
          text: deleteBookDialog.bookTitle
          color: theme.textMuted
          font.pixelSize: 12
          font.family: root.uiFont
          elide: Text.ElideRight
        }

        Text {
          text: deleteBookDialog.bookPath
          color: theme.textMuted
          font.pixelSize: 11
          font.family: root.uiFont
          elide: Text.ElideRight
        }
      }
    }

    function openForBook(item) {
      if (!item) return
      bookId = item.id
      bookTitle = item.title
      bookPath = item.path
      open()
    }
  }

  Dialog {
    id: bulkTagsDialog
    title: "Bulk edit tags/collection"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 520
    height: 320

    property var bookIds: []
    property string errorText: ""

    onOpened: {
      errorText = ""
      updateTagsCheck.checked = false
      updateCollectionCheck.checked = false
      bulkTagsField.text = ""
      bulkCollectionField.text = ""
    }

    onAccepted: {
      if (!bookIds || bookIds.length === 0) {
        errorText = "No items selected"
        return
      }
      if (!updateTagsCheck.checked && !updateCollectionCheck.checked) {
        errorText = "Select what to update"
        return
      }
      if (!libraryModel.updateTagsCollection(bookIds,
                                             bulkTagsField.text,
                                             bulkCollectionField.text,
                                             updateTagsCheck.checked,
                                             updateCollectionCheck.checked)) {
        errorText = libraryModel.lastError
        return
      }
      bulkTagsDialog.close()
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
          text: qsTr("Selected books: %1").arg(bulkTagsDialog.bookIds.length)
          color: theme.textMuted
          font.pixelSize: 12
          font.family: root.uiFont
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 8

          CheckBox {
            id: updateCollectionCheck
            text: "Update collection"
          }

          TextField {
            id: bulkCollectionField
            Layout.fillWidth: true
            placeholderText: "Collection"
            enabled: updateCollectionCheck.checked
          }
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 8

          CheckBox {
            id: updateTagsCheck
            text: "Update tags"
          }

          TextField {
            id: bulkTagsField
            Layout.fillWidth: true
            placeholderText: "Tags (comma-separated)"
            enabled: updateTagsCheck.checked
          }
        }

        Text {
          text: bulkTagsDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }
      }
    }

    function openForIds(ids) {
      bookIds = ids || []
      open()
    }
  }

  Dialog {
    id: deleteBooksDialog
    title: "Remove selected books?"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 420
    height: 200

    property var bookIds: []

    onAccepted: {
      if (deleteBooksDialog.bookIds && deleteBooksDialog.bookIds.length > 0) {
        libraryModel.removeBooks(deleteBooksDialog.bookIds)
      }
      bookIds = []
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
          text: qsTr("This will remove %1 book(s) from your library.")
                .arg(deleteBooksDialog.bookIds.length)
          color: theme.textPrimary
          font.pixelSize: 13
          font.family: root.uiFont
          wrapMode: Text.WordWrap
        }

        Text {
          text: "Files on disk are not deleted."
          color: theme.textMuted
          font.pixelSize: 12
          font.family: root.uiFont
        }
      }
    }

    function openForIds(ids) {
      bookIds = ids || []
      open()
    }
  }

  Dialog {
    id: formatWarningDialog
    title: "Format unavailable"
    modal: true
    standardButtons: Dialog.NoButton
    width: 460
    height: 200

    property string messageText: ""

    onOpened: {
      if (messageText.length === 0) {
        messageText = "This format is experimental and temporarily disabled."
      }
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
          text: formatWarningDialog.messageText
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
          wrapMode: Text.WordWrap
        }
      }
    }
  }

  Dialog {
    id: setupDialog
    title: "Set Passphrase"
    modal: true
    standardButtons: Dialog.Ok
    closePolicy: Popup.NoAutoClose
    width: root.isAndroid ? Math.min(420, root.width - 24) : Math.min(520, root.width - 80)
    height: root.isAndroid ? Math.min(520, root.height - 24) : Math.min(520, root.height - 80)

    property string errorText: ""
    property bool noPassphrase: false

    onOpened: {
      errorText = ""
      passField.text = ""
      confirmField.text = ""
      noPassphrase = false
    }

    function handleAccept() {
      console.log("Setup dialog accepted")
      var chosenPass = passField.text
      if (!noPassphrase) {
        const strength = passphraseScore(passField.text)
        if (passField.text.length < 6) {
          errorText = "Passphrase must be at least 6 characters"
          return
        }
        if (strength <= 2) {
          errorText = "Passphrase is too weak"
          return
        }
        if (passField.text !== confirmField.text) {
          errorText = "Passphrases do not match"
          return
        }
      } else {
        chosenPass = ""
      }
      const setupOk = vault.setupNew(chosenPass)
      console.log("Vault setup", setupOk, "error", vault.lastError)
      if (setupOk) {
        sessionPassphrase = chosenPass
        autoUnlockArmed = false
        const unlockOk = vault.unlock(chosenPass)
        console.log("Vault unlock after setup", unlockOk, "error", vault.lastError)
        if (unlockOk) {
          markActivity()
        }
        if (settings.rememberPassphrase && vault.keychainAvailable) {
          vault.storePassphrase(chosenPass)
        }
        markActivity()
        setupDialog.close()
      } else {
        errorText = vault.lastError
      }
    }

    footer: ColumnLayout {
      width: parent.width
      spacing: root.isAndroid ? 8 : 10
      Button {
        Layout.fillWidth: true
        Layout.preferredHeight: root.isAndroid ? 44 : 48
        text: "Create"
        onClicked: setupDialog.handleAccept()
      }
      Button {
        Layout.fillWidth: true
        Layout.preferredHeight: root.isAndroid ? 44 : 48
        text: "Cancel"
        onClicked: setupDialog.close()
      }
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ScrollView {
        id: setupScroll
        anchors.fill: parent
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
          id: setupColumn
          width: Math.max(1, setupScroll.availableWidth - (root.isAndroid ? 16 : 40))
          anchors.top: parent.top
          anchors.topMargin: (root.isAndroid ? 8 : 20) + root.safeTopPadding
          anchors.horizontalCenter: parent.horizontalCenter
          spacing: root.isAndroid ? 10 : 14

        Text {
          text: "Create a passphrase for your encrypted library."
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
        }

        TextField {
          id: passField
          Layout.fillWidth: true
          Layout.preferredHeight: root.isAndroid ? 44 : 48
          echoMode: TextInput.Password
          placeholderText: "Passphrase"
          enabled: !setupDialog.noPassphrase
          onTextChanged: setupDialog.errorText = ""
        }

        TextField {
          id: confirmField
          Layout.fillWidth: true
          Layout.preferredHeight: root.isAndroid ? 44 : 48
          echoMode: TextInput.Password
          placeholderText: "Confirm passphrase"
          enabled: !setupDialog.noPassphrase
          onTextChanged: setupDialog.errorText = ""
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 12

          Text {
            text: "No passphrase"
            color: theme.textMuted
            font.pixelSize: 13
            font.family: root.uiFont
          }

          Item { Layout.fillWidth: true }

          Switch {
          id: noPassphraseCheck
          checked: setupDialog.noPassphrase
          onToggled: {
            setupDialog.noPassphrase = checked
            setupDialog.errorText = ""
            if (checked) {
              passField.text = ""
              confirmField.text = ""
            }
          }
          }
        }

        Text {
          visible: setupDialog.noPassphrase
          text: "Warning: anyone with access to your vault file can open your library."
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
          wrapMode: Text.WordWrap
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 8
          visible: !setupDialog.noPassphrase

          Rectangle {
            Layout.fillWidth: true
            height: 8
            radius: 4
            color: theme.panelHighlight

            Rectangle {
              width: parent.width * Math.min(1, passphraseScore(passField.text) / 6)
              height: parent.height
              radius: 4
              color: passphraseStrengthColor(passField.text)
            }
          }

          Text {
            text: passphraseStrengthLabel(passField.text)
            color: passphraseStrengthColor(passField.text)
            font.pixelSize: 12
            font.family: root.uiFont
          }
        }

        Text {
          text: setupDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 12

          Text {
            text: "Remember passphrase"
            color: theme.textMuted
            font.pixelSize: 13
            font.family: root.uiFont
          }

          Item { Layout.fillWidth: true }

          Switch {
            checked: settings.rememberPassphrase
            enabled: vault.keychainAvailable
            onToggled: settings.rememberPassphrase = checked
          }

          Text {
            text: vault.keychainAvailable ? (settings.rememberPassphrase ? "Stored in keychain" : "Prompt each time")
                                          : "Keychain unavailable"
            color: theme.textMuted
            font.pixelSize: 12
            font.family: root.uiFont
          }
        }

        Item {
          Layout.fillWidth: true
          height: (root.isAndroid ? 12 : 20) + root.safeBottomPadding
        }
        }
      }
    }
  }

  Dialog {
    id: unlockDialog
    title: "Unlock Library"
    modal: true
    standardButtons: Dialog.NoButton
    closePolicy: Popup.NoAutoClose
    width: root.isAndroid ? Math.min(420, root.width - 24) : Math.min(520, root.width - 80)
    height: root.isAndroid ? Math.min(420, root.height - 24) : Math.min(420, root.height - 80)

    property string errorText: ""
    property bool noPassphrase: false

    onOpened: {
      errorText = ""
      unlockField.text = ""
      noPassphrase = false
    }

    function handleAccept() {
      console.log("Unlock dialog accepted")
      const pass = noPassphrase ? "" : unlockField.text
      const unlockOk = vault.unlock(pass)
      console.log("Vault unlock", unlockOk, "error", vault.lastError)
      if (unlockOk) {
        sessionPassphrase = pass
        autoUnlockArmed = false
        markActivity()
        if (settings.rememberPassphrase && vault.keychainAvailable) {
          vault.storePassphrase(pass)
        }
        unlockDialog.close()
      } else {
        errorText = vault.lastError
      }
    }

    footer: ColumnLayout {
      width: parent.width
      spacing: root.isAndroid ? 8 : 10
      Button {
        Layout.fillWidth: true
        Layout.preferredHeight: root.isAndroid ? 44 : 48
        text: "Unlock"
        onClicked: unlockDialog.handleAccept()
      }
      Button {
        Layout.fillWidth: true
        Layout.preferredHeight: root.isAndroid ? 44 : 48
        text: "Cancel"
        onClicked: unlockDialog.close()
      }
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ScrollView {
        id: unlockScroll
        anchors.fill: parent
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
          id: unlockColumn
          width: Math.max(1, unlockScroll.availableWidth - (root.isAndroid ? 16 : 40))
          anchors.top: parent.top
          anchors.topMargin: (root.isAndroid ? 8 : 20) + root.safeTopPadding
          anchors.horizontalCenter: parent.horizontalCenter
          spacing: root.isAndroid ? 10 : 14

        Text {
          text: "Enter your passphrase."
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
        }

        TextField {
          id: unlockField
          Layout.fillWidth: true
          Layout.preferredHeight: root.isAndroid ? 44 : 48
          echoMode: TextInput.Password
          placeholderText: "Passphrase"
          enabled: !unlockDialog.noPassphrase
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 12

          Text {
            text: "Unlock without passphrase"
            color: theme.textMuted
            font.pixelSize: 13
            font.family: root.uiFont
          }

          Item { Layout.fillWidth: true }

          Switch {
            checked: unlockDialog.noPassphrase
            onToggled: {
              unlockDialog.noPassphrase = checked
              unlockDialog.errorText = ""
              if (checked) {
                unlockField.text = ""
              }
            }
          }
        }

        Text {
          text: unlockDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 12

          Text {
            text: "Remember passphrase"
            color: theme.textMuted
            font.pixelSize: 13
            font.family: root.uiFont
          }

          Item { Layout.fillWidth: true }

          Switch {
            checked: settings.rememberPassphrase
            enabled: vault.keychainAvailable
            onToggled: settings.rememberPassphrase = checked
          }

          Text {
            text: vault.keychainAvailable ? (settings.rememberPassphrase ? "Stored in keychain" : "Prompt each time")
                                          : "Keychain unavailable"
            color: theme.textMuted
            font.pixelSize: 12
            font.family: root.uiFont
          }
        }

        Item {
          Layout.fillWidth: true
          height: (root.isAndroid ? 12 : 20) + root.safeBottomPadding
        }
        }
      }
    }
  }

  Dialog {
    id: lockDialog
    title: "Lock Library"
    modal: true
    standardButtons: Dialog.Ok
    closePolicy: Popup.NoAutoClose
    width: root.isAndroid ? Math.min(420, root.width - 24) : Math.min(520, root.width - 80)
    height: root.isAndroid ? Math.min(360, root.height - 24) : Math.min(360, root.height - 80)

    property string errorText: ""

    onOpened: {
      errorText = ""
      lockField.text = ""
    }

    onAccepted: {
      if (vault.lock(lockField.text)) {
        autoUnlockArmed = false
        if (!settings.rememberPassphrase) {
          sessionPassphrase = ""
        }
        lockDialog.close()
      } else {
        errorText = vault.lastError
      }
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ScrollView {
        id: lockScroll
        anchors.fill: parent
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
          id: lockColumn
          width: Math.max(1, lockScroll.availableWidth - (root.isAndroid ? 16 : 40))
          anchors.top: parent.top
          anchors.topMargin: (root.isAndroid ? 8 : 20) + root.safeTopPadding
          anchors.horizontalCenter: parent.horizontalCenter
          spacing: root.isAndroid ? 10 : 14

        Text {
          text: "Re-enter passphrase to lock."
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
        }

        TextField {
          id: lockField
          Layout.fillWidth: true
          Layout.preferredHeight: root.isAndroid ? 44 : 48
          echoMode: TextInput.Password
          placeholderText: "Passphrase"
        }

        Text {
          text: lockDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }

        Item {
          Layout.fillWidth: true
          height: (root.isAndroid ? 12 : 20) + root.safeBottomPadding
        }
        }
      }
    }
  }

  background: Rectangle {
    gradient: Gradient {
      GradientStop { position: 0.0; color: theme.bgTop }
      GradientStop { position: 1.0; color: theme.bgBottom }
    }
  }

  Component {
    id: textReader

    Item {
      anchors.fill: parent
      clip: true
      ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Rectangle {
          height: root.isAndroid ? (root.isPortrait ? 48 : 56) : 72
          radius: 12
          color: theme.panelHighlight
          Layout.fillWidth: true

          RowLayout {
            anchors.fill: parent
            anchors.margins: root.isAndroid ? (root.isPortrait ? 8 : 10) : 12
            spacing: root.isAndroid ? (root.isPortrait ? 8 : 10) : 12

            Button {
              text: "Prev"
              enabled: reader.chapterCount > 0 && reader.currentChapterIndex > 0
              onClicked: reader.prevChapter()
            }

            Button {
              text: "Next"
              enabled: reader.chapterCount > 0 && reader.currentChapterIndex + 1 < reader.chapterCount
              onClicked: reader.nextChapter()
            }

            Text {
              text: reader.currentChapterTitle.length > 0 ? reader.currentChapterTitle : "Chapter"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
              elide: Text.ElideRight
              Layout.fillWidth: true
            }

            TextField {
              id: chapterField
              Layout.preferredWidth: 60
              placeholderText: "Page"
              inputMethodHints: Qt.ImhDigitsOnly
              validator: IntValidator { bottom: 1; top: Math.max(1, reader.chapterCount) }
              onAccepted: {
                const page = parseInt(text)
                if (!isNaN(page)) {
                  reader.goToChapter(page - 1)
                }
              }
              Binding {
                target: chapterField
                property: "text"
                value: reader.chapterCount > 0 ? String(reader.currentChapterIndex + 1) : "1"
                when: !chapterField.activeFocus
              }
            }

            Slider {
              Layout.preferredWidth: 180
              from: 1
              to: Math.max(1, reader.chapterCount)
              stepSize: 1
              value: reader.chapterCount > 0 ? reader.currentChapterIndex + 1 : 1
              onMoved: reader.goToChapter(Math.max(0, Math.round(value - 1)))
            }

            Text {
              text: qsTr("%1 / %2")
                    .arg(reader.chapterCount > 0 ? reader.currentChapterIndex + 1 : 1)
                    .arg(Math.max(1, reader.chapterCount))
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }
        }

        Flickable {
          id: textScroll
          Layout.fillWidth: true
          Layout.fillHeight: true
          contentWidth: textBlock.width
          contentHeight: textBlock.height
          flickableDirection: Flickable.VerticalFlick
          clip: true

          TextEdit {
            id: textBlock
            width: textScroll.width
            height: contentHeight
            readOnly: true
            selectByMouse: !root.isAndroid
            activeFocusOnPress: !root.isAndroid
            cursorVisible: !root.isAndroid
            text: root.displayTextForReader()
            color: theme.textPrimary
            font.pixelSize: root.textFontSizeFor(reader.currentFormat)
            font.family: root.textFontFamilyFor(reader.currentFormat)
            wrapMode: TextEdit.Wrap
            textFormat: TextEdit.RichText
            onSelectionStartChanged: {
              root.textSelectionStart = selectionStart
            }
            onSelectionEndChanged: {
              root.textSelectionEnd = selectionEnd
              root.textSelectionText = selectedText
            }
            clip: true
          }
        }
      }
    }
  }

  Component {
    id: imageReader

    Item {
      id: imageReaderView
      clip: true
      property real controlBarHeight: root.isAndroid ? (root.isPortrait ? 40 : 44) : 72
      property real controlFontSize: root.isAndroid ? (root.isPortrait ? 10 : 11) : 14
      property real controlMargin: root.isAndroid ? (root.isPortrait ? 3 : 4) : 10
      property real zoom: 1.0
      property real minZoom: settings.comicMinZoom
      property real maxZoom: settings.comicMaxZoom
      property real zoomStep: settings.comicZoomStep
      property real pinchStartZoom: 1.0
      property bool pinching: false
      property real targetZoom: 1.0
      property string fitMode: "page" // page, width, height
      property real baseScale: 1.0
      property real sourceW: 1.0
      property real sourceH: 1.0
      property real secondarySourceW: 0.0
      property real secondarySourceH: 0.0
      property real effectiveScale: baseScale * zoom
      property bool spreadEnabled: settings.comicTwoPageSpread
      property bool spreadActive: spreadEnabled && reader.hasImages && reader.imageCount > 1
                                 && (settings.comicSpreadInPortrait || imageFlick.width > imageFlick.height)
      property bool secondaryAvailable: spreadActive && (reader.currentImageIndex + 1) < reader.imageCount
      property int lastImageIndex: -1
      property string lastPath: ""
      property int spreadSpacing: 12
      function clampZoom(value) {
        return Math.max(minZoom, Math.min(maxZoom, value))
      }
      function normalizeFitMode(value) {
        if (value === "width" || value === "height" || value === "page") {
          return value
        }
        return "page"
      }
      function applyFitMode(mode) {
        fitMode = normalizeFitMode(mode)
        zoom = 1.0
        recomputeBase()
        if (settings.comicRememberFitMode && reader.currentPath && reader.currentPath.length > 0) {
          settings.setComicFitModeForPath(reader.currentPath, fitMode)
        }
      }
      function canAdvance(direction) {
        if (reader.imageCount <= 0) {
          return false
        }
        const step = spreadActive ? 2 : 1
        const delta = direction * (settings.comicReadingDirection === "rtl" ? -1 : 1) * step
        const target = reader.currentImageIndex + delta
        return target >= 0 && target < reader.imageCount
      }
      function loadFitModeForBook() {
        var mode = settings.comicDefaultFitMode
        if (settings.comicRememberFitMode && reader.currentPath && reader.currentPath.length > 0) {
          var stored = settings.comicFitModeForPath(reader.currentPath)
          if (stored && stored.length > 0) {
            mode = stored
          }
        }
        fitMode = normalizeFitMode(mode)
        zoom = 1.0
        recomputeBase()
      }
      function recomputeBase() {
        if (imageItem.sourceSize.width <= 0 || imageItem.sourceSize.height <= 0) {
          sourceW = Math.max(1, imageItem.sourceSize.width)
          sourceH = Math.max(1, imageItem.sourceSize.height)
        } else {
          sourceW = imageItem.sourceSize.width
          sourceH = imageItem.sourceSize.height
        }

        if (secondaryAvailable) {
          if (imageItemSecondary.sourceSize.width <= 0 || imageItemSecondary.sourceSize.height <= 0) {
            secondarySourceW = Math.max(1, imageItemSecondary.sourceSize.width)
            secondarySourceH = Math.max(1, imageItemSecondary.sourceSize.height)
          } else {
            secondarySourceW = imageItemSecondary.sourceSize.width
            secondarySourceH = imageItemSecondary.sourceSize.height
          }
        } else {
          secondarySourceW = 0
          secondarySourceH = 0
        }

        var spreadW = sourceW + (secondaryAvailable ? secondarySourceW : 0)
        var spreadH = Math.max(sourceH, secondarySourceH)
        if (spreadW <= 0) spreadW = sourceW
        if (spreadH <= 0) spreadH = sourceH

        var spacing = secondaryAvailable ? spreadSpacing : 0
        var availableW = imageFlick.width - spacing
        var availableH = imageFlick.height
        if (!isFinite(availableW) || availableW <= 0) availableW = 1
        if (!isFinite(availableH) || availableH <= 0) availableH = 1
        if (fitMode === "width") {
          baseScale = availableW / spreadW
        } else if (fitMode === "height") {
          baseScale = availableH / spreadH
        } else {
          baseScale = Math.min(availableW / spreadW, availableH / spreadH)
        }
        if (!isFinite(baseScale) || baseScale <= 0) {
          baseScale = 1.0
        }
      }
      anchors.fill: parent

      onWidthChanged: recomputeBase()
      onHeightChanged: recomputeBase()
      onSpreadActiveChanged: recomputeBase()

      Connections {
        target: reader
        function onCurrentChanged() {
          if (!reader.hasImages) {
            return
          }
          if (reader.currentPath !== imageReaderView.lastPath) {
            imageReaderView.lastPath = reader.currentPath
            imageReaderView.lastImageIndex = reader.currentImageIndex
            imageReaderView.loadFitModeForBook()
            return
          }
          if (reader.currentImageIndex !== imageReaderView.lastImageIndex) {
            imageReaderView.lastImageIndex = reader.currentImageIndex
            if (settings.comicResetZoomOnPageChange) {
              imageReaderView.zoom = 1.0
            }
            imageReaderView.recomputeBase()
          }
        }
      }

      Item {
        anchors.fill: parent

        Rectangle {
          id: imageControlsBar
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.top: parent.top
          anchors.leftMargin: root.isAndroid ? (root.isPortrait ? 1 : 2) : 8
          anchors.rightMargin: root.isAndroid ? (root.isPortrait ? 1 : 2) : 8
          anchors.topMargin: root.isAndroid ? (root.isPortrait ? 1 : 2) : 8
          height: imageReaderView.controlBarHeight
          radius: 8
          color: theme.panelHighlight
          clip: true

          Flickable {
            id: imageControlsFlick
            anchors.fill: parent
            anchors.margins: imageReaderView.controlMargin
            contentWidth: imageControlsRow.implicitWidth + 4
            contentHeight: height
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            interactive: root.isAndroid

            RowLayout {
              id: imageControlsRow
              height: imageControlsFlick.height
              spacing: root.isAndroid ? 6 : 12

            Button {
              text: "Prev"
              font.pixelSize: imageReaderView.controlFontSize
              onClicked: root.advanceComicImage(-1)
              enabled: canAdvance(-1)
            }

            Button {
              text: "Next"
              font.pixelSize: imageReaderView.controlFontSize
              onClicked: root.advanceComicImage(1)
              enabled: canAdvance(1)
            }

            Button {
              text: "Fit Page"
              font.pixelSize: imageReaderView.controlFontSize
              onClicked: applyFitMode("page")
            }

            Button {
              text: "Fit Width"
              font.pixelSize: imageReaderView.controlFontSize
              onClicked: applyFitMode("width")
              visible: !root.isAndroid
            }

            Button {
              text: "Fit Height"
              font.pixelSize: imageReaderView.controlFontSize
              onClicked: applyFitMode("height")
              visible: !root.isAndroid
            }

            Button {
              text: "-"
              font.pixelSize: imageReaderView.controlFontSize
              onClicked: zoom = clampZoom(zoom - zoomStep)
            }

            Button {
              text: "+"
              font.pixelSize: imageReaderView.controlFontSize
              onClicked: zoom = clampZoom(zoom + zoomStep)
            }

            Text {
              text: qsTr("%1%").arg(Math.round(zoom * 100))
              color: theme.textMuted
              font.pixelSize: imageReaderView.controlFontSize
              font.family: root.uiFont
            }

            TextField {
              id: imagePageField
              Layout.preferredWidth: 60
              placeholderText: "Page"
              inputMethodHints: Qt.ImhDigitsOnly
              validator: IntValidator { bottom: 1; top: Math.max(1, reader.imageCount) }
              onAccepted: {
                const page = parseInt(text)
                if (!isNaN(page)) {
                  reader.goToImage(page - 1)
                }
              }
              Binding {
                target: imagePageField
                property: "text"
                value: reader.imageCount > 0 ? String(reader.currentImageIndex + 1) : ""
                when: !imagePageField.activeFocus
              }
            }

            Slider {
              Layout.preferredWidth: root.isAndroid ? 120 : 180
              from: 1
              to: Math.max(1, reader.imageCount)
              stepSize: 1
              value: reader.imageCount > 0 ? reader.currentImageIndex + 1 : 1
              onMoved: reader.goToImage(Math.max(0, Math.round(value - 1)))
              visible: !root.isAndroid
            }

            Text {
              text: reader.imageCount > 0
                    ? ((imageReaderItem && imageReaderItem.spreadActive === true) && reader.currentImageIndex + 1 < reader.imageCount
                       ? qsTr("%1-%2 / %3")
                           .arg(reader.currentImageIndex + 1)
                           .arg(reader.currentImageIndex + 2)
                           .arg(reader.imageCount)
                       : qsTr("%1 / %2").arg(reader.currentImageIndex + 1).arg(reader.imageCount))
                    : ""
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
            }
          }
        }

        Flickable {
          id: imageFlick
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.top: imageControlsBar.bottom
          anchors.bottom: parent.bottom
          anchors.leftMargin: root.isAndroid ? 2 : 8
          anchors.rightMargin: root.isAndroid ? 2 : 8
          anchors.bottomMargin: root.isAndroid ? 2 : 8
          anchors.topMargin: root.isAndroid ? 4 : 8
          contentWidth: Math.max(imageRow.width, width)
          contentHeight: Math.max(imageRow.height, height)
          interactive: imageReaderView.zoom > 1.01 || imageReaderView.pinching
          clip: true
          boundsBehavior: Flickable.StopAtBounds
          onWidthChanged: imageReaderView.recomputeBase()
          onHeightChanged: imageReaderView.recomputeBase()

          WheelHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: function(wheel) {
              const step = wheel.angleDelta.y > 0 ? zoomStep : -zoomStep
              zoom = clampZoom(zoom + step)
            }
          }

          PinchHandler {
            id: comicPinchHandler
            target: null
            enabled: reader.hasImages
            minimumScale: imageReaderView.minZoom
            maximumScale: imageReaderView.maxZoom
            grabPermissions: PointerHandler.CanTakeOverFromAnything
            onActiveChanged: {
              imageReaderView.pinching = active
              if (active) {
                imageReaderView.pinchStartZoom = imageReaderView.zoom
                imageReaderView.targetZoom = imageReaderView.zoom
              }
            }
            onScaleChanged: {
              if (active) {
                const desired = imageReaderView.clampZoom(imageReaderView.pinchStartZoom * scale)
                imageReaderView.targetZoom = desired
                imageReaderView.zoom = imageReaderView.clampZoom(
                  imageReaderView.zoom + (desired - imageReaderView.zoom) * 0.35
                )
              }
            }
          }

          DragHandler {
            id: comicSwipeHandler
            target: null
            enabled: reader.hasImages && !pinching && zoom <= 1.01
            xAxis.enabled: true
            yAxis.enabled: false
            minimumPointCount: 1
            maximumPointCount: 1
            grabPermissions: PointerHandler.CanTakeOverFromAnything

            property real startX: 0
            property real startY: 0

            onActiveChanged: {
              if (active) {
                startX = centroid.position.x
                startY = centroid.position.y
                return
              }
              const dx = centroid.position.x - startX
              const dy = centroid.position.y - startY
              if (Math.abs(dx) > 60 && Math.abs(dx) > Math.abs(dy)) {
                if (dx < 0) {
                  root.advanceComicImage(1)
                } else {
                  root.advanceComicImage(-1)
                }
              }
            }
          }

          TapHandler {
            acceptedButtons: Qt.LeftButton
            onDoubleTapped: {
              if (fitMode === "width") {
                applyFitMode("page")
              } else {
                applyFitMode("width")
              }
            }
          }

          MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            onClicked: {
              if (!root.pickAnnotationAnchor || reader.currentImageIndex < 0) {
                return
              }
              const local = imageItem.mapFromItem(imageFlick, mouse.x, mouse.y)
              if (local.x < 0 || local.y < 0 || local.x > imageItem.width || local.y > imageItem.height) {
                return
              }
              const nx = local.x / imageItem.width
              const ny = local.y / imageItem.height
              const locator = root.makeAnchorLocator(reader.currentImageIndex, nx, ny)
              root.pickAnnotationAnchor = false
              annotationDialog.openForAnchor(locator)
            }
          }

          Row {
            id: imageRow
            spacing: spreadSpacing
            layoutDirection: settings.comicReadingDirection === "rtl"
                            ? Qt.RightToLeft
                            : Qt.LeftToRight
            x: (isFinite(width) && isFinite(imageFlick.width))
               ? Math.max(0, (imageFlick.width - width) / 2)
               : 0
            y: (isFinite(height) && isFinite(imageFlick.height))
               ? Math.max(0, (imageFlick.height - height) / 2)
               : 0

            Image {
              id: imageItem
              source: reader.currentImageUrl.toString().length > 0
                      ? (reader.currentImageUrl.toString() + "?t=" + reader.imageReloadToken)
                      : ""
              fillMode: Image.PreserveAspectFit
              asynchronous: true
              cache: false
              smooth: settings.comicSmoothScaling
              width: sourceW * effectiveScale
              height: sourceH * effectiveScale
              onSourceChanged: recomputeBase()
              onStatusChanged: {
                if (status === Image.Error) {
                  console.warn("Image load failed", source, imageItem.errorString)
                } else if (status === Image.Ready) {
                  console.info("Image loaded", source, sourceSize.width, sourceSize.height)
                  recomputeBase()
                }
              }
            }

            Image {
              id: imageItemSecondary
              source: secondaryAvailable
                      ? (reader.imageUrlAt(reader.currentImageIndex + 1).toString()
                         + "?t=" + reader.imageReloadToken)
                      : ""
              fillMode: Image.PreserveAspectFit
              asynchronous: true
              cache: false
              smooth: settings.comicSmoothScaling
              visible: secondaryAvailable
              width: secondaryAvailable ? secondarySourceW * effectiveScale : 0
              height: secondaryAvailable ? secondarySourceH * effectiveScale : 0
              onSourceChanged: recomputeBase()
              onStatusChanged: {
                if (status === Image.Error) {
                  console.warn("Image load failed", source, imageItemSecondary.errorString)
                } else if (status === Image.Ready) {
                  console.info("Image loaded", source, sourceSize.width, sourceSize.height)
                  recomputeBase()
                }
              }
            }
          }

          Repeater {
            model: root.anchorsForCurrentPage()
            delegate: Rectangle {
              readonly property var pinStyle: root.pinStyleForType(modelData.type)
              width: 12
              height: 12
              radius: pinStyle.radius
              color: modelData.color || "#ffb347"
              rotation: pinStyle.rotation
              transformOrigin: Item.Center
              x: imageRow.x + imageItem.x + modelData.x * imageItem.width - width / 2
              y: imageRow.y + imageItem.y + modelData.y * imageItem.height - height / 2
              visible: imageItem.width > 0 && imageItem.height > 0

              MouseArea {
                anchors.fill: parent
                onClicked: annotationDialog.openForEdit({
                  id: modelData.id,
                  locator: modelData.locator,
                  type: modelData.type,
                  text: modelData.text,
                  color: modelData.color
                })
              }
            }
          }

          // ScrollBars disabled on Android builds to avoid qmlcachegen issues.
        }

        Rectangle {
          visible: root.androidDebugOverlay
          anchors.left: imageFlick.left
          anchors.right: imageFlick.right
          anchors.top: imageFlick.top
          height: root.isAndroid ? 90 : 0
          radius: 8
          color: "#80000000"
          z: 10

          Text {
            anchors.fill: parent
            anchors.margins: 8
            color: "#ffffff"
            font.pixelSize: 11
            font.family: root.monoFont
            text: "flick=" + Math.round(imageFlick.width) + "x" + Math.round(imageFlick.height)
                  + " row=" + Math.round(imageRow.width) + "x" + Math.round(imageRow.height)
                  + " base=" + baseScale.toFixed(3) + " zoom=" + zoom.toFixed(2)
                  + " src=" + Math.round(sourceW) + "x" + Math.round(sourceH)
          }
        }
      }
    }
  }

  Item {
    id: pageContainer
    anchors.fill: parent
    anchors.leftMargin: root.safeLeft
    anchors.rightMargin: root.safeRight
    anchors.topMargin: root.safeTopPadding
    anchors.bottomMargin: bottomTabs.visible ? 0 : root.safeBottomPadding

    StackLayout {
      id: mainPages
      anchors.fill: parent
      currentIndex: root.currentTab

      Loader { sourceComponent: libraryPage }
      Loader { sourceComponent: readerPage }
      Loader { sourceComponent: notesPage }
      Loader { sourceComponent: settingsPage }
    }
  }

  Component {
    id: notesPage

    Item {
      width: parent ? parent.width : 0
      height: parent ? parent.height : 0

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        RowLayout {
          Layout.fillWidth: true
          spacing: 12

          Text {
            text: "Notes"
            color: theme.textPrimary
            font.pixelSize: 22
            font.family: root.uiFont
          }

          Item { Layout.fillWidth: true }

          Button {
            text: "Export"
            font.family: root.uiFont
            enabled: annotationModel.count > 0
            onClicked: exportAnnotationsDialog.open()
          }
        }

        TextField {
          Layout.fillWidth: true
          placeholderText: "Search annotations"
          text: annotationFilterText
          onTextChanged: {
            annotationFilterText = text
            rebuildAnnotationFilter()
          }
        }

        RowLayout {
          Layout.fillWidth: true
          spacing: 12

          ComboBox {
            Layout.fillWidth: true
            model: ["all", "note", "highlight", "bookmark"]
            currentIndex: Math.max(0, model.indexOf(annotationFilterType))
            onActivated: {
              annotationFilterType = model[currentIndex]
              rebuildAnnotationFilter()
            }
          }

          ComboBox {
            Layout.fillWidth: true
            model: ["all", "#ffb347", "#7bdff2", "#c3f584", "#f4a7d3", "#f07167", "#ffd166", "#bdb2ff"]
            currentIndex: Math.max(0, model.indexOf(annotationFilterColor))
            onActivated: {
              annotationFilterColor = model[currentIndex]
              rebuildAnnotationFilter()
            }
          }
        }

        Rectangle {
          Layout.fillWidth: true
          Layout.fillHeight: true
          radius: 12
          color: theme.panel
          border.color: theme.panelHighlight
          border.width: 1

          ListView {
            anchors.fill: parent
            anchors.margins: 10
            clip: true
            model: filteredAnnotations

            delegate: Rectangle {
              width: ListView.view ? ListView.view.width : 0
              height: 110
              radius: 8
              color: index % 2 === 0 ? theme.panelHighlight : theme.panel

              Column {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                Text {
                  text: root.locatorDisplay(model.locator) + " • " + model.type
                  color: theme.textMuted
                  font.pixelSize: 11
                  font.family: root.uiFont
                  elide: Text.ElideRight
                }

                Text {
                  text: model.text
                  color: theme.textPrimary
                  font.pixelSize: 13
                  font.family: root.uiFont
                  wrapMode: Text.WordWrap
                  elide: Text.ElideRight
                }

                Text {
                  text: model.createdAt
                  color: theme.textMuted
                  font.pixelSize: 10
                  font.family: root.uiFont
                }
              }

              MouseArea {
                anchors.fill: parent
                onClicked: {
                  if (model.locator && model.locator.length > 0) {
                    reader.jumpToLocator(model.locator)
                    root.openReader(true)
                  }
                }
              }

              Rectangle {
                width: 10
                height: 10
                radius: 5
                color: model.color
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 8
              }
            }

            Text {
              anchors.centerIn: parent
              visible: filteredAnnotations.count === 0
              text: "No annotations yet"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }
        }
      }
    }
  }

  Component {
    id: settingsPage

    Item {
      width: parent ? parent.width : 0
      height: parent ? parent.height : 0

      StackLayout {
        anchors.fill: parent
        currentIndex: root.settingsSubPage

        Item {
          width: parent ? parent.width : 0
          height: parent ? parent.height : 0

          ScrollView {
            id: settingsScroll
            anchors.fill: parent
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
          id: settingsColumn
          width: Math.max(1, settingsScroll.availableWidth - (root.isAndroid ? 16 : 44))
          anchors.top: parent.top
          anchors.topMargin: (root.isAndroid ? 8 : 22) + root.safeTopPadding
          x: Math.max(0, (settingsScroll.availableWidth - width) / 2)
          spacing: root.isAndroid ? 12 : 20

          Text {
            text: "Settings"
            color: theme.textPrimary
            font.pixelSize: 22
            font.family: root.uiFont
          }

          Rectangle {
            Layout.fillWidth: true
            radius: 16
            color: theme.panel
            implicitHeight: syncPanelContent.implicitHeight + 36

            ColumnLayout {
              id: syncPanelContent
              anchors.fill: parent
              anchors.margins: 18
              spacing: 14

              Text {
                text: "Sync"
                color: theme.textPrimary
                font.pixelSize: 16
                font.family: root.uiFont
              }

              RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                  text: "Enable sync"
                  color: theme.textMuted
                  font.pixelSize: 13
                  font.family: root.uiFont
                }

                Item { Layout.fillWidth: true }

                Switch {
                  checked: syncManager.enabled
                  onToggled: syncManager.enabled = checked
                }
              }

              Text {
                text: syncManager.status
                color: theme.textMuted
                font.pixelSize: 12
                font.family: root.uiFont
                visible: syncManager.status.length > 0
              }

              TextField {
                Layout.fillWidth: true
                Layout.preferredHeight: 46
                placeholderText: "Device name"
                text: syncManager.deviceName
                enabled: syncManager.enabled
                onTextChanged: syncManager.deviceName = text
              }

              RowLayout {
                Layout.fillWidth: true
                spacing: 10

                TextField {
                  Layout.fillWidth: true
                  Layout.preferredHeight: 46
                  placeholderText: "Pairing PIN"
                  echoMode: TextInput.Password
                  text: syncManager.pin
                  enabled: syncManager.enabled
                  inputMethodHints: Qt.ImhDigitsOnly
                  onTextChanged: syncManager.pin = text
                }

                Button {
                  Layout.preferredHeight: 46
                  text: syncManager.discovering ? "Stop" : "Discover"
                  font.family: root.uiFont
                  enabled: syncManager.enabled
                  onClicked: {
                    if (syncManager.discovering) {
                      syncManager.stopDiscovery()
                    } else {
                      syncManager.startDiscovery()
                    }
                  }
                }
              }
            }
          }

          Rectangle {
            Layout.fillWidth: true
            radius: 16
            color: theme.panel
            implicitHeight: securityPanelContent.implicitHeight + 36

            ColumnLayout {
              id: securityPanelContent
              anchors.fill: parent
              anchors.margins: 18
              spacing: 14

              Text {
                text: "Security"
                color: theme.textPrimary
                font.pixelSize: 16
                font.family: root.uiFont
              }

              RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                  text: "Auto-lock"
                  color: theme.textMuted
                  font.pixelSize: 13
                  font.family: root.uiFont
                }

                Item { Layout.fillWidth: true }

                Switch {
                  checked: settings.autoLockEnabled
                  onToggled: settings.autoLockEnabled = checked
                }
              }

              RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                  text: "Auto-lock after"
                  color: theme.textMuted
                  font.pixelSize: 13
                  font.family: root.uiFont
                }

                Slider {
                  Layout.fillWidth: true
                  from: 1
                  to: 240
                  stepSize: 1
                  value: settings.autoLockMinutes
                  enabled: settings.autoLockEnabled
                  onMoved: settings.autoLockMinutes = Math.round(value)
                }

                Text {
                  text: settings.autoLockMinutes + " min"
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }
              }
            }
          }

          Rectangle {
            Layout.fillWidth: true
            radius: 16
            color: theme.panel
            implicitHeight: readingPanelContent.implicitHeight + 36

            ColumnLayout {
              id: readingPanelContent
              anchors.fill: parent
              anchors.margins: 18
              spacing: 14

              Text {
                text: "Reading"
                color: theme.textPrimary
                font.pixelSize: 16
                font.family: root.uiFont
              }

              Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                text: "Format settings"
                font.family: root.uiFont
                onClicked: root.openFormatSettings()
              }

              Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                text: "Keyboard shortcuts"
                font.family: root.uiFont
                onClicked: keyboardDialog.open()
              }
            }
          }

          Rectangle {
            Layout.fillWidth: true
            radius: 16
            color: theme.panel
            implicitHeight: systemPanelContent.implicitHeight + 36

            ColumnLayout {
              id: systemPanelContent
              anchors.fill: parent
              anchors.margins: 18
              spacing: 14

              Text {
                text: "System"
                color: theme.textPrimary
                font.pixelSize: 16
                font.family: root.uiFont
              }

              Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                text: "Advanced settings"
                font.family: root.uiFont
                onClicked: settingsDialog.open()
              }

              Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                text: "Check for updates"
                font.family: root.uiFont
                onClicked: updateDialog.open()
              }

              Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                text: "About"
                font.family: root.uiFont
                onClicked: aboutDialog.open()
              }
            }
          }

          Item {
            Layout.fillWidth: true
            height: (root.isAndroid ? 12 : 22) + root.safeBottomPadding
          }
        }
          }
        }

        Item {
          width: parent ? parent.width : 0
          height: parent ? parent.height : 0

          Loader {
            anchors.fill: parent
            sourceComponent: formatSettingsPage
          }
        }
      }
    }
  }

  Component {
    id: libraryPage

    Item {
      width: parent ? parent.width : 0
      height: parent ? parent.height : 0
      property bool selectionMode: false
      property var selectedIds: []
      property string viewMode: "list"
      property string pendingSearch: ""
      property bool filtersExpanded: false
      ListModel { id: collectionFilterModel }
      ListModel { id: tagFilterModel }

      Timer {
        id: searchDebounce
        interval: 250
        repeat: false
        onTriggered: libraryModel.searchQuery = libraryPage.pendingSearch
      }

      function isSelected(id) {
        return selectedIds.indexOf(id) !== -1
      }

      function toggleSelected(id) {
        var idx = selectedIds.indexOf(id)
        if (idx === -1) {
          selectedIds = selectedIds.concat([id])
        } else {
          var copy = selectedIds.slice()
          copy.splice(idx, 1)
          selectedIds = copy
        }
      }

      function clearSelection() {
        selectedIds = []
      }

      function selectAllVisible() {
        var ids = []
        for (var i = 0; i < libraryModel.count; ++i) {
          const item = libraryModel.get(i)
          if (item && item.id !== undefined) ids.push(item.id)
        }
        selectedIds = ids
      }

      function invertSelection() {
        var ids = []
        for (var i = 0; i < libraryModel.count; ++i) {
          const item = libraryModel.get(i)
          if (!item || item.id === undefined) continue
          if (selectedIds.indexOf(item.id) === -1) {
            ids.push(item.id)
          }
        }
        selectedIds = ids
      }

      function clearFilters() {
        libraryPage.pendingSearch = ""
        librarySearch.text = ""
        libraryModel.searchQuery = ""
        libraryModel.filterCollection = "__all__"
        libraryModel.filterTag = "__all__"
        libraryModel.sortKey = "title"
        libraryModel.sortDescending = false
      }

      function hasActiveFilters() {
        return (libraryModel.searchQuery && libraryModel.searchQuery.length > 0)
          || (libraryModel.filterCollection && libraryModel.filterCollection !== "__all__")
          || (libraryModel.filterTag && libraryModel.filterTag !== "__all__")
      }

      function rebuildFilterOptions() {
        collectionFilterModel.clear()
        tagFilterModel.clear()
        collectionFilterModel.append({ label: "All Collections", value: "__all__" })
        collectionFilterModel.append({ label: "Unassigned", value: "__none__" })
        tagFilterModel.append({ label: "All Tags", value: "__all__" })
        tagFilterModel.append({ label: "Untagged", value: "__none__" })
        const collections = libraryModel.availableCollections || []
        for (var c = 0; c < collections.length; ++c) {
          collectionFilterModel.append({ label: collections[c], value: collections[c] })
        }
        const tags = libraryModel.availableTags || []
        for (var t = 0; t < tags.length; ++t) {
          tagFilterModel.append({ label: tags[t], value: tags[t] })
        }
      }

      Connections {
        target: libraryModel
        function onAvailableCollectionsChanged() { rebuildFilterOptions() }
        function onAvailableTagsChanged() { rebuildFilterOptions() }
        function onReadyChanged() { rebuildFilterOptions() }
      }

      Component.onCompleted: {
        rebuildFilterOptions()
      }

      Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 18

        Rectangle {
          id: header
          radius: 18
          color: theme.panel
          width: parent.width

          ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            RowLayout {
              Layout.fillWidth: true
              spacing: 12

              Text {
                text: "Library"
                color: theme.textPrimary
                font.pixelSize: 26
                font.family: root.uiFont
              }

              Text {
                text: qsTr("%1 books").arg(libraryModel.totalCount)
                color: theme.textMuted
                font.pixelSize: 14
                font.family: root.uiFont
                verticalAlignment: Text.AlignVCenter
              }

              Item { Layout.fillWidth: true }

              Button {
                text: "Add"
                onClicked: addMenu.open()
                font.family: root.uiFont
                enabled: libraryModel.ready && !libraryModel.bulkImportActive
              }
            }

            RowLayout {
              Layout.fillWidth: true
              spacing: 10

              TextField {
                id: librarySearch
                Layout.fillWidth: true
                placeholderText: "Search library"
                text: pendingSearch
                onTextChanged: {
                  pendingSearch = text
                  searchDebounce.restart()
                }
              }

              Button {
                text: "Clear"
                font.family: root.uiFont
                enabled: librarySearch.text.length > 0 || libraryModel.searchQuery.length > 0
                onClicked: {
                  if (libraryPage) {
                    libraryPage.pendingSearch = ""
                  }
                  librarySearch.text = ""
                  libraryModel.searchQuery = ""
                }
              }
            }

            RowLayout {
              Layout.fillWidth: true
              spacing: 10

              Button {
                text: filtersExpanded ? "Hide filters" : "Filters"
                font.family: root.uiFont
                onClicked: filtersExpanded = !filtersExpanded
              }

              Button {
                text: viewMode === "list" ? "Grid view" : "List view"
                font.family: root.uiFont
                onClicked: viewMode = viewMode === "list" ? "grid" : "list"
              }

              Button {
                text: selectionMode ? "Done" : "Select"
                font.family: root.uiFont
                onClicked: {
                  selectionMode = !selectionMode
                  if (!selectionMode) {
                    clearSelection()
                  }
                }
              }

              Item { Layout.fillWidth: true }

              Button {
                text: "Settings"
                font.family: root.uiFont
                onClicked: root.openSettings()
              }

              Button {
                text: "About"
                font.family: root.uiFont
                onClicked: aboutDialog.open()
              }
            }

            RowLayout {
              Layout.fillWidth: true
              spacing: 10
              visible: selectionMode

              Button {
                text: "Select all"
                font.family: root.uiFont
                onClicked: selectAllVisible()
              }

              Button {
                text: "Invert"
                font.family: root.uiFont
                enabled: libraryModel.count > 0
                onClicked: invertSelection()
              }

              Button {
                text: "Clear"
                font.family: root.uiFont
                onClicked: clearSelection()
              }

              Button {
                text: qsTr("Edit tags (%1)").arg(selectedIds.length)
                font.family: root.uiFont
                enabled: selectedIds.length > 0
                onClicked: bulkTagsDialog.openForIds(selectedIds)
              }

              Button {
                text: qsTr("Remove (%1)").arg(selectedIds.length)
                font.family: root.uiFont
                enabled: selectedIds.length > 0
                onClicked: deleteBooksDialog.openForIds(selectedIds)
              }
            }
          }
        }

        Rectangle {
          id: bulkImportBar
          visible: libraryModel.bulkImportActive
          height: visible ? 44 : 0
          radius: 12
          color: theme.panel
          width: parent.width

          RowLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 12

            Text {
              text: qsTr("Importing %1 / %2")
                      .arg(libraryModel.bulkImportDone)
                      .arg(libraryModel.bulkImportTotal)
              color: theme.textMuted
              font.pixelSize: 14
              font.family: root.uiFont
            }

            ProgressBar {
              id: bulkProgress
              Layout.fillWidth: true
              from: 0
              to: Math.max(1, libraryModel.bulkImportTotal)
              value: libraryModel.bulkImportDone
            }

            Button {
              text: "Cancel"
              font.family: root.uiFont
              onClicked: libraryModel.cancelBulkImport()
            }
          }
        }

        Rectangle {
          id: filterBar
          radius: 14
          color: theme.panelHighlight
          width: parent.width
          visible: filtersExpanded

          ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            RowLayout {
              Layout.fillWidth: true
              spacing: 10

              Text {
                text: "Collection"
                color: theme.textMuted
                font.pixelSize: 12
                font.family: root.uiFont
                Layout.preferredWidth: 90
              }

              ComboBox {
                id: collectionFilter
                Layout.fillWidth: true
                model: collectionFilterModel
                textRole: "label"
                currentIndex: {
                  for (var i = 0; i < model.count; ++i) {
                    if (model.get(i).value === libraryModel.filterCollection) return i
                  }
                  return 0
                }
                onActivated: libraryModel.filterCollection = model.get(currentIndex).value
              }
            }

            RowLayout {
              Layout.fillWidth: true
              spacing: 10

              Text {
                text: "Tag"
                color: theme.textMuted
                font.pixelSize: 12
                font.family: root.uiFont
                Layout.preferredWidth: 90
              }

              ComboBox {
                id: tagFilter
                Layout.fillWidth: true
                model: tagFilterModel
                textRole: "label"
                currentIndex: {
                  for (var i = 0; i < model.count; ++i) {
                    if (model.get(i).value === libraryModel.filterTag) return i
                  }
                  return 0
                }
                onActivated: libraryModel.filterTag = model.get(currentIndex).value
              }
            }

            RowLayout {
              Layout.fillWidth: true
              spacing: 10

              Text {
                text: "Sort"
                color: theme.textMuted
                font.pixelSize: 12
                font.family: root.uiFont
                Layout.preferredWidth: 90
              }

              ComboBox {
                id: sortCombo
                Layout.fillWidth: true
                model: [
                  { label: "Title", key: "title" },
                  { label: "Author", key: "authors" },
                  { label: "Series", key: "series" },
                  { label: "Publisher", key: "publisher" },
                  { label: "Collection", key: "collection" },
                  { label: "Format", key: "format" },
                  { label: "Added", key: "added" }
                ]
                textRole: "label"
                currentIndex: {
                  for (var i = 0; i < model.length; ++i) {
                    if (model[i].key === libraryModel.sortKey) return i
                  }
                  return 0
                }
                onActivated: libraryModel.sortKey = model[currentIndex].key
              }

              Button {
                text: libraryModel.sortDescending ? "↓" : "↑"
                font.family: root.uiFont
                onClicked: libraryModel.sortDescending = !libraryModel.sortDescending
              }
            }

            RowLayout {
              Layout.fillWidth: true
              spacing: 10

              Button {
                text: "Reset filters"
                font.family: root.uiFont
                enabled: hasActiveFilters() || libraryModel.sortKey !== "title" || libraryModel.sortDescending
                onClicked: clearFilters()
              }

              Button {
                text: "Lock"
                font.family: root.uiFont
                visible: vault.state === VaultController.Unlocked
                onClicked: {
                  if (sessionPassphrase.length > 0) {
                    if (vault.lock(sessionPassphrase)) {
                      autoUnlockArmed = false
                      if (!settings.rememberPassphrase) {
                        sessionPassphrase = ""
                      }
                    }
                  } else {
                    lockDialog.open()
                  }
                }
              }

              Item { Layout.fillWidth: true }
            }
          }
        }

        Rectangle {
          id: libraryPanel
          radius: 18
          color: theme.panel
          anchors.horizontalCenter: parent.horizontalCenter
          width: parent.width
          height: parent.height
                  - header.height
                  - bulkImportBar.height
                  - (filtersExpanded ? filterBar.height : 0)
                  - 96

          Item {
            anchors.fill: parent

            ColumnLayout {
              anchors.fill: parent
              anchors.margins: 12
              spacing: 10

              Item {
                id: libraryListArea
                Layout.fillWidth: true
                Layout.fillHeight: true

                ListView {
                  id: listView
                  anchors.fill: parent
                  model: libraryModel
                  reuseItems: true
                  clip: true
                  spacing: 8
                  visible: viewMode === "list"

                  delegate: Rectangle {
                    radius: 12
                    height: 120
                    width: listView.width
                    color: index % 2 === 0 ? theme.panelHighlight : theme.panel
                    border.width: isSelected(model.id) ? 2 : 0
                    border.color: theme.accent

                RowLayout {
                  anchors.fill: parent
                  anchors.margins: 16
                  spacing: 16

                  Item {
                    id: libraryContent
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    MouseArea {
                      anchors.fill: parent
                      onClicked: {
                        if (selectionMode) {
                          toggleSelected(model.id)
                          return
                        }
                        reader.close()
                        reader.openFileAsync(model.path)
                        annotationModel.libraryItemId = model.id
                        root.openReader(true)
                      }
                      onPressAndHold: {
                        if (!selectionMode) {
                          selectionMode = true
                        }
                        toggleSelected(model.id)
                      }
                    }

                    Row {
                      anchors.fill: parent
                      spacing: 16

                      Rectangle {
                        width: selectionMode ? 22 : 0
                        height: selectionMode ? 22 : 0
                        radius: 4
                        color: isSelected(model.id) ? theme.accent : "transparent"
                        border.width: selectionMode ? 1 : 0
                        border.color: theme.textMuted
                        visible: selectionMode
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                          anchors.centerIn: parent
                          text: isSelected(model.id) ? "✓" : ""
                          color: "#0f141a"
                          font.pixelSize: 12
                          font.family: root.uiFont
                        }

                        MouseArea {
                          anchors.fill: parent
                          onClicked: toggleSelected(model.id)
                        }
                      }

                      Rectangle {
                        width: 52
                        height: 56
                        radius: 8
                        color: theme.accentAlt

                        Text {
                          anchors.centerIn: parent
                          text: model.format.toUpperCase()
                          color: "#0f141a"
                          font.pixelSize: 12
                          font.family: root.uiFont
                        }
                      }

                      Column {
                        spacing: 4
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                          text: model.title
                          color: theme.textPrimary
                          font.pixelSize: 17
                          font.family: root.uiFont
                          elide: Text.ElideRight
                          width: libraryContent.width - 140
                        }

                        Text {
                          text: {
                            var parts = [];
                            if (model.authors && model.authors.length > 0) parts.push(model.authors);
                            if (model.series && model.series.length > 0) parts.push(model.series);
                            if (model.publisher && model.publisher.length > 0) parts.push(model.publisher);
                            if (model.collection && model.collection.length > 0) parts.push(model.collection);
                            if (model.tags && model.tags.length > 0) parts.push(model.tags);
                            return parts.length > 0 ? parts.join(" • ") : ""
                          }
                          color: theme.textMuted
                          font.pixelSize: 12
                          font.family: root.uiFont
                          elide: Text.ElideRight
                          width: libraryContent.width - 140
                          visible: text.length > 0
                        }

                        Text {
                          text: (model.description && model.description.length > 0) ? model.description : model.path
                          color: theme.textMuted
                          font.pixelSize: 12
                          font.family: root.uiFont
                          elide: Text.ElideRight
                          width: libraryContent.width - 140
                        }
                      }
                    }

                    Rectangle {
                      visible: model.annotationCount > 0
                      radius: 12
                      height: 24
                      width: Math.max(32, badgeText.implicitWidth + 16)
                      color: theme.accent
                      anchors.right: parent.right
                      anchors.verticalCenter: parent.verticalCenter
                      anchors.rightMargin: 12

                      Text {
                        id: badgeText
                        anchors.centerIn: parent
                        text: model.annotationCount
                        color: "#0f141a"
                        font.pixelSize: 12
                        font.family: root.uiFont
                      }
                    }
                  }
                }
                  }
                }

                GridView {
                  id: gridView
                  anchors.fill: parent
                  model: libraryModel
                  reuseItems: true
                  clip: true
                  visible: viewMode === "grid"
                  cellWidth: 232
                  cellHeight: 272

                  delegate: Rectangle {
                    width: gridView.cellWidth
                    height: gridView.cellHeight
                    radius: 14
                    color: theme.panelHighlight
                    border.width: isSelected(model.id) ? 2 : 0
                    border.color: theme.accent

                MouseArea {
                  anchors.fill: parent
                  onClicked: {
                    if (selectionMode) {
                      toggleSelected(model.id)
                      return
                    }
                    reader.close()
                    reader.openFileAsync(model.path)
                    annotationModel.libraryItemId = model.id
                    root.openReader(true)
                  }
                }

                Column {
                  anchors.fill: parent
                  anchors.margins: 12
                  spacing: 8

                  Rectangle {
                    width: parent.width
                    height: 140
                    radius: 10
                    color: theme.panel
                    Image {
                      id: gridCover
                      anchors.fill: parent
                      source: model.coverPath && model.coverPath.length > 0
                              ? root.fileUrl(model.coverPath)
                              : root.fileUrl(settings.iconPath)
                      fillMode: Image.PreserveAspectFit
                      cache: false
                    }

                    Text {
                      anchors.centerIn: parent
                      text: model.format.toUpperCase()
                      color: theme.textMuted
                      font.pixelSize: 18
                      font.family: root.uiFont
                      visible: gridCover.status !== Image.Ready
                    }
                  }

                  Text {
                    text: model.title
                    color: theme.textPrimary
                    font.pixelSize: 14
                    font.family: root.uiFont
                    elide: Text.ElideRight
                    maximumLineCount: 2
                    wrapMode: Text.Wrap
                  }

                  Text {
                    text: model.authors
                    color: theme.textMuted
                    font.pixelSize: 12
                    font.family: root.uiFont
                    elide: Text.ElideRight
                  }
                }

                Rectangle {
                  visible: selectionMode
                  width: 22
                  height: 22
                  radius: 4
                  color: isSelected(model.id) ? theme.accent : "transparent"
                  border.width: 1
                  border.color: theme.textMuted
                  anchors.right: parent.right
                  anchors.top: parent.top
                  anchors.rightMargin: 8
                  anchors.topMargin: 8

                  Text {
                    anchors.centerIn: parent
                    text: isSelected(model.id) ? "✓" : ""
                    color: "#0f141a"
                    font.pixelSize: 12
                    font.family: root.uiFont
                  }
                }
                  }
                }
              }

              RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Button {
                  text: "Prev"
                  font.family: root.uiFont
                  enabled: libraryModel.pageIndex > 0
                  onClicked: libraryModel.prevPage()
                }

                Text {
                  text: qsTr("Page %1 / %2")
                        .arg(libraryModel.pageIndex + 1)
                        .arg(libraryModel.pageCount)
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                Button {
                  text: "Next"
                  font.family: root.uiFont
                  enabled: libraryModel.pageIndex + 1 < libraryModel.pageCount
                  onClicked: libraryModel.nextPage()
                }

                Text {
                  text: "Go to"
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                TextField {
                  id: pageJumpField
                  Layout.preferredWidth: 64
                  placeholderText: "Page"
                  inputMethodHints: Qt.ImhDigitsOnly
                  validator: IntValidator { bottom: 1; top: Math.max(1, libraryModel.pageCount) }
                  onAccepted: {
                    const page = parseInt(text)
                    if (!isNaN(page)) {
                      libraryModel.goToPage(page - 1)
                    }
                  }
                  Binding {
                    target: pageJumpField
                    property: "text"
                    value: String(libraryModel.pageIndex + 1)
                    when: !pageJumpField.activeFocus
                  }
                }

                Button {
                  text: "Go"
                  font.family: root.uiFont
                  enabled: pageJumpField.text.length > 0
                  onClicked: {
                    const page = parseInt(pageJumpField.text)
                    if (!isNaN(page)) {
                      libraryModel.goToPage(page - 1)
                    }
                  }
                }

                Item { Layout.fillWidth: true }

                Text {
                  text: "Page size"
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                ComboBox {
                  id: pageSizeCombo
                  model: [25, 50, 100, 200]
                  currentIndex: {
                    var idx = model.indexOf(libraryModel.pageSize)
                    return idx >= 0 ? idx : 1
                  }
                  onActivated: libraryModel.pageSize = model[currentIndex]
                }
              }
            }
          }
        }

        Text {
          text: libraryModel.lastError
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }
      }

      Rectangle {
        id: addFab
        width: 56
        height: 56
        radius: 28
        color: theme.accent
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 22
        visible: !selectionMode
        z: 10

        Text {
          anchors.centerIn: parent
          text: "+"
          color: "#0f141a"
          font.pixelSize: 28
          font.family: root.uiFont
        }

        MouseArea {
          anchors.fill: parent
          onClicked: addMenu.open()
        }
      }
    }
  }

  Component {
    id: readerPage

    Item {
      property string sidebarMode: "toc"
      property int readerDrawerTab: 0
      width: parent ? parent.width : 0
      height: parent ? parent.height : 0

      onSidebarModeChanged: {
        if (reader.currentPath && reader.currentPath.length > 0) {
          settings.setSidebarModeForPath(reader.currentPath, sidebarMode)
        }
      }

      Connections {
        target: reader
        function onCurrentChanged() {
          if (reader.currentPath && reader.currentPath.length > 0) {
            sidebarMode = settings.sidebarModeForPath(reader.currentPath)
          }
        }
      }

      Drawer {
        id: readerDrawer
        edge: Qt.RightEdge
        width: Math.min(root.width * 0.86, 360)
        height: parent.height
        modal: true

        Rectangle {
          anchors.fill: parent
          color: theme.panel

          ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            RowLayout {
              Layout.fillWidth: true
              spacing: 10

              Text {
                text: "Reader"
                color: theme.textPrimary
                font.pixelSize: 18
                font.family: root.uiFont
              }

              Item { Layout.fillWidth: true }

              Button {
                text: "Close"
                font.family: root.uiFont
                onClicked: readerDrawer.close()
              }
            }

            TabBar {
              id: readerDrawerTabs
              Layout.fillWidth: true
              currentIndex: readerDrawerTab
              onCurrentIndexChanged: readerDrawerTab = currentIndex

              TabButton { text: "Contents"; font.family: root.uiFont }
              TabButton { text: "Notes"; font.family: root.uiFont }
            }

            StackLayout {
              Layout.fillWidth: true
              Layout.fillHeight: true
              currentIndex: readerDrawerTab

              Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Text {
                  anchors.centerIn: parent
                  visible: reader.tocCount === 0
                  text: "No TOC available"
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                ListView {
                  visible: reader.tocCount > 0
                  anchors.fill: parent
                  clip: true
                  model: reader.tocCount
                  delegate: Rectangle {
                    width: ListView.view ? ListView.view.width : 0
                    height: 60
                    radius: 10
                    color: reader.tocChapterIndex(index) === reader.currentChapterIndex
                           ? theme.accentAlt
                           : (index % 2 === 0 ? theme.panel : theme.panelHighlight)

                    Row {
                      anchors.fill: parent
                      anchors.margins: 10
                      spacing: 10

                      Text {
                        text: (index + 1) + "."
                        color: reader.tocChapterIndex(index) === reader.currentChapterIndex
                               ? "#0f141a" : theme.textMuted
                        font.pixelSize: 11
                        font.family: root.uiFont
                      }

                      Text {
                        text: reader.tocTitle(index).length > 0
                              ? reader.tocTitle(index)
                              : "Chapter " + (index + 1)
                        color: reader.tocChapterIndex(index) === reader.currentChapterIndex
                               ? "#0f141a" : theme.textPrimary
                        font.pixelSize: 13
                        font.family: root.uiFont
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                      }
                    }

                    MouseArea {
                      anchors.fill: parent
                      onClicked: {
                        const chapterIndex = reader.tocChapterIndex(index)
                        if (chapterIndex >= 0) {
                          reader.goToChapter(chapterIndex)
                          readerDrawer.close()
                        }
                      }
                    }
                  }
                }
              }

              Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                  anchors.fill: parent
                  spacing: 8

                  TextField {
                    Layout.fillWidth: true
                    placeholderText: "Search annotations"
                    text: annotationFilterText
                    onTextChanged: {
                      annotationFilterText = text
                      rebuildAnnotationFilter()
                    }
                  }

                  RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ComboBox {
                      Layout.fillWidth: true
                      model: ["all", "note", "highlight", "bookmark"]
                      currentIndex: Math.max(0, model.indexOf(annotationFilterType))
                      onActivated: {
                        annotationFilterType = model[currentIndex]
                        rebuildAnnotationFilter()
                      }
                    }

                    ComboBox {
                      Layout.fillWidth: true
                      model: ["all", "#ffb347", "#7bdff2", "#c3f584", "#f4a7d3", "#f07167", "#ffd166", "#bdb2ff"]
                      currentIndex: Math.max(0, model.indexOf(annotationFilterColor))
                      onActivated: {
                        annotationFilterColor = model[currentIndex]
                        rebuildAnnotationFilter()
                      }
                    }
                  }

                  Button {
                    text: "Export annotations"
                    font.family: root.uiFont
                    onClicked: exportAnnotationsDialog.open()
                  }

                  ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: filteredAnnotations
                    delegate: Rectangle {
                      width: parent.width
                      height: 110
                      radius: 10
                      color: index % 2 === 0 ? theme.panel : theme.panelHighlight

                      Column {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 4

                        Text {
                          text: root.locatorDisplay(model.locator) + " • " + model.type
                          color: theme.textMuted
                          font.pixelSize: 11
                          font.family: root.uiFont
                          elide: Text.ElideRight
                        }

                        Text {
                          text: model.text
                          color: theme.textPrimary
                          font.pixelSize: 13
                          font.family: root.uiFont
                          wrapMode: Text.WordWrap
                          elide: Text.ElideRight
                        }

                        Text {
                          text: model.createdAt
                          color: theme.textMuted
                          font.pixelSize: 10
                          font.family: root.uiFont
                        }
                      }

                      MouseArea {
                        anchors.fill: parent
                        onClicked: {
                          reader.jumpToLocator(model.locator)
                          readerDrawer.close()
                        }
                      }

                      Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: model.color
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 8
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }

      Menu {
        id: readerMenu

        MenuItem {
          text: tts.speaking ? "Stop speaking" : "Speak"
          enabled: tts.available && reader.ttsAllowed
          onTriggered: {
            if (tts.speaking) {
              tts.stop()
            } else {
              tts.speak(reader.currentPlainText)
            }
          }
        }

        MenuItem {
          text: "Queue TTS"
          enabled: tts.available && reader.ttsAllowed
          onTriggered: tts.enqueue(reader.currentPlainText)
        }

        MenuItem {
          text: "Annotate"
          onTriggered: annotationDialog.open()
        }

        MenuItem {
          text: "Settings"
          onTriggered: root.openSettings()
        }

        MenuItem {
          text: "Lock Library"
          visible: vault.state === VaultController.Unlocked
          onTriggered: {
            if (sessionPassphrase.length > 0) {
              if (vault.lock(sessionPassphrase)) {
                autoUnlockArmed = false
                if (!settings.rememberPassphrase) {
                  sessionPassphrase = ""
                }
              }
              reader.close()
              root.goToLibrary()
            } else {
              lockDialog.open()
            }
          }
        }
      }

      Item {
        anchors.fill: parent
        anchors.margins: root.isAndroid ? 2 : 24

        Rectangle {
          id: readerTopBar
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.top: parent.top
          height: root.isAndroid ? (root.isPortrait ? 44 : 48) : 64
          radius: 16
          color: theme.panel
          z: 2
          visible: root.readerChromeVisible

          Flickable {
            id: readerTopFlick
            anchors.fill: parent
            anchors.margins: root.isAndroid ? (root.isPortrait ? 6 : 8) : 18
            contentWidth: readerTopRow.implicitWidth + 4
            contentHeight: height
            boundsBehavior: Flickable.StopAtBounds
            interactive: root.isAndroid
            clip: true

            RowLayout {
              id: readerTopRow
              height: readerTopFlick.height
              spacing: root.isAndroid ? 8 : 16

            Button {
              text: "Back"
              font.family: root.uiFont
              onClicked: {
                reader.close()
                root.goToLibrary()
              }
            }

            Text {
              text: reader.currentTitle
              color: theme.textPrimary
              font.pixelSize: root.isAndroid ? 16 : 18
              font.family: root.uiFont
              elide: Text.ElideRight
              Layout.fillWidth: true
            }

            Text {
              text: root.readerPaginationLabel()
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
              visible: text.length > 0
            }

            Button {
              text: "Contents"
              font.family: root.uiFont
              onClicked: readerDrawer.open()
            }

            ToolButton {
              text: "⋯"
              font.pixelSize: 20
              onClicked: readerMenu.open()
            }
            }
          }
        }

        Rectangle {
          id: readerFrame
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.top: root.readerChromeVisible ? readerTopBar.bottom : parent.top
          anchors.bottom: parent.bottom
          anchors.topMargin: root.readerChromeVisible ? (root.isAndroid ? 6 : 16) : 0
          radius: 18
          color: theme.panel
          clip: true
          z: 1

          TapHandler {
            acceptedDevices: PointerDevice.TouchScreen
            onTapped: root.readerChromeVisible = !root.readerChromeVisible
          }

          DragHandler {
            id: readerSwipeHandler
            target: null
            enabled: !reader.hasImages
            xAxis.enabled: true
            yAxis.enabled: false
            minimumPointCount: 1
            maximumPointCount: 1
            grabPermissions: PointerHandler.CanTakeOverFromAnything

            property real startX: 0
            property real startY: 0

            onActiveChanged: {
              if (active) {
                startX = centroid.position.x
                startY = centroid.position.y
                return
              }
              const dx = centroid.position.x - startX
              const dy = centroid.position.y - startY
              if (reader.hasImages && imageReaderItem && imageReaderItem.zoom > 1.01) {
                return
              }
              if (Math.abs(dx) > 60 && Math.abs(dx) > Math.abs(dy)) {
                if (dx < 0) {
                  root.advanceReader(1)
                } else {
                  root.advanceReader(-1)
                }
              }
            }
          }

          Loader {
            id: readerContent
            anchors.fill: parent
            anchors.leftMargin: root.isAndroid ? 2 : 12
            anchors.rightMargin: root.isAndroid ? 2 : 12
            anchors.topMargin: root.isAndroid ? 2 : 12
            anchors.bottomMargin: root.isAndroid ? 2 : 12
            active: true
            clip: true
            sourceComponent: reader.hasImages ? imageReader : textReader
            onLoaded: root.imageReaderItem = item
            onItemChanged: root.imageReaderItem = item
          }

          Rectangle {
            id: readerBottomBar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: root.isAndroid ? 4 : 12
            height: root.isAndroid ? 52 : 68
            radius: 14
            color: theme.panel
            visible: root.readerChromeVisible && !reader.hasImages
            z: 3

            Flickable {
              id: readerBottomFlick
              anchors.fill: parent
              anchors.margins: root.isAndroid ? 6 : 12
              contentWidth: readerBottomRow.implicitWidth + 4
              contentHeight: height
              clip: true
              boundsBehavior: Flickable.StopAtBounds
              interactive: root.isAndroid

              RowLayout {
                id: readerBottomRow
                height: readerBottomFlick.height
                spacing: root.isAndroid ? 8 : 10

              Button {
                text: "Prev"
                font.family: root.uiFont
                onClicked: advanceReader(-1)
              }

              Button {
                text: "Next"
                font.family: root.uiFont
                onClicked: advanceReader(1)
              }

              Item { Layout.fillWidth: true }

              Slider {
                Layout.fillWidth: true
                from: 0
                to: Math.max(0, root.readerPaginationCount() - 1)
                value: Math.max(0, root.readerPaginationIndex())
                visible: root.readerPaginationCount() > 0
                onMoved: root.goToReaderPagination(Math.round(value))
              }

              TextField {
                id: readerPageField
                Layout.preferredWidth: 64
                placeholderText: reader.hasImages ? "Page" : "Chapter"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 1; top: Math.max(1, root.readerPaginationCount()) }
                onAccepted: {
                  const page = parseInt(text)
                  if (!isNaN(page)) {
                    root.goToReaderPagination(page - 1)
                  }
                }
                Binding {
                  target: readerPageField
                  property: "text"
                  value: root.readerPaginationIndex() >= 0 ? String(root.readerPaginationIndex() + 1) : ""
                  when: !readerPageField.activeFocus
                }
                visible: root.readerPaginationCount() > 0
              }
              }
            }
          }

          Rectangle {
            anchors.fill: parent
            color: "#aa0d1015"
            radius: 18
            visible: reader.busy

            Column {
              anchors.centerIn: parent
              spacing: 10

              BusyIndicator {
                running: reader.busy
                width: 48
                height: 48
              }

              Text {
                text: "Loading..."
                color: theme.textPrimary
                font.pixelSize: 16
                font.family: root.uiFont
              }
            }
          }
        }

        Text {
          text: reader.lastError
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }
      }

      
    }
  }

  Dialog {
    id: updateDialog
    title: "Updates"
    modal: true
    standardButtons: Dialog.Ok
    width: Math.min(520, root.width - 80)

    contentItem: Rectangle {
      color: theme.panel
      radius: 16

      Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Row {
          spacing: 10
          visible: updateManager.state === UpdateManager.Checking || updateManager.state === UpdateManager.Applying

          BusyIndicator {
            running: updateManager.state === UpdateManager.Checking || updateManager.state === UpdateManager.Applying
            width: 20
            height: 20
          }

          Text {
            text: updateManager.status
            color: theme.textPrimary
            font.pixelSize: 14
            font.family: root.uiFont
          }
        }

        Text {
          text: (updateManager.state === UpdateManager.Checking || updateManager.state === UpdateManager.Applying) ? "" : updateManager.status
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
          visible: updateManager.state !== UpdateManager.Checking && updateManager.state !== UpdateManager.Applying
        }

        Text {
          text: updateManager.summary
          color: theme.textMuted
          font.pixelSize: 12
          font.family: root.uiFont
          visible: updateManager.summary.length > 0
        }

        Text {
          text: "Current version: " + Qt.application.version
          color: theme.textMuted
          font.pixelSize: 12
          font.family: root.uiFont
        }

        TextArea {
          readOnly: true
          text: updateManager.details
          visible: updateManager.details.length > 0
          wrapMode: TextEdit.Wrap
          color: theme.textPrimary
          background: Rectangle { color: "transparent" }
        }

        Text {
          text: "Restart the app to use the new version."
          color: theme.textMuted
          font.pixelSize: 12
          font.family: root.uiFont
          visible: updateManager.state === UpdateManager.UpToDate && updateManager.status === "Update applied"
        }
      }
    }
  }

  Dialog {
    id: restartDialog
    title: "Restart Required"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: Math.min(420, root.width - 80)

    contentItem: Rectangle {
      color: theme.panel
      radius: 16

      Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
          text: "Update applied. Restart now to use the new version?"
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
          wrapMode: Text.Wrap
        }
      }
    }

    onAccepted: {
      updateManager.restartApp()
      Qt.quit()
    }
  }

  Dialog {
    id: keyboardDialog
    title: "Keyboard Shortcuts"
    modal: true
    standardButtons: Dialog.Close
    width: Math.min(640, root.width - 80)
    height: Math.min(520, root.height - 80)

    contentItem: Rectangle {
      color: theme.panel
      radius: 16

      ScrollView {
        anchors.fill: parent
        anchors.margins: 12

        ColumnLayout {
          width: parent.width
          spacing: 12

          Text {
            text: "Library"
            color: theme.textPrimary
            font.pixelSize: 18
            font.family: root.uiFont
          }

          Repeater {
            model: [
              { id: "open_book", label: "Open book" },
              { id: "import_folder", label: "Import folder" },
              { id: "focus_search", label: "Focus search" }
            ]
            delegate: ColumnLayout {
              Layout.fillWidth: true
              spacing: 4

              readonly property string conflictText: root.bindingConflictText(modelData.id)

              RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                  text: modelData.label
                  color: theme.textPrimary
                  font.pixelSize: 13
                  font.family: root.uiFont
                  Layout.preferredWidth: 180
                }

                TextField {
                  id: keyField
                  Layout.fillWidth: true
                  text: root.bindingText(modelData.id)
                  placeholderText: "Ctrl+O"
                  onEditingFinished: settings.setKeyBinding(modelData.id, text)
                  background: Rectangle {
                    radius: 6
                    color: theme.panel
                    border.width: 1
                    border.color: conflictText.length > 0 ? theme.accent : theme.panelHighlight
                  }
                  Binding {
                    target: keyField
                    property: "text"
                    value: root.bindingText(modelData.id)
                    when: !keyField.activeFocus
                  }
                }
              }

              Text {
                text: conflictText
                color: theme.accent
                font.pixelSize: 11
                font.family: root.uiFont
                wrapMode: Text.Wrap
                visible: conflictText.length > 0
              }
            }
          }

          Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.panelHighlight
          }

          Text {
            text: "Reader"
            color: theme.textPrimary
            font.pixelSize: 18
            font.family: root.uiFont
          }

          Repeater {
            model: [
              { id: "close_book", label: "Close current book" },
              { id: "next", label: "Next page or chapter" },
              { id: "prev", label: "Previous page or chapter" },
              { id: "jump_start", label: "Jump to start" },
              { id: "jump_end", label: "Jump to end" }
            ]
            delegate: ColumnLayout {
              Layout.fillWidth: true
              spacing: 4

              readonly property string conflictText: root.bindingConflictText(modelData.id)

              RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                  text: modelData.label
                  color: theme.textPrimary
                  font.pixelSize: 13
                  font.family: root.uiFont
                  Layout.preferredWidth: 180
                }

                TextField {
                  id: keyField
                  Layout.fillWidth: true
                  text: root.bindingText(modelData.id)
                  onEditingFinished: settings.setKeyBinding(modelData.id, text)
                  background: Rectangle {
                    radius: 6
                    color: theme.panel
                    border.width: 1
                    border.color: conflictText.length > 0 ? theme.accent : theme.panelHighlight
                  }
                  Binding {
                    target: keyField
                    property: "text"
                    value: root.bindingText(modelData.id)
                    when: !keyField.activeFocus
                  }
                }
              }

              Text {
                text: conflictText
                color: theme.accent
                font.pixelSize: 11
                font.family: root.uiFont
                wrapMode: Text.Wrap
                visible: conflictText.length > 0
              }
            }
          }

          Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.panelHighlight
          }

          Text {
            text: "Text & Zoom"
            color: theme.textPrimary
            font.pixelSize: 18
            font.family: root.uiFont
          }

          Repeater {
            model: [
              { id: "zoom_in", label: "Increase text size or zoom in images" },
              { id: "zoom_out", label: "Decrease text size or zoom out images" },
              { id: "zoom_reset", label: "Reset image zoom to page fit" }
            ]
            delegate: ColumnLayout {
              Layout.fillWidth: true
              spacing: 4

              readonly property string conflictText: root.bindingConflictText(modelData.id)

              RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                  text: modelData.label
                  color: theme.textPrimary
                  font.pixelSize: 13
                  font.family: root.uiFont
                  Layout.preferredWidth: 180
                }

                TextField {
                  id: keyField
                  Layout.fillWidth: true
                  text: root.bindingText(modelData.id)
                  onEditingFinished: settings.setKeyBinding(modelData.id, text)
                  background: Rectangle {
                    radius: 6
                    color: theme.panel
                    border.width: 1
                    border.color: conflictText.length > 0 ? theme.accent : theme.panelHighlight
                  }
                  Binding {
                    target: keyField
                    property: "text"
                    value: root.bindingText(modelData.id)
                    when: !keyField.activeFocus
                  }
                }
              }

              Text {
                text: conflictText
                color: theme.accent
                font.pixelSize: 11
                font.family: root.uiFont
                wrapMode: Text.Wrap
                visible: conflictText.length > 0
              }
            }
          }

          Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.panelHighlight
          }

          Text {
            text: "Text To Speech"
            color: theme.textPrimary
            font.pixelSize: 18
            font.family: root.uiFont
          }

          Repeater {
            model: [
              { id: "toggle_tts", label: "Toggle speak / stop" }
            ]
            delegate: ColumnLayout {
              Layout.fillWidth: true
              spacing: 4

              readonly property string conflictText: root.bindingConflictText(modelData.id)

              RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                  text: modelData.label
                  color: theme.textPrimary
                  font.pixelSize: 13
                  font.family: root.uiFont
                  Layout.preferredWidth: 180
                }

                TextField {
                  id: keyField
                  Layout.fillWidth: true
                  text: root.bindingText(modelData.id)
                  onEditingFinished: settings.setKeyBinding(modelData.id, text)
                  background: Rectangle {
                    radius: 6
                    color: theme.panel
                    border.width: 1
                    border.color: conflictText.length > 0 ? theme.accent : theme.panelHighlight
                  }
                  Binding {
                    target: keyField
                    property: "text"
                    value: root.bindingText(modelData.id)
                    when: !keyField.activeFocus
                  }
                }
              }

              Text {
                text: conflictText
                color: theme.accent
                font.pixelSize: 11
                font.family: root.uiFont
                wrapMode: Text.Wrap
                visible: conflictText.length > 0
              }
            }
          }

          Text {
            text: "Shortcuts are disabled while a modal dialog is open."
            color: theme.textMuted
            font.pixelSize: 12
            font.family: root.uiFont
            wrapMode: Text.Wrap
          }

          Text {
            text: "Conflicting shortcuts are highlighted in orange."
            color: theme.textMuted
            font.pixelSize: 12
            font.family: root.uiFont
            wrapMode: Text.Wrap
          }

          Text {
            text: "Use commas to add multiple bindings (e.g. Right, PageDown, Space)."
            color: theme.textMuted
            font.pixelSize: 12
            font.family: root.uiFont
            wrapMode: Text.Wrap
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
              text: "Reset Key Bindings"
              font.family: root.uiFont
              onClicked: settings.resetKeyBindings()
            }

            Item { Layout.fillWidth: true }
          }
        }
      }
    }
  }

  Component {
    id: formatSettingsPage

    Rectangle {
      anchors.fill: parent
      color: theme.panel
      radius: 0

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        RowLayout {
          Layout.fillWidth: true
          spacing: 12

          Button {
            text: "Back"
            font.family: root.uiFont
            onClicked: root.settingsSubPage = 0
          }

          Text {
            text: "Format Settings"
            color: theme.textPrimary
            font.pixelSize: 18
            font.family: root.uiFont
          }

          Item { Layout.fillWidth: true }
        }

        Text {
          text: "Adjust per-format rendering and reading options."
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
          wrapMode: Text.Wrap
        }

        Flow {
          Layout.fillWidth: true
          spacing: 8

          Repeater {
            model: ["EPUB", "FB2", "TXT", "MOBI", "PDF", "Comics", "DJVU"]
            delegate: Button {
              id: formatTabButton
              text: modelData
              checkable: true
              checked: root.formatSettingsTabIndex === index
              onClicked: root.formatSettingsTabIndex = index
              font.family: root.uiFont
              padding: 6
              leftPadding: 12
              rightPadding: 12

              background: Rectangle {
                radius: 10
                color: formatTabButton.checked ? theme.accentAlt : theme.panelHighlight
                border.color: formatTabButton.checked ? theme.accentAlt : theme.panelHighlight
              }

              contentItem: Text {
                text: formatTabButton.text
                color: formatTabButton.checked ? "#0f141a" : theme.textPrimary
                font.pixelSize: 12
                font.family: root.uiFont
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
              }
            }
          }
        }



        ScrollView {
          Layout.fillWidth: true
          Layout.fillHeight: true
          clip: true

          ColumnLayout {
            width: parent.width
            spacing: 18
            StackLayout {
              Layout.fillWidth: true
              currentIndex: root.formatSettingsTabIndex

              ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                  text: "EPUB"
                  color: theme.textPrimary
                  font.pixelSize: 20
                  font.family: root.uiFont
                }

                Text {
                  text: "EPUB reading and layout."
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Font size"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 12
                    to: 36
                    stepSize: 1
                    value: settings.epubFontSize
                    onMoved: settings.epubFontSize = Math.round(value)
                  }

                  SpinBox {
                    from: 12
                    to: 36
                    value: settings.epubFontSize
                    editable: true
                    onValueModified: settings.epubFontSize = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Line height"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 1.0
                    to: 2.0
                    stepSize: 0.05
                    value: settings.epubLineHeight
                    onMoved: settings.epubLineHeight = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.epubLineHeight.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Show images"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.epubShowImages
                    onToggled: settings.epubShowImages = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Text align"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["left", "justify", "center", "right"]
                    currentIndex: Math.max(0, model.indexOf(settings.epubTextAlign))
                    onActivated: settings.epubTextAlign = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Paragraph spacing"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 3.0
                    stepSize: 0.05
                    value: settings.epubParagraphSpacing
                    onMoved: settings.epubParagraphSpacing = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.epubParagraphSpacing.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Paragraph indent"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 3.0
                    stepSize: 0.05
                    value: settings.epubParagraphIndent
                    onMoved: settings.epubParagraphIndent = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.epubParagraphIndent.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Image max width"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 10
                    to: 100
                    stepSize: 5
                    value: settings.epubImageMaxWidth
                    onMoved: settings.epubImageMaxWidth = Math.round(value)
                  }

                  Text {
                    text: settings.epubImageMaxWidth + "%"
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Image spacing"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 4.0
                    stepSize: 0.05
                    value: settings.epubImageSpacing
                    onMoved: settings.epubImageSpacing = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.epubImageSpacing.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                Text {
                  text: settings.formatSettingsPath("epub")
                  color: theme.textMuted
                  font.pixelSize: 11
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Button {
                    text: "Reset EPUB"
                    onClicked: settings.resetEpubDefaults()
                    font.family: root.uiFont
                  }

                  Item { Layout.fillWidth: true }
                }

                Rectangle { height: 1; color: theme.panelHighlight; Layout.fillWidth: true }
              }

              ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                  text: "FB2"
                  color: theme.textPrimary
                  font.pixelSize: 20
                  font.family: root.uiFont
                }

                Text {
                  text: "FB2 reading and layout."
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Font size"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 12
                    to: 36
                    stepSize: 1
                    value: settings.fb2FontSize
                    onMoved: settings.fb2FontSize = Math.round(value)
                  }

                  SpinBox {
                    from: 12
                    to: 36
                    value: settings.fb2FontSize
                    editable: true
                    onValueModified: settings.fb2FontSize = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Line height"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 1.0
                    to: 2.0
                    stepSize: 0.05
                    value: settings.fb2LineHeight
                    onMoved: settings.fb2LineHeight = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.fb2LineHeight.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Show images"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.fb2ShowImages
                    onToggled: settings.fb2ShowImages = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Text align"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["left", "justify", "center", "right"]
                    currentIndex: Math.max(0, model.indexOf(settings.fb2TextAlign))
                    onActivated: settings.fb2TextAlign = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Paragraph spacing"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 3.0
                    stepSize: 0.05
                    value: settings.fb2ParagraphSpacing
                    onMoved: settings.fb2ParagraphSpacing = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.fb2ParagraphSpacing.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Paragraph indent"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 3.0
                    stepSize: 0.05
                    value: settings.fb2ParagraphIndent
                    onMoved: settings.fb2ParagraphIndent = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.fb2ParagraphIndent.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Image max width"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 10
                    to: 100
                    stepSize: 5
                    value: settings.fb2ImageMaxWidth
                    onMoved: settings.fb2ImageMaxWidth = Math.round(value)
                  }

                  Text {
                    text: settings.fb2ImageMaxWidth + "%"
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Image spacing"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 4.0
                    stepSize: 0.05
                    value: settings.fb2ImageSpacing
                    onMoved: settings.fb2ImageSpacing = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.fb2ImageSpacing.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                Text {
                  text: settings.formatSettingsPath("fb2")
                  color: theme.textMuted
                  font.pixelSize: 11
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Button {
                    text: "Reset FB2"
                    onClicked: settings.resetFb2Defaults()
                    font.family: root.uiFont
                  }

                  Item { Layout.fillWidth: true }
                }

                Rectangle { height: 1; color: theme.panelHighlight; Layout.fillWidth: true }
              }

              ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                  text: "TXT"
                  color: theme.textPrimary
                  font.pixelSize: 20
                  font.family: root.uiFont
                }

                Text {
                  text: "Plain text rendering."
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Font size"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 12
                    to: 36
                    stepSize: 1
                    value: settings.txtFontSize
                    onMoved: settings.txtFontSize = Math.round(value)
                  }

                  SpinBox {
                    from: 12
                    to: 36
                    value: settings.txtFontSize
                    editable: true
                    onValueModified: settings.txtFontSize = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Line height"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 1.0
                    to: 2.0
                    stepSize: 0.05
                    value: settings.txtLineHeight
                    onMoved: settings.txtLineHeight = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.txtLineHeight.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Encoding"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["auto", "utf-8", "utf-16le", "utf-16be", "utf-32le", "utf-32be", "latin1"]
                    currentIndex: Math.max(0, model.indexOf(settings.txtEncoding))
                    onActivated: settings.txtEncoding = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Tab width"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: 16
                    stepSize: 1
                    value: settings.txtTabWidth
                    onMoved: settings.txtTabWidth = Math.round(value)
                  }

                  Text {
                    text: settings.txtTabWidth.toString()
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Trim whitespace"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.txtTrimWhitespace
                    onToggled: settings.txtTrimWhitespace = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Auto chapters"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.txtAutoChapters
                    onToggled: settings.txtAutoChapters = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Monospace"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.txtMonospace
                    onToggled: settings.txtMonospace = checked
                  }
                }

                Text {
                  text: settings.formatSettingsPath("txt")
                  color: theme.textMuted
                  font.pixelSize: 11
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Button {
                    text: "Reset TXT"
                    onClicked: settings.resetTxtDefaults()
                    font.family: root.uiFont
                  }

                  Item { Layout.fillWidth: true }
                }

                Rectangle { height: 1; color: theme.panelHighlight; Layout.fillWidth: true }
              }

              ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                  text: "MOBI/AZW/PRC"
                  color: theme.textPrimary
                  font.pixelSize: 20
                  font.family: root.uiFont
                }

                Text {
                  text: "Kindle-format rendering."
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Font size"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 12
                    to: 36
                    stepSize: 1
                    value: settings.mobiFontSize
                    onMoved: settings.mobiFontSize = Math.round(value)
                  }

                  SpinBox {
                    from: 12
                    to: 36
                    value: settings.mobiFontSize
                    editable: true
                    onValueModified: settings.mobiFontSize = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Line height"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 1.0
                    to: 2.0
                    stepSize: 0.05
                    value: settings.mobiLineHeight
                    onMoved: settings.mobiLineHeight = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.mobiLineHeight.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Show images"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.mobiShowImages
                    onToggled: settings.mobiShowImages = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Text align"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["left", "justify", "center", "right"]
                    currentIndex: Math.max(0, model.indexOf(settings.mobiTextAlign))
                    onActivated: settings.mobiTextAlign = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Paragraph spacing"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 3.0
                    stepSize: 0.05
                    value: settings.mobiParagraphSpacing
                    onMoved: settings.mobiParagraphSpacing = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.mobiParagraphSpacing.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Paragraph indent"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 3.0
                    stepSize: 0.05
                    value: settings.mobiParagraphIndent
                    onMoved: settings.mobiParagraphIndent = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.mobiParagraphIndent.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Image max width"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 10
                    to: 100
                    stepSize: 5
                    value: settings.mobiImageMaxWidth
                    onMoved: settings.mobiImageMaxWidth = Math.round(value)
                  }

                  Text {
                    text: settings.mobiImageMaxWidth + "%"
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Image spacing"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.0
                    to: 4.0
                    stepSize: 0.05
                    value: settings.mobiImageSpacing
                    onMoved: settings.mobiImageSpacing = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.mobiImageSpacing.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                Text {
                  text: settings.formatSettingsPath("mobi")
                  color: theme.textMuted
                  font.pixelSize: 11
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Button {
                    text: "Reset MOBI"
                    onClicked: settings.resetMobiDefaults()
                    font.family: root.uiFont
                  }

                  Item { Layout.fillWidth: true }
                }

                Rectangle { height: 1; color: theme.panelHighlight; Layout.fillWidth: true }
              }

              ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                  text: "PDF"
                  color: theme.textPrimary
                  font.pixelSize: 20
                  font.family: root.uiFont
                }

                Text {
                  text: "PDF rendering quality and caching."
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Quality preset"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["custom", "fast", "balanced", "high"]
                    currentIndex: model.indexOf(settings.pdfRenderPreset)
                    onActivated: settings.pdfRenderPreset = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Render DPI"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 72
                    to: 240
                    stepSize: 6
                    value: settings.pdfDpi
                    enabled: settings.pdfRenderPreset === "custom"
                    onMoved: settings.pdfDpi = Math.round(value)
                  }

                  SpinBox {
                    from: 72
                    to: 240
                    value: settings.pdfDpi
                    editable: true
                    enabled: settings.pdfRenderPreset === "custom"
                    onValueModified: settings.pdfDpi = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Cache pages"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 5
                    to: 120
                    stepSize: 1
                    value: settings.pdfCacheLimit
                    onMoved: settings.pdfCacheLimit = Math.round(value)
                  }

                  SpinBox {
                    from: 5
                    to: 120
                    value: settings.pdfCacheLimit
                    editable: true
                    onValueModified: settings.pdfCacheLimit = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Cache policy"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["fifo", "lru"]
                    currentIndex: model.indexOf(settings.pdfCachePolicy)
                    onActivated: settings.pdfCachePolicy = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Prefetch distance"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: 6
                    stepSize: 1
                    value: settings.pdfPrefetchDistance
                    onMoved: settings.pdfPrefetchDistance = Math.round(value)
                  }

                  SpinBox {
                    from: 0
                    to: 6
                    value: settings.pdfPrefetchDistance
                    editable: true
                    onValueModified: settings.pdfPrefetchDistance = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Pre-render pages"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 1
                    to: 12
                    stepSize: 1
                    value: settings.pdfPreRenderPages
                    onMoved: settings.pdfPreRenderPages = Math.round(value)
                  }

                  SpinBox {
                    from: 1
                    to: 12
                    value: settings.pdfPreRenderPages
                    editable: true
                    onValueModified: settings.pdfPreRenderPages = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Prefetch strategy"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["forward", "symmetric", "backward"]
                    currentIndex: model.indexOf(settings.pdfPrefetchStrategy)
                    onActivated: settings.pdfPrefetchStrategy = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Progressive render"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.pdfProgressiveRendering
                    text: settings.pdfProgressiveRendering ? "On" : "Off"
                    onToggled: settings.pdfProgressiveRendering = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12
                  enabled: settings.pdfProgressiveRendering

                  Text {
                    text: "Preview DPI"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 48
                    to: settings.pdfDpi
                    stepSize: 6
                    value: settings.pdfProgressiveDpi
                    onMoved: settings.pdfProgressiveDpi = Math.round(value)
                  }

                  SpinBox {
                    from: 48
                    to: settings.pdfDpi
                    value: settings.pdfProgressiveDpi
                    editable: true
                    onValueModified: settings.pdfProgressiveDpi = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Color mode"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["color", "grayscale"]
                    currentIndex: model.indexOf(settings.pdfColorMode)
                    onActivated: settings.pdfColorMode = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Background"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["white", "transparent", "theme", "custom"]
                    currentIndex: model.indexOf(settings.pdfBackgroundMode)
                    onActivated: settings.pdfBackgroundMode = model[currentIndex]
                  }

                  TextField {
                    Layout.preferredWidth: 120
                    text: settings.pdfBackgroundColor
                    placeholderText: "#202633"
                    enabled: settings.pdfBackgroundMode !== "transparent"
                    onEditingFinished: settings.pdfBackgroundColor = text
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Max size"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  SpinBox {
                    from: 0
                    to: 20000
                    value: settings.pdfMaxWidth
                    editable: true
                    onValueModified: settings.pdfMaxWidth = value
                  }

                  SpinBox {
                    from: 0
                    to: 20000
                    value: settings.pdfMaxHeight
                    editable: true
                    onValueModified: settings.pdfMaxHeight = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Image format"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["png", "jpeg"]
                    currentIndex: model.indexOf(settings.pdfImageFormat)
                    onActivated: settings.pdfImageFormat = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12
                  enabled: settings.pdfImageFormat === "jpeg"

                  Text {
                    text: "JPEG quality"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 1
                    to: 100
                    stepSize: 1
                    value: settings.pdfJpegQuality
                    onMoved: settings.pdfJpegQuality = Math.round(value)
                  }

                  SpinBox {
                    from: 1
                    to: 100
                    value: settings.pdfJpegQuality
                    editable: true
                    onValueModified: settings.pdfJpegQuality = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Extract text"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.pdfExtractText
                    text: settings.pdfExtractText ? "On" : "Off"
                    onToggled: settings.pdfExtractText = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Tile size"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: 4096
                    stepSize: 256
                    value: settings.pdfTileSize
                    onMoved: settings.pdfTileSize = Math.round(value / 256) * 256
                  }

                  SpinBox {
                    from: 0
                    to: 8192
                    value: settings.pdfTileSize
                    editable: true
                    onValueModified: settings.pdfTileSize = value
                  }
                }

                Text {
                  text: settings.formatSettingsPath("pdf")
                  color: theme.textMuted
                  font.pixelSize: 11
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Button {
                    text: "Reset PDF"
                    onClicked: settings.resetPdfDefaults()
                    font.family: root.uiFont
                  }

                  Item { Layout.fillWidth: true }
                }

                Rectangle { height: 1; color: theme.panelHighlight; Layout.fillWidth: true }
              }

              ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                  text: "Comics"
                  color: theme.textPrimary
                  font.pixelSize: 20
                  font.family: root.uiFont
                }

                Text {
                  text: "CBZ/CBR zoom and sorting."
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Min zoom"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.2
                    to: 6.0
                    stepSize: 0.1
                    value: settings.comicMinZoom
                    onMoved: settings.comicMinZoom = Math.round(value * 10) / 10
                  }

                  Text {
                    text: settings.comicMinZoom.toFixed(1)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Max zoom"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 1.0
                    to: 8.0
                    stepSize: 0.1
                    value: settings.comicMaxZoom
                    onMoved: settings.comicMaxZoom = Math.round(value * 10) / 10
                  }

                  Text {
                    text: settings.comicMaxZoom.toFixed(1)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Default fit"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["page", "width", "height"]
                    currentIndex: Math.max(0, model.indexOf(settings.comicDefaultFitMode))
                    onActivated: settings.comicDefaultFitMode = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Remember per-book"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.comicRememberFitMode
                    onToggled: settings.comicRememberFitMode = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Reading direction"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["ltr", "rtl"]
                    currentIndex: Math.max(0, model.indexOf(settings.comicReadingDirection))
                    onActivated: settings.comicReadingDirection = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Reset zoom on page change"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.comicResetZoomOnPageChange
                    onToggled: settings.comicResetZoomOnPageChange = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Zoom step"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0.05
                    to: 0.5
                    stepSize: 0.05
                    value: settings.comicZoomStep
                    onMoved: settings.comicZoomStep = Math.round(value * 100) / 100
                  }

                  Text {
                    text: settings.comicZoomStep.toFixed(2)
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 48
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Smooth scaling"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.comicSmoothScaling
                    onToggled: settings.comicSmoothScaling = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Two-page spread"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.comicTwoPageSpread
                    onToggled: settings.comicTwoPageSpread = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Spread in portrait"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.comicSpreadInPortrait
                    onToggled: settings.comicSpreadInPortrait = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Sort mode"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["path", "filename", "archive"]
                    currentIndex: Math.max(0, model.indexOf(settings.comicSortMode))
                    onActivated: settings.comicSortMode = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Descending"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.comicSortDescending
                    onToggled: settings.comicSortDescending = checked
                  }
                }

                Text {
                  text: settings.formatSettingsPath("cbz")
                  color: theme.textMuted
                  font.pixelSize: 11
                  font.family: root.uiFont
                }

                Text {
                  text: settings.formatSettingsPath("cbr")
                  color: theme.textMuted
                  font.pixelSize: 11
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Button {
                    text: "Reset Comics"
                    onClicked: settings.resetComicDefaults()
                    font.family: root.uiFont
                  }

                  Item { Layout.fillWidth: true }
                }

                Rectangle { height: 1; color: theme.panelHighlight; Layout.fillWidth: true }
              }

              ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                  text: "DJVU"
                  color: theme.textPrimary
                  font.pixelSize: 20
                  font.family: root.uiFont
                }

                Text {
                  text: "DJVU rendering controls."
                  color: theme.textMuted
                  font.pixelSize: 12
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Render DPI"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 72
                    to: 240
                    stepSize: 6
                    value: settings.djvuDpi
                    onMoved: settings.djvuDpi = Math.round(value)
                  }

                  SpinBox {
                    from: 72
                    to: 240
                    value: settings.djvuDpi
                    editable: true
                    onValueModified: settings.djvuDpi = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Cache pages"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 5
                    to: 120
                    stepSize: 1
                    value: settings.djvuCacheLimit
                    onMoved: settings.djvuCacheLimit = Math.round(value)
                  }

                  SpinBox {
                    from: 5
                    to: 120
                    value: settings.djvuCacheLimit
                    editable: true
                    onValueModified: settings.djvuCacheLimit = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Prefetch distance"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: 6
                    stepSize: 1
                    value: settings.djvuPrefetchDistance
                    onMoved: settings.djvuPrefetchDistance = Math.round(value)
                  }

                  SpinBox {
                    from: 0
                    to: 6
                    value: settings.djvuPrefetchDistance
                    editable: true
                    onValueModified: settings.djvuPrefetchDistance = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Pre-render pages"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  Slider {
                    Layout.fillWidth: true
                    from: 1
                    to: 12
                    stepSize: 1
                    value: settings.djvuPreRenderPages
                    onMoved: settings.djvuPreRenderPages = Math.round(value)
                  }

                  SpinBox {
                    from: 1
                    to: 12
                    value: settings.djvuPreRenderPages
                    editable: true
                    onValueModified: settings.djvuPreRenderPages = value
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Cache policy"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["fifo", "lru"]
                    currentIndex: model.indexOf(settings.djvuCachePolicy)
                    onActivated: settings.djvuCachePolicy = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Output format"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["ppm", "tiff"]
                    currentIndex: model.indexOf(settings.djvuImageFormat)
                    onActivated: settings.djvuImageFormat = model[currentIndex]
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Extract text"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  CheckBox {
                    checked: settings.djvuExtractText
                    onToggled: settings.djvuExtractText = checked
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Text {
                    text: "Rotation"
                    color: theme.textMuted
                    font.pixelSize: 13
                    font.family: root.uiFont
                    Layout.preferredWidth: 120
                  }

                  ComboBox {
                    Layout.fillWidth: true
                    model: ["0", "90", "180", "270"]
                    currentIndex: model.indexOf(String(settings.djvuRotation))
                    onActivated: settings.djvuRotation = parseInt(model[currentIndex])
                  }
                }

                Text {
                  text: settings.formatSettingsPath("djvu")
                  color: theme.textMuted
                  font.pixelSize: 11
                  font.family: root.uiFont
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 12

                  Button {
                    text: "Reset DJVU"
                    onClicked: settings.resetDjvuDefaults()
                    font.family: root.uiFont
                  }

                  Item { Layout.fillWidth: true }
                }

              }
            }

            Rectangle {
              Layout.fillWidth: true
              height: 1
              color: theme.panelHighlight
            }

            RowLayout {
              Layout.fillWidth: true
              spacing: 12

              Button {
                text: "Back to Settings"
                onClicked: {
                  root.settingsSubPage = 0
                }
                font.family: root.uiFont
              }

              Item { Layout.fillWidth: true }
            }
          }
        }
      }
    }
  }

  Dialog {
    id: settingsDialog
    title: "Settings"
    modal: false
    closePolicy: Popup.NoAutoClose
    standardButtons: Dialog.NoButton
    x: 0
    y: 0
    width: root.width
    height: root.height

    contentItem: Rectangle {
      color: theme.panel
      radius: 16

      ScrollView {
        anchors.fill: parent
        anchors.margins: 12

        ColumnLayout {
          width: parent.width
          spacing: 18

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
              text: "Back"
              font.family: root.uiFont
              onClicked: settingsDialog.close()
            }

            Text {
              text: "Settings"
              color: theme.textPrimary
              font.pixelSize: 18
              font.family: root.uiFont
            }

            Item { Layout.fillWidth: true }
          }

          Text {
            text: "Updates"
            color: theme.textPrimary
            font.pixelSize: 20
            font.family: root.uiFont
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Check for updates"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 160
            }

            Button {
              text: "Check"
              font.family: root.uiFont
              enabled: updateManager.canUpdate
              onClicked: {
                updateDialog.open()
                updateManager.checkForUpdates()
              }
            }

            Button {
              text: "Apply"
              font.family: root.uiFont
              enabled: updateManager.canUpdate && updateManager.state === UpdateManager.UpdateAvailable
              onClicked: {
                updateDialog.open()
                updateManager.applyUpdate()
              }
            }

            Text {
              text: updateManager.status
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }

          Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.panelHighlight
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Keyboard shortcuts"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 160
            }

            Button {
              text: "Open"
              font.family: root.uiFont
              onClicked: keyboardDialog.open()
            }

            Button {
              text: "Format settings"
              font.family: root.uiFont
              onClicked: root.openFormatSettings()
            }
          }

          Text {
            text: "Sync"
            color: theme.textPrimary
            font.pixelSize: 20
            font.family: root.uiFont
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Enable sync"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            CheckBox {
              checked: syncManager.enabled
              onToggled: syncManager.enabled = checked
            }

            Text {
              text: syncManager.status
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Device name"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            TextField {
              Layout.fillWidth: true
              text: syncManager.deviceName
              enabled: syncManager.enabled
              onTextChanged: syncManager.deviceName = text
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Pairing PIN"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            TextField {
              Layout.preferredWidth: 140
              echoMode: TextInput.Password
              text: syncManager.pin
              enabled: syncManager.enabled
              inputMethodHints: Qt.ImhDigitsOnly
              onTextChanged: syncManager.pin = text
            }

            Button {
              text: syncManager.discovering ? "Stop" : "Discover"
              font.family: root.uiFont
              enabled: syncManager.enabled
              onClicked: {
                if (syncManager.discovering) {
                  syncManager.stopDiscovery()
                } else {
                  syncManager.startDiscovery()
                }
              }
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Conflict policy"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            ComboBox {
              Layout.fillWidth: true
              enabled: syncManager.enabled
              model: ["newer", "prefer_local", "prefer_remote"]
              currentIndex: {
                const policy = syncManager.conflictPolicy
                if (policy === "prefer_local") return 1
                if (policy === "prefer_remote") return 2
                return 0
              }
              onActivated: {
                syncManager.conflictPolicy = model[currentIndex]
              }
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Transfer files"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            CheckBox {
              checked: syncManager.transferEnabled
              enabled: syncManager.enabled
              onToggled: syncManager.transferEnabled = checked
            }

            Text {
              text: syncManager.transferEnabled ? "Enabled" : "Disabled"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Max transfer"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            Slider {
              Layout.fillWidth: true
              from: 1
              to: 512
              stepSize: 1
              value: syncManager.transferMaxMb
              enabled: syncManager.enabled && syncManager.transferEnabled
              onMoved: syncManager.transferMaxMb = Math.round(value)
            }

            SpinBox {
              from: 1
              to: 512
              value: syncManager.transferMaxMb
              editable: true
              enabled: syncManager.enabled && syncManager.transferEnabled
              onValueModified: syncManager.transferMaxMb = value
            }

            Text {
              text: "MB"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12
            visible: syncManager.transferActive

            Text {
              text: "Transfer"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            ProgressBar {
              Layout.fillWidth: true
              from: 0
              to: Math.max(1, syncManager.transferTotal)
              value: syncManager.transferDone
            }

            Text {
              text: syncManager.transferDone + "/" + syncManager.transferTotal
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12
            visible: syncManager.uploadActive

            Text {
              text: "Upload"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            ProgressBar {
              Layout.fillWidth: true
              from: 0
              to: Math.max(1, syncManager.uploadTotal)
              value: syncManager.uploadDone
            }

            Text {
              text: syncManager.uploadDone + "/" + syncManager.uploadTotal
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }

          Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.panelHighlight
          }

          Text {
            text: "Devices"
            color: theme.textPrimary
            font.pixelSize: 16
            font.family: root.uiFont
          }

          ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            clip: true
            model: syncManager.devices
            delegate: Rectangle {
              width: ListView.view ? ListView.view.width : 0
              height: 56
              radius: 8
              color: index % 2 === 0 ? theme.panelHighlight : theme.panel

              RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 10

                ColumnLayout {
                  Layout.fillWidth: true
                  spacing: 2

                  Text {
                    text: modelData.name + (modelData.paired ? " • paired" : "")
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.family: root.uiFont
                    elide: Text.ElideRight
                  }

                  Text {
                    text: modelData.address + ":" + modelData.port
                    color: theme.textMuted
                    font.pixelSize: 11
                    font.family: root.uiFont
                  }

                  Text {
                    text: modelData.lastSync && modelData.lastSync.length > 0
                          ? "Last sync: " + modelData.lastSync
                          : "Last sync: never"
                    color: theme.textMuted
                    font.pixelSize: 10
                    font.family: root.uiFont
                  }
                }

                Button {
                  text: modelData.paired ? "Unpair" : "Pair"
                  font.family: root.uiFont
                  enabled: syncManager.enabled
                  onClicked: {
                    if (modelData.paired) {
                      syncManager.unpair(modelData.id)
                    } else {
                      syncManager.requestPairing(modelData.id)
                    }
                  }
                }

                Button {
                  text: "Sync"
                  font.family: root.uiFont
                  visible: modelData.paired
                  enabled: syncManager.enabled
                  onClicked: syncManager.syncNow(modelData.id)
                }
              }
            }
          }

          Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.panelHighlight
          }

          Text {
            text: "Security"
            color: theme.textPrimary
            font.pixelSize: 20
            font.family: root.uiFont
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Auto-lock"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            CheckBox {
              checked: settings.autoLockEnabled
              onToggled: settings.autoLockEnabled = checked
            }

            Text {
              text: settings.autoLockEnabled ? "Enabled" : "Disabled"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Auto-lock after"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            Slider {
              Layout.fillWidth: true
              from: 1
              to: 240
              stepSize: 1
              value: settings.autoLockMinutes
              enabled: settings.autoLockEnabled
              onMoved: settings.autoLockMinutes = Math.round(value)
            }

            SpinBox {
              from: 1
              to: 240
              value: settings.autoLockMinutes
              editable: true
              enabled: settings.autoLockEnabled
              onValueModified: settings.autoLockMinutes = value
            }

            Text {
              text: "min"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Remember passphrase"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            CheckBox {
              checked: settings.rememberPassphrase
              enabled: vault.keychainAvailable
              onToggled: settings.rememberPassphrase = checked
            }

            Text {
              text: settings.rememberPassphrase ? "Stored in keychain" : "Prompt each time"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }

          Text {
            visible: !vault.keychainAvailable
            text: "Keychain unavailable — persistent passphrase storage disabled."
            color: theme.textMuted
            font.pixelSize: 12
            font.family: root.uiFont
          }

          Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.panelHighlight
          }

          Text {
            text: "Text To Speech"
            color: theme.textPrimary
            font.pixelSize: 20
            font.family: root.uiFont
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Voice"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            ComboBox {
              Layout.fillWidth: true
              enabled: tts.available && tts.voiceLabels.length > 0
              model: tts.voiceLabels
              currentIndex: {
                const keys = tts.voiceKeys
                const selectedKey = settings.ttsVoiceKey.length > 0 ? settings.ttsVoiceKey : tts.voiceKey
                for (var i = 0; i < keys.length; ++i) {
                  if (keys[i] === selectedKey) return i
                }
                return 0
              }
              onActivated: {
                if (currentIndex >= 0 && currentIndex < tts.voiceKeys.length) {
                  settings.ttsVoiceKey = tts.voiceKeys[currentIndex]
                }
              }
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Rate"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            Slider {
              Layout.fillWidth: true
              from: -1.0
              to: 1.0
              stepSize: 0.05
              value: settings.ttsRate
              enabled: tts.available
              onMoved: settings.ttsRate = Math.round(value * 100) / 100
            }

            Text {
              text: settings.ttsRate.toFixed(2)
              color: theme.textPrimary
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 48
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Pitch"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            Slider {
              Layout.fillWidth: true
              from: -1.0
              to: 1.0
              stepSize: 0.05
              value: settings.ttsPitch
              enabled: tts.available
              onMoved: settings.ttsPitch = Math.round(value * 100) / 100
            }

            Text {
              text: settings.ttsPitch.toFixed(2)
              color: theme.textPrimary
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 48
            }
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
              text: "Volume"
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 120
            }

            Slider {
              Layout.fillWidth: true
              from: 0.0
              to: 1.0
              stepSize: 0.05
              value: settings.ttsVolume
              enabled: tts.available
              onMoved: settings.ttsVolume = Math.round(value * 100) / 100
            }

            Text {
              text: settings.ttsVolume.toFixed(2)
              color: theme.textPrimary
              font.pixelSize: 13
              font.family: root.uiFont
              Layout.preferredWidth: 48
            }
          }

          Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.panelHighlight
          }

          RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
              text: "Reset defaults"
              onClicked: settings.resetDefaults()
              font.family: root.uiFont
            }

            Button {
              text: "Reload file"
              onClicked: settings.reload()
              font.family: root.uiFont
            }

            Item { Layout.fillWidth: true }

            Text {
              text: settings.settingsPath
              color: theme.textMuted
              font.pixelSize: 11
              font.family: root.uiFont
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignRight
              Layout.fillWidth: true
            }
          }
        }
      }
    }
  }

  Dialog {
    id: aboutDialog
    title: "About & Licenses"
    modal: true
    standardButtons: Dialog.Close
    width: Math.min(960, root.width - 80)
    height: Math.min(680, root.height - 80)

    property var entries: []
    property string selectedPath: ""

    onOpened: {
      entries = licenseManager.licenses()
      if (entries.length > 0) {
        selectedPath = entries[0].path
      }
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 16

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
          Layout.fillWidth: true
          spacing: 12

          ColumnLayout {
            Layout.fillWidth: true

            Text {
              text: "My Ereader"
              color: theme.textPrimary
              font.pixelSize: 22
              font.family: root.uiFont
            }

            Text {
              text: "Bundled third-party licenses"
              color: theme.textMuted
              font.pixelSize: 14
              font.family: root.uiFont
            }
          }

          ColumnLayout {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            Text {
              text: "Version " + Qt.application.version
              color: theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              horizontalAlignment: Text.AlignRight
            }

            QQ.Text {
              text: "https://github.com/BigRangaTech/my-ereader"
              color: theme.accentAlt
              font.pixelSize: 12
              font.family: root.uiFont
              horizontalAlignment: Text.AlignRight
              wrapMode: Text.Wrap
              width: 220
              MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Qt.openUrlExternally(parent.text)
              }
            }
          }
        }

        RowLayout {
          Layout.fillWidth: true
          Layout.fillHeight: true
          spacing: 12

          Rectangle {
            Layout.preferredWidth: 260
            Layout.fillHeight: true
            radius: 12
            color: theme.panelHighlight

            ListView {
              anchors.fill: parent
              anchors.margins: 8
              clip: true
              model: aboutDialog.entries
              delegate: Rectangle {
                width: parent.width
                height: 44
                radius: 8
                color: aboutDialog.selectedPath === modelData.path
                       ? (typeof Theme !== "undefined" ? theme.accentAlt : "#7bdff2")
                       : "transparent"

                MouseArea {
                  anchors.fill: parent
                  onClicked: {
                    aboutDialog.selectedPath = modelData.path
                  }
                }

                Text {
                  anchors.verticalCenter: parent.verticalCenter
                  anchors.left: parent.left
                  anchors.leftMargin: 10
                  text: modelData.name
                  color: aboutDialog.selectedPath === modelData.path ? "#0f141a" : theme.textPrimary
                  font.pixelSize: 13
                  font.family: root.uiFont
                  elide: Text.ElideRight
                  width: parent.width - 20
                }
              }
            }
          }

          Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: theme.panelHighlight

            TextArea {
              anchors.fill: parent
              anchors.margins: 12
              readOnly: true
              wrapMode: TextArea.Wrap
              text: aboutDialog.selectedPath.length > 0
                    ? licenseManager.readFile(aboutDialog.selectedPath)
                    : "Select a license"
              color: theme.textPrimary
              font.pixelSize: 13
              font.family: root.uiFont
            }
          }
        }
      }
    }
  }
}
