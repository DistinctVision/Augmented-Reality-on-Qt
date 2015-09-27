import QtQuick 2.0
import QtMultimedia 5.4

Item {
    Rectangle {
        x: (parent.width - width) * 0.5
        y: (parent.height - height) * 0.5
        width: parent.width * 0.8
        height: parent.height

        color: Qt.rgba(0.2, 0.2, 0.2)

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

            delegate: Rectangle {
                id: rectItem
                width: parent.width
                height: 60
                radius: 5
                color: mouseArea.pressed ? "gray" : "white"
                Text {
                    text: name
                    anchors.centerIn: parent
                    color: "black"
                    scale: 1.5
                }
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    onReleased: {
                        listView.parent.parent.clickItem(deviceId);
                        listView.parent.parent.visible = false;
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        listModel.clear();
        var devices = QtMultimedia.availableCameras;
        for (var i=0; i<devices.length; ++i) {
            listModel.append({ name: devices[i].displayName, deviceId: devices[i].deviceId });
        }
        if (devices.length === 1){
            clickItem(devices[0]);
            visible = false;
        }
    }

    signal clickItem(string deviceId);
}

