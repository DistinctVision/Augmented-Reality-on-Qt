#ifndef ANDROIDSURFACETEXTURE_H
#define ANDROIDSURFACETEXTURE_H

#if defined(QT_ANDROIDEXTRAS_LIB)

#include <QtGui/qopengl.h>
#include <QObject>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>

#include <QMap>
#include <QMatrix4x4>

class AndroidJniSurfaceTexture : public QObject
{
    Q_OBJECT

public:
    explicit AndroidJniSurfaceTexture(int textureId);
    ~AndroidJniSurfaceTexture();

    int textureID() const { return m_texID; }
    jobject surfaceTexture();
    jobject surfaceView();
    jobject surfaceHolder();
    inline bool isValid() const { return m_surfaceTexture.isValid(); }

    QMatrix4x4 getTransformMatrix();
    void release();
    void updateTexImage();

    static bool initJNI(JNIEnv* env);

signals:
    void frameAvailable();

private:
    int m_texID;
    QAndroidJniObject m_surfaceTexture;
    QAndroidJniObject m_surfaceView;
    QAndroidJniObject m_surfaceHolder;

private:
    static QMap<int, AndroidJniSurfaceTexture*> _global_objectMap;
    static void _notifyFrameAvailable(JNIEnv* , jobject , int id);
};

#endif

#endif // ANDROIDSURFACETEXTURE_H
