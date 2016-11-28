import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.2
import ARCameraQml 1.0

Item {
    property alias arTracker: sceneDebugTracker.arTracker
    property variant imageReceivers: [ sceneDebugTracker.arTracker ]
    property variant scenes: [ sceneDebugTracker, sceneDebugMap, sceneGrid ]

    anchors.fill: parent

    onArTrackerChanged: {
        trackingUI.updateState();
        trackingUI.updateQuality();
    }

    ARSceneDebugTracker {
        id: sceneDebugTracker
    }

    ARSceneDebugMap {
        id: sceneDebugMap
        arTracker: sceneDebugTracker.arTracker
    }

    ARSceneGrid {
        id: sceneGrid
        arTracker: sceneDebugTracker.arTracker
    }

    PerformanceMonitor {
        id: performanceMonitor
        arTracker: sceneDebugTracker.arTracker
        countUsedTimes: 5
        anchors.fill: parent
    }

    TrackingUI {
        id: trackingUI
        trackingState: sceneDebugTracker.arTracker.trackingState
        trackingQuality: sceneDebugTracker.arTracker.trackingQuality
        onNextTrackingState: sceneDebugTracker.arTracker.nextTrackingState()
    }
}
