#ifndef ARFRAMEHANDLER_H
#define ARFRAMEHANDLER_H

#include "QScrollEngine/QScrollEngine.h"
#include "QScrollEngine/QScene.h"
#include <QAbstractVideoFilter>
#include <QAbstractVideoBuffer>
#include <QMutex>
#include <QtGui/qopengl.h>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QList>
#include <QVector2D>
#include <QMatrix3x3>
#include <QPair>
#include <QQueue>
#include <QStandardPaths>
#include <QQuickItem>
#include <QPluginLoader>
#include "AR/Image.h"
#include "TMath/TVector.h"

#include "ARScene.h"
#include "FrameReceiver.h"
#include "TextureRenderer.h"

class ARFrameHandlerRunnable;

class ARFrameHandler:
        public QAbstractVideoFilter
{
    Q_OBJECT

    Q_PROPERTY(QSize frameResolution READ frameResolution WRITE setFrameResolution NOTIFY frameResolutionChanged)
    Q_PROPERTY(QList<QObject*> scenes READ scenes WRITE setScenes NOTIFY scenesChanged)
    Q_PROPERTY(QList<QObject*> imageReceivers READ imageReceivers WRITE setImageReceivers
               NOTIFY imageReceiversChanged)
    Q_PROPERTY(QList<double> cameraParameters READ cameraParametersQml WRITE setCameraParametersQml NOTIFY cameraParametersChanged)
    Q_PROPERTY(bool fixedOpticalCenter READ fixedOpticalCenter WRITE setFixedOpticalCenter NOTIFY fixedOpticalCenterChanged)
    Q_PROPERTY(bool disabledRadialDistortion READ disabledRadialDistortion WRITE setDisabledRadialDistortion
               NOTIFY disabledRadialDistortionChanged)

public:
    QVideoFilterRunnable* createFilterRunnable();

    ARFrameHandler();
    ~ARFrameHandler();

public:
    QSize frameResolution() const;
    void setFrameResolution(const QSize& frameResolution);

    const QList<QObject*> scenes() const;
    void setScenes(const QList<QObject*>& scenes);

    const QList<QObject*> imageReceivers() const;
    void setImageReceivers(const QList<QObject*>& imageReceivers);

    TMath::TVectord cameraParameters() const;
    void setCameraParameters(const TMath::TVectord& parameters);

    QList<double> cameraParametersQml() const;
    void setCameraParametersQml(const QList<double>& parameters);

    bool fixedOpticalCenter() const;
    void setFixedOpticalCenter(bool enabled);

    bool disabledRadialDistortion() const;
    void setDisabledRadialDistortion(bool disabled);

    Q_INVOKABLE void pressed(int id, float x, float y);
    Q_INVOKABLE void released(int id);

signals:
    void frameResolutionChanged();
    void scenesChanged();
    void imageReceiversChanged();
    void cameraParametersChanged();
    void fixedOpticalCenterChanged();
    void disabledRadialDistortionChanged();

private:
    friend class ARFrameHandlerRunnable;

    mutable QMutex m_mutex;
    ARFrameHandlerRunnable* m_runnable;
    QSize m_frameResolution;
    QList<QObject*> m_scenes;
    QList<QObject*> m_imageReceivers;
    TMath::TVectord m_cameraParameters;
    TMath::TVector<bool> m_fixedCameraParamters;
};

class ARFrameHandlerRunnable:
        public QVideoFilterRunnable
{
public:
    ARFrameHandlerRunnable(ARFrameHandler* parent);
    ~ARFrameHandlerRunnable();
    QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat& surfaceFormat, RunFlags flags) override;

    ARFrameHandler* parentItem();

    void preDraw();
    void drawContent();
    void setFrameTexture(GLuint textureId, const QMatrix3x3& textureMatrix, bool egl);

    AR::Point2i getFrameSize(const AR::Point2i& baseSize) const;

    void sendFrameImage(const AR::ImageRef<AR::Rgba>& frameImage, const QMatrix3x3& textureMatrix,
                        const QVideoFrame::PixelFormat& pixelFormat);

private:
    ARFrameHandler* m_parentItem;
    QMutex m_mutex;
    QScrollEngine::QScrollEngineContext* m_context;
    QList<QPair<ARScene*, QScrollEngine::QScene*>> m_scenes;

    GLuint m_textureFrameId;
    bool m_eglTextureFrame;
    QMatrix3x3 m_textureMatrix;
    bool m_externTextureFrame;

    QOpenGLFramebufferObject* m_frameFBO;
    AR::Image<AR::Rgba> m_sourceImage;
    AR::Point2i m_sizeFrame;

    TextureRenderer m_textureRenderer;
    QOpenGLShaderProgram m_shaderBW;
    QOpenGLFramebufferObject* m_arFrameFbo;

    void _initializeShader();
    void _initialize(const AR::Point2i& size, bool externTexture, GLuint textureId);
    void _updateActions();
    void _updateScenesQml();
    void _drawFrame();
    void _drawTrackingResultOnGL() const;
    void _getImageFromTexture(AR::Image<AR::Rgba>& outImage, GLuint textureId);

};

class TextureVideoBuffer :
        public QAbstractVideoBuffer
{
public:
    TextureVideoBuffer(GLuint textureId)
        : QAbstractVideoBuffer(GLTextureHandle)
        , m_textureId(textureId)
    {}

    virtual ~TextureVideoBuffer() {}

    MapMode mapMode() const { return NotMapped; }
    uchar* map(MapMode, int*, int*) { return 0; }
    void unmap() {}

    QVariant handle() const
    {
        return QVariant::fromValue<unsigned int>(m_textureId);
    }

private:
    GLuint m_textureId;
};

#endif // ARFRAMEHANDLER_H
