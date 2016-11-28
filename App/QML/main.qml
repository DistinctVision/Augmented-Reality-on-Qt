import QtQuick 2.2
import QtMultimedia 5.4
import QtQuick.Window 2.1
import QtQuick.Controls 1.2
import ARCameraQml 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480

    Camera {
        id: mainCamera

        captureMode: Camera.CaptureViewfinder
        imageCapture {
        }

        videoRecorder {
            frameRate: 30
            videoEncodingMode: CameraRecorder.AverageBitRateEncoding
        }
    }

    VideoOutput {
        anchors.fill: parent
        source: mainCamera

        filters: [ arUtils.arFrameHandler ]

        Text {
            id: textState
            color: "white"
        }
    }

    ARUtils {
        id: arUtils
        anchors.fill: parent
        visibleChangeCameraButton: (dialog_selectDevice.countDevices > 1)

        onChangeCamera: dialog_selectDevice.visible = true;

        initConfig {
            maxPixelError: 6.0
            epsDistanceForPlane: 0.1
            countTimes: 1000
            mapScale: 10.0
            minCountFeatures: 80
            maxCountFeatures: 100
            minCountMapPoints: 40
            minDisparity: 30.0
            frameGridSize: "18x18"
            featureCornerBarier: 5
            featureDetectionThreshold: 10.0
            countImageLevels: 3
            minImageLevelForFeature: -1
            maxImageLevelForFeature: -1
            featureCursorSize: "15x15"
            pixelEps: 1e-3
            maxNumberIterationsForOpticalFlow: 50
        }
        trackerConfig {
            countImageLevels: 3
            mapPoint_goodSuccessLimit: 4
            mapPoint_failedLimit: 8
            candidate_failedLimit: 4
            maxNumberOfUsedKeyFrames: 5
            frameBorder: 2
            maxNumberOfFeaturesOnFrame: 60
            frameGridSize: "18x18"
            featureCursorSize: "5x5"
            featureMaxNumberIterations: 10
            pixelEps: 4e-3
            locationEps: 1e-5
            locationMaxPixelError: 12.0
            locationNumberIterations: 0
            numberPointsForSructureOptimization: 20
            numberIterationsForStructureOptimization: 20
            toleranceOfCreatingFrames: 0.2
            preferredNumberTrackingPoints: 25
            minNumberTrackingPoints: 20
            sizeOfSmallImage: 48
            maxCountKeyFrames: 10
            tracker_eps: 1e-5
            tracker_numberIterations: 15
            tracker_minImageLevel: 1
            tracker_maxImageLevel: -1
            tracker_cursorSize: "3x3"
        }
        mapPointsDetectorConfig {
            maxNumberOfUsedFrames: 5
            maxNumberOfSearchSteps: 1000
            seedConvergenceSquaredSigmaThresh: 20.0
            sizeOfCommitMapPoints: 5
            frameGridSize: "18x18"
            featureCornerBarier: 5
            featureDetectionThreshold: 10.0
            minImageLevelForFeature: -1;
            maxImageLevelForFeature: -1;
            featureCursorSize: "15x15"
            pixelEps: 1e-3
            maxNumberIterationsForOpticalFlow: 50
            maxCountCandidatePoints: 100
        }
        calibrationConfig {
            countImageLevels: 4
            minImageLevelForCorner: 1
            maxImageLevelForCorner: 2
            minDistanceForCorner: 50.0
            cornerPatchPixelSize: 30
            cornerPathEps: 1e-3
            maxCountIterations: 200
            maxError: 100.0
            minCornersForGrabbedFrame: 20
            expandByStepMaxDistFrac: 0.15
            meanGate: 30
            minGain: 10.0
            counOfUsedCalibrationFrames: 5
        }
        frameResolution: "0x480"

        onSaveCameraLog: fileJsonConfig.saveLogInfoOfCamera(path, mainCamera)
    }

    Dialog_SelectDevice {
        id: dialog_selectDevice
        enabled: visible
        visible: false

        anchors.fill: parent

        Component.onCompleted: {
            clearDevices();
            var devices = QtMultimedia.availableCameras;
            for (var i=0; i<devices.length; ++i) {
                addDevice(devices[i].displayName, devices[i].deviceId);
            }
        }

        onClickDevice: {
            if (mainCamera.deviceId !== deviceId) {
                mainCamera.stop();
                mainCamera.deviceId = deviceId;
                mainCamera.start();
            }
            visible = false;
        }
    }
}
