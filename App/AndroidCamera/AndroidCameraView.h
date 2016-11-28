#ifndef ANDROIDCAMERAVIEW_H
#define ANDROIDCAMERAVIEW_H

#if defined(QT_ANDROIDEXTRAS_LIB)

#include <QSize>
#include <QRectF>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QQuickWindow>
#include <QOpenGLExtraFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QQuickItem>
#include <QString>
#include <QList>
#include <QCameraInfo>
#include <QMutex>
#include <QOpenGLShaderProgram>
#include "AndroidJniSurfaceTexture.h"
#include "AndroidJniCamera.h"

#include "App/ARFrameHandler.h"
#include "AR/Image.h"
#include "PixelsReader.h"

class AndroidCameraViewRenderer;

class AndroidCameraView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int selectedCameraId READ selectedCameraId WRITE setSelectedCameraId NOTIFY selectedCameraIdChanged)
    Q_PROPERTY(QSize targetPreviewSize READ targetPreviewSize WRITE setTargetPreviewSize NOTIFY targetPreviewSizeChanged)
    Q_PROPERTY(QSize previewSize READ previewSize NOTIFY previewSizeChanged)
    Q_PROPERTY(ARFrameHandler* arFrameHandler READ arFrameHandler WRITE setArFrameHandler NOTIFY arFrameHandlerChanged)

public:
    AndroidCameraView();
    ~AndroidCameraView();

    int selectedCameraId() const;
    void setSelectedCameraId(int selectedCameraId);

    AndroidJniCamera::AndroidCameraInfo getCameraInfo() const;

    Q_INVOKABLE QList<QString> getAviableCameras() const;

    QSize targetPreviewSize() const;
    void setTargetPreviewSize(const QSize& targetPreviewSize);

    QSize previewSize() const;

    AndroidJniSurfaceTexture* surfaceTexture() const;

    QRectF boundingRect() const override;

    bool cameraRunning() const;

    ARFrameHandler* arFrameHandler() const;
    void setArFrameHandler(ARFrameHandler* arFrameHandler);

    QMatrix3x3 surfaceTextureMatrix() const;

public slots:
    void sync();
    void cleanup();

signals:
    void selectedCameraIdChanged();
    void targetPreviewSizeChanged();
    void previewSizeChanged();
    void arFrameHandlerChanged();

private slots:
    void handleWindowChanged(QQuickWindow* win);

private:
    friend class AndroidCameraViewRenderer;

    mutable QMutex m_mutex;
    AndroidJniSurfaceTexture* m_surfaceTexture;
    QMatrix3x3 m_surfaceTextureMatrix;
    AndroidJniCamera* m_camera;
    int m_selectedCameraId;
    QSize m_targetPreviewSize;
    QSize m_previewSize;

    AndroidCameraViewRenderer* m_renderer;
    ARFrameHandler* m_arFrameHandler;

    void _updatePreviewSize();
    bool _startPreview(QOpenGLFunctions* glFunctions);
};

class AndroidCameraViewRenderer:
        public QObject,
        public QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    AndroidCameraViewRenderer(AndroidCameraView* parentView);
    ~AndroidCameraViewRenderer();

    ARFrameHandler* arFrameHandler() const;
    void setArFrameHandler(ARFrameHandler* arFrameHandler);

public slots:
    void render();

    void frameAvailable();

private:
    mutable QMutex m_mutex;

    AndroidCameraView* m_view;
    QOpenGLShaderProgram* m_shaderProgramRGB;
    QOpenGLShaderProgram* m_shaderProgramBW;
    bool m_frameAvailable;
    bool m_frameInitialized;

    ARFrameHandler* m_arFrameHandler;
    ARFrameHandlerRunnable* m_arFrameHandlerRunnable;
    QOpenGLFramebufferObject* m_frameBufferObject;

    PixelsReader* m_pixelsReader;

    void _initGL();

    static const GLfloat gl_vertex_data[8];
    static const GLfloat gl_texCoord_data[8];
};

#endif

#endif // ANDROIDCAMERAVIEW_H
