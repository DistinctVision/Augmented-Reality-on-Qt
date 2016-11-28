import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.2
import ARCameraQml 1.0

Item {
    property alias arTracker: sceneSpace.arTracker
    property variant imageReceivers: [ sceneSpace.arTracker ]
    property variant scenes: [ sceneDebugTracker, sceneSpace ]

    anchors.fill: parent

    onArTrackerChanged: {
        trackingUI.updateState();
        trackingUI.updateQuality();
    }

    ARSceneSpace {
        id: sceneSpace
    }

    ARSceneDebugTracker {
        id: sceneDebugTracker
        arTracker: sceneSpace.arTracker
    }

    TrackingUI {
        id: trackingUI
        trackingState: sceneSpace.arTracker.trackingState
        trackingQuality: sceneSpace.arTracker.trackingQuality
        onNextTrackingState: sceneSpace.arTracker.nextTrackingState()
    }
}
