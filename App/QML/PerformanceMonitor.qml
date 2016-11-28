import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import ARCameraQml 1.0
import ARCameraQml_models 1.0

Item {

    property alias arTracker: performanceModel.arTracker
    property alias countUsedTimes: performanceModel.countUsedTimes
    property bool expand: false

    anchors.fill: parent

    ARPerformanceMonitorModel {
        id: performanceModel
    }

    Timer {
        interval: 300
        running: true
        repeat: true
        onTriggered: performanceModel.updateModel()
    }

    Item {
        x: 0
        y: parent.height - height

        width: parent.width * 0.25
        height: parent.height * 0.1

        clip: true

        Rectangle {
            id: listViewShortBackground
            anchors.fill: parent

            border.width: listViewShortMouseArea.containsMouse ? 2 : 1
            border.color: "#888"
            opacity: 0.6
            radius: 5
            gradient: Gradient {
                GradientStop { position: 0; color: listViewShortMouseArea.pressed ? "#ccc" : "#eee" }
                GradientStop { position: 1; color: listViewShortMouseArea.pressed ? "#aaa" : "#ccc" }
            }
        }

        ListView {
            id: listViewShort

            x: 7
            y: 7

            width: Math.max(parent.width - 14, 0)
            height: Math.max(parent.height - 14, 0)

            model: performanceModel
            orientation: ListView.Horizontal

            delegate:
                Rectangle {
                    width: sizeOfPart * listViewShort.width
                    height: listViewShort.height

                    color: performanceModel.getItemColor(index)
                }

            Text {
                anchors.fill: parent

                text: performanceModel.commonTime + " " + qsTr("ms")

                fontSizeMode: Text.Fit;
                minimumPixelSize: 10;
                font.pixelSize: 22
                wrapMode: Text.WordWrap
            }

            add: Transition {
                NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 100 }
                NumberAnimation { property: "scale"; from: 0; to: 1.0; duration: 100 }
            }

            remove: Transition {
                NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 100 }
                NumberAnimation { property: "scale"; from: 1.0; to: 0; duration: 100 }
            }
        }

        MouseArea {
            id: listViewShortMouseArea
            anchors.fill: parent
            onClicked: expand = !expand;
        }
    }

    Rectangle {
        visible: (height > 0.0)

        x: 0
        y: parent.height - listViewShortBackground.parent.height - height

        width: listViewShortBackground.width
        height: expand ? listViewExpand.height + 10 : 0

        radius: 5

        clip: true

        color: "#ccc"

        ListView {
            id: listViewExpand

            x: 5
            y: 5

            width: parent.width - 10
            height: expand ? contentHeight :0

            model: performanceModel
            orientation: ListView.Vertical

            clip: true

            delegate: Item {

                x: 0
                y: 0
                width: listViewShortBackground.width
                height: listViewShortBackground.height

                Rectangle {
                    x: 0
                    y: 0
                    width: sizeOfPart * parent.width
                    height: parent.height

                    color: performanceModel.getItemColor(index)
                }

                Text {
                    anchors.fill: parent
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    fontSizeMode: Text.Fit
                    minimumPixelSize: 10
                    font.pixelSize: 18

                    text: name
                }
            }

            add: Transition {
                NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 200 }
                NumberAnimation { property: "scale"; from: 0.0; to: 1.0; duration: 200 }
            }

            remove: Transition {
                NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200 }
                NumberAnimation { property: "scale"; from: 1.0; to: 0.0; duration: 200 }
            }

            Behavior on height {
                NumberAnimation { duration: 200 }
            }
        }

    }


}
