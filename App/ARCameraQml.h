#ifndef QARCAMERAQML
#define QARCAMERAQML

#include "QScrollEngine/QScrollEngine.h"
#include "QScrollEngine/QScene.h"
#include <QQuickPaintedItem>
#include <QQueue>
#include <QPainter>
#include <QVector2D>
#include <QMatrix2x2>
#include <QQuaternion>
#include <QRectF>
#include <QMutex>
#include <QTimer>
#include <QEvent>
#include <QOpenGLTexture>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QVideoRendererControl>
#include <QtQuick/QSGNode>
#include <QOpenGLFramebufferObject>
#include <QMediaObject>
#include <QSGSimpleTextureNode>
#include <QStandardPaths>
#include "AR/FrameProvider.h"
#include "AR/ARSystem.h"
#include "AR/Reconstructor3D.h"

class VideoSurface_ForQQuickItem:
        public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit VideoSurface_ForQQuickItem(QQuickItem* parentQuickItem);
    ~VideoSurface_ForQQuickItem();

    bool present(const QVideoFrame& frame) override;
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType type=QAbstractVideoBuffer::NoHandle) const override;
    bool start(const QVideoSurfaceFormat& format) override;
    AR::FrameProvider* frameProvider() { return _frameProvider; }
    void setQScrollEngineContext(QScrollEngine::QScrollEngineContext* context);
    QScrollEngine::QScrollEngineContext* qScrollEngineContext() const { return _context; }
    bool isFrameAvailable() const { return _frameAvailable; }
    void provideFrame();

    void scheduleOpenGLContextUpdate();

    void setMatrixTexture(const QMatrix2x2& matrix);
    QMatrix2x2 matrixTexture() const { return _matrixTexture; }

private slots:
    void updateOpenGLContext();

private:
    QQuickItem* _parentQuickItem;
    QScrollEngine::QScrollEngineContext* _context;
    AR::FrameProvider* _frameProvider;
    QVideoFrame _frame;
    bool _frameAvailable;
    QMatrix2x2 _matrixTexture;
};

class SceneInterface:
        public QScrollEngine::QScene
{
public:
    virtual void beginUpdate() = 0;
    virtual void endUpdate() = 0;

    AR::FrameProvider* frameProvider() const { return _frameProvider; }
    void setFrameProvider(AR::FrameProvider* frameProvider) { _frameProvider = frameProvider; }
    AR::ARSystem* arSystem() const { return _arSystem; }
    void setARSystem(AR::ARSystem* arSystem) { _arSystem = arSystem; }

protected:
    AR::FrameProvider* _frameProvider;
    AR::ARSystem* _arSystem;
};

class ARCameraQml:
        public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(State)
    Q_ENUMS(TypeAction)

    Q_PROPERTY(QObject* mediaObject READ mediaObject WRITE setMediaObject NOTIFY mediaObjectChanged)
    Q_PROPERTY(QString textStatus READ textStatus NOTIFY textStatusChanged)
    Q_PROPERTY(float cameraVerticalAngle READ cameraVerticalAngle WRITE setCameraVerticalAngle NOTIFY cameraVerticalAngleChanged)
    Q_PROPERTY(State arCameraState READ arCameraState NOTIFY arCameraStateChanged)
    Q_PROPERTY(QVariant procentReconstruction READ procentReconstruction NOTIFY procentReconstructionChanged)
    Q_PROPERTY(QVariant haveReconstruct READ haveReconstruct NOTIFY haveReconstructChanged)
    Q_PROPERTY(bool haveLoadModel READ haveLoadModel NOTIFY haveLoadModelChanged)

public:
    enum State: int
    {
        NotTracking = AR::ARSystem::NotTracking,
        TrackingBegin = AR::ARSystem::TrackingBegin,
        TrackingNow = AR::ARSystem::TrackingNow,
        Reconstruction3D = AR::ARSystem::Reconstruction3D,
        LostTracking = AR::ARSystem::LostTracking,
        LostReconstruction3D = AR::ARSystem::LostReconstruction3D
    };

    enum TypeAction: int
    {
        NextState,
        FinishReconstruct,
        SaveEntity,
        LoadEntity,
        CreateScene,
        DeleteScene
    };

    static void declareQml()
    {
        qmlRegisterType<ARCameraQml>("ARCameraQml", 1, 0, "ARCameraItem");
    }

    explicit ARCameraQml(QQuickItem* parent = nullptr);
    ~ARCameraQml();

    QObject* mediaObject() const { return static_cast<QObject*>(_mediaObject); }
    void setMediaObject(QObject* object);
    void setMediaObject(QMediaObject* mediaObject);

    void setCameraVerticalAngle(float angle);
    float cameraVerticalAngle() const { return _cameraVerticalAngle; }

    State arCameraState() { return _currentState; }

    QVariant procentReconstruction() const { return QVariant(_procentReconstruction); }
    bool haveReconstruct() const;
    bool haveLoadModel() const;

public slots:
    void addAction(TypeAction action, QString parameters = "") { _queueActions.push_back(QPair<TypeAction, QString>(action, parameters));
                                                                 update(); }
    QString textStatus() const { return _textStatus; }

    void updateSize(const QRectF& rect);
    void updateOrientation(Qt::ScreenOrientation screenOrientation);
    void slotUpdateProcess(float procent);
    QString starndartWritableDir() const { return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation); }
    void finishReconstruct() { addAction(FinishReconstruct); }
Q_INVOKABLE
    int countScenes() const { return 5; }
Q_INVOKABLE
    QString sceneName(int index) const;
Q_INVOKABLE
    void createScene(int index) { addAction(CreateScene, QString::number(index)); }
Q_INVOKABLE
    void deleteScene() { addAction(DeleteScene); }

signals:
    void mediaObjectChanged();
    void textStatusChanged();
    void cameraVerticalAngleChanged();
    void arCameraStateChanged();
    void procentReconstructionChanged();
    void startRecontruct();
    void endReconstruct();
    void needToScene();
    void haveReconstructChanged();
    void haveLoadModelChanged();

private:
    void _createScene(int index);
    void _deleteScene();
    QSGNode* updatePaintNode(QSGNode* node, UpdatePaintNodeData* );

    QScrollEngine::QScrollEngineContext* _context;\
    SceneInterface* _scene;
    AR::IPoint _currentResolution;
    QRectF _currentBoundingRect;
    QRectF _currentTextureRect;
    bool _isVerticalOrientation;
    QOpenGLFramebufferObject* _FBOs[3];
    bool _mirrored;
    QQueue<QPair<TypeAction, QString>> _queueActions;

    VideoSurface_ForQQuickItem _surface;

    AR::ARSystem _arSystem;
    State _currentState;
    float _procentReconstruction;

    QMediaObject* _mediaObject;
    QString _textStatus;
    float _cameraVerticalAngle;
    QQuaternion _cameraOrientation;

    void _clearFBOs();
    void _generateFBOs(const AR::IPoint resolution, const AR::IPoint& resolutionAR);
    void _initialize(QSGSimpleTextureNode* textureNode);
    void _updateCamera3d();
    void _updateActions();
};

#endif
