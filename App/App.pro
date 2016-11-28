QT += qml quick widgets multimedia
!no_desktop: QT += widgets

CONFIG += c++11

TARGET = App
TEMPLATE = app

INCLUDEPATH += .
INCLUDEPATH += $$PWD/../AddedSource

include ($$PWD/../AddedSource/QScrollEngine/QScrollEngine.pri)
include ($$PWD/../AddedSource/AR/AR.pri)
include ($$PWD/../AddedSource/TMath/TMath.pri)
android: {
include (AndroidCamera.pri)
}

SOURCES += main.cpp \
    App/FrameReceiver.cpp \
    App/TextureRenderer.cpp \
    App/ARScene.cpp \
    App/ARTracker.cpp \
    App/ARSceneShip.cpp \
    App/ARFrameHandler.cpp \
    App/ARSceneGrid.cpp \
    App/ARCameraCalibrator.cpp \
    App/ARPerformanceMonitorModel.cpp \
    App/ARSceneDebugTracker.cpp \
    App/ARSceneDebugMap.cpp \
    App/ARSceneSpace.cpp

HEADERS += \
    App/FrameReceiver.h \
    App/TextureRenderer.h \
    App/ARScene.h \
    App/ARTracker.h \
    App/ARSceneShip.h \
    App/ARFrameHandler.h \
    App/ARSceneGrid.h \
    App/ARInitConfig.h \
    App/ARTrackerConfig.h \
    App/ARMapPointsDetectorConfig.h \
    App/ARCameraCalibrator.h \
    App/ARCalibrationConfig.h \
    App/ARPerformanceMonitorModel.h \
    App/FileJsonConfig.h \
    App/ARSceneDebugTracker.h \
    App/ARSceneDebugMap.h \
    App/ARSceneSpace.h

RESOURCES += \
    resources.qrc \
    qmlresources.qrc
