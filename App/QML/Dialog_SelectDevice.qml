import QtQuick 2.0
import QtMultimedia 5.4

Item {

    property alias countDevices: listModel.count

    function clearDevices() {
        listModel.clear();
    }

    function addDevice(name, deviceId) {
        listModel.append({ name: name, deviceId: deviceId });
    }

    Rectangle {
        x: (parent.width - width) * 0.5
        y: (parent.height - height) * 0.5
        width: parent.width * 0.8
        height: parent.height
        radius: 10
        color: "#99666688"

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

            preferredHighlightBegin: parent.height * 0.5 - 60
            preferredHighlightEnd: parent.height * 0.5
            highlightRangeMode: ListView.StrictlyEnforceRange


            delegate: ShineButton {
                id: rectItem
                width: parent.width
                height: 60
                radius: 5
                fillStyle: "#666666"
                strokeStyle: "#FFFFFF"
                text: name
                onClicked: {
                    listView.parent.parent.clickDevice(deviceId);
                    listView.parent.parent.visible = false;
                }
            }
        }
    }

    Component.onCompleted: {
    }

    signal clickDevice(string deviceId)
}

