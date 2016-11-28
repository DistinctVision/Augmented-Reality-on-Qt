import QtQuick 2.0
import QtQuick.Window 2.1
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4
import ARCameraQml 1.0
import AndroidCamera 1.0

ApplicationWindow {
    visible: true
    visibility: Window.FullScreen

    color: "#00000000"

    AndroidCameraView {
        anchors.fill: parent
        selectedCameraId: 0
        arFrameHandler: arUtils.arFrameHandler
        targetPreviewSize: "0x480"

        ARUtils {
            id: arUtils
            anchors.fill: parent
            visibleChangeCameraButton: (dialog_selectDevice.countDevices > 1)

            initConfig {
                maxPixelError: 5.0
                epsDistanceForPlane: 0.1
                countTimes: 1000
                mapScale: 10.0
                minCountFeatures: 50
                maxCountFeatures: 150
                minCountMapPoints: 30
                minDisparity: 20.0
                frameGridSize: "18x18"
                featureCornerBarier: 5
                featureDetectionThreshold: 10.0
                countImageLevels: 3
                minImageLevelForFeature: -1
                maxImageLevelForFeature: -1
                featureCursorSize: "8x8"
                pixelEps: 5e-3
                maxNumberIterationsForOpticalFlow: 30
            }
            trackerConfig {
                countImageLevels: 3
                mapPoint_goodSuccessLimit: 4
                mapPoint_failedLimit: 16
                candidate_failedLimit: 8
                maxNumberOfUsedKeyFrames: 5
                frameBorder: 1
                maxNumberOfFeaturesOnFrame: 100
                frameGridSize: "18x18"
                featureCursorSize: "4x4"
                featureMaxNumberIterations: 10
                pixelEps: 1e-2
                locationEps: 1e-5
                locationMaxPixelError: 15.0
                locationNumberIterations: 10
                numberPointsForSructureOptimization: 20
                numberIterationsForStructureOptimization: 5
                toleranceOfCreatingFrames: 0.2
                preferredNumberTrackingPoints: 25
                minNumberTrackingPoints: 20
                sizeOfSmallImage: 64
                maxCountKeyFrames: 10
                tracker_eps: 1e-4
                tracker_numberIterations: 15
                tracker_minImageLevel: 2
                tracker_maxImageLevel: -1
                tracker_cursorSize: "2x2"
            }
            mapPointsDetectorConfig {
                maxNumberOfUsedFrames: 3
                maxNumberOfSearchSteps: 100
                seedConvergenceSquaredSigmaThresh: 20.0
                sizeOfCommitMapPoints: 10
                frameGridSize: "18x18"
                featureCornerBarier: 5
                featureDetectionThreshold: 10.0
                minImageLevelForFeature: 1;
                maxImageLevelForFeature: -1;
                featureCursorSize: "8x8"
                pixelEps: 1e-2
                maxNumberIterationsForOpticalFlow: 16
                maxCountCandidatePoints: 100
            }
            calibrationConfig {
                countImageLevels: 1
                minImageLevelForCorner: 0
                maxImageLevelForCorner: 0
                minDistanceForCorner: 10.0
                cornerPatchPixelSize: 30
                cornerPathEps: 1e-3
                maxCountIterations: 100
                maxError: 100.0
                minCornersForGrabbedFrame: 20
                expandByStepMaxDistFrac: 0.1
                meanGate: 25
                minGain: 10.0
                counOfUsedCalibrationFrames: 5
            }
            frameResolution: "0x240"

            onChangeCamera: dialog_selectDevice.visible = true;
            onSaveCameraLog: fileJsonConfig.saveLogInfoOfCameraView(path, parent);
        }

        Dialog_SelectDevice {
            id: dialog_selectDevice
            enabled: visible
            visible: false

            anchors.fill: parent

            Component.onCompleted: {
                clearDevices();
                var devices = parent.getAviableCameras();
                for (var i=0; i<devices.length; ++i) {
                    addDevice(devices[i], i.toString());
                }
            }

            onClickDevice: {
                var id = parseInt(deviceId);
                if (isNaN(id)) {
                    console.log("deviceId is NaN");
                    return;
                }
                if (id !== parent.selectedCameraId) {
                    parent.selectedCameraId = id;
                }
                visible = false;
            }
        }
    }
}
