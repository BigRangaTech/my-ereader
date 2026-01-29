import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 6.2
import QtQuick.Layouts 1.15
import QtQuick 2.15 as QQ

import Ereader 1.0

ApplicationWindow {
  id: root
  width: 1200
  height: 800
  visible: true
  title: "My Ereader"

  readonly property string uiFont: "Space Grotesk"
  readonly property string readingFont: "Literata"

  function textFontSizeFor(format) {
    const f = (format || "").toLowerCase()
    if (f === "epub") return settings.epubFontSize
    if (f === "fb2") return settings.fb2FontSize
    if (f === "txt") return settings.txtFontSize
    if (f === "mobi" || f === "azw" || f === "azw3" || f === "azw4" || f === "prc") return settings.mobiFontSize
    return settings.readingFontSize
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

  LibraryModel {
    id: libraryModel
  }

  AnnotationModel {
    id: annotationModel
  }

  ReaderController {
    id: reader
  }

  SettingsManager {
    id: settings
  }

  SyncManager {
    id: syncManager
  }

  TtsController {
    id: tts
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
    nameFilters: ["Books (*.epub *.pdf *.mobi *.azw *.azw3 *.fb2 *.cbz *.cbr *.txt)"]
    onAccepted: {
      var path = ""
      if (selectedFile && selectedFile.toLocalFile) {
        path = selectedFile.toLocalFile()
      } else if (selectedFile) {
        path = selectedFile.toString().replace("file://", "")
        path = decodeURIComponent(path)
      }
      if (path.length > 0) {
        const ext = path.split(".").pop().toLowerCase()
        if (ext === "mobi" || ext === "azw" || ext === "azw3" || ext === "azw4" || ext === "prc") {
          formatWarningDialog.messageText =
              "MOBI/AZW support is experimental and temporarily disabled.\n" +
              "Update libmobi to re-enable this format."
          formatWarningDialog.open()
        } else {
          libraryModel.addBook(path)
        }
      }
    }
  }

  Dialog {
    id: annotationDialog
    title: "Add Annotation"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 460
    height: 260

    property string errorText: ""
    property string locatorText: ""
    property string noteText: ""

    onOpened: {
      errorText = ""
      locatorField.text = ""
      noteField.text = ""
      locatorText = ""
      noteText = ""
    }

    onAccepted: {
      if (locatorField.text.trim().length === 0) {
        errorText = "Locator required (page/chapter)"
        return
      }
      if (!annotationModel.addAnnotation(locatorField.text, "note", noteField.text, "#ffb347")) {
        errorText = annotationModel.lastError
        return
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

        TextField {
          id: locatorField
          placeholderText: "Locator (page/chapter id)"
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

          Text {
            id: textBlock
            width: textScroll.width
            text: reader.currentText
            color: theme.textPrimary
            font.pixelSize: root.textFontSizeFor(reader.currentFormat)
            font.family: root.readingFont
            wrapMode: Text.WordWrap
            lineHeight: root.textLineHeightFor(reader.currentFormat)
            lineHeightMode: Text.ProportionalHeight
            textFormat: (reader.currentFormat === "mobi"
                         || reader.currentFormat === "azw"
                         || reader.currentFormat === "azw3"
                         || reader.currentFormat === "azw4"
                         || reader.currentFormat === "prc")
                        ? Text.PlainText
                        : (reader.currentTextIsRich ? Text.RichText : Text.PlainText)
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

            Item { Layout.fillWidth: true }

            Button {
              text: "Add"
              onClicked: fileDialog.open()
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
          id: libraryPanel
          radius: 18
          color: theme.panel
          anchors.horizontalCenter: parent.horizontalCenter
          width: parent.width
          height: parent.height - header.height - 96

          ListView {
            id: listView
            anchors.fill: parent
            anchors.margins: 12
            model: libraryModel
            clip: true
            spacing: 8

            delegate: Rectangle {
              radius: 12
              height: 106
              width: listView.width
              color: index % 2 === 0 ? theme.panelHighlight : theme.panel

              MouseArea {
                anchors.fill: parent
                onClicked: {
                  reader.close()
                  const ext = model.path.split(".").pop().toLowerCase()
                  if (ext === "mobi" || ext === "azw" || ext === "azw3" || ext === "azw4" || ext === "prc") {
                    formatWarningDialog.messageText =
                        "MOBI/AZW support is experimental and temporarily disabled.\n" +
                        "Update libmobi to re-enable this format."
                    formatWarningDialog.open()
                  } else {
                    reader.openFileAsync(model.path)
                    annotationModel.libraryItemId = model.id
                    stack.push(readerPage)
                  }
                }
              }

              Row {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 16

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
                    width: listView.width - 180
                  }

                  Text {
                    text: {
                      var parts = [];
                      if (model.authors && model.authors.length > 0) parts.push(model.authors);
                      if (model.series && model.series.length > 0) parts.push(model.series);
                      if (model.publisher && model.publisher.length > 0) parts.push(model.publisher);
                      return parts.length > 0 ? parts.join(" • ") : ""
                    }
                    color: theme.textMuted
                    font.pixelSize: 12
                    font.family: root.uiFont
                    elide: Text.ElideRight
                    width: listView.width - 180
                    visible: text.length > 0
                  }

                  Text {
                    text: (model.description && model.description.length > 0) ? model.description : model.path
                    color: theme.textMuted
                    font.pixelSize: 12
                    font.family: root.uiFont
                    elide: Text.ElideRight
                    width: listView.width - 180
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
              source: reader.currentCoverUrl
              visible: reader.currentCoverUrl.toString().length > 0
              fillMode: Image.PreserveAspectFit
              width: 40
              height: 56
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
              enabled: tts.available
              onClicked: {
                if (tts.speaking) {
                  tts.stop()
                } else {
                  tts.speak(reader.currentPlainText)
                }
              }
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

                    ListView {
                      id: annotationList
                      anchors.fill: parent
                      clip: true
                      model: annotationModel
                      delegate: Rectangle {
                        width: parent.width
                        height: 70
                        radius: 8
                        color: index % 2 === 0 ? theme.panel : theme.panelHighlight

                        Column {
                          anchors.fill: parent
                          anchors.margins: 8
                          spacing: 4

                          Text {
                            text: model.locator + " • " + model.type
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
                        }

                        MouseArea {
                          anchors.fill: parent
                          onClicked: reader.jumpToLocator(model.locator)
                          onPressAndHold: annotationModel.deleteAnnotation(model.id)
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

          Text {
            text: settings.formatSettingsPath("epub")
            color: theme.textMuted
            font.pixelSize: 11
            font.family: root.uiFont
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

          Text {
            text: settings.formatSettingsPath("fb2")
            color: theme.textMuted
            font.pixelSize: 11
            font.family: root.uiFont
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

          Text {
            text: settings.formatSettingsPath("txt")
            color: theme.textMuted
            font.pixelSize: 11
            font.family: root.uiFont
          }

          Rectangle { height: 1; color: theme.panelHighlight; Layout.fillWidth: true }

          Text {
            text: "MOBI (experimental)"
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

          Text {
            text: settings.formatSettingsPath("mobi")
            color: theme.textMuted
            font.pixelSize: 11
            font.family: root.uiFont
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
              value: settings.pdfDpi
              onMoved: settings.pdfDpi = Math.round(value)
            }

            SpinBox {
              from: 72
              to: 240
              value: settings.pdfDpi
              editable: true
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

          Text {
            text: settings.formatSettingsPath("djvu")
            color: theme.textMuted
            font.pixelSize: 11
            font.family: root.uiFont
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
