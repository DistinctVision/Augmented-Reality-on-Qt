import QtQuick 2.0

Item {

    property alias currentIndex : listView.currentIndex

    Rectangle {
        x: (parent.width - width) * 0.5
        y: (parent.height - height) * 0.5
        width: parent.width * 0.8
        height: parent.height

        gradient: Gradient {
            GradientStop { position: 0.0; color: "white"; }
            GradientStop { position: 1.0; color: Qt.rgba(0.5, 0.5, 1.0, 0.5); }
        }

        ListModel {
            id: listModel
        }

        ListView {
            id: listView
            model: listModel
            x: (parent.width - width) * 0.5
            y: (parent.height - height) * 0.5
            width: parent.width * 0.8
            height: parent.height
            spacing: 10

            Gradient {
                id: gradientSelected
                GradientStop { position: 0.0; color: Qt.rgba(0.5, 0.8, 1.0, 1.0); }
                GradientStop { position: 0.8; color: Qt.rgba(0.8, 0.9, 1.0, 1.0); }
                GradientStop { position: 1.0; color: Qt.rgba(0.6, 0.7, 0.8, 1.0); }
            }
            Gradient {
                id: gradientUnselected
                GradientStop { position: 0.0; color: Qt.rgba(0.7, 0.8, 1.0, 1.0); }
                GradientStop { position: 0.8; color: Qt.rgba(0.9, 0.95, 1.0, 1.0); }
                GradientStop { position: 1.0; color: Qt.rgba(0.7, 0.8, 0.9, 1.0); }
            }

            delegate: Rectangle {
                id: rectItem
                width: parent.width
                height: 60
                radius: 5
                gradient: mouseArea.pressed ? gradientSelected : gradientUnselected
                Text {
                    text: name
                    anchors.centerIn: parent
                    scale: 1.5
                }
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    onReleased: {
                        listView.parent.parent.clickItem(index);
                    }
                }
            }
        }
    }

    function append(item) { listModel.append(item); }
    function clear() { listModel.clear(); }

    signal clickItem(int index);
}

