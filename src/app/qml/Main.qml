import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15

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

  Component.onCompleted: {
    libraryModel.openDefault()
  }

  FileDialog {
    id: fileDialog
    title: "Add book"
    nameFilters: ["Books (*.epub *.pdf *.mobi *.azw *.azw3 *.fb2 *.txt)"]
    onAccepted: {
      const path = selectedFile.toLocalFile()
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
}
