import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.2
import QtQuick.Dialogs 1.2
import ARCameraQml 1.0

Item {
    property alias cameraParameters: cameraCalibrator.cameraParameters
    property alias fixedOpticalCenter: cameraCalibrator.fixedOpticalCenter
    property alias disabledRadialDistortion: cameraCalibrator.disabledRadialDistortion
    property alias calibrationConfig: cameraCalibrator.calibrationConfig

    property variant imageReceivers: [ cameraCalibrator.imageReceiver ]
    property variant scenes: [ cameraCalibrator ]

    anchors.fill: parent

    CameraCalibrator {
        id: cameraCalibrator

        enabled: true
        imageReceiver.enabled: true

        pauseOnDetectGrid: false

        onDetectGrid: {
            if (pauseOnDetectGrid) {
                windowGrab.enabled = true
            }
        }
    }

    Rectangle {
        id: inputCameraParameters

        x: (parent.width - width) * 0.5
        y: (parent.height - height) * 0.2
        z: 10

        width: parent.width * 0.7
        height: childrenRect.height + 20
        radius: 10

        visible: false
        enabled: visible

        Item {

            x: 20
            y: 20
            width: parent.width - 40
            height: childrenRect.height + 40

            Column {
                width: parent.width
                height: childrenRect.height
                spacing: 15

                Text {
                    text: qsTr("Input camera parameters:")
                }

                Rectangle {
                    width: parent.width
                    height: 46

                    color: "white"
                    radius: 3

                    TextField {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        id: inputCameraParameters_editText
                        width: parent.width - 34
                        height: 46
                        focus: true
                    }
                }
            }

            Row {
                y: parent.children[0].height + 20
                width: parent.width
                height: childrenRect.height

                spacing: width - (children[0].width + children[1].width)

                Button {
                    text: qsTr("Cancel")
                    onClicked: inputCameraParameters.visible = false;
                }
                Button {
                    text: qsTr("Ok")
                    onClicked: {
                        var listOfStrs = inputCameraParameters_editText.text.split(",", 5);
                        var listOfParameters = [];
                        for (var i = 0; i < listOfStrs.length; ++i) {
                            listOfParameters.push(parseFloat(listOfStrs[i]));
                        }
                        cameraCalibrator.cameraParameters = listOfParameters;
                        inputCameraParameters.visible = false;
                    }
                }
            }
        }
    }

    Rectangle {
        id: textStateBackground

        x: textState.x - 5
        y: textState.y - 5
        width: textState.width + 10
        height: textState.height + 10
        radius: 5
        border.color: "white"
        border.width: 1
        color: "black"
        opacity: 0.5
    }

    Text {
        id: textState
        color: "white"
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        text: "%1 %2 %3 %4 %5".arg(
                  cameraCalibrator.cameraParameters[0].toFixed(2)).arg(
                  cameraCalibrator.cameraParameters[1].toFixed(2)).arg(
                  cameraCalibrator.cameraParameters[2].toFixed(2)).arg(
                  cameraCalibrator.cameraParameters[3].toFixed(2)).arg(
                  cameraCalibrator.cameraParameters[4].toFixed(2))
    }

    MouseArea {
        anchors.fill: textStateBackground
        onPressed: textStateBackground.border.width = 4;
        onReleased: textStateBackground.border.width = 1;
        onClicked: {
            var p = cameraCalibrator.cameraParameters;
            inputCameraParameters_editText.text =
                    p[0].toFixed(2) + ", " +
                    p[1].toFixed(2) + ", " +
                    p[2].toFixed(2) + ", " +
                    p[3].toFixed(2) + ", " +
                    p[4].toFixed(2);
            inputCameraParameters.visible = true;
        }
    }

    ShineButton {
        id: buttonGrab

        enabled: true
        visible: enabled

        x: (parent.width - width) * 0.5
        y: parent.height - height * 1.5
        width: Math.min(parent.width, parent.height) * 0.3
        height: width * 0.4
        radius: 4.0
        text: qsTr("Grab grid")
        strokeStyle: "white"
        //fontPixelSize: Math.min(width, height) * 0.3

        onClicked: {
            cameraCalibrator.pauseOnDetectGrid = true
            buttonGrab.enabled = false
        }
    }

    Item {
        id: calibrationAddedParameters

        enabled: true
        visible: enabled

        x: 0
        y: (parent.height - height) * 0.5

        width: calibrationAddedParameters_list.width + 10
        height: calibrationAddedParameters_list.height + 10

        Rectangle {
            anchors.fill: parent

            radius: 5

            color: "white"
            opacity: 0.6
        }

        Column {

            id: calibrationAddedParameters_list

            x: 5
            y: 5

            visible: enabled
            spacing: 10

            CheckBox {
                text: qsTr("Fixed optical center")
                checked: cameraCalibrator.fixedOpticalCenter
                onClicked: {
                    cameraCalibrator.fixedOpticalCenter = !cameraCalibrator.fixedOpticalCenter
                }
            }
            CheckBox {
                text: qsTr("Disable radial distortion")
                checked: cameraCalibrator.disabledRadialDistortion
                onClicked: {
                    cameraCalibrator.disabledRadialDistortion
                            = !cameraCalibrator.disabledRadialDistortion
                }
            }
            ShineButton {
                radius: 5
                text: qsTr("Reset")
                strokeStyle: "#0000"
                fillStyle: "#F33"
                width: Math.min(parent.width * 0.9, 150)
                height: width * 0.4

                onClicked: cameraCalibrator.reset()
            }
        }
    }

    Item {
        id: windowGrab

        enabled: false
        visible: enabled

        x: (parent.width - width) * 0.5
        y: parent.height - height * 1.5
        width: Math.min(parent.width, parent.height) * 0.5
        height: width * 0.4

        Rectangle {
            anchors.fill: parent
            radius: 10.0
            opacity: 0.4
            color: "#002288"
            border {
                width: 2
                color: "white"
            }
        }

        Text {
            x: 0
            y: height * 0.5
            width: parent.width
            height: parent.height * 0.1
            color: "white"

            text: qsTr("Grab this frame?")
            horizontalAlignment: Text.AlignHCenter
        }

        ShineButton {
            x: parent.width * 0.5 - width * 1.1
            y: parent.height - height * 1.2
            width: Math.min(parent.width, parent.height)
            height: width * 0.4
            radius: 4.0
            text: qsTr("Yes")
            strokeStyle: "white"
            //fontPixelSize: Math.min(width, height) * 0.3

            onClicked: {
                cameraCalibrator.grabGrid()
                cameraCalibrator.imageReceiver.enabled = true
                cameraCalibrator.pauseOnDetectGrid = false
                windowGrab.enabled = false
                buttonGrab.enabled = true
            }
        }

        ShineButton {
            x: parent.width * 0.5 + width * 0.1
            y: parent.height - height * 1.2
            width: Math.min(parent.width, parent.height)
            height: width * 0.4
            radius: 4.0
            text: qsTr("No")
            strokeStyle: "white"
            //fontPixelSize: Math.min(width, height) * 0.3

            onClicked: {
                cameraCalibrator.imageReceiver.enabled = true
                cameraCalibrator.pauseOnDetectGrid = false
                windowGrab.enabled = false
                buttonGrab.enabled = true
            }
        }
    }
}
