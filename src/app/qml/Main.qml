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
    onStateChanged: {
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
    onReadyChanged: {
      if (libraryModel.ready) {
        annotationModel.attachConnection(libraryModel.connectionName())
      }
    }
  }

  FileDialog {
    id: fileDialog
    title: "Add book"
    nameFilters: ["Books (*.epub *.pdf *.mobi *.azw *.azw3 *.fb2 *.txt)"]
    onAccepted: {
      const path = selectedFile.toString().replace("file://", "")
      if (path.length > 0) {
        libraryModel.addBook(path)
      }
    }
  }

  Dialog {
    id: annotationDialog
    title: "Add Annotation"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

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
      width: 420
      height: 240

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
    id: setupDialog
    title: "Set Passphrase"
    modal: true
    standardButtons: Dialog.Ok
    closePolicy: Popup.NoAutoClose

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
      width: 420
      height: 220

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
      width: 420
      height: 180

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
      width: 420
      height: 180

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
      width: parent.width
      height: parent.height

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
              height: 84
              width: listView.width
              color: index % 2 === 0 ? theme.panelHighlight : theme.panel

              MouseArea {
                anchors.fill: parent
                onClicked: {
                if (reader.openFile(model.path)) {
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
                  spacing: 6
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
                    text: model.path
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
      width: parent.width
      height: parent.height

      Column {
        anchors.fill: parent
        spacing: 16
        padding: 24

        Rectangle {
          height: 72
          radius: 16
          color: theme.panel
          width: parent.width

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
                  tts.speak(reader.currentText)
                }
              }
            }

            Button {
              text: "Annotate"
              font.family: root.uiFont
              onClicked: annotationDialog.open()
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

                Text {
                  text: "Annotations"
                  color: theme.textPrimary
                  font.pixelSize: 16
                  font.family: root.uiFont
                }

                ListView {
                  id: annotationList
                  Layout.fillWidth: true
                  Layout.fillHeight: true
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
                      onPressAndHold: annotationModel.deleteAnnotation(model.id)
                    }
                  }
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
              font.pixelSize: 20
              font.family: root.readingFont
              wrapMode: Text.WordWrap
              lineHeight: 1.4
              lineHeightMode: Text.ProportionalHeight
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
