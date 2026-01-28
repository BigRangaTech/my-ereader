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

  LibraryModel {
    id: libraryModel
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

  Component.onCompleted: {
    libraryModel.openDefault()
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

  background: Rectangle {
    gradient: Gradient {
      GradientStop { position: 0.0; color: Theme.bgTop }
      GradientStop { position: 1.0; color: Theme.bgBottom }
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
      anchors.fill: parent

      Column {
        anchors.fill: parent
        spacing: 16
        padding: 24

        Rectangle {
          id: header
          height: 72
          radius: 16
          color: Theme.panel
          width: parent.width

          RowLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 16

            Text {
              text: "Library"
              color: Theme.textPrimary
              font.pixelSize: 28
              font.family: root.uiFont
            }

            Rectangle {
              width: 1
              height: parent.height - 16
              color: Theme.panelHighlight
            }

            Text {
              text: qsTr("%1 books").arg(libraryModel.count)
              color: Theme.textMuted
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
          }
        }

        Rectangle {
          id: libraryPanel
          radius: 18
          color: Theme.panel
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
              color: index % 2 === 0 ? Theme.panelHighlight : Theme.panel

              MouseArea {
                anchors.fill: parent
                onClicked: {
                  if (reader.openFile(model.path)) {
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
                  color: Theme.accentAlt

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
                    color: Theme.textPrimary
                    font.pixelSize: 18
                    font.family: root.uiFont
                    elide: Text.ElideRight
                    width: listView.width - 180
                  }

                  Text {
                    text: model.path
                    color: Theme.textMuted
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
          color: Theme.accent
          font.pixelSize: 12
          font.family: root.uiFont
        }
      }
    }
  }

  Component {
    id: readerPage

    Item {
      anchors.fill: parent

      Column {
        anchors.fill: parent
        spacing: 16
        padding: 24

        Rectangle {
          height: 72
          radius: 16
          color: Theme.panel
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
              color: Theme.textPrimary
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
          }
        }

        Rectangle {
          radius: 18
          color: Theme.panel
          width: parent.width
          height: parent.height - 120

          Flickable {
            id: textScroll
            anchors.fill: parent
            anchors.margins: 20
            contentWidth: textBlock.width
            contentHeight: textBlock.height
            clip: true

            Text {
              id: textBlock
              width: textScroll.width
              text: reader.currentText
              color: Theme.textPrimary
              font.pixelSize: 20
              font.family: root.readingFont
              wrapMode: Text.WordWrap
              lineHeight: 1.4
              lineHeightMode: Text.ProportionalHeight
            }
          }
        }

        Text {
          text: reader.lastError
          color: Theme.accent
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
      color: Theme.panel
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
              color: Theme.textPrimary
              font.pixelSize: 22
              font.family: root.uiFont
            }

            Text {
              text: "Bundled third-party licenses"
              color: Theme.textMuted
              font.pixelSize: 14
              font.family: root.uiFont
            }
          }

          ColumnLayout {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            Text {
              text: "Version 0.1.0"
              color: Theme.textMuted
              font.pixelSize: 13
              font.family: root.uiFont
              horizontalAlignment: Text.AlignRight
            }

            QQ.Text {
              text: "https://github.com/BigRangaTech/my-ereader"
              color: Theme.accentAlt
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
            color: Theme.panelHighlight

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
                       ? (typeof Theme !== "undefined" ? Theme.accentAlt : "#7bdff2")
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
                  color: aboutDialog.selectedPath === modelData.path ? "#0f141a" : Theme.textPrimary
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
            color: Theme.panelHighlight

            TextArea {
              anchors.fill: parent
              anchors.margins: 12
              readOnly: true
              wrapMode: TextArea.Wrap
              text: aboutDialog.selectedPath.length > 0
                    ? licenseManager.readFile(aboutDialog.selectedPath)
                    : "Select a license"
              color: Theme.textPrimary
              font.pixelSize: 13
              font.family: root.uiFont
            }
          }
        }
      }
    }
  }
}
