import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.2
import ARCameraQml 1.0

Item {
    property int trackingState: ARTracker.Undefining
    property int trackingQuality: ARTracker.Ugly

    anchors.fill: parent

    signal nextTrackingState()

    onTrackingStateChanged: updateState()
    onTrackingQualityChanged: updateQuality()

    Component.onCompleted: {
        updateQuality();
        updateState();
    }

    ShineButton {
        id: buttonState1

        enabled: true
        visible: enabled

        x: (parent.width - width) * 0.5
        y: parent.height - height * 1.5
        z: 1
        width: Math.min(parent.width, parent.height) * 0.3
        height: width * 0.4
        radius: 4.0
        strokeStyle: "white"
        fillStyle: Qt.rgba(0.0, 0.1, 0.5)
        //fontPixelSize: Math.min(width, height) * 0.3

        onClicked: {
            nextTrackingState();
        }
    }

    ShineButton {
        id: buttonState2

        enabled: true
        visible: enabled

        x: (parent.width - width) * 0.5
        y: parent.height - height * 1.5
        z: 0
        width: Math.min(parent.width, parent.height) * 0.3
        height: width * 0.4
        radius: 4.0
        strokeStyle: "white"
        fillStyle: Qt.rgba(0.5, 0.1, 0.0)
        //fontPixelSize: Math.min(width, height) * 0.3

        onClicked: {
            nextTrackingState();
        }
    }

    Text {
        id: textQuality
        color: "white"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    function updateState() {
        switch (trackingState) {
        case ARTracker.Undefining:
            textQuality.visible = false;
            buttonState1.enabled = true;
            buttonState1.text = qsTr("Start");
            buttonState2.enabled = false;
            break;
        case ARTracker.CaptureFirstFrame:
            textQuality.visible = false;
            buttonState1.enabled = false;
            buttonState2.enabled = false;
            break;
        case ARTracker.CaptureSecondFrame:
            textQuality.visible = false;
            buttonState1.enabled = true;
            buttonState1.text = qsTr("Force");
            buttonState2.enabled = false;
            break;
        case ARTracker.Tracking:
            textQuality.visible = true;
            buttonState1.enabled = false;
            buttonState1.text = qsTr("Reset");
            buttonState2.enabled = false;
            break;
        case ARTracker.LostTracking:
            textQuality.visible = false;
            buttonState1.enabled = false;
            buttonState2.enabled = true;
            buttonState2.text = qsTr("Reset");
            break;
        default:
            break;
        }
    }

    function updateQuality() {
        var strBegin = qsTr("Tracking qulity is ");
        switch (trackingQuality) {
        case ARTracker.Good:
            textQuality.text = strBegin + qsTr("good");
            break;
        case ARTracker.Bad:
            textQuality.text = strBegin + qsTr("bad");
            break;
        case ARTracker.Ugly:
            textQuality.text = strBegin + qsTr("ugly");
            break;
        }
    }
}
