#ifndef ANDROIDCAMERA_H
#define ANDROIDCAMERA_H

#if defined(QT_ANDROIDEXTRAS_LIB)

#include <QObject>
#include <QSize>
#include <QRect>
#include <QCamera>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <jni.h>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include "AndroidJniSurfaceTexture.h"

class AndroidJniCamera : public QObject
{
    Q_OBJECT
    Q_ENUMS(CameraFacing)
    Q_ENUMS(ImageFormat)

public:
    struct AndroidCameraInfo
    {
        QByteArray name;
        QString description;
        QCamera::Position position;
        int orientation;
    };

    enum CameraFacing {
        CameraFacingBack = 0,
        CameraFacingFront = 1
    };

    enum ImageFormat { // same values as in android.graphics.ImageFormat Java class
        Unknown = 0,
        RGB565 = 4,
        NV16 = 16,
        NV21 = 17,
        YUY2 = 20,
        JPEG = 256,
        YV12 = 842094169
    };

    ~AndroidJniCamera();

    static AndroidJniCamera* open(int cameraId);

    int cameraId() const;

    bool lock();
    bool unlock();
    bool reconnect();
    void release();

    CameraFacing getFacing();
    int getNativeOrientation();

    QSize getPreferredPreviewSizeForVideo();
    QList<QSize> getSupportedPreviewSizes();

    ImageFormat getPreviewFormat();
    void setPreviewFormat(ImageFormat fmt);

    QSize previewSize() const;
    void setPreviewSize(const QSize& size);
    bool setPreviewTexture(AndroidJniSurfaceTexture* surfaceTexture);

    bool isZoomSupported();
    int getMaxZoom();
    QList<int> getZoomRatios();
    int getZoom();
    void setZoom(int value);

    QStringList getSupportedFlashModes();
    QString getFlashMode();
    void setFlashMode(const QString& value);

    QStringList getSupportedFocusModes();
    QString getFocusMode();
    void setFocusMode(const QString& value);

    int getMaxNumFocusAreas();
    QList<QRect> getFocusAreas();
    void setFocusAreas(const QList<QRect>& areas);

    void autoFocus();
    void cancelAutoFocus();

    bool isAutoExposureLockSupported();
    bool getAutoExposureLock();
    void setAutoExposureLock(bool toggle);

    bool isAutoWhiteBalanceLockSupported();
    bool getAutoWhiteBalanceLock();
    void setAutoWhiteBalanceLock(bool toggle);

    int getExposureCompensation();
    void setExposureCompensation(int value);
    float getExposureCompensationStep();
    int getMinExposureCompensation();
    int getMaxExposureCompensation();

    QStringList getSupportedSceneModes();
    QString getSceneMode();
    void setSceneMode(const QString& value);

    QStringList getSupportedWhiteBalance();
    QString getWhiteBalance();
    void setWhiteBalance(const QString& value);

    int getRotation() const;
    void setRotation(int rotation);

    QList<QSize> getSupportedPictureSizes();
    void setPictureSize(const QSize& size);
    void setJpegQuality(int quality);

    void setRecordingHint(bool hint);

    QString get(const QString& key);
    void set(const QString& key, const QString& value);

    int getInt(const QString& key);
    void set(const QString& key, int value);

    void startPreview();
    void stopPreview();

    void takePicture();

    void fetchEachFrame(bool fetch);
    void fetchLastPreviewFrame();
    QAndroidJniObject getCameraObject();

    static int getNumberOfCameras();
    static void getCameraInfo(int id, AndroidCameraInfo* info);

    static bool initJNI(JNIEnv* env);

signals:
    void previewSizeChanged();
    void previewStarted();
    void previewStopped();

    void autoFocusStarted();
    void autoFocusComplete(bool success);

    void whiteBalanceChanged();

    void pictureExposed();
    void pictureCaptured(const QByteArray& data);
    void previewFetched(const QByteArray& preview);
    void frameFetched(const QByteArray& frame);

private:
    AndroidJniCamera(QThread* worker);
    QScopedPointer<QThread> m_worker;
    int m_cameraId;
    QMutex m_parametersMutex;
    QSize m_previewSize;
    int m_rotation;
    QAndroidJniObject m_info;
    QAndroidJniObject m_parameters;
    QAndroidJniObject m_camera;
    QAndroidJniObject m_cameraListener;

private slots:
    bool _init(int cameraId);
    bool _lock();
    bool _unlock();
    bool _reconnect();
    void _release();
    void _setPreviewFormat(ImageFormat fmt);
    void _updatePreviewSize();
    bool _setPreviewTexture(void* surfaceTexture);
    void _setZoom(int value);
    QStringList _callParametersStringListMethod(const QByteArray& methodName);
    void _setFlashMode(const QString& value);
    void _setFocusMode(const QString& value);
    void _setFocusAreas(const QList<QRect>& areas);
    void _autoFocus();
    void _cancelAutoFocus();
    void _setAutoExposureLock(bool toggle);
    void _setAutoWhiteBalanceLock(bool toggle);
    void _setExposureCompensation(int value);
    void _setSceneMode(const QString& value);
    void _setWhiteBalance(const QString& value);
    void _updateRotation();
    void _setPictureSize(const QSize& size);
    void _setJpegQuality(int quality);
    void _takePicture();
    void _fetchEachFrame(bool fetch);
    void _fetchLastPreviewFrame();
    void _setRecordingHint(bool hint);
    void _set(const QString& key, const QString& value);
    void _set(const QString& key, int value);
    void _startPreview();
    void _stopPreview();
    void _applyParameters();

private:
    static QMap<int, AndroidJniCamera*> _global_cameraMap;
    static QMutex _global_cameraMapMutex;
    static inline bool _exceptionCheckAndClear(QAndroidJniEnvironment& env);
    static QRect _areaToRect(jobject areaObj);
    static QAndroidJniObject _rectToArea(const QRect& rect);
    static void _notifyAutoFocusComplete(JNIEnv* , jobject, int id, jboolean success);
    static void _notifyPictureExposed(JNIEnv* , jobject, int id);
    static void _notifyPictureCaptured(JNIEnv* env, jobject, int id, jbyteArray data);
    static void _notifyFrameFetched(JNIEnv* env, jobject, int id, jbyteArray data);
};

#endif

#endif // ANDROIDCAMERA_H
