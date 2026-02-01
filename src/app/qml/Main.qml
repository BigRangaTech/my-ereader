import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 6.2
import QtQuick.Layouts 1.15
import QtQuick 2.15 as QQ
import QtTextToSpeech 6.2

import Ereader 1.0

ApplicationWindow {
  id: root
  width: 1200
  height: 800
  visible: true
  title: "My Ereader"

  readonly property string uiFont: "Space Grotesk"
  readonly property string readingFont: "Literata"
  readonly property string monoFont: "JetBrains Mono"
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

  onClosing: {
    if (vault.state === VaultController.Unlocked && sessionPassphrase.length > 0) {
      vault.lock(sessionPassphrase)
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
  }

  TtsController {
    id: ttsBackend
  }

  TextToSpeech {
    id: qmlTts
    rate: settings.ttsRate
    pitch: settings.ttsPitch
    volume: settings.ttsVolume
    onStateChanged: {
      if (tts.useQml && state === TextToSpeech.Ready) {
        tts._speakNext()
      }
    }
    Component.onCompleted: {
      if (tts.useQml) {
        tts.refreshQmlVoices()
      }
    }
  }

  QtObject {
    id: tts
    readonly property bool useQml: !ttsBackend.available
    readonly property bool available: useQml ? (qmlTts.state !== TextToSpeech.Error) : ttsBackend.available
    readonly property bool speaking: useQml ? (qmlTts.state === TextToSpeech.Speaking) : ttsBackend.speaking
    property var qmlQueue: []
    property var qmlVoiceKeys: [""]
    property var qmlVoiceLabels: ["System default"]
    property var qmlVoices: [null]
    property string qmlVoiceKey: ""
    readonly property int queueLength: useQml ? qmlQueue.length : ttsBackend.queueLength
    readonly property var voiceKeys: useQml ? qmlVoiceKeys : ttsBackend.voiceKeys
    readonly property var voiceLabels: useQml ? qmlVoiceLabels : ttsBackend.voiceLabels
    readonly property string voiceKey: useQml ? qmlVoiceKey : ttsBackend.voiceKey

    function refreshQmlVoices() {
      var voices = []
      if (qmlTts.availableVoices !== undefined) {
        if (typeof qmlTts.availableVoices === "function") {
          voices = qmlTts.availableVoices()
        } else {
          voices = qmlTts.availableVoices
        }
      } else if (qmlTts.voices !== undefined) {
        if (typeof qmlTts.voices === "function") {
          voices = qmlTts.voices()
        } else {
          voices = qmlTts.voices
        }
      }
      var keys = [""]
      var labels = ["System default"]
      var voiceObjs = [null]
      for (var i = 0; i < voices.length; ++i) {
        var v = voices[i]
        var name = v.name || ""
        var localeName = ""
        if (v.locale !== undefined) {
          if (typeof v.locale === "string") {
            localeName = v.locale
          } else if (v.locale && v.locale.name) {
            localeName = v.locale.name
          } else if (v.locale && v.locale.toString) {
            localeName = v.locale.toString()
          }
        }
        var key = name + "|" + localeName
        var label = localeName.length > 0 ? (name + " (" + localeName + ")") : name
        keys.push(key)
        labels.push(label.length > 0 ? label : "Voice " + (i + 1))
        voiceObjs.push(v)
      }
      qmlVoiceKeys = keys
      qmlVoiceLabels = labels
      qmlVoices = voiceObjs
      if (settings.ttsVoiceKey && settings.ttsVoiceKey.length > 0) {
        setQmlVoiceKey(settings.ttsVoiceKey)
      }
    }

    function setQmlVoiceKey(key) {
      qmlVoiceKey = key || ""
      if (!key || key.length === 0) {
        return
      }
      for (var i = 0; i < qmlVoiceKeys.length; ++i) {
        if (qmlVoiceKeys[i] === key) {
          var voice = qmlVoices[i]
          if (voice) {
            qmlTts.voice = voice
          }
          return
        }
      }
    }

    function speak(text) {
      if (!text || text.trim().length === 0) return false
      if (useQml) {
        qmlQueue = []
        qmlTts.stop()
        return _qmlSpeak(text)
      }
      return ttsBackend.speak(text)
    }

    function enqueue(text) {
      if (!text || text.trim().length === 0) return
      if (useQml) {
        var next = qmlQueue.slice()
        next.push(text)
        qmlQueue = next
        if (qmlTts.state === TextToSpeech.Ready) {
          _speakNext()
        }
        return
      }
      ttsBackend.enqueue(text)
    }

    function stop() {
      if (useQml) {
        qmlQueue = []
        qmlTts.stop()
        return
      }
      ttsBackend.stop()
    }

    function clearQueue() {
      if (useQml) {
        qmlQueue = []
        return
      }
      ttsBackend.clearQueue()
    }

    function _speakNext() {
      if (!useQml || qmlQueue.length === 0) return
      var next = qmlQueue.slice()
      var text = next.shift()
      qmlQueue = next
      _qmlSpeak(text)
    }

    function _qmlSpeak(text) {
      if (typeof qmlTts.say === "function") {
        qmlTts.say(text)
        return true
      }
      if (typeof qmlTts.speak === "function") {
        qmlTts.speak(text)
        return true
      }
      return false
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
    ttsBackend.rate = settings.ttsRate
    ttsBackend.pitch = settings.ttsPitch
    ttsBackend.volume = settings.ttsVolume
    if (settings.ttsVoiceKey.length > 0) {
      ttsBackend.voiceKey = settings.ttsVoiceKey
    }
    if (tts.useQml) {
      tts.refreshQmlVoices()
      if (settings.ttsVoiceKey.length > 0) {
        tts.setQmlVoiceKey(settings.ttsVoiceKey)
      }
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
      if (tts.useQml) {
        tts.setQmlVoiceKey(settings.ttsVoiceKey)
      }
    }
  }

  Connections {
    target: qmlTts
    ignoreUnknownSignals: true
    function onAvailableVoicesChanged() {
      if (tts.useQml) {
        tts.refreshQmlVoices()
      }
    }
    function onVoicesChanged() {
      if (tts.useQml) {
        tts.refreshQmlVoices()
      }
    }
  }

  Connections {
    target: vault
    function onStateChanged() {
      if (vault.state === VaultController.Locked) {
        annotationModel.attachDatabase("")
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
    nameFilters: ["Books (*.epub *.pdf *.mobi *.azw *.azw3 *.fb2 *.cbz *.cbr *.djvu *.djv *.txt)"]
    onAccepted: {
      const path = localPathFromUrl(selectedFile)
      if (path.length > 0) {
        libraryModel.addBook(path)
      }
    }
  }

  FileDialog {
    id: bulkFileDialog
    title: "Add books"
    fileMode: FileDialog.OpenFiles
    nameFilters: ["Books (*.epub *.pdf *.mobi *.azw *.azw3 *.fb2 *.cbz *.cbr *.djvu *.djv *.txt)"]
    onAccepted: {
      const paths = []
      if (selectedFiles && selectedFiles.length > 0) {
        for (let i = 0; i < selectedFiles.length; i++) {
          const p = localPathFromUrl(selectedFiles[i])
          if (p.length > 0) paths.push(p)
        }
      } else {
        const single = localPathFromUrl(selectedFile)
        if (single.length > 0) paths.push(single)
      }
      if (paths.length > 0) {
        libraryModel.addBooks(paths)
      }
    }
  }

  FileDialog {
    id: folderDialog
    title: "Add folder"
    fileMode: FileDialog.OpenFile
    options: FileDialog.ShowDirsOnly
    onAccepted: {
      const folder = localPathFromUrl(selectedFile)
      if (folder.length > 0) {
        libraryModel.addFolder(folder, true)
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
      text: "Add files (bulk)"
      onTriggered: bulkFileDialog.open()
    }
    MenuItem {
      text: "Add folder"
      onTriggered: folderDialog.open()
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
    standardButtons: Dialog.Ok
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
    width: 460
    height: 260

    property string errorText: ""

    onOpened: {
      errorText = ""
      passField.text = ""
      confirmField.text = ""
    }

    onAccepted: {
      if (passField.text.length < 6) {
        errorText = "Passphrase too short"
        return
      }
      if (passField.text !== confirmField.text) {
        errorText = "Passphrases do not match"
        return
      }
      if (vault.setupNew(passField.text)) {
        sessionPassphrase = passField.text
        setupDialog.close()
      } else {
        errorText = vault.lastError
      }
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
          text: "Create a passphrase for your encrypted library."
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
        }

        TextField {
          id: passField
          echoMode: TextInput.Password
          placeholderText: "Passphrase"
        }

        TextField {
          id: confirmField
          echoMode: TextInput.Password
          placeholderText: "Confirm passphrase"
        }

        Text {
          text: setupDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }
      }
    }
  }

  Dialog {
    id: unlockDialog
    title: "Unlock Library"
    modal: true
    standardButtons: Dialog.Ok
    closePolicy: Popup.NoAutoClose
    width: 460
    height: 220

    property string errorText: ""

    onOpened: {
      errorText = ""
      unlockField.text = ""
    }

    onAccepted: {
      if (vault.unlock(unlockField.text)) {
        sessionPassphrase = unlockField.text
        unlockDialog.close()
      } else {
        errorText = vault.lastError
      }
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
          text: "Enter your passphrase."
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
        }

        TextField {
          id: unlockField
          echoMode: TextInput.Password
          placeholderText: "Passphrase"
        }

        Text {
          text: unlockDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
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
    width: 460
    height: 220

    property string errorText: ""

    onOpened: {
      errorText = ""
      lockField.text = ""
    }

    onAccepted: {
      if (vault.lock(lockField.text)) {
        sessionPassphrase = ""
        lockDialog.close()
      } else {
        errorText = vault.lastError
      }
    }

    contentItem: Rectangle {
      color: theme.panel
      radius: 12

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        Text {
          text: "Re-enter passphrase to lock."
          color: theme.textPrimary
          font.pixelSize: 14
          font.family: root.uiFont
        }

        TextField {
          id: lockField
          echoMode: TextInput.Password
          placeholderText: "Passphrase"
        }

        Text {
          text: lockDialog.errorText
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
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
          height: 72
          radius: 12
          color: theme.panelHighlight
          Layout.fillWidth: true

          RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

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
              Layout.preferredWidth: 60
              text: reader.chapterCount > 0 ? String(reader.currentChapterIndex + 1) : "1"
              placeholderText: "Page"
              inputMethodHints: Qt.ImhDigitsOnly
              validator: IntValidator { bottom: 1; top: Math.max(1, reader.chapterCount) }
              onAccepted: {
                const page = parseInt(text)
                if (!isNaN(page)) {
                  reader.goToChapter(page - 1)
                }
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
          clip: true

          TextEdit {
            id: textBlock
            width: textScroll.width
            height: contentHeight
            readOnly: true
            selectByMouse: true
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
      clip: true
      property real zoom: 1.0
      property real minZoom: settings.comicMinZoom
      property real maxZoom: settings.comicMaxZoom
      property real pinchStartZoom: 1.0
      property string fitMode: "page" // page, width, height
      property real baseScale: 1.0
      property real sourceW: 1.0
      property real sourceH: 1.0
      property real effectiveScale: baseScale * zoom
      function clampZoom(value) {
        return Math.max(minZoom, Math.min(maxZoom, value))
      }
      function recomputeBase() {
        if (imageItem.sourceSize.width <= 0 || imageItem.sourceSize.height <= 0) {
          baseScale = 1.0
          sourceW = Math.max(1, imageItem.sourceSize.width)
          sourceH = Math.max(1, imageItem.sourceSize.height)
          return
        }
        sourceW = imageItem.sourceSize.width
        sourceH = imageItem.sourceSize.height
        if (fitMode === "width") {
          baseScale = imageFlick.width / sourceW
        } else if (fitMode === "height") {
          baseScale = imageFlick.height / sourceH
        } else {
          baseScale = Math.min(imageFlick.width / sourceW, imageFlick.height / sourceH)
        }
      }
      anchors.fill: parent

      onWidthChanged: recomputeBase()
      onHeightChanged: recomputeBase()

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Rectangle {
          Layout.fillWidth: true
          Layout.preferredHeight: 72
          radius: 8
          color: theme.panelHighlight

          RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            Button {
              text: "Prev"
              onClicked: reader.prevImage()
              enabled: reader.currentImageIndex > 0
            }

            Button {
              text: "Next"
              onClicked: reader.nextImage()
              enabled: reader.currentImageIndex + 1 < reader.imageCount
            }

            Button {
              text: "Fit Page"
              onClicked: {
                fitMode = "page"
                zoom = 1.0
                recomputeBase()
              }
            }

            Button {
              text: "Fit Width"
              onClicked: {
                fitMode = "width"
                zoom = 1.0
                recomputeBase()
              }
            }

            Button {
              text: "Fit Height"
              onClicked: {
                fitMode = "height"
                zoom = 1.0
                recomputeBase()
              }
            }

            Button {
              text: "-"
              onClicked: zoom = clampZoom(zoom - 0.1)
            }

            Button {
              text: "+"
              onClicked: zoom = clampZoom(zoom + 0.1)
            }

            Text {
              text: qsTr("%1%").arg(Math.round(zoom * 100))
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }

            TextField {
              Layout.preferredWidth: 60
              text: reader.imageCount > 0 ? String(reader.currentImageIndex + 1) : ""
              placeholderText: "Page"
              inputMethodHints: Qt.ImhDigitsOnly
              validator: IntValidator { bottom: 1; top: Math.max(1, reader.imageCount) }
              onAccepted: {
                const page = parseInt(text)
                if (!isNaN(page)) {
                  reader.goToImage(page - 1)
                }
              }
            }

            Slider {
              Layout.preferredWidth: 180
              from: 1
              to: Math.max(1, reader.imageCount)
              stepSize: 1
              value: reader.imageCount > 0 ? reader.currentImageIndex + 1 : 1
              onMoved: reader.goToImage(Math.max(0, Math.round(value - 1)))
            }

            Text {
              text: reader.imageCount > 0
                    ? qsTr("%1 / %2").arg(reader.currentImageIndex + 1).arg(reader.imageCount)
                    : ""
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }
          }
        }

        Flickable {
          id: imageFlick
          Layout.fillWidth: true
          Layout.fillHeight: true
          contentWidth: Math.max(imageItem.width, width)
          contentHeight: Math.max(imageItem.height, height)
          clip: true

          WheelHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: function(wheel) {
              const step = wheel.angleDelta.y > 0 ? 0.1 : -0.1
              zoom = clampZoom(zoom + step)
            }
          }

          PinchArea {
            anchors.fill: parent
            onPinchStarted: pinchStartZoom = zoom
            onPinchUpdated: zoom = clampZoom(pinchStartZoom * pinch.scale)
          }

          TapHandler {
            acceptedButtons: Qt.LeftButton
            onDoubleTapped: {
              if (fitMode === "width") {
                fitMode = "page"
              } else {
                fitMode = "width"
              }
              zoom = 1.0
              recomputeBase()
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

          Image {
            id: imageItem
            source: reader.currentImageUrl.toString().length > 0
                    ? (reader.currentImageUrl.toString() + "?t=" + reader.imageReloadToken)
                    : ""
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            cache: false
            width: sourceW * effectiveScale
            height: sourceH * effectiveScale
            x: Math.max(0, (imageFlick.width - width) / 2)
            y: Math.max(0, (imageFlick.height - height) / 2)
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
              x: imageItem.x + modelData.x * imageItem.width - width / 2
              y: imageItem.y + modelData.y * imageItem.height - height / 2
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

          ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
          ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
        }
      }
    }
  }

  StackView {
    id: stack
    anchors.fill: parent
    initialItem: libraryPage
    pushEnter: Transition {
      NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 220 }
    }
    popExit: Transition {
      NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 160 }
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

        var collectionSet = {}
        var tagSet = {}
        for (var i = 0; i < libraryModel.count; ++i) {
          const item = libraryModel.get(i)
          if (!item) continue
          if (item.collection && item.collection.length > 0) {
            collectionSet[item.collection] = true
          }
          if (item.tags && item.tags.length > 0) {
            const parts = item.tags.split(",")
            for (var p = 0; p < parts.length; ++p) {
              const tag = parts[p].trim()
              if (tag.length > 0) {
                tagSet[tag] = true
              }
            }
          }
        }
        const collections = Object.keys(collectionSet).sort()
        for (var c = 0; c < collections.length; ++c) {
          collectionFilterModel.append({ label: collections[c], value: collections[c] })
        }
        const tags = Object.keys(tagSet).sort()
        for (var t = 0; t < tags.length; ++t) {
          tagFilterModel.append({ label: tags[t], value: tags[t] })
        }
      }

      Connections {
        target: libraryModel
        function onCountChanged() { rebuildFilterOptions() }
        function onReadyChanged() { rebuildFilterOptions() }
      }

      Component.onCompleted: {
        rebuildFilterOptions()
      }

      Column {
        anchors.fill: parent
        spacing: 16
        padding: 24

        Rectangle {
          id: header
          height: 72
          radius: 16
          color: theme.panel
          width: parent.width

          RowLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 16

            Text {
              text: "Library"
              color: theme.textPrimary
              font.pixelSize: 28
              font.family: root.uiFont
            }

            Rectangle {
              width: 1
              height: parent.height - 16
              color: theme.panelHighlight
            }

            Text {
              text: qsTr("%1 books").arg(libraryModel.count)
              color: theme.textMuted
              font.pixelSize: 16
              font.family: root.uiFont
              verticalAlignment: Text.AlignVCenter
            }

            RowLayout {
              spacing: 6
              Layout.preferredWidth: 260

              TextField {
                id: librarySearch
                Layout.fillWidth: true
                placeholderText: "Search library"
                text: libraryPage.pendingSearch
                onTextChanged: {
                  libraryPage.pendingSearch = text
                  searchDebounce.restart()
                }
              }

              Button {
                text: "✕"
                font.family: root.uiFont
                enabled: libraryPage.pendingSearch.length > 0 || libraryModel.searchQuery.length > 0
                onClicked: {
                  libraryPage.pendingSearch = ""
                  librarySearch.text = ""
                  libraryModel.searchQuery = ""
                }
              }
            }

            ComboBox {
              id: collectionFilter
              Layout.preferredWidth: 180
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

            ComboBox {
              id: tagFilter
              Layout.preferredWidth: 160
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

            ComboBox {
              id: sortCombo
              Layout.preferredWidth: 150
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

            Button {
              text: "Reset"
              font.family: root.uiFont
              enabled: hasActiveFilters() || libraryModel.sortKey !== "title" || libraryModel.sortDescending
              onClicked: clearFilters()
            }

            Item { Layout.fillWidth: true }

            Button {
              text: viewMode === "list" ? "Grid" : "List"
              font.family: root.uiFont
              onClicked: viewMode = viewMode === "list" ? "grid" : "list"
            }

            Button {
              text: selectionMode ? "Cancel" : "Select"
              font.family: root.uiFont
              onClicked: {
                selectionMode = !selectionMode
                if (!selectionMode) {
                  clearSelection()
                }
              }
            }

            Button {
              text: "Select all"
              font.family: root.uiFont
              visible: selectionMode
              onClicked: selectAllVisible()
            }

            Button {
              text: "Invert"
              font.family: root.uiFont
              visible: selectionMode
              enabled: libraryModel.count > 0
              onClicked: invertSelection()
            }

            Button {
              text: "Clear"
              font.family: root.uiFont
              visible: selectionMode
              onClicked: clearSelection()
            }

            Button {
              text: qsTr("Edit tags (%1)").arg(selectedIds.length)
              font.family: root.uiFont
              visible: selectionMode
              enabled: selectedIds.length > 0
              onClicked: bulkTagsDialog.openForIds(selectedIds)
            }

            Button {
              text: qsTr("Remove (%1)").arg(selectedIds.length)
              font.family: root.uiFont
              visible: selectionMode
              enabled: selectedIds.length > 0
              onClicked: deleteBooksDialog.openForIds(selectedIds)
            }

            Button {
              text: "Add"
              onClicked: addMenu.open()
              font.family: root.uiFont
              enabled: libraryModel.ready
            }

            Button {
              text: "Settings"
              font.family: root.uiFont
              onClicked: settingsDialog.open()
            }

            Button {
              text: "About"
              font.family: root.uiFont
              onClicked: aboutDialog.open()
            }

            Button {
              text: "Lock"
              font.family: root.uiFont
              visible: vault.state === VaultController.Unlocked
              onClicked: {
                if (sessionPassphrase.length > 0) {
                  vault.lock(sessionPassphrase)
                  sessionPassphrase = ""
                } else {
                  lockDialog.open()
                }
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
          }
        }

        Rectangle {
          id: filterBar
          radius: 14
          color: theme.panelHighlight
          width: parent.width
          height: 54

          RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 14

            Text {
              text: "Collections"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }

            Flow {
              Layout.fillWidth: true
              spacing: 6
              Repeater {
                model: collectionFilterModel
                delegate: Rectangle {
                  visible: index < 6
                  radius: 10
                  color: model.value === libraryModel.filterCollection ? theme.accent : theme.panel
                  border.width: 1
                  border.color: theme.panelHighlight
                  height: 26
                  width: label.implicitWidth + 18

                  Text {
                    id: label
                    anchors.centerIn: parent
                    text: model.label
                    color: model.value === libraryModel.filterCollection ? "#0f141a" : theme.textPrimary
                    font.pixelSize: 11
                    font.family: root.uiFont
                  }

                  MouseArea {
                    anchors.fill: parent
                    onClicked: libraryModel.filterCollection = model.value
                  }
                }
              }
            }

            Text {
              text: "Tags"
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }

            Flow {
              Layout.fillWidth: true
              spacing: 6
              Repeater {
                model: tagFilterModel
                delegate: Rectangle {
                  visible: index < 6
                  radius: 10
                  color: model.value === libraryModel.filterTag ? theme.accent : theme.panel
                  border.width: 1
                  border.color: theme.panelHighlight
                  height: 26
                  width: tagLabel.implicitWidth + 18

                  Text {
                    id: tagLabel
                    anchors.centerIn: parent
                    text: model.label
                    color: model.value === libraryModel.filterTag ? "#0f141a" : theme.textPrimary
                    font.pixelSize: 11
                    font.family: root.uiFont
                  }

                  MouseArea {
                    anchors.fill: parent
                    onClicked: libraryModel.filterTag = model.value
                  }
                }
              }
            }
          }
        }

        Rectangle {
          id: libraryPanel
          radius: 18
          color: theme.panel
          anchors.horizontalCenter: parent.horizontalCenter
          width: parent.width
          height: parent.height - header.height - 96

          Item {
            anchors.fill: parent

            ListView {
              id: listView
              anchors.fill: parent
              anchors.margins: 12
              model: libraryModel
              clip: true
              spacing: 8
              visible: viewMode === "list"

              delegate: Rectangle {
                radius: 12
                height: 106
                width: listView.width
                color: index % 2 === 0 ? theme.panelHighlight : theme.panel
                border.width: isSelected(model.id) ? 2 : 0
                border.color: theme.accent

                RowLayout {
                  anchors.fill: parent
                  anchors.margins: 14
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
                        stack.push(readerPage)
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
                          font.pixelSize: 18
                          font.family: root.uiFont
                          elide: Text.ElideRight
                          width: libraryContent.width - 180
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
                          width: libraryContent.width - 180
                          visible: text.length > 0
                        }

                        Text {
                          text: (model.description && model.description.length > 0) ? model.description : model.path
                          color: theme.textMuted
                          font.pixelSize: 12
                          font.family: root.uiFont
                          elide: Text.ElideRight
                          width: libraryContent.width - 180
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

                  ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 6

                    Button {
                      text: "Edit"
                      font.family: root.uiFont
                      onClicked: editBookDialog.openForBook({
                        id: model.id,
                        title: model.title,
                        authors: model.authors,
                        series: model.series,
                        publisher: model.publisher,
                        collection: model.collection,
                        tags: model.tags,
                        description: model.description,
                        path: model.path
                      })
                    }

                    Button {
                      text: "Remove"
                      font.family: root.uiFont
                      onClicked: deleteBookDialog.openForBook({
                        id: model.id,
                        title: model.title,
                        path: model.path
                      })
                    }
                  }
                }
              }
            }

            GridView {
              id: gridView
              anchors.fill: parent
              anchors.margins: 12
              model: libraryModel
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
                    stack.push(readerPage)
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
        }

        Text {
          text: libraryModel.lastError
          color: theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }
      }
    }
  }

  Component {
    id: readerPage

    Item {
      property string sidebarMode: "toc"
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

      Column {
        anchors.fill: parent
        spacing: 16
        padding: 24

        Rectangle {
          height: 72
          radius: 16
          color: theme.panel
          width: parent.width
          z: 2

          RowLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 16

            Button {
              text: "Back"
              font.family: root.uiFont
              onClicked: {
                reader.close()
                stack.pop()
              }
            }

            Image {
              source: reader.currentCoverUrl.toString().length > 0
                      ? reader.currentCoverUrl
                      : root.fileUrl(settings.iconPath)
              visible: source.toString().length > 0
              fillMode: Image.PreserveAspectFit
              width: 40
              height: 56
              Layout.preferredWidth: 40
              Layout.preferredHeight: 56
              Layout.maximumWidth: 40
              Layout.maximumHeight: 56
              cache: false
            }

            Text {
              text: reader.currentTitle
              color: theme.textPrimary
              font.pixelSize: 20
              font.family: root.uiFont
              elide: Text.ElideRight
              Layout.fillWidth: true
            }

            Item { width: 10; height: 10 }

            Button {
              text: tts.speaking ? "Stop" : "Speak"
              font.family: root.uiFont
              enabled: tts.available && reader.ttsAllowed
              onClicked: {
                if (tts.speaking) {
                  tts.stop()
                } else {
                  tts.speak(reader.currentPlainText)
                }
              }
            }

            Button {
              text: "Queue"
              font.family: root.uiFont
              enabled: tts.available && reader.ttsAllowed
              onClicked: tts.enqueue(reader.currentPlainText)
            }

            Text {
              text: tts.queueLength > 0 ? qsTr("Queue %1").arg(tts.queueLength) : ""
              color: theme.textMuted
              font.pixelSize: 12
              font.family: root.uiFont
            }

            Button {
              text: "Annotate"
              font.family: root.uiFont
              onClicked: annotationDialog.open()
            }

            Button {
              text: "Settings"
              font.family: root.uiFont
              onClicked: settingsDialog.open()
            }

            Button {
              text: "Lock"
              font.family: root.uiFont
              visible: vault.state === VaultController.Unlocked
              onClicked: {
                if (sessionPassphrase.length > 0) {
                  vault.lock(sessionPassphrase)
                  sessionPassphrase = ""
                  reader.close()
                  stack.pop()
                } else {
                  lockDialog.open()
                }
              }
            }
          }
        }

        Rectangle {
          radius: 18
          color: theme.panel
          width: parent.width
          height: parent.height - 120
          clip: true
          z: 1

          RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            Rectangle {
              Layout.preferredWidth: 280
              Layout.fillHeight: true
              radius: 12
              color: theme.panelHighlight

              ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                ButtonGroup {
                  id: sidebarGroup
                  exclusive: true
                  onCheckedButtonChanged: {
                    if (checkedButton === tocButton) {
                      sidebarMode = "toc"
                    } else if (checkedButton === annotationsButton) {
                      sidebarMode = "annotations"
                    }
                  }
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 8

                  Button {
                    id: tocButton
                    text: "TOC"
                    checkable: true
                    ButtonGroup.group: sidebarGroup
                    checked: sidebarMode === "toc"
                  }

                  Button {
                    id: annotationsButton
                    text: "Annotations"
                    checkable: true
                    ButtonGroup.group: sidebarGroup
                    checked: sidebarMode === "annotations"
                  }
                }

                StackLayout {
                  Layout.fillWidth: true
                  Layout.fillHeight: true
                  currentIndex: sidebarMode === "toc" ? 0 : 1

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
                        height: 56
                        radius: 8
                        color: reader.tocChapterIndex(index) === reader.currentChapterIndex
                               ? theme.accentAlt
                               : (index % 2 === 0 ? theme.panel : theme.panelHighlight)

                        Row {
                          anchors.fill: parent
                          anchors.margins: 8
                          spacing: 8

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

                      RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Button {
                          text: "Export"
                          font.family: root.uiFont
                          onClicked: exportAnnotationsDialog.open()
                        }

                        Item { Layout.fillWidth: true }
                      }

                      Rectangle {
                        Layout.fillWidth: true
                        radius: 8
                        color: theme.panel
                        border.color: "#2b3446"
                        border.width: 1

                        Column {
                          anchors.fill: parent
                          anchors.margins: 8
                          spacing: 6

                          Text {
                            text: "Pin legend"
                            color: theme.textPrimary
                            font.pixelSize: 12
                            font.family: root.uiFont
                          }

                          Flow {
                            width: parent.width
                            spacing: 12

                            Repeater {
                              model: ["note", "highlight", "bookmark"]
                              delegate: Row {
                                spacing: 6
                                readonly property var pinStyle: root.pinStyleForType(modelData)

                                Rectangle {
                                  width: 10
                                  height: 10
                                  radius: pinStyle.radius
                                  rotation: pinStyle.rotation
                                  transformOrigin: Item.Center
                                  color: theme.accent
                                }

                                Text {
                                  text: root.pinLabelForType(modelData)
                                  color: theme.textMuted
                                  font.pixelSize: 11
                                  font.family: root.uiFont
                                }
                              }
                            }
                          }

                          Text {
                            text: "Pins appear on image/PDF pages. Shape = type, color = annotation color."
                            color: theme.textMuted
                            font.pixelSize: 10
                            font.family: root.uiFont
                            wrapMode: Text.WordWrap
                          }
                        }
                      }

                      ListView {
                        id: annotationList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: filteredAnnotations
                        delegate: Rectangle {
                          width: parent.width
                          height: 110
                          radius: 8
                          color: index % 2 === 0 ? theme.panel : theme.panelHighlight

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
                            onClicked: reader.jumpToLocator(model.locator)
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

                          Row {
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.margins: 6
                            spacing: 6

                            Button {
                              text: "Edit"
                              font.family: root.uiFont
                              onClicked: annotationDialog.openForEdit({
                                id: model.id,
                                locator: model.locator,
                                type: model.type,
                                text: model.text,
                                color: model.color
                              })
                            }

                            Button {
                              text: "Delete"
                              font.family: root.uiFont
                              onClicked: deleteAnnotationDialog.openForAnnotation({
                                id: model.id,
                                locator: model.locator,
                                text: model.text
                              })
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }

            Loader {
              id: readerContent
              Layout.fillWidth: true
              Layout.fillHeight: true
              active: true
              clip: true
              sourceComponent: reader.hasImages ? imageReader : textReader
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
    id: settingsDialog
    title: "Settings"
    modal: true
    standardButtons: Dialog.Close
    width: Math.min(860, root.width - 80)
    height: Math.min(720, root.height - 80)

    contentItem: Rectangle {
      color: theme.panel
      radius: 16

      ScrollView {
        anchors.fill: parent
        anchors.margins: 12

        ColumnLayout {
          width: parent.width
          spacing: 18

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

          Text {
            text: "EPUB"
            color: theme.textPrimary
            font.pixelSize: 20
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

          Text {
            text: "FB2"
            color: theme.textPrimary
            font.pixelSize: 20
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

          Text {
            text: "TXT"
            color: theme.textPrimary
            font.pixelSize: 20
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

          Text {
            text: "MOBI/AZW/PRC"
            color: theme.textPrimary
            font.pixelSize: 20
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

          Text {
            text: "PDF"
            color: theme.textPrimary
            font.pixelSize: 20
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

          Text {
            text: "Comics"
            color: theme.textPrimary
            font.pixelSize: 20
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

          Text {
            text: "DJVU"
            color: theme.textPrimary
            font.pixelSize: 20
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
              text: "Prefetch"
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

          Rectangle { height: 1; color: theme.panelHighlight; Layout.fillWidth: true }

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
              text: "Version 0.1.0"
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
