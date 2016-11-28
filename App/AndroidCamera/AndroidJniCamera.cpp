#include "AndroidJniCamera.h"
#include "AndroidJniSurfaceTexture.h"

#include <QStringList>
#include <QMutex>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include <QThread>

AndroidJniCamera::AndroidJniCamera(QThread* worker)
    : QObject(),
      m_worker(worker),
      m_parametersMutex(QMutex::Recursive)
{
}

AndroidJniCamera::~AndroidJniCamera()
{
    if (m_camera.isValid()) {
        _global_cameraMapMutex.lock();
        _global_cameraMap.remove(m_cameraId);
        _global_cameraMapMutex.unlock();
    }

    release();
    m_worker->exit();
    m_worker->wait(5000);
}

AndroidJniCamera* AndroidJniCamera::open(int cameraId)
{
    QThread* worker = new QThread;
    AndroidJniCamera* q = new AndroidJniCamera(worker);
    worker->start();
    q->moveToThread(worker);
    //connect(worker, &QThread::finished, q, &AndroidJniCamera::deleteLater);
    bool ok = true;
    QMetaObject::invokeMethod(q, "_init", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, ok), Q_ARG(int, cameraId));
    if (!ok) {
        worker->quit();
        worker->wait(5000);
        delete q;
        delete worker;
        return nullptr;
    }

    _global_cameraMapMutex.lock();
    _global_cameraMap.insert(cameraId, q);
    _global_cameraMapMutex.unlock();
    return q;
}

bool AndroidJniCamera::_init(int cameraId)
{
    m_cameraId = cameraId;
    QAndroidJniEnvironment env;
    m_camera = QAndroidJniObject::callStaticObjectMethod("android/hardware/Camera",
                                                         "open",
                                                         "(I)Landroid/hardware/Camera;",
                                                         cameraId);
    if (_exceptionCheckAndClear(env) || !m_camera.isValid())
        return false;

    m_cameraListener = QAndroidJniObject("ru/qtproject/artechproject86/ARCameraListener", "(I)V", m_cameraId);
    m_info = QAndroidJniObject("android/hardware/Camera$CameraInfo");
    m_camera.callStaticMethod<void>("android/hardware/Camera",
                                    "getCameraInfo",
                                    "(ILandroid/hardware/Camera$CameraInfo;)V",
                                    cameraId,
                                    m_info.object());

    QAndroidJniObject params = m_camera.callObjectMethod("getParameters",
                                                         "()Landroid/hardware/Camera$Parameters;");
    m_parameters = QAndroidJniObject(params);

    return true;
}

int AndroidJniCamera::cameraId() const
{
    return m_cameraId;
}

bool AndroidJniCamera::lock()
{
    bool ok = true;
    QMetaObject::invokeMethod(this, "_lock", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, ok));
    return ok;
}

bool AndroidJniCamera::_lock()
{
    QAndroidJniEnvironment env;
    m_camera.callMethod<void>("lock");
    return !_exceptionCheckAndClear(env);
}

bool AndroidJniCamera::unlock()
{
    bool ok = true;
    QMetaObject::invokeMethod(this, "_unlock", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, ok));
    return ok;
}

bool AndroidJniCamera::_unlock()
{
    QAndroidJniEnvironment env;
    m_camera.callMethod<void>("unlock");
    return !_exceptionCheckAndClear(env);
}

bool AndroidJniCamera::reconnect()
{
    bool ok = true;
    QMetaObject::invokeMethod(this, "_reconnect", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, ok));
    return ok;
}

bool AndroidJniCamera::_reconnect()
{
    QAndroidJniEnvironment env;
    m_camera.callMethod<void>("reconnect");
    return !_exceptionCheckAndClear(env);
}

void AndroidJniCamera::release()
{
    QMetaObject::invokeMethod(this, "_release", Qt::BlockingQueuedConnection);
}

void AndroidJniCamera::_release()
{
    m_previewSize = QSize();
    m_parametersMutex.lock();
    m_parameters = QAndroidJniObject();
    m_parametersMutex.unlock();
    if (m_camera.isValid())
        m_camera.callMethod<void>("release");
}

AndroidJniCamera::CameraFacing AndroidJniCamera::getFacing()
{
    return CameraFacing(m_info.getField<jint>("facing"));
}

int AndroidJniCamera::getNativeOrientation()
{
    return m_info.getField<jint>("orientation");
}

QSize AndroidJniCamera::getPreferredPreviewSizeForVideo()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return QSize();

    QAndroidJniObject size = m_parameters.callObjectMethod("getPreferredPreviewSizeForVideo",
                                                           "()Landroid/hardware/Camera$Size;");

    if (!size.isValid())
        return QSize();

    return QSize(size.getField<jint>("width"), size.getField<jint>("height"));
}

QList<QSize> AndroidJniCamera::getSupportedPreviewSizes()
{
    QList<QSize> list;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (m_parameters.isValid()) {
        QAndroidJniObject sizeList = m_parameters.callObjectMethod("getSupportedPreviewSizes",
                                                                   "()Ljava/util/List;");
        int count = sizeList.callMethod<jint>("size");
        for (int i = 0; i < count; ++i) {
            QAndroidJniObject size = sizeList.callObjectMethod("get",
                                                               "(I)Ljava/lang/Object;",
                                                               i);
            list.append(QSize(size.getField<jint>("width"), size.getField<jint>("height")));
        }

        qSort(list.begin(), list.end(), [](const QSize& a, const QSize& b) {
                return (a.width() * a.height()) < (b.width() * b.height()); });
    }

    return list;
}

AndroidJniCamera::ImageFormat AndroidJniCamera::getPreviewFormat()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return AndroidJniCamera::Unknown;

    return AndroidJniCamera::ImageFormat(m_parameters.callMethod<jint>("getPreviewFormat"));
}

void AndroidJniCamera::setPreviewFormat(ImageFormat fmt)
{
    QMetaObject::invokeMethod(this, "_setPreviewFormat", Q_ARG(AndroidJniCamera::ImageFormat, fmt));
}

void AndroidJniCamera::_setPreviewFormat(ImageFormat fmt)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setPreviewFormat", "(I)V", jint(fmt));
    _applyParameters();
}

QSize AndroidJniCamera::previewSize() const
{
    return m_previewSize;
}

void AndroidJniCamera::setPreviewSize(const QSize &size)
{
    m_parametersMutex.lock();
    bool areParametersValid = m_parameters.isValid();
    m_parametersMutex.unlock();
    if (!areParametersValid)
        return;

    m_previewSize = size;
    QMetaObject::invokeMethod(this, "_updatePreviewSize");
}

void AndroidJniCamera::_updatePreviewSize()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (m_previewSize.isValid()) {
        m_parameters.callMethod<void>("setPreviewSize", "(II)V", m_previewSize.width(), m_previewSize.height());
        _applyParameters();
    }

    emit previewSizeChanged();
}

bool AndroidJniCamera::setPreviewTexture(AndroidJniSurfaceTexture* surfaceTexture)
{
    bool ok = true;
    QMetaObject::invokeMethod(this,
                              "_setPreviewTexture",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(bool, ok),
                              Q_ARG(void *, surfaceTexture ? surfaceTexture->surfaceTexture() : 0));
    return ok;
}

bool AndroidJniCamera::_setPreviewTexture(void* surfaceTexture)
{
    QAndroidJniEnvironment env;
    m_camera.callMethod<void>("setPreviewTexture",
                              "(Landroid/graphics/SurfaceTexture;)V",
                              static_cast<jobject>(surfaceTexture));
    return !_exceptionCheckAndClear(env);
}

bool AndroidJniCamera::isZoomSupported()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return false;

    return m_parameters.callMethod<jboolean>("isZoomSupported");
}

int AndroidJniCamera::getMaxZoom()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return 0;

    return m_parameters.callMethod<jint>("getMaxZoom");
}

QList<int> AndroidJniCamera::getZoomRatios()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    QList<int> ratios;

    if (m_parameters.isValid()) {
        QAndroidJniObject ratioList = m_parameters.callObjectMethod("getZoomRatios",
                                                                    "()Ljava/util/List;");
        int count = ratioList.callMethod<jint>("size");
        for (int i = 0; i < count; ++i) {
            QAndroidJniObject zoomRatio = ratioList.callObjectMethod("get",
                                                                     "(I)Ljava/lang/Object;",
                                                                     i);

            ratios.append(zoomRatio.callMethod<jint>("intValue"));
        }
    }

    return ratios;
}

int AndroidJniCamera::getZoom()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return 0;

    return m_parameters.callMethod<jint>("getZoom");
}

void AndroidJniCamera::setZoom(int value)
{
    QMetaObject::invokeMethod(this, "_setZoom", Q_ARG(int, value));
}

void AndroidJniCamera::_setZoom(int value)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setZoom", "(I)V", value);
    _applyParameters();
}

QStringList AndroidJniCamera::_callParametersStringListMethod(const QByteArray& methodName)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    QStringList stringList;

    if (m_parameters.isValid()) {
        QAndroidJniObject list = m_parameters.callObjectMethod(methodName.constData(),
                                                               "()Ljava/util/List;");

        if (list.isValid()) {
            int count = list.callMethod<jint>("size");
            for (int i = 0; i < count; ++i) {
                QAndroidJniObject string = list.callObjectMethod("get",
                                                                 "(I)Ljava/lang/Object;",
                                                                 i);
                stringList.append(string.toString());
            }
        }
    }

    return stringList;
}

QStringList AndroidJniCamera::getSupportedFlashModes()
{
    return _callParametersStringListMethod("getSupportedFlashModes");
}

QString AndroidJniCamera::getFlashMode()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    QString value;

    if (m_parameters.isValid()) {
        QAndroidJniObject flashMode = m_parameters.callObjectMethod("getFlashMode",
                                                                    "()Ljava/lang/String;");
        if (flashMode.isValid())
            value = flashMode.toString();
    }

    return value;
}

void AndroidJniCamera::setFlashMode(const QString& value)
{
    QMetaObject::invokeMethod(this, "_setFlashMode", Q_ARG(QString, value));
}

void AndroidJniCamera::_setFlashMode(const QString& value)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setFlashMode",
                                  "(Ljava/lang/String;)V",
                                  QAndroidJniObject::fromString(value).object());
    _applyParameters();
}

QStringList AndroidJniCamera::getSupportedFocusModes()
{
    return _callParametersStringListMethod("getSupportedFocusModes");
}

QString AndroidJniCamera::getFocusMode()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    QString value;

    if (m_parameters.isValid()) {
        QAndroidJniObject focusMode = m_parameters.callObjectMethod("getFocusMode",
                                                                    "()Ljava/lang/String;");
        if (focusMode.isValid())
            value = focusMode.toString();
    }

    return value;
}

void AndroidJniCamera::setFocusMode(const QString& value)
{
    QMetaObject::invokeMethod(this, "_setFocusMode", Q_ARG(QString, value));
}

void AndroidJniCamera::_setFocusMode(const QString& value)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setFocusMode",
                                  "(Ljava/lang/String;)V",
                                  QAndroidJniObject::fromString(value).object());
    _applyParameters();
}

int AndroidJniCamera::getMaxNumFocusAreas()
{
    if (QtAndroid::androidSdkVersion() < 14)
        return 0;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return 0;

    return m_parameters.callMethod<jint>("getMaxNumFocusAreas");
}

QList<QRect> AndroidJniCamera::getFocusAreas()
{
    QList<QRect> areas;

    if (QtAndroid::androidSdkVersion() < 14)
        return areas;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (m_parameters.isValid()) {
        QAndroidJniObject list = m_parameters.callObjectMethod("getFocusAreas",
                                                               "()Ljava/util/List;");

        if (list.isValid()) {
            int count = list.callMethod<jint>("size");
            for (int i = 0; i < count; ++i) {
                QAndroidJniObject area = list.callObjectMethod("get",
                                                               "(I)Ljava/lang/Object;",
                                                               i);

                areas.append(_areaToRect(area.object()));
            }
        }
    }

    return areas;
}

void AndroidJniCamera::setFocusAreas(const QList<QRect>& areas)
{
    QMetaObject::invokeMethod(this, "_setFocusAreas", Q_ARG(QList<QRect>, areas));
}

void AndroidJniCamera::_setFocusAreas(const QList<QRect>& areas)
{
    if (QtAndroid::androidSdkVersion() < 14)
        return;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    QAndroidJniObject list;

    if (!areas.isEmpty()) {
        QAndroidJniEnvironment env;
        QAndroidJniObject arrayList("java/util/ArrayList", "(I)V", areas.size());
        for (int i = 0; i < areas.size(); ++i) {
            arrayList.callMethod<jboolean>("add",
                                           "(Ljava/lang/Object;)Z",
                                           _rectToArea(areas.at(i)).object());
            _exceptionCheckAndClear(env);
        }
        list = arrayList;
    }

    m_parameters.callMethod<void>("setFocusAreas", "(Ljava/util/List;)V", list.object());

    _applyParameters();
}

void AndroidJniCamera::autoFocus()
{
    QMetaObject::invokeMethod(this, "_autoFocus");
}


void AndroidJniCamera::_autoFocus()
{
    m_camera.callMethod<void>("autoFocus",
                              "(Landroid/hardware/Camera$AutoFocusCallback;)V",
                              m_cameraListener.object());
    emit autoFocusStarted();
}

void AndroidJniCamera::cancelAutoFocus()
{
    QMetaObject::invokeMethod(this, "_cancelAutoFocus", Qt::QueuedConnection);
}

void AndroidJniCamera::_cancelAutoFocus()
{
    m_camera.callMethod<void>("cancelAutoFocus");
}

bool AndroidJniCamera::isAutoExposureLockSupported()
{
    if (QtAndroid::androidSdkVersion() < 14)
        return false;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return false;

    return m_parameters.callMethod<jboolean>("isAutoExposureLockSupported");
}

bool AndroidJniCamera::getAutoExposureLock()
{
    if (QtAndroid::androidSdkVersion() < 14)
        return false;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return false;

    return m_parameters.callMethod<jboolean>("getAutoExposureLock");
}

void AndroidJniCamera::setAutoExposureLock(bool toggle)
{
    QMetaObject::invokeMethod(this, "_setAutoExposureLock", Q_ARG(bool, toggle));
}

void AndroidJniCamera::_setAutoExposureLock(bool toggle)
{
    if (QtAndroid::androidSdkVersion() < 14)
        return;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setAutoExposureLock", "(Z)V", toggle);
    _applyParameters();
}

bool AndroidJniCamera::isAutoWhiteBalanceLockSupported()
{
    if (QtAndroid::androidSdkVersion() < 14)
        return false;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return false;

    return m_parameters.callMethod<jboolean>("isAutoWhiteBalanceLockSupported");
}

bool AndroidJniCamera::getAutoWhiteBalanceLock()
{
    if (QtAndroid::androidSdkVersion() < 14)
        return false;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return false;

    return m_parameters.callMethod<jboolean>("getAutoWhiteBalanceLock");
}

void AndroidJniCamera::setAutoWhiteBalanceLock(bool toggle)
{
    QMetaObject::invokeMethod(this, "_setAutoWhiteBalanceLock", Q_ARG(bool, toggle));
}

void AndroidJniCamera::_setAutoWhiteBalanceLock(bool toggle)
{
    if (QtAndroid::androidSdkVersion() < 14)
        return;

    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setAutoWhiteBalanceLock", "(Z)V", toggle);
    _applyParameters();
}

int AndroidJniCamera::getExposureCompensation()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return 0;

    return m_parameters.callMethod<jint>("getExposureCompensation");
}

void AndroidJniCamera::setExposureCompensation(int value)
{
    QMetaObject::invokeMethod(this, "_setExposureCompensation", Q_ARG(int, value));
}

void AndroidJniCamera::_setExposureCompensation(int value)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setExposureCompensation", "(I)V", value);
    _applyParameters();
}

float AndroidJniCamera::getExposureCompensationStep()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return 0;

    return m_parameters.callMethod<jfloat>("getExposureCompensationStep");
}

int AndroidJniCamera::getMinExposureCompensation()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return 0;

    return m_parameters.callMethod<jint>("getMinExposureCompensation");
}

int AndroidJniCamera::getMaxExposureCompensation()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return 0;

    return m_parameters.callMethod<jint>("getMaxExposureCompensation");
}

QStringList AndroidJniCamera::getSupportedSceneModes()
{
    return _callParametersStringListMethod("getSupportedSceneModes");
}

QString AndroidJniCamera::getSceneMode()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    QString value;

    if (m_parameters.isValid()) {
        QAndroidJniObject sceneMode = m_parameters.callObjectMethod("getSceneMode",
                                                                    "()Ljava/lang/String;");
        if (sceneMode.isValid())
            value = sceneMode.toString();
    }

    return value;
}

void AndroidJniCamera::setSceneMode(const QString& value)
{
    QMetaObject::invokeMethod(this, "_setSceneMode", Q_ARG(QString, value));
}

void AndroidJniCamera::_setSceneMode(const QString& value)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setSceneMode",
                                  "(Ljava/lang/String;)V",
                                  QAndroidJniObject::fromString(value).object());
    _applyParameters();
}

QStringList AndroidJniCamera::getSupportedWhiteBalance()
{
    return _callParametersStringListMethod("getSupportedWhiteBalance");
}

QString AndroidJniCamera::getWhiteBalance()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    QString value;

    if (m_parameters.isValid()) {
        QAndroidJniObject wb = m_parameters.callObjectMethod("getWhiteBalance",
                                                             "()Ljava/lang/String;");
        if (wb.isValid())
            value = wb.toString();
    }

    return value;
}

void AndroidJniCamera::setWhiteBalance(const QString& value)
{
    QMetaObject::invokeMethod(this, "_setWhiteBalance", Q_ARG(QString, value));
}

void AndroidJniCamera::_setWhiteBalance(const QString& value)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setWhiteBalance",
                                  "(Ljava/lang/String;)V",
                                  QAndroidJniObject::fromString(value).object());
    _applyParameters();

    emit whiteBalanceChanged();
}


void AndroidJniCamera::setRotation(int rotation)
{
    m_parametersMutex.lock();
    bool areParametersValid = m_parameters.isValid();
    m_parametersMutex.unlock();
    if (!areParametersValid)
        return;

    m_rotation = rotation;
    QMetaObject::invokeMethod(this, "_updateRotation");
}

void AndroidJniCamera::_updateRotation()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    m_parameters.callMethod<void>("setRotation", "(I)V", m_rotation);
    _applyParameters();
}

int AndroidJniCamera::getRotation() const
{
    return m_rotation;
}

QList<QSize> AndroidJniCamera::getSupportedPictureSizes()
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    QList<QSize> list;

    if (m_parameters.isValid()) {
        QAndroidJniObject sizeList = m_parameters.callObjectMethod("getSupportedPictureSizes",
                                                                   "()Ljava/util/List;");
        int count = sizeList.callMethod<jint>("size");
        for (int i = 0; i < count; ++i) {
            QAndroidJniObject size = sizeList.callObjectMethod("get",
                                                               "(I)Ljava/lang/Object;",
                                                               i);
            list.append(QSize(size.getField<jint>("width"), size.getField<jint>("height")));
        }

        qSort(list.begin(), list.end(), [](const QSize& a, const QSize& b) {
            return (a.width() * a.height()) < (b.width() * b.height()); });
    }

    return list;
}

void AndroidJniCamera::setPictureSize(const QSize& size)
{
    QMetaObject::invokeMethod(this, "_setPictureSize", Q_ARG(QSize, size));
}

void AndroidJniCamera::_setPictureSize(const QSize& size)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setPictureSize", "(II)V", size.width(), size.height());
    _applyParameters();
}

void AndroidJniCamera::setJpegQuality(int quality)
{
    QMetaObject::invokeMethod(this, "_setJpegQuality", Q_ARG(int, quality));
}

void AndroidJniCamera::_setJpegQuality(int quality)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setJpegQuality", "(I)V", quality);
    _applyParameters();
}

void AndroidJniCamera::takePicture()
{
    QMetaObject::invokeMethod(this, "_takePicture", Qt::BlockingQueuedConnection);
}

void AndroidJniCamera::_takePicture()
{
    m_camera.callMethod<void>("takePicture", "(Landroid/hardware/Camera$ShutterCallback;"
                                             "Landroid/hardware/Camera$PictureCallback;"
                                             "Landroid/hardware/Camera$PictureCallback;)V",
                                              m_cameraListener.object(),
                                              jobject(0),
                                              m_cameraListener.object());
}

void AndroidJniCamera::fetchEachFrame(bool fetch)
{
    QMetaObject::invokeMethod(this, "_fetchEachFrame", Q_ARG(bool, fetch));
}

void AndroidJniCamera::_fetchEachFrame(bool fetch)
{
    m_cameraListener.callMethod<void>("fetchEachFrame", "(Z)V", fetch);
}

void AndroidJniCamera::fetchLastPreviewFrame()
{
    QMetaObject::invokeMethod(this, "_fetchLastPreviewFrame");
}

void AndroidJniCamera::_fetchLastPreviewFrame()
{
    QAndroidJniEnvironment env;
    QAndroidJniObject data = m_cameraListener.callObjectMethod("lockAndFetchPreviewBuffer", "()[B");
    if (!data.isValid()) {
        m_cameraListener.callMethod<void>("unlockPreviewBuffer");
        return;
    }
    const int arrayLength = env->GetArrayLength(static_cast<jbyteArray>(data.object()));
    QByteArray bytes(arrayLength, Qt::Uninitialized);
    env->GetByteArrayRegion(static_cast<jbyteArray>(data.object()),
                            0,
                            arrayLength,
                            reinterpret_cast<jbyte *>(bytes.data()));
    m_cameraListener.callMethod<void>("unlockPreviewBuffer");

    emit previewFetched(bytes);
}

QAndroidJniObject AndroidJniCamera::getCameraObject()
{
    return m_camera;
}

int AndroidJniCamera::getNumberOfCameras()
{
    return QAndroidJniObject::callStaticMethod<jint>("android/hardware/Camera",
                                                     "getNumberOfCameras");
}

void AndroidJniCamera::getCameraInfo(int id, AndroidCameraInfo* info)
{
    Q_ASSERT(info);

    QAndroidJniObject cameraInfo("android/hardware/Camera$CameraInfo");
    QAndroidJniObject::callStaticMethod<void>("android/hardware/Camera",
                                              "getCameraInfo",
                                              "(ILandroid/hardware/Camera$CameraInfo;)V",
                                              id, cameraInfo.object());

    AndroidJniCamera::CameraFacing facing = AndroidJniCamera::CameraFacing(cameraInfo.getField<jint>("facing"));
    // The orientation provided by Android is counter-clockwise, we need it clockwise
    info->orientation = (360 - cameraInfo.getField<jint>("orientation")) % 360;

    switch (facing) {
    case AndroidJniCamera::CameraFacingBack:
        info->name = QByteArray("back");
        info->description = QStringLiteral("Rear-facing camera");
        info->position = QCamera::BackFace;
        break;
    case AndroidJniCamera::CameraFacingFront:
        info->name = QByteArray("front");
        info->description = QStringLiteral("Front-facing camera");
        info->position = QCamera::FrontFace;
        break;
    default:
        break;
    }
}

void AndroidJniCamera::setRecordingHint(bool hint)
{
    if (QtAndroid::androidSdkVersion() < 14)
        return;

    QMetaObject::invokeMethod(this, "_setRecordingHint", Q_ARG(bool, hint));
}

void AndroidJniCamera::_setRecordingHint(bool hint)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("setRecordingHint", "(Z)V", hint);
    _applyParameters();
}

QString AndroidJniCamera::get(const QString& key)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    QString value;

    if (m_parameters.isValid()) {
        QAndroidJniObject wb = m_parameters.callObjectMethod("get",
                                                             "(Ljava/lang/String;)Ljava/lang/String;",
                                                             QAndroidJniObject::fromString(key).object());
        if (wb.isValid())
            value = wb.toString();
    }

    return value;
}

void AndroidJniCamera::set(const QString& key, const QString& value)
{
    QMetaObject::invokeMethod(this, "_set", Q_ARG(QString, key), Q_ARG(QString, value));
}

void AndroidJniCamera::_set(const QString& key, const QString& value)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("set", "(Ljava/lang/String;Ljava/lang/String;)V",
                                  QAndroidJniObject::fromString(key).object(),
                                  QAndroidJniObject::fromString(value).object());
    _applyParameters();
}

int AndroidJniCamera::getInt(const QString& key)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    int value = 0;

    if (m_parameters.isValid()) {
        value = m_parameters.callMethod<jint>("getInt", "(Ljava/lang/String;)I;",
                                              QAndroidJniObject::fromString(key).object());
    }

    return value;
}

void AndroidJniCamera::set(const QString& key, int value)
{
    QMetaObject::invokeMethod(this, "_set", Q_ARG(QString, key), Q_ARG(int, value));
}

void AndroidJniCamera::_set(const QString& key, int value)
{
    QMutexLocker parametersLocker(&m_parametersMutex);

    if (!m_parameters.isValid())
        return;

    m_parameters.callMethod<void>("set", "(Ljava/lang/String;I)V",
                                  QAndroidJniObject::fromString(key).object(), value);
    _applyParameters();
}

void AndroidJniCamera::startPreview()
{
    QMetaObject::invokeMethod(this, "_startPreview");
}

void AndroidJniCamera::_startPreview()
{
    //We need to clear preview buffers queue here, but there is no method to do it
    //Though just resetting preview callback do the trick
    m_camera.callMethod<void>("setPreviewCallbackWithBuffer",
                              "(Landroid/hardware/Camera$PreviewCallback;)V",
                              jobject(0));
    m_cameraListener.callMethod<void>("preparePreviewBuffer", "(Landroid/hardware/Camera;)V", m_camera.object());
    QAndroidJniObject buffer = m_cameraListener.callObjectMethod<jbyteArray>("callbackBuffer");
    m_camera.callMethod<void>("addCallbackBuffer", "([B)V", buffer.object());
    m_camera.callMethod<void>("setPreviewCallbackWithBuffer",
                              "(Landroid/hardware/Camera$PreviewCallback;)V",
                              m_cameraListener.object());
    m_camera.callMethod<void>("startPreview");
    emit previewStarted();
}

void AndroidJniCamera::stopPreview()
{
    QMetaObject::invokeMethod(this, "_stopPreview");
}

void AndroidJniCamera::_stopPreview()
{
    m_camera.callMethod<void>("stopPreview");
    emit previewStopped();
}

void AndroidJniCamera::_applyParameters()
{
    QAndroidJniEnvironment env;
    m_camera.callMethod<void>("setParameters",
                              "(Landroid/hardware/Camera$Parameters;)V",
                              m_parameters.object());
    _exceptionCheckAndClear(env);
}

QMap<int, AndroidJniCamera*> AndroidJniCamera::_global_cameraMap;
QMutex AndroidJniCamera::_global_cameraMapMutex;

inline bool AndroidJniCamera::_exceptionCheckAndClear(QAndroidJniEnvironment& env)
{
    if (Q_UNLIKELY(env->ExceptionCheck())) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }

    return false;
}

QRect AndroidJniCamera::_areaToRect(jobject areaObj)
{
    QAndroidJniObject area(areaObj);
    QAndroidJniObject rect = area.getObjectField("rect", "Landroid/graphics/Rect;");

    return QRect(rect.getField<jint>("left"),
                 rect.getField<jint>("top"),
                 rect.callMethod<jint>("width"),
                 rect.callMethod<jint>("height"));
}

QAndroidJniObject AndroidJniCamera::_rectToArea(const QRect& rect)
{
    QAndroidJniObject jrect("android/graphics/Rect",
                     "(IIII)V",
                     rect.left(), rect.top(), rect.right(), rect.bottom());

    QAndroidJniObject area("android/hardware/Camera$Area",
                    "(Landroid/graphics/Rect;I)V",
                    jrect.object(), 500);

    return area;
}

void AndroidJniCamera::_notifyAutoFocusComplete(JNIEnv* , jobject, int id, jboolean success)
{
    QMutexLocker locker(&_global_cameraMapMutex);
    AndroidJniCamera* obj = _global_cameraMap.value(id, 0);
    if (obj)
        emit obj->autoFocusComplete(success);
}

void AndroidJniCamera::_notifyPictureExposed(JNIEnv* , jobject, int id)
{
    QMutexLocker locker(&_global_cameraMapMutex);
    AndroidJniCamera* obj = _global_cameraMap.value(id, 0);
    if (obj)
        emit obj->pictureExposed();
}

void AndroidJniCamera::_notifyPictureCaptured(JNIEnv* env, jobject, int id, jbyteArray data)
{
    QMutexLocker locker(&_global_cameraMapMutex);
    AndroidJniCamera* obj = _global_cameraMap.value(id, 0);
    if (obj) {
        const int arrayLength = env->GetArrayLength(data);
        QByteArray bytes(arrayLength, Qt::Uninitialized);
        env->GetByteArrayRegion(data, 0, arrayLength, (jbyte*)bytes.data());
        emit obj->pictureCaptured(bytes);
    }
}

void AndroidJniCamera::_notifyFrameFetched(JNIEnv* env, jobject, int id, jbyteArray data)
{
    QMutexLocker locker(&_global_cameraMapMutex);
    AndroidJniCamera* obj = _global_cameraMap.value(id, 0);
    if (obj) {
        const int arrayLength = env->GetArrayLength(data);
        QByteArray bytes(arrayLength, Qt::Uninitialized);
        env->GetByteArrayRegion(data, 0, arrayLength, (jbyte*)bytes.data());

        emit obj->frameFetched(bytes);
    }
}

bool AndroidJniCamera::initJNI(JNIEnv* env)
{
    jclass clazz = env->FindClass("ru/qtproject/artechproject86/ARCameraListener");

    if (clazz == 0) {
        return false;
    }

    static const JNINativeMethod methods[] = {
        {"notifyAutoFocusComplete", "(IZ)V", (void *)_notifyAutoFocusComplete},
        {"notifyPictureExposed", "(I)V", (void *)_notifyPictureExposed},
        {"notifyPictureCaptured", "(I[B)V", (void *)_notifyPictureCaptured},
        {"notifyFrameFetched", "(I[B)V", (void *)_notifyFrameFetched}
    };

    if (env->RegisterNatives(clazz, methods,
                             sizeof(methods) / sizeof(methods[0])) != JNI_OK) {
        return false;
    }

    return true;
}
