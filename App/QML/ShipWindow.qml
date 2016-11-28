import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.2
import ARCameraQml 1.0

Item {
    property alias arTracker: sceneShip.arTracker
    property variant imageReceivers: [ sceneShip.arTracker ]
    property variant scenes: [ sceneDebugTracker, sceneShip ]

    anchors.fill: parent

    onArTrackerChanged: {
        trackingUI.updateState();
        trackingUI.updateQuality();
    }

    ARSceneShip {
        id: sceneShip
    }

    ARSceneDebugTracker {
        id: sceneDebugTracker
        arTracker: sceneShip.arTracker
    }

    TrackingUI {
        id: trackingUI
        trackingState: sceneShip.arTracker.trackingState
        trackingQuality: sceneShip.arTracker.trackingQuality
        onNextTrackingState: sceneShip.arTracker.nextTrackingState()
    }
}
