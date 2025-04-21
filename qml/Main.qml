import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: mainWindow

    width: 1200
    height: 800
    minimumWidth: 480
    minimumHeight: 220

    visible: true
    title: qsTr("Гистограмма частоты повторения слов")

    ColumnLayout {
        anchors {
            fill: parent
            topMargin: 10
            bottomMargin: 10
            leftMargin: 10
            rightMargin: 10
        }

        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            uniformCellSizes: true

            spacing: 10

            Button {
                Layout.fillWidth: true

                text: qsTr("Открыть")
                enabled: !controller?.running

                onClicked: {
                    fileDialog.open()
                }
            }

            Button {
                Layout.fillWidth: true

                text: qsTr("Старт")
                enabled: !controller?.running && controller?.fileUrl !== null

                onClicked: {
                    controller.startProcessing()
                }
            }

            Button {
                Layout.fillWidth: true

                text: controller?.paused ? qsTr("Возобновить") : qsTr("Пауза")
                enabled: Boolean(controller?.running)

                onClicked: {
                    controller.paused ? controller.resume() : controller.pause()
                }
            }

            Button {
                Layout.fillWidth: true

                text: qsTr("Отмена")
                enabled: controller?.running || wordModel?.count > 0

                onClicked: {
                    controller.cancel()
                }
            }
        }

        ProgressBar {
            Layout.fillWidth: true

            from: 0
            to: 100
            value: Number(controller?.progress)
        }

        Rectangle {
            id: histogramArea

            readonly property real slotWidth: width / Math.max(wordModel?.count ?? 0, 1)
            readonly property int labelAreaHeight: 20
            readonly property int textAdvance: 4

            Layout.fillWidth: true
            Layout.fillHeight: true

            color: "#f0f0f0"
            radius: 4

            border.color: "#cccccc"
            border.width: 1

            Repeater {
                model: wordModel

                delegate: Item {
                    width: histogramArea.slotWidth * 0.8
                    height: histogramArea.height
                    x: histogramArea.slotWidth * index + (histogramArea.slotWidth - width) / 2
                    anchors.bottom: histogramArea.bottom

                    Rectangle {
                        id: bar

                        width: parent.width
                        height: (wordModel.maxCount > 0 ? count / wordModel.maxCount : 0) * (histogramArea.height - 2*histogramArea.labelAreaHeight - 2*histogramArea.textAdvance)

                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            bottom: parent.bottom
                            topMargin: histogramArea.labelAreaHeight + histogramArea.textAdvance
                            bottomMargin: histogramArea.labelAreaHeight + histogramArea.textAdvance
                        }

                        color: "#4682B4"
                        radius: 2
                    }

                    ElidedText {
                        width: parent.width + histogramArea.textAdvance

                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            bottom: bar.top
                            bottomMargin: 2
                        }

                        text: count
                    }

                    ElidedText {
                        width: parent.width + histogramArea.textAdvance

                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            top: bar.bottom
                            topMargin: 2
                        }

                        text: word
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: (wordModel?.count === 0 && !controller?.running)
                      ? qsTr("Нет данных для отображения")
                      : ""
                color: "#808080"
                font.pixelSize: 16
            }
        }

        FileDialog {
            id: fileDialog
            title: qsTr("Select Text File")
            nameFilters: [ "Text Files (*.txt)" ]
            onAccepted: {
                controller.fileUrl = selectedFile
            }
        }
    }

    component ElidedText: Text {
        property alias hovered: hover.hovered

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight

        font.pixelSize: 14

        ToolTip.visible: truncated && hovered
        ToolTip.text: text

        HoverHandler {
            id: hover
        }
    }
}
