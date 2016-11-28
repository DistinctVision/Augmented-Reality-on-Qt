#include <QApplication>

#include "QScrollEngine/QScrollEngine.h"

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlApplicationEngine>
#include <QGuiApplication>
#include <QQuickView>
#include <QScreen>
#include "App/ARFrameHandler.h"
#include "App/ARSceneShip.h"
#include "App/ARSceneGrid.h"
#include "App/ARCameraCalibrator.h"
#include "App/ARTracker.h"
#include "App/ARSceneDebugTracker.h"
#include "App/ARSceneDebugMap.h"
#include "App/ARPerformanceMonitorModel.h"
#include "App/ARSceneSpace.h"
#include "App/ARSceneCubes.h"
#include "App/FileJsonConfig.h"

#include <QSurfaceFormat>
#include <QtQml/QQmlContext>
#include <QQuickItem>

//#include "AR/MapInitializer.h"

#if defined(__ANDROID__)
#include "AndroidCamera/AndroidCameraView.h"


Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    typedef union {
        JNIEnv* nativeEnvironment;
        void* venv;
    } UnionJNIEnvToVoid;

    UnionJNIEnvToVoid uenv;
    uenv.venv = nullptr;

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK)
        return JNI_ERR;

    JNIEnv* jniEnv = uenv.nativeEnvironment;

    if (!AndroidJniSurfaceTexture::initJNI(jniEnv) ||
            !AndroidJniCamera::initJNI(jniEnv)) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_4;
}

#endif

int main(int argc, char* argv[])
{
    /*QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
#if defined(_WINDOWS_)
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setVersion(3, 3);
#else
    fmt.setRenderableType(QSurfaceFormat::OpenGLES);
    fmt.setVersion(3, 0);
#endif
    QSurfaceFormat::setDefaultFormat(fmt);*/

    using namespace QScrollEngine;
    QGuiApplication app(argc, argv);
    app.setOrganizationName("ARProject");
    app.setApplicationName("AR");
    app.setApplicationVersion("0.7");
    qmlRegisterType<FrameReceiver>("ARCameraQml", 1, 0, "FrameReceiver");
    qmlRegisterType<ARScene>("ARCameraQml", 1, 0, "ARScene");
    qmlRegisterType<ARSceneShip>("ARCameraQml", 1, 0, "ShipScene");
    qmlRegisterType<ARFrameHandler>("ARCameraQml", 1, 0, "ARFrameHandler");
    qmlRegisterType<ARCalibrationConfig>("ARCameraQml", 1, 0, "ARCalibrationConfig");
    qmlRegisterType<ARCameraCalibrator>("ARCameraQml", 1, 0, "CameraCalibrator");
    qmlRegisterType<ARCameraCalibrator_ImageReceiver>("ARCameraQml", 1, 0, "CameraCalibrator_ImageReceiver");
    qmlRegisterType<ARInitConfig>("ARCameraQml", 1, 0, "ARInitConfig");
    qmlRegisterType<ARTrackerConfig>("ARCameraQml", 1, 0, "ARTrackerConfig");
    qmlRegisterType<ARMapPointsDetectorConfig>("ARCameraQml", 1, 0, "ARMapPointsDetectorConfig");
    qmlRegisterType<ARTracker>("ARCameraQml", 1, 0, "ARTracker");
    qmlRegisterType<ARSceneShip>("ARCameraQml", 1, 0, "ARSceneShip");
    qmlRegisterType<ARSceneGrid>("ARCameraQml", 1, 0, "ARSceneGrid");
    qmlRegisterType<ARSceneSpace>("ARCameraQml", 1, 0, "ARSceneSpace");
    qmlRegisterType<ARSceneDebugTracker>("ARCameraQml", 1, 0, "ARSceneDebugTracker");
    qmlRegisterType<ARSceneDebugMap>("ARCameraQml", 1, 0, "ARSceneDebugMap");
    qmlRegisterType<ARPerformanceMonitorModel>("ARCameraQml_models", 1, 0, "ARPerformanceMonitorModel");
    qmlRegisterType<FileJsonConfig>("ARCameraQml", 1, 0, "FileJsonConfig");
#if defined(__ANDROID__)
    qmlRegisterType<AndroidCameraView>("AndroidCamera", 1, 0, "AndroidCameraView");
    QQmlApplicationEngine engine(QUrl("qrc:///QML/main_android.qml"));
#else
    QQmlApplicationEngine engine(QUrl("qrc:///QML/main.qml"));
#endif

    //view.setResizeMode(QQuickView::SizeRootObjectToView);
    // Qt.quit() called in embedded .qml by default only emits
    // quit() signal, so do this (optionally use Qt.exit()).
    QObject::connect(&engine, SIGNAL(quit()), qApp, SLOT(quit()));
    return app.exec();
}
