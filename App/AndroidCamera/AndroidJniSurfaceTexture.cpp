#include "AndroidJniSurfaceTexture.h"

#if defined(QT_ANDROIDEXTRAS_LIB)

#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include <jni.h>
#include <QMap>

AndroidJniSurfaceTexture::AndroidJniSurfaceTexture(int textureId)
    : QObject()
    , m_texID(textureId)
{
    // API level 11 or higher is required
    if (QtAndroid::androidSdkVersion() < 11) {
        qWarning("Camera preview and video playback require Android 3.0 (API level 11) or later.");
        return;
    }

    QAndroidJniEnvironment env;
    m_surfaceTexture = QAndroidJniObject("android/graphics/SurfaceTexture", "(I)V", jint(textureId));
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    if (m_surfaceTexture.isValid())
        _global_objectMap.insert(textureId, this);

    QAndroidJniObject listener("ru/qtproject/artechproject86/ARSurfaceTextureListener", "(I)V", jint(textureId));
    m_surfaceTexture.callMethod<void>("setOnFrameAvailableListener",
                                      "(Landroid/graphics/SurfaceTexture$OnFrameAvailableListener;)V",
                                      listener.object());
}

AndroidJniSurfaceTexture::~AndroidJniSurfaceTexture()
{
    if (QtAndroid::androidSdkVersion() > 13 && m_surfaceView.isValid())
        m_surfaceView.callMethod<void>("release");

    if (m_surfaceTexture.isValid()) {
        release();
        _global_objectMap.remove(m_texID);
    }
}

QMatrix4x4 AndroidJniSurfaceTexture::getTransformMatrix()
{
    QMatrix4x4 matrix;
    if (!m_surfaceTexture.isValid())
        return matrix;

    QAndroidJniEnvironment env;

    jfloatArray array = env->NewFloatArray(16);
    m_surfaceTexture.callMethod<void>("getTransformMatrix", "([F)V", array);
    env->GetFloatArrayRegion(array, 0, 16, matrix.data());
    env->DeleteLocalRef(array);

    return matrix;
}

void AndroidJniSurfaceTexture::release()
{
    if (QtAndroid::androidSdkVersion() < 14)
        return;

    m_surfaceTexture.callMethod<void>("release");
}

void AndroidJniSurfaceTexture::updateTexImage()
{
    if (!m_surfaceTexture.isValid())
        return;

    m_surfaceTexture.callMethod<void>("updateTexImage");
}

jobject AndroidJniSurfaceTexture::surfaceTexture()
{
    return m_surfaceTexture.object();
}

jobject AndroidJniSurfaceTexture::surfaceView()
{
    return m_surfaceView.object();
}

jobject AndroidJniSurfaceTexture::surfaceHolder()
{
    if (!m_surfaceHolder.isValid()) {
        m_surfaceView = QAndroidJniObject("android/view/Surface",
                                          "(Landroid/graphics/SurfaceTexture;)V",
                                          m_surfaceTexture.object());

        m_surfaceHolder = QAndroidJniObject("ru/qtproject/artechproject86/ARSurfaceTextureHolder",
                                            "(Landroid/view/Surface;)V",
                                            m_surfaceView.object());
    }

    return m_surfaceHolder.object();
}

QMap<int, AndroidJniSurfaceTexture*> AndroidJniSurfaceTexture::_global_objectMap;

void AndroidJniSurfaceTexture::_notifyFrameAvailable(JNIEnv* , jobject, int id)
{
    AndroidJniSurfaceTexture* obj = AndroidJniSurfaceTexture::_global_objectMap.value(id, 0);
    if (obj)
        emit obj->frameAvailable();
}

bool AndroidJniSurfaceTexture::initJNI(JNIEnv* env)
{
    // SurfaceTexture is available since API 11.
    if (QtAndroid::androidSdkVersion() < 11)
        return false;

    jclass clazz = env->FindClass("ru/qtproject/artechproject86/ARSurfaceTextureListener");

    if (clazz == 0) {
        return false;
    }

    static const JNINativeMethod methods[] = {
        {"notifyFrameAvailable", "(I)V", (void *)_notifyFrameAvailable}
    };

    if (env->RegisterNatives(clazz, methods,
                             sizeof(methods) / sizeof(methods[0])) != JNI_OK) {
        return false;
    }
    return true;
}

#endif
