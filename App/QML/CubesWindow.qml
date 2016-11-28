import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.2
import ARCameraQml 1.0

Item {
    property alias arTracker: sceneCubes.arTracker
    property variant imageReceivers: [ sceneCubes.arTracker ]
    property variant scenes: [ sceneDebugTracker, sceneCubes ]

    anchors.fill: parent

    onArTrackerChanged: {
        trackingUI.updateState();
        trackingUI.updateQuality();
    }

    ARSceneCubes {
        id: sceneCubes
    }

    ARSceneDebugTracker {
        id: sceneDebugTracker
        arTracker: sceneCubes.arTracker
    }

    TrackingUI {
        id: trackingUI
        trackingState: sceneCubes.arTracker.trackingState
        trackingQuality: sceneCubes.arTracker.trackingQuality
        onNextTrackingState: sceneCubes.arTracker.nextTrackingState()
    }
}
