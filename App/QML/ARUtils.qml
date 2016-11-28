import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import ARCameraQml 1.0

Item {
    property alias arFrameHandler: arFrameHandler
    property alias frameResolution: arFrameHandler.frameResolution
    property bool visibleChangeCameraButton: false
    property alias initConfig: initConfig_c
    property alias trackerConfig: trackerConfig_c
    property alias mapPointsDetectorConfig: mapPointsDetectorConfig_c
    property alias calibrationConfig: calibrationConfig_c
    property alias fileJsonConfig: fileJsonConfig

    signal changeCamera()
    signal saveCameraLog(url path)

    ARCalibrationConfig {
        id: calibrationConfig_c
    }

    ARInitConfig {
        id: initConfig_c
        onConfigChanged: arTracker.initConfig.copy(initConfig_c)
    }

    ARMapPointsDetectorConfig {
        id: mapPointsDetectorConfig_c
        onConfigChanged: arTracker.mapPointsDetectorConfig.copy(mapPointsDetectorConfig_c)
    }

    ARTrackerConfig {
        id: trackerConfig_c
        onConfigChanged: arTracker.trackerConfig.copy(trackerConfig_c)
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onPressed: arFrameHandler.pressed(0, mouse.x, mouse.y)
        onReleased: arFrameHandler.released(0)
    }

    ARFrameHandler {
        id: arFrameHandler
        frameResolution: fileJsonConfig.frameResolution
        scenes: loader.item.scenes
        imageReceivers: loader.item.imageReceivers
    }

    ARTracker {
        id: arTracker
        enabled: true

        cameraParameters: arFrameHandler.cameraParameters
    }

    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: spaceWindow
        onSourceComponentChanged: {
            if (item.hasOwnProperty('arTracker')) {
                item.arTracker = arTracker;
            }
        }
    }

    Component {
        id: spaceWindow
        SpaceWindow {
        }
    }

    Component {
        id: shipWindow
        ShipWindow {
        }
    }

    /*Component {
        id: cubesWindow
        CubesWindow {
        }
    }*/

    Component {
        id: trackerWindow
        TrackerWindow {
        }
    }

    Component {
        id: calibrationWindow
        CalibrationWindow {
            Component.onCompleted: {
                cameraParameters = arFrameHandler.cameraParameters;
                fixedOpticalCenter = arFrameHandler.fixedOpticalCenter;
                disabledRadialDistortion = arFrameHandler.disabledRadialDistortion;
                calibrationConfig.copy(calibrationConfig_c);
            }
            onCameraParametersChanged: {
                arFrameHandler.cameraParameters = cameraParameters;
            }
            onFixedOpticalCenterChanged: {
                arFrameHandler.fixedOpticalCenter = fixedOpticalCenter;
            }
            onDisabledRadialDistortionChanged: {
                arFrameHandler.disabledRadialDistortion = disabledRadialDistortion;
            }
        }
    }

    function setSpace() {
        loader.sourceComponent = spaceWindow
    }

    function setShip() {
        loader.sourceComponent = shipWindow;
    }

    function setCubes() {
        loader.sourceComponent = cubesWindow
    }

    function setTracker() {
        loader.sourceComponent = trackerWindow;
    }

    function setCalibration() {
        loader.sourceComponent = calibrationWindow;
    }

    FileDialog {
        id: fileDialog
        modality: Qt.ApplicationModal
        visible: false
        nameFilters: [ "JSON file (*.txt)" ]
        title: "Please choose a path"
        folder: fileJsonConfig.getDefaultDirectory()
        selectMultiple: false
        //selectExisting: false
        //selectFolder: true

        property bool save: false
        property int saveMode: 0

        onAccepted: {
            console.log("You chose: " + fileUrl.toString());
            if (save) {
                if (saveMode === 0) {
                    fileJsonConfig.clear();
                    fileJsonConfig.addCalibrationConfig(calibrationConfig_c);
                    fileJsonConfig.addInitConfig(initConfig_c);
                    fileJsonConfig.addMapPointsDetectorConfig(mapPointsDetectorConfig_c);
                    fileJsonConfig.addTrackerConfig(trackerConfig_c);
                    fileJsonConfig.save(fileUrl + "/config.txt");
                } else {
                    saveCameraLog(fileUrl + "/cameraLog.txt")
                }
            } else {
                fileJsonConfig.load(fileUrl, calibrationConfig_c, initConfig_c, mapPointsDetectorConfig_c, trackerConfig_c);
                if (loader.sourceComponent === calibrationWindow) {
                    loader.item.calibrationCondig.copy(calibrationConfig_c);
                }
            }
            close();
            parent.parent.forceActiveFocus();
        }
        onRejected: {
            console.log("Canceled");
            close();
            parent.parent.forceActiveFocus();
        }
    }

    FileJsonConfig {
        id: fileJsonConfig
    }

    /*Rectangle {
        id: dialogFrameQuality

        x: (parent.width - width) * 0.5
        y: (parent.height - height) * 0.25
        z: 10

        width: parent.width * 0.5
        height: childrenRect.height + 20
        radius: 10

        visible: false
        enabled: visible

        color: "#dddddddd"

        Column {
            x: 10
            y: 10
            width: parent.width - 20

            spacing: 20

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                font.pointSize: 20
                text: qsTr("Frame qulity: ") + sliderFrameQuality.value.toFixed(2).toString()
                horizontalAlignment: Text.Center
            }

            Slider {
                id: sliderFrameQuality

                width: parent.width

                minimumValue: 0.0
                maximumValue: 1.0
            }

            Button {
                x: (parent.width - width) * 0.5

                text: qsTr("Ok")

                onClicked: {
                    dialogFrameQuality.visible = false;
                    frameQuality = sliderFrameQuality.value;
                }
            }
        }
    }*/

    ScrollView {

        width: parent.width
        height: row.height
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        horizontalScrollBarPolicy: Qt.ScrollBarAsNeeded

        Row {
            id: row
            spacing: 10
            width: childrenRect.width
            height: childrenRect.height

            Rectangle {
                width: childrenRect.width
                height: childrenRect.height
                radius: 5
                border {
                    width: 2
                    color: "black"
                }
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#88dddddd"; }
                    GradientStop { position: 1.0; color: "#88777777"; }
                }

                Row {
                    width: childrenRect.width
                    height: childrenRect.height


                    ToolButton {
                        text: qsTr("Space")
                        onClicked: arUtils.setSpace()
                    }
                    Rectangle { width: 1; height: parent.height; color: "black" }
                    ToolButton {
                        text: qsTr("Ship")
                        onClicked: arUtils.setShip()
                    }
                    Rectangle { width: 1; height: parent.height; color: "black" }
                    /*ToolButton {
                        text: qsTr("Cubes")
                        onClicked: arUtils.setCubes()
                    }
                    Rectangle { width: 1; height: parent.height; color: "black" }*/
                    ToolButton {
                        text: qsTr("Tracker")
                        onClicked: arUtils.setTracker()
                    }
                    Rectangle { width: 1; height: parent.height; color: "black" }
                    ToolButton {
                        text: qsTr("Calibration")
                        onClicked: arUtils.setCalibration()
                    }
                }
            }

            Rectangle {
                enabled: visible
                visible: visibleChangeCameraButton
                width: childrenRect.width
                height: childrenRect.height
                radius: 5
                border {
                    width: 2
                    color: "black"
                }
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#88dddddd"; }
                    GradientStop { position: 1.0; color: "#88777777"; }
                }

                Row {
                    width: childrenRect.width
                    height: childrenRect.height
                    ToolButton {
                        text: qsTr("Change camera")
                        onClicked: changeCamera();
                    }
                }
            }

            Rectangle {
                radius: 5
                width: childrenRect.width
                height: childrenRect.height
                border {
                    width: 2
                    color: "black"
                }
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#88dddddd"; }
                    GradientStop { position: 1.0; color: "#88777777"; }
                }

                Row {
                    width: childrenRect.width
                    height: childrenRect.height
                    ToolButton {
                        text: qsTr("Save config")
                        onClicked: {
                            fileDialog.selectFolder = true;
                            fileDialog.selectExisting = false;
                            fileDialog.save = true;
                            fileDialog.saveMode = 0;
                            fileDialog.open();
                        }
                    }
                    Rectangle { width: 1; height: parent.height; color: "black" }
                    ToolButton {
                        text: qsTr("Load config")
                        onClicked: {
                            fileDialog.selectFolder = false;
                            fileDialog.selectExisting = true;
                            fileDialog.save = false;
                            fileDialog.open();
                        }
                    }
                }
            }

            /*Rectangle {
                radius: 5
                width: childrenRect.width
                height: childrenRect.height
                border {
                    width: 2
                    color: "black"
                }
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#88dddddd"; }
                    GradientStop { position: 1.0; color: "#88777777"; }
                }

                ToolButton {
                    text: qsTr("Frame quality")
                    onClicked: {
                        sliderFrameQuality.value = frameQuality;
                        dialogFrameQuality.visible = true;
                    }
                }
            }

            Rectangle {
                radius: 5
                width: childrenRect.width
                height: childrenRect.height
                border {
                    width: 2
                    color: "black"
                }
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#88dddddd"; }
                    GradientStop { position: 1.0; color: "#88777777"; }
                }

                ToolButton {
                    text: qsTr("Save camera log")
                    onClicked: {
                        fileDialog.selectFolder = true;
                        fileDialog.selectExisting = false;
                        fileDialog.save = true;
                        fileDialog.saveMode = 1;
                        fileDialog.open();
                    }
                }
            }*/
        }
    }
}
