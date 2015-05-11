#include "App/ARCameraQml.h"
#include <QMediaService>
#include <QVideoRendererControl>
#include <QCameraInfo>
#include <QTime>
#include <QDebug>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>
#include <QScreen>
#include <QMatrix4x4>
#include <QSizeF>
#include <utility>
#include <functional>
#include "AR/TMath.h"
#include "AR/Reconstructor3D.h"
#include "QScrollEngine/Shaders/QSh.h"
#include "App/SceneShip.h"
#include "App/SceneSpace.h"
#include "App/SceneGrid.h"
#include "App/ScenePigPetr.h"
#include "App/SceneBreakout.h"
#include "App/SceneLoadModel.h"

void VideoSurface_ForQQuickItem::scheduleOpenGLContextUpdate()
{
    QMetaObject::invokeMethod(this, "updateOpenGLContext");
}

void VideoSurface_ForQQuickItem::updateOpenGLContext()
{
    if (_context == nullptr)
        return;
    this->setProperty("GLContext", QVariant::fromValue<QObject*>(_context->openGLContext()));
}

VideoSurface_ForQQuickItem::VideoSurface_ForQQuickItem(QQuickItem* parentQuickItem)
{
    _parentQuickItem = parentQuickItem;
    _frameAvailable = false;
    _context = nullptr;
    _frameProvider = nullptr;
    _frame = QVideoFrame();
    _matrixTexture.setToIdentity();
}

void VideoSurface_ForQQuickItem::setMatrixTexture(const QMatrix2x2& matrix)
{
    _matrixTexture = matrix;
    if (_frameProvider) {
        _frameProvider->setMatrixTexture(_matrixTexture);
    }
}

VideoSurface_ForQQuickItem::~VideoSurface_ForQQuickItem()
{
    if (_frameProvider)
        delete _frameProvider;
}

void VideoSurface_ForQQuickItem::setQScrollEngineContext(QScrollEngine::QScrollEngineContext* context)
{
    _context = context;
}

QList<QVideoFrame::PixelFormat> VideoSurface_ForQQuickItem::supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType) const
{
    return AR::FrameProvider::supportedPixelFormats(handleType);
}

bool VideoSurface_ForQQuickItem::start(const QVideoSurfaceFormat& format)
{
    if (!supportedPixelFormats(format.handleType()).contains(format.pixelFormat())) {
        qDebug() << format.handleType() << " " << format.pixelFormat() << " - format is not supported.";
        return false;
    }
    return QAbstractVideoSurface::start(format);
}

void VideoSurface_ForQQuickItem::provideFrame()
{
    if (_context == nullptr)
        return;
    if (_frameAvailable == false)
        return;
    if (_frameProvider == nullptr) {
        _frameProvider = AR::FrameProvider::createFrameProvider(_frame.pixelFormat());
        if (_frameProvider == nullptr) {
            return;
        } else {
            _frameProvider->setMatrixTexture(_matrixTexture);
        }
    } else if (_frameProvider->pixelFormat() != _frame.pixelFormat()) {
        delete _frameProvider;
        _frameProvider = AR::FrameProvider::createFrameProvider(_frame.pixelFormat());
        if (_frameProvider == nullptr) {
            return;
        } else {
            _frameProvider->setMatrixTexture(_matrixTexture);
        }
    }
    if (_frame.handleType() == QAbstractVideoBuffer::GLTextureHandle) {
        _frameProvider->setHandleTexture(_context, _frame.handle().value<GLuint>(),
                                         _frame.width(), _frame.height());
    } else if (_frame.handleType() == QAbstractVideoBuffer::NoHandle) {
        if (_frame.map(QAbstractVideoBuffer::ReadOnly)) {
            _frameProvider->setDataTexture(_context, reinterpret_cast<const GLvoid*>(_frame.bits()), _frame.width(), _frame.height());
            _frame.unmap();
        }
    }
    _frameAvailable = false;
}

bool VideoSurface_ForQQuickItem::present(const QVideoFrame& frame)
{
    if ((_frameAvailable) || (_context == nullptr))
        return true;
    _context->lock();
    if (!frame.isValid())
        return false;
    _frame = frame;
    _frameAvailable = true;
    _context->unlock();
    QMetaObject::invokeMethod(_parentQuickItem, "update");
    return true;
}

bool ARCameraQml::haveReconstruct() const
{
    return (dynamic_cast<SceneGrid*>(_scene) != nullptr);
}

bool ARCameraQml::haveLoadModel() const
{
    return (dynamic_cast<SceneLoadModel*>(_scene) != nullptr);
}

QString ARCameraQml::sceneName(int index) const
{
    switch (index) {
    case 0:
        return "Sun system";
    case 1:
        return "Ship";
    case 2:
        return "Pig";
    case 3:
        return "Load model";
    case 4:
        return "Reconstruction";
    default:
        break;
    }
    return "";
}

void ARCameraQml::_createScene(int index)
{
    using namespace QScrollEngine;
    if (_scene)
        return;
    switch (index) {
    case 0:
        _scene = new SceneSpace(_context);
        break;
    case 1:
        _scene = new SceneShip(_context);
        break;
    case 2:
        _scene = new ScenePigPetr(_context);
        break;
    case 3:
        _scene = new SceneLoadModel(_context);
        break;
    case 4:
        _scene = new SceneGrid(_context);
        _arSystem.setEntity(new QEntity(_scene));
        _arSystem.reset();
        break;
    default:
        break;
    }
    _scene->setARSystem(&_arSystem);
    emit haveLoadModelChanged();
    emit haveReconstructChanged();
    this->update();
}

void ARCameraQml::_deleteScene()
{
    if (_scene == nullptr)
        return;
    if (_arSystem.reconstructor3D()->isRunning())
        return;
    delete _scene;
    _scene = nullptr;
    emit haveLoadModelChanged();
    emit haveReconstructChanged();
}

ARCameraQml::ARCameraQml(QQuickItem* parent):
    QQuickItem(parent),
    _surface(this)
{
    setFlag(QQuickItem::ItemHasContents);
    _context = new QScrollEngine::QScrollEngineContext(nullptr);
    //setRenderTarget(QQuickPaintedItem::FramebufferObject);
    _currentResolution = AR::IPoint(-1, -1);
    _mediaObject = nullptr;
    _FBOs[0] = _FBOs[1] = _FBOs[2] = nullptr;
    _cameraVerticalAngle = 30.0f;
    _isVerticalOrientation = false;
    _mirrored = false;
    _scene = nullptr;
    _currentState = NotTracking;
    connect(_arSystem.reconstructor3D(), SIGNAL(finished()), this, SLOT(finishReconstruct()));
    connect(_arSystem.reconstructor3D(), SIGNAL(updateProgress(float)), this, SLOT(slotUpdateProcess(float)));
    _procentReconstruction = 0.0f;
}

ARCameraQml::~ARCameraQml()
{
    _clearFBOs();
    //delete _context;
}

void ARCameraQml::setCameraVerticalAngle(float angle)
{
    _cameraVerticalAngle = angle;
    emit cameraVerticalAngleChanged();
}

void ARCameraQml::_updateCamera3d()
{
    QScrollEngine::QCamera3D* camera3d = _context->camera;
    camera3d->setProjectionParameters(1.0f, 100.0f, _cameraVerticalAngle, QSize(_currentResolution.x, _currentResolution.y));
    camera3d->calculateProjectionMatrix();
    _arSystem.setCameraParameters(camera3d, !_mirrored, true);
    if (_isVerticalOrientation)
        camera3d->swapAspectXY();
}

void ARCameraQml::_clearFBOs()
{
    QOpenGLContext* glContext = _context->openGLContext();
    if ((glContext == nullptr) || (_FBOs[0] == nullptr))
        return;
    delete _FBOs[0];
    delete _FBOs[1];
    delete _FBOs[2];
    _FBOs[0] = nullptr;
}

void ARCameraQml::_generateFBOs(const AR::IPoint resolution, const AR::IPoint& resolutionAR)
{
    _FBOs[0] = new QOpenGLFramebufferObject(resolution.x, resolution.y, QOpenGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D, GL_RGBA);
    _FBOs[1] = new QOpenGLFramebufferObject(resolutionAR.x, resolutionAR.y, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGBA);
    _FBOs[2] = new QOpenGLFramebufferObject(resolutionAR.x / 4, resolutionAR.y, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGBA);
}

void ARCameraQml::_initialize(QSGSimpleTextureNode* textureNode)
{
    _currentResolution = AR::IPoint::ZERO;
    QQuickWindow* nodeWindow = window();
    if (nodeWindow) {
        QScreen* screen = nodeWindow->screen();
        if (screen) {
            updateSize(boundingRect());
            textureNode->setRect(_currentTextureRect);
            _context->resizeContext(_currentResolution.x, _currentResolution.y);
            _updateCamera3d();
        }
    }
    _generateFBOs(_currentResolution, _arSystem.resolution());
    if ((nodeWindow) && (_FBOs[0]->texture() > 0)) {
        if (textureNode->texture())
            delete textureNode->texture();
        textureNode->setTexture(nodeWindow->createTextureFromId(_FBOs[0]->texture(), _FBOs[0]->size()));
        textureNode->texture()->setFiltering(QSGTexture::Nearest);
        textureNode->texture()->updateBindOptions(true);
    }
    textureNode->markDirty(QSGNode::DirtyForceUpdate);
}

void ARCameraQml::_updateActions()
{
    using namespace QScrollEngine;
    while (!_queueActions.empty()) {
        QPair<TypeAction, QString> action = *_queueActions.begin();
        _queueActions.pop_front();
        switch (action.first) {
        case NextState: {
            AR::FrameProvider* frameProvider = _surface.frameProvider();
            if (frameProvider)
                _arSystem.reconstructor3D()->setRgbSwap((frameProvider->pixelFormat() == QVideoFrame::Format_BGR32));
            if ((_arSystem.state() == AR::ARSystem::Reconstruction3D) && (!haveReconstruct())) {
                _arSystem.reset();
            } else if (_arSystem.state() == AR::ARSystem::LostReconstruction3D) {
                _arSystem.reset();
            } else {
                _arSystem.nextState();
                if ((_arSystem.state() == AR::ARSystem::Reconstruction3D) && (_arSystem.entity()))
                    _arSystem.entity()->deleteParts();
            }
        } break;
        case FinishReconstruct: {
            if (_arSystem.entity()) {
                _arSystem.entity()->deleteParts();
                _arSystem.reconstructor3D()->createFinishEntity();
            }
            emit endReconstruct();
            update();
        } break;
        case SaveEntity: {
            if (_arSystem.entity()) {
                //QString dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
                QUrl url(action.second);
                QString dir = url.toLocalFile();
                if (dir.right(1) != "/")
                    dir += "/";
                _context->saveEntity(_arSystem.entity(), dir + "reconstruction.3ds", dir);
            }
        } break;
        case LoadEntity: {
            SceneLoadModel* scene = dynamic_cast<SceneLoadModel*>(_scene);
            if (scene) {
                QUrl url(action.second);
                scene->loadModel(url.toLocalFile());
            }
        } break;
        case CreateScene: {
            if (_scene) {
                delete _scene;
                _scene = nullptr;
            }
            bool ok;
            int index = action.second.toInt(&ok);
            if (ok)
                _createScene(index);
        } break;
        case DeleteScene: {
            _deleteScene();
        } break;
        }
    }
}

QSGNode* ARCameraQml::updatePaintNode(QSGNode* node, UpdatePaintNodeData* )
{
    using namespace QScrollEngine;
    _context->lock();
    QSGSimpleTextureNode* textureNode = dynamic_cast<QSGSimpleTextureNode*>(node);
    if (_context->openGLContext() != QOpenGLContext::currentContext()) {
        _clearFBOs();
        _context->setOpenGLContext(QOpenGLContext::currentContext());
        _context->setPostEffectUsed(false);
        _context->setEnableClearing(false);
        _surface.setQScrollEngineContext(_context);
        _arSystem.setContext(_context);
        if (textureNode == nullptr)
            textureNode = new QSGSimpleTextureNode();
        _context->unlock();
        _initialize(textureNode);
        _context->lock();
    } else if (textureNode == nullptr) {
        _context->unlock();
        textureNode = new QSGSimpleTextureNode();
        _initialize(textureNode);
        _context->lock();
    }
    GLint defaultFBO = 0;
    _context->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
    AR::FrameProvider* frameProvider = _surface.frameProvider();
    if (frameProvider) {
        AR::IPoint resolution = frameProvider->originalTextureSize();
        if ((resolution != _currentResolution) || (_currentBoundingRect != boundingRect())) {
            _context->unlock();
            _initialize(textureNode);
            _context->lock();
        }
    }
    textureNode->markDirty(QSGNode::DirtyForceUpdate);
    _updateActions();
    if (_scene == nullptr) {
        _context->unlock();
        emit needToScene();
        return textureNode;
    }
    if (_arSystem.reconstructor3D()->isRunning()) {
        _context->unlock();
        _context->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
        textureNode->setRect(_currentTextureRect);
        qDebug() << "Thread is running.";
        return textureNode;
    }
    _scene->setFrameProvider(nullptr);
    _scene->beginUpdate();
    _context->glEnable(GL_CULL_FACE);
    _context->glFrontFace(GL_CW);
    if (!_surface.isActive()) {
        _surface.scheduleOpenGLContextUpdate();
        QObject* glThreadCallback = (_surface.property("_q_GLThreadCallback")).value<QObject*>();
        if (glThreadCallback) {
            QEvent event(QEvent::User);
            glThreadCallback->event(&event);
        }
        if (!_context->postEffectUsed())
            _FBOs[0]->bind();
        _context->beginPaint();
    } else {
        bool frameAvailable = _surface.isFrameAvailable();
        _surface.provideFrame();
        if (frameProvider) {
            _scene->setFrameProvider(frameProvider);
            frameProvider->setUsedTransform(false);
            if (frameAvailable) {
                QTime timer;
                timer.start();
                _arSystem.tracking(frameProvider, _context, _FBOs[1], _FBOs[2]);
                int time = timer.elapsed();
                _textStatus = QString::number(frameProvider->timeProvideLuminanceFrame()) + "/" + QString::number(time);
                QMatrix4x4 matrix = _arSystem.matrixWorld();
                QQuaternion orientation;
                QOtherMathFunctions::matrixToQuaternion(matrix, orientation);
                QVector3D position(matrix(0, 3), matrix(1, 3), matrix(2, 3));
                _scene->position = position;
                _scene->orientation = orientation;
                emit textStatusChanged();
            }
            frameProvider->setUsedTransform(true);
            if (!_context->postEffectUsed())
                _FBOs[0]->bind();
            _context->beginPaint();
            _context->glDisable(GL_DEPTH_TEST);
            _context->glDepthMask(GL_FALSE);
            _context->glViewport(0, 0, _currentResolution.x, _currentResolution.y);
            frameProvider->bindColorShader(_context);
            _context->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            using namespace TMath;
            QMatrix2x2 transform;
            if (_mirrored) {
                QMatrix2x2 frameTransform = frameProvider->matrixTexture();
                transform(0, 0) = frameTransform(0, 0);
                transform(0, 1) = - frameTransform(0, 1);
                transform(1, 0) = - frameTransform(1, 0);
                transform(1, 1) = frameTransform(1, 1);
            } else {
                transform = frameProvider->matrixTexture();
            }
            _arSystem.drawResultOnGL(_context, transform);
        } else {
            if (!_context->postEffectUsed())
                _FBOs[0]->bind();
            _context->beginPaint();
        }
    }
    _context->unlock();
    _scene->endUpdate();
    State currentState = State(_arSystem.state());
    if (currentState != _currentState) {
        _currentState = currentState;
        emit arCameraStateChanged();
    }
    if ((currentState == TrackingNow) || (currentState == Reconstruction3D))
        _context->drawScenes();
    _context->endPaint(_FBOs[0]->handle());
    _context->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
    textureNode->setRect(_currentTextureRect);
    return textureNode;
}

void ARCameraQml::setMediaObject(QObject* object)
{
    if (object == nullptr)
        return;
    QMediaObject* mediaObject = dynamic_cast<QMediaObject*>(object);
    if (mediaObject == nullptr) {
        object = object->property("mediaObject").value<QObject*>();
        mediaObject = dynamic_cast<QMediaObject*>(object);
    }
    if (mediaObject) {
        setMediaObject(mediaObject);
    } else {
        qDebug() << Q_FUNC_INFO << ": Failed to set media object";
    }
}

void ARCameraQml::updateSize(const QRectF& rect)
{
    _currentBoundingRect = rect;
    AR::FrameProvider* frameProvider = _surface.frameProvider();
    _currentResolution = (frameProvider) ? frameProvider->originalTextureSize() : _arSystem.resolution();
    Qt::ScreenOrientation screenOrientation = ((rect.width() > rect.height())) ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
    updateOrientation(screenOrientation);
    AR::IPoint resolution = _currentResolution;
    switch (screenOrientation) {
    case Qt::LandscapeOrientation: {
    }
        break;
    case Qt::PortraitOrientation: {
        std::swap(resolution.x, resolution.y);\
    }
        break;
    default: {
    }
    }
    float ratio = resolution.x / static_cast<float>(resolution.y);
    _currentTextureRect.setHeight(rect.height());
    _currentTextureRect.setWidth(ratio * _currentTextureRect.height());
    if (_currentTextureRect.width() > rect.width()) {
        _currentTextureRect.setWidth(rect.width());
        _currentTextureRect.setHeight(_currentTextureRect.width() / ratio);
    }
    _currentTextureRect = QRectF(rect.x() + (rect.width() - _currentTextureRect.width()) * 0.5f,
                                 rect.y() + (rect.height() - _currentTextureRect.height()) * 0.5f,
                                 _currentTextureRect.width(), _currentTextureRect.height());
}

void ARCameraQml::updateOrientation(Qt::ScreenOrientation screenOrientation)
{
    QMatrix2x2 matrixTexture;
    QCamera* camera = dynamic_cast<QCamera*>(_mediaObject);
    float angle = 0.0f;
    if (camera) {
        QCameraInfo cameraInfo(*camera);
        angle = cameraInfo.orientation();
        switch (cameraInfo.position()) {
        case QCamera::UnspecifiedPosition: {
            angle += 90.0f;
            _mirrored = false;
        } break;
        case QCamera::BackFace: {
            angle += 0.0f;
            _mirrored = true;
        } break;
        case QCamera::FrontFace: {
            angle = 360.0f - angle;
            _mirrored = true;
        } break;
        }
    }
    switch (screenOrientation) {
    case Qt::PrimaryOrientation: {
        angle += 0.0f;
        _isVerticalOrientation = false;
    } break;
    case Qt::PortraitOrientation: {
        angle += 180.0f;
        _isVerticalOrientation = true;
    } break;
    case Qt::LandscapeOrientation: {
        angle += 270.0f;
        _isVerticalOrientation = false;
    } break;
    case Qt::InvertedPortraitOrientation: {
        angle += 0.0f;
        _isVerticalOrientation = true;
    } break;
    case Qt::InvertedLandscapeOrientation: {
        angle += 0.0f;
        _isVerticalOrientation = false;
    } break;
    }
    _cameraOrientation = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, angle - 180.0f);
    _context->camera->setOrientation(_cameraOrientation);
    angle *= static_cast<float>(M_PI) / static_cast<float>(180.0f);
    float signMirror = _mirrored ? -1.0f : 1.0f;
    matrixTexture(1, 0) = - qSin(angle);
    matrixTexture(1, 1) = qCos(angle);
    matrixTexture(0, 0) = matrixTexture(1, 1) * signMirror;
    matrixTexture(0, 1) = - matrixTexture(1, 0) * signMirror;
    _surface.setMatrixTexture(matrixTexture);
}

void ARCameraQml::setMediaObject(QMediaObject* mediaObject)
{
    if (mediaObject != _mediaObject) {
        QMediaService* service = mediaObject->service();
        if (service) {
            QVideoRendererControl* rendererControl = dynamic_cast<QVideoRendererControl*>(
                        service->requestControl(QVideoRendererControl_iid));

            if (rendererControl) {
                rendererControl->setSurface(&_surface);
                //updateOrientation();
                _mediaObject = mediaObject;
                emit mediaObjectChanged();
                return;
            }
        }
    }
    qDebug() << Q_FUNC_INFO << ": Failed to set media object";
}

void ARCameraQml::slotUpdateProcess(float procent)
{
    _procentReconstruction = procent;
    emit procentReconstructionChanged();
    if (_procentReconstruction == 0.0f)
        emit startRecontruct();
}
