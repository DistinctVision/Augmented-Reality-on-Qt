#include "ARFrameHandler.h"
#include <QCameraInfo>
#include <QTime>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>
#include <QScreen>
#include <QMatrix4x4>
#include <QSettings>
#include <QSizeF>
#include <utility>
#include <functional>
#include <cmath>
#include "TMath/TMath.h"
#include "QScrollEngine/QOtherMathFunctions.h"
#include "QScrollEngine/Shaders/QSh.h"
#include "AR/ImageProcessing.h"
#include "AR/Camera.h"

QVideoFilterRunnable* ARFrameHandler::createFilterRunnable()
{
    m_runnable = new ARFrameHandlerRunnable(this);
    return m_runnable;
}

ARFrameHandler::ARFrameHandler()
{
    m_runnable = nullptr;
    m_frameResolution = QSize(0, 480);

    m_fixedCameraParamters = TMath::TVector<bool>(5);
    m_fixedCameraParamters.fill(false);
    m_fixedCameraParamters(2) = true;
    m_fixedCameraParamters(3) = true;
    m_fixedCameraParamters(4) = true;

    m_cameraParameters = AR::Camera::defaultCameraParameters;
    m_cameraParameters(2) = 0.5;
    m_cameraParameters(3) = 0.5;
    m_cameraParameters(4) = 0.0;

    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/confing.ini";
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup("Calibration");
    if (settings.contains("CameraParameters")) {
        QList<QVariant> list = settings.value("CameraParameters").value<QList<QVariant>>();
        m_cameraParameters = AR::Camera::defaultCameraParameters;
        int size = qMin(5, list.size());
        for (int i=0; i<size; ++i) {
            bool success = false;
            double value = list.at(i).toDouble(&success);
            if (success && (value == value))
                m_cameraParameters(i) = value;
        }
        setCameraParameters(m_cameraParameters);
    }
    if (settings.contains("FixedOpticalCenter")) {
        bool success = false;
        int fixedOpticalCenter = settings.value("FixedOpticalCenter").toInt(&success);
        if (success) {
            setFixedOpticalCenter(fixedOpticalCenter > 0);
        }
    }
    if (settings.contains("DisabledRadialDistortion")) {
        bool success = true;
        int disabledRadialDistortion = settings.value("DisabledRadialDistortion").toInt(&success);
        if (success) {
            setDisabledRadialDistortion(disabledRadialDistortion > 0);
        }
    }
    settings.endGroup();
}

ARFrameHandler::~ARFrameHandler()
{
    QMutexLocker ml(&m_mutex);
    for (QList<QObject*>::iterator it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        ARScene* arScene = static_cast<ARScene*>(*it);
        arScene->freeScene();
        arScene->setParent(nullptr);
    }
}

QList<double> ARFrameHandler::cameraParametersQml() const
{
    TMath::TVectord parameters = cameraParameters();
    return QList<double>() << parameters(0) << parameters(1) << parameters(2) << parameters(3) << parameters(4);
}

void ARFrameHandler::setCameraParametersQml(const QList<double>& parameters)
{
    TMath::TVectord p(5);
    int size = std::min(parameters.size(), 5);
    int i=0;
    for (i=0; i<size; ++i)
        p(i) = parameters[i];
    for (; i<5; ++i)
        p(i) = 0.0;
    setCameraParameters(p);
}

bool ARFrameHandler::fixedOpticalCenter() const
{
    return m_fixedCameraParamters(2);
}

void ARFrameHandler::setFixedOpticalCenter(bool enabled)
{
    if (enabled) {
        m_fixedCameraParamters(2) = true;
        m_fixedCameraParamters(3) = true;
        TMath::TVectord cameraParamters = this->cameraParameters();
        cameraParamters(2) = 0.5;
        cameraParamters(3) = 0.5;
        setCameraParameters(cameraParamters);
    } else {
        m_fixedCameraParamters(2) = false;
        m_fixedCameraParamters(3) = false;
    }
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/confing.ini", QSettings::IniFormat);
    settings.beginGroup("Calibration");
    settings.setValue("FixedOpticalCenter", QVariant(enabled ? 1 : 0));
    settings.endGroup();
    emit fixedOpticalCenterChanged();
}

bool ARFrameHandler::disabledRadialDistortion() const
{
    return m_fixedCameraParamters(4);
}

void ARFrameHandler::setDisabledRadialDistortion(bool disabled)
{
    if (disabled) {
        m_fixedCameraParamters(4) = true;
        TMath::TVectord cameraParamters = this->cameraParameters();
        cameraParamters(4) = 0.0;
        setCameraParameters(cameraParamters);
    } else {
        m_fixedCameraParamters(4) = false;
    }
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/confing.ini", QSettings::IniFormat);
    settings.beginGroup("Calibration");
    settings.setValue("DisabledRadialDistortion", QVariant(disabled ? 1 : 0));
    settings.endGroup();
    emit disabledRadialDistortionChanged();
}

QSize ARFrameHandler::frameResolution() const
{
    return m_frameResolution;
}

void ARFrameHandler::setFrameResolution(const QSize& frameResolution)
{
    m_frameResolution = frameResolution;
    emit frameResolutionChanged();
}

const QList<QObject*> ARFrameHandler::scenes() const
{
    QMutexLocker ml(&m_mutex);
    return m_scenes;
}

void ARFrameHandler::setScenes(const QList<QObject*>& scenes)
{
    m_mutex.lock();
    for (QList<QObject*>::iterator it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        ARScene* arScene = static_cast<ARScene*>(*it);
        arScene->freeScene();
        arScene->setParent(nullptr);
    }
    m_scenes = scenes;
    for (int i=m_scenes.size()-1; i>=0; --i) {
        ARScene* arScene = dynamic_cast<ARScene*>(m_scenes[i]);
        if (arScene == nullptr) {
            m_scenes.removeAt(i);
        } else {
            arScene->setParent(this);
        }
    }
    m_mutex.unlock();
    emit scenesChanged();
}

const QList<QObject*> ARFrameHandler::imageReceivers() const
{
    QMutexLocker ml(&m_mutex);
    return m_imageReceivers;
}

void ARFrameHandler::setImageReceivers(const QList<QObject*>& imageReceivers)
{
    m_mutex.lock();
    for (QList<QObject*>::iterator it = m_imageReceivers.begin(); it != m_imageReceivers.end(); ++it) {
        (*it)->setParent(nullptr);
    }
    m_imageReceivers = imageReceivers;
    for (int i=m_imageReceivers.size()-1; i>=0; --i) {
        FrameReceiver* imageReceiver = dynamic_cast<FrameReceiver*>(m_imageReceivers[i]);
        if (imageReceiver == nullptr) {
            m_imageReceivers.removeAt(i);
        } else {
            imageReceiver->setParent(this);
        }
    }
    m_mutex.unlock();
    emit imageReceiversChanged();
}

TMath::TVectord ARFrameHandler::cameraParameters() const
{
    return m_cameraParameters;
}

void ARFrameHandler::setCameraParameters(const TMath::TVectord& parameters)
{
    assert(parameters.size() == 5);
    m_cameraParameters = parameters;
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/confing.ini", QSettings::IniFormat);
    settings.beginGroup("Calibration");
    settings.setValue("CameraParameters", QVariant(QList<QVariant>() <<
                                              parameters(0) <<
                                              parameters(1) <<
                                              parameters(2) <<
                                              parameters(3) <<
                                              parameters(4)));
    settings.endGroup();
    emit cameraParametersChanged();
}


void ARFrameHandler::pressed(int id, float x, float y)
{
    m_mutex.lock();
    for (int i = 0; i < m_scenes.size(); ++i) {
        dynamic_cast<ARScene*>(m_scenes[i])->onPressed(id, x, y);
    }
    m_mutex.unlock();
}

void ARFrameHandler::released(int id)
{
    m_mutex.lock();
    for (int i = 0; i < m_scenes.size(); ++i) {
        dynamic_cast<ARScene*>(m_scenes[i])->onReleased(id);
    }
    m_mutex.unlock();
}

ARFrameHandlerRunnable::ARFrameHandlerRunnable(ARFrameHandler* parent)
{
    m_parentItem = parent;
    m_context = new QScrollEngine::QScrollEngineContext(QOpenGLContext::currentContext());
    _initializeShader();
    m_textureFrameId = std::numeric_limits<GLuint>::max();
    m_externTextureFrame = true;
    m_eglTextureFrame = false;
    m_frameFBO = nullptr;
    m_arFrameFbo = nullptr;
    m_sizeFrame.setZero();
}

ARFrameHandlerRunnable::~ARFrameHandlerRunnable()
{
    using namespace QScrollEngine;
    QMutexLocker ml(&m_mutex);
    m_textureRenderer.clear();
    if (m_arFrameFbo)
        delete m_arFrameFbo;
    if (m_frameFBO)
        delete m_frameFBO;
    if (!m_externTextureFrame)
        m_context->glDeleteTextures(1, &m_textureFrameId);
    if (m_parentItem->m_runnable == this) {
        QList<QObject*>& scenes = m_parentItem->m_scenes;
        for (QList<QPair<ARScene*, QScene*>>::iterator it = m_scenes.begin(); it != m_scenes.end(); ++it) {
            for (QList<QObject*>::iterator itB = scenes.begin(); itB != scenes.end(); ++itB) {
                if (dynamic_cast<ARScene*>(*itB) == it->first) {
                    if (it->first->scene() == it->second)
                        it->first->freeScene();
                    break;
                }
            }
            delete it->second;
        }
    } else {
        for (QList<QPair<ARScene*, QScene*>>::iterator it = m_scenes.begin(); it != m_scenes.end(); ++it) {
            delete it->second;
        }
    }
    delete m_context;
}

void ARFrameHandlerRunnable::_updateScenesQml()
{
    using namespace QScrollEngine;
    QMutexLocker ml(&m_parentItem->m_mutex);
    QList<QObject*>& scenes = m_parentItem->m_scenes;
    QVector<int> founded;
    founded.resize(scenes.size());
    founded.fill(0, founded.size());
    int i, j;
    for (i=m_scenes.size()-1; i>=0; --i) {
        QPair<ARScene*, QScene*>& s = m_scenes[i];
        bool found = false;
        j=0;
        for (QList<QObject*>::const_iterator it = scenes.cbegin(); it != scenes.cend(); ++it) {
            if (static_cast<const ARScene*>(*it) == s.first) {
                found = true;
                founded[j] = 1;
                if (s.second == nullptr) {
                    s.second = s.first->createScene(m_context);
                    s.first->setFrameTexture(m_textureFrameId, m_textureMatrix, m_eglTextureFrame);
                } else if (s.first->scene() != s.second) {
                    delete s.second;
                    s.second = s.first->createScene(m_context);
                    s.first->setFrameTexture(m_textureFrameId, m_textureMatrix, m_eglTextureFrame);
                }
            }
            ++j;
        }
        if (!found) {
            delete s.second;
            m_scenes.removeAt(i);
        }
    }
    for (i=0; i<founded.size(); ++i) {
        if (founded[i] == 0) {
            QPair<ARScene*, QScene*> p;
            p.first = static_cast<ARScene*>(scenes[i]);
            p.second = p.first->createScene(m_context);
            m_scenes.push_back(p);
            p.first->setFrameTexture(m_textureFrameId, m_textureMatrix, m_eglTextureFrame);
        }
    }
}

ARFrameHandler* ARFrameHandlerRunnable::parentItem()
{
    return m_parentItem;
}

void ARFrameHandlerRunnable::_initialize(const AR::Point2i& size, bool externTexture, GLuint textureId)
{
    //GLint defaultFBOId = 0;
    //_context->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBOId);
    m_context->glActiveTexture(GL_TEXTURE0);
    m_sizeFrame = size;
    if (m_frameFBO) {
        delete m_frameFBO;
    }
    if (!m_externTextureFrame) {
        m_context->glDeleteTextures(1, &m_textureFrameId);
    }
    m_frameFBO = new QOpenGLFramebufferObject(m_sizeFrame.x, m_sizeFrame.y,
                                              QOpenGLFramebufferObject::CombinedDepthStencil,
                                              GL_TEXTURE_2D);
    if (externTexture) {
        m_externTextureFrame = true;
        m_textureFrameId = textureId;
    } else {
        m_externTextureFrame = false;
        m_context->glGenTextures(1, &m_textureFrameId);
        m_context->glBindTexture(GL_TEXTURE_2D, m_textureFrameId);
        m_context->glTexImage2D(GL_TEXTURE_2D, 0, 4, m_sizeFrame.x, m_sizeFrame.y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                                m_sourceImage.data());
    }
    m_textureMatrix.setToIdentity();
    m_mutex.unlock();
    setFrameTexture(m_textureFrameId, m_textureMatrix, false);
    m_mutex.lock();
    if (m_arFrameFbo) {
        delete m_arFrameFbo;
        m_arFrameFbo = nullptr;
    }
    //_context->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBOId);
}

void ARFrameHandlerRunnable::_initializeShader()
{
    const char* textVertex =
            "attribute mediump vec2 vertex;\n"
            "attribute mediump vec2 texCoord;\n"
            "uniform mat3 textureMatrix;\n"
            "varying mediump vec2 texc;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(vertex, 0, 1);\n"
            "    texc = (textureMatrix * vec3(texCoord, 1)).xy;\n"
            "}\n";
    const char* textFragmentBW =
        "uniform sampler2D texture;\n"
        "varying mediump vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "    highp vec3 color = texture2D(texture, texc).rgb;\n"
        "    highp float gray = dot(color.rgb, vec3(0.3333, 0.3333, 0.3333));"
        "    gl_FragColor = vec4(gray, gray, gray, 1);\n"
        "}\n";
    m_shaderBW.addShaderFromSourceCode(QOpenGLShader::Vertex, textVertex);
    m_shaderBW.addShaderFromSourceCode(QOpenGLShader::Fragment, textFragmentBW);
    m_shaderBW.bindAttributeLocation("vertex", 0);
    m_shaderBW.bindAttributeLocation("texCoord", 1);
    m_shaderBW.link();
}

void ARFrameHandlerRunnable::_drawTrackingResultOnGL() const
{
    using namespace AR;
    /*_context->glDisable(GL_DEPTH_TEST);
    Point2i resolution = m_arSystem.resolution();
    QVector2D v;
    QVector2D r;
    std::vector<QVector2D> lines;
    Point2f scale(2.0f / static_cast<float>(resolution.x),
                  2.0f / static_cast<float>(resolution.y));
    if (_state == ARSystem::InitializeTracking) {
        const HomographyInitializer& initializer = _arSystem.getInitializer();
        if (!initializer.firstKeyFrameIsSet())
            return;
        const Map::KeyFrame& firstFrame = initializer.firstKeyFrame();
        const Map::KeyFrame& secondFrame = initializer.secondKeyFrameIsSet() ?
                    initializer.secondKeyFrame() : _arSystem.getCurrentKeyFrame();
        for (std::size_t i=0; i<firstFrame.projectedPoints.size(); ++i) {
            const Point2f& firstF = firstFrame.projectedPoints[i];
            v.setX(firstF.x * scale.x - 1.0f);
            v.setY(firstF.y * scale.y - 1.0f);
            //r.setX(transform(0, 0) * v.x() + transform(0, 1) * v.y());
            //r.setY(transform(1, 0) * v.x() + transform(1, 1) * v.y());
            lines.push_back(v);
            const Point2f& secondF = secondFrame.projectedPoints[i];
            v.setX(secondF.x * scale.x - 1.0f);
            v.setY(secondF.y * scale.y - 1.0f);
            //r.setX(transform(0, 0) * v.x() + transform(0, 1) * v.y());
            //r.setY(transform(1, 0) * v.x() + transform(1, 1) * v.y());
            lines.push_back(v);
        }
    } else if (_state == ARSystem::TrackingNow) {
        return;
        const Map::KeyFrame& currentFrame = m_arSystem.getCurrentKeyFrame();
        Point2f delta0(5.0f, 5.0f), delta1(-5.0f, -5.0f);
        Point2f delta2(5.0f, -5.0f), delta3(-5.0f, 5.0f);
        Point2f c0, c1, c2, c3;
        for (std::size_t i=0; i<currentFrame.projectedPoints.size(); ++i) {
            const Point2f& currentF = currentFrame.projectedPoints[i];
            c0 = currentF + delta0;
            c1 = currentF + delta1;
            c2 = currentF + delta2;
            c3 = currentF + delta3;
            v.setX(c0.x * scale.x - 1.0f);
            v.setY(c0.y * scale.y - 1.0f);
            lines.push_back(v);
            v.setX(c1.x * scale.x - 1.0f);
            v.setY(c1.y * scale.y - 1.0f);
            lines.push_back(v);
            v.setX(c2.x * scale.x - 1.0f);
            v.setY(c2.y * scale.y - 1.0f);
            lines.push_back(v);
            v.setX(c3.x * scale.x - 1.0f);
            v.setY(c3.y * scale.y - 1.0f);
            lines.push_back(v);
        }
    }
    m_context->drawLines(lines.data(), lines.size(), QColor(255, 0, 0));
    m_context->glEnable(GL_DEPTH_TEST);*/
}

void ARFrameHandlerRunnable::_drawFrame()
{
    m_textureRenderer.draw(m_textureFrameId, m_textureMatrix);
}

void ARFrameHandlerRunnable::preDraw()
{
    using namespace QScrollEngine;
    QMutexLocker ml(&m_mutex);
    _updateScenesQml();
    for (QList<QPair<ARScene*, QScene*>>::iterator it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        if (it->first->enabled()) {
            it->first->beginUpdate();
            if (it->second)
                it->second->setEnabled(true);
        } else {
            if (it->second)
                it->second->setEnabled(false);
        }
    }
}

void ARFrameHandlerRunnable::drawContent()
{
    using namespace QScrollEngine;
    QMutexLocker ml(&m_mutex);
    for (QList<QPair<ARScene*, QScene*>>::iterator it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        if (it->first->enabled())
            it->first->endUpdate();
    }
    m_context->drawScenes();
    _drawTrackingResultOnGL();
}

void ARFrameHandlerRunnable::setFrameTexture(GLuint textureId, const QMatrix3x3& textureMatrix, bool egl)
{
    using namespace QScrollEngine;
    QMutexLocker ml(&m_mutex);
    m_textureFrameId = textureId;
    m_eglTextureFrame = egl;
    m_textureMatrix = textureMatrix;
    for (QList<QPair<ARScene*, QScene*>>::iterator it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        it->first->setFrameTexture(m_textureFrameId, m_textureMatrix, m_eglTextureFrame);
    }
}

AR::Point2i ARFrameHandlerRunnable::getFrameSize(const AR::Point2i& baseSize) const
{
    if (m_parentItem == nullptr)
        return baseSize;
    QSize r = m_parentItem->frameResolution();
    if (r.width() == 0) {
        if (r.height() == 0) {
            return baseSize;
        } else {
            return AR::Point2i((int)(baseSize.x * (r.height() / (float)baseSize.y)), r.height());
        }
    } else {
        if (r.height() == 0) {
            return AR::Point2i(r.width(), (int)(baseSize.y * (r.width() / (float)baseSize.x)));
        }
    }
    float rw = r.width() / (float) baseSize.x;
    float rh = r.height() / (float) baseSize.y;
    if (rw < rh) {
        return AR::Point2i(r.width(), baseSize.y * rh);
    }
    return AR::Point2i(baseSize.x * rh, r.height());
}

void ARFrameHandlerRunnable::_getImageFromTexture(AR::Image<AR::Rgba>& outImage, GLuint textureId)
{
    if (m_arFrameFbo == nullptr) {
        AR::Point2i size((int)(std::pow(2, (int)std::ceil(std::log((float)outImage.width()) / std::log(2.0f)))),
                         (int)(std::pow(2, (int)std::ceil(std::log((float)outImage.height()) / std::log(2.0f)))));
        m_arFrameFbo = new QOpenGLFramebufferObject(size.x, size.y, GL_TEXTURE_2D);
    } else if ((m_arFrameFbo->width() < outImage.width()) || (m_arFrameFbo->height() < outImage.height())) {
        AR::Point2i size((int)(std::pow(2, (int)std::ceil(std::log((float)outImage.width()) / std::log(2.0f)))),
                         (int)(std::pow(2, (int)std::ceil(std::log((float)outImage.height()) / std::log(2.0f)))));
        delete m_arFrameFbo;
        m_arFrameFbo = new QOpenGLFramebufferObject(size.x, size.y, GL_TEXTURE_2D);
    }
    m_context->glViewport(0, 0, outImage.width(), outImage.height());
    m_arFrameFbo->bind();

    m_textureRenderer.draw(textureId, m_textureMatrix);

    m_context->glReadPixels(0, 0, outImage.width(), outImage.height(),
                            GL_RGBA, GL_UNSIGNED_BYTE, outImage.data());
}

void ARFrameHandlerRunnable::sendFrameImage(const AR::ImageRef<AR::Rgba>& frameImage,
                                            const QMatrix3x3& textureMatrix,
                                            const QVideoFrame::PixelFormat& pixelFormat)
{
    QMutexLocker ml(&m_mutex);
    QMutexLocker ml2(&m_parentItem->m_mutex);
    if ((frameImage.width() != 0) && (frameImage.height() != 0)) {
        float cameraAspect = m_context->camera->aspect();
        float frameAspect = frameImage.width() / (float)frameImage.height();
        if (std::fabs(cameraAspect - frameAspect) > std::numeric_limits<float>::epsilon()) {
            m_context->camera->setResolution(frameImage.width(), frameImage.height());
            m_context->camera->calculateProjectionMatrix();
        }
    }
    QList<QObject*>& imageReceivers = m_parentItem->m_imageReceivers;
    for (QList<QObject*>::iterator it = imageReceivers.begin(); it != imageReceivers.end(); ++it) {
        FrameReceiver* imageReceiver = static_cast<FrameReceiver*>(*it);
        if (imageReceiver && imageReceiver->enabled()) {
            imageReceiver->receiveFrame(frameImage, textureMatrix, pixelFormat);
        }
    }
}

QVideoFrame ARFrameHandlerRunnable::run(QVideoFrame* input, const QVideoSurfaceFormat& surfaceFormat, RunFlags flags)
{
    Q_UNUSED(surfaceFormat);
    Q_UNUSED(flags);
    using namespace AR;
    if (m_parentItem->m_runnable != this)
        return *input;
    m_mutex.lock();
    m_context->glDisable(GL_DEPTH_TEST);
    m_context->glDepthMask(GL_FALSE);
    QTime time;
    time.start();
    bool rawFrame = true;
    bool frameIsReceived = false;
    m_eglTextureFrame = false;
    GLint defaultFBOId = 0;
    m_context->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBOId);
    if (input->map(QAbstractVideoBuffer::ReadOnly)) {
        if (input->handleType() == QAbstractVideoBuffer::NoHandle) {
            if ((m_sizeFrame.x != input->width()) || (m_sizeFrame.y != input->height()) || (m_externTextureFrame)) {
                _initialize(Point2i(input->width(), input->height()), false, 0);
            }
            Image<Rgba> frameImage(m_sizeFrame, reinterpret_cast<Rgba*>(input->bits()), false);
            /*Image<uchar> si = _arSystem.getCurrentKeyFrame().smallImage;
            if (si.data() != nullptr) {
                Point2f step(160.0f, 120.0f);
                Image<Rgba> sirgb = si.convert<Rgba>([](const uchar& a) { return Rgba(a, a, a); });
                Painter::drawImage<Rgba>(frameImage, sirgb, Point2f(0.0f, 0.0f), step);
                const Map& map = _arSystem.map();
                std::size_t countKeyFrames = map.countKeyFrames();

                Point2f b(0.0f, step.y);
                for (std::size_t i=0; i<countKeyFrames; ++i) {
                    map.keyFrame(i).smallImage.convert<Rgba>(sirgb, [](const uchar& a) { return Rgba(a, a, a); });
                    Painter::drawImage<Rgba>(frameImage, sirgb, b, step);
                    b.y += step.y;
                    if (b.y > frameImage.height()) {
                        b.y = 0.0f;
                        b.x += step.x;
                    }
                    if (b.x > frameImage.width())
                        break;
                }
            }*/

            m_context->glBindTexture(GL_TEXTURE_2D, m_textureFrameId);
#if (__ANDROID__)
            m_context->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameImage.width(), frameImage.height(),
                                       GL_RGBA, GL_UNSIGNED_BYTE, frameImage.data());
#else
            m_context->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameImage.width(), frameImage.height(),
                                       GL_BGRA, GL_UNSIGNED_BYTE, frameImage.data());
#endif
            m_context->glGenerateMipmap(GL_TEXTURE_2D);
            Point2i frameSize = getFrameSize(m_sizeFrame);
            if (frameSize == m_sizeFrame) {
                m_sourceImage = frameImage;
                frameIsReceived = true;
            } else {
                if ((m_sourceImage.size() != frameSize) || (!m_sourceImage.autoDeleting()))
                    m_sourceImage = Image<Rgba>(frameSize);
                _getImageFromTexture(m_sourceImage, m_textureFrameId);
                rawFrame = false;
                frameIsReceived = true;
            }
        } else if (input->handleType() == QAbstractVideoBuffer::GLTextureHandle) {
            GLuint texId = input->handle().toUInt();
            if ((m_sizeFrame.x != input->width()) || (m_sizeFrame.y != input->height()) || (m_textureFrameId != texId)) {
                _initialize(AR::Point2i(input->width(), input->height()), true, texId);
            }
            Point2i frameSize = getFrameSize(m_sizeFrame);
            if (frameSize == m_sizeFrame) {
                frameIsReceived = false;
            } else {
                if ((m_sourceImage.size() != frameSize) || (!m_sourceImage.autoDeleting()))
                    m_sourceImage = Image<Rgba>(frameSize);
                _getImageFromTexture(m_sourceImage, m_textureFrameId);
                rawFrame = false;
                frameIsReceived = true;
            }
        } else {
            input->unmap();
            m_mutex.unlock();
            return *input;
        }
    }

    m_mutex.unlock();
    preDraw();
    m_frameFBO->bind();
    m_context->glViewport(0, 0, m_sizeFrame.x, m_sizeFrame.y);
    _drawFrame();
    if (!frameIsReceived) {
        if ((m_sourceImage.size() != m_sizeFrame) || (!m_sourceImage.autoDeleting()))
            m_sourceImage = Image<Rgba>(m_sizeFrame);
        m_context->glReadPixels(0, 0, m_sizeFrame.x, m_sizeFrame.y, GL_UNSIGNED_BYTE, GL_RGBA, m_sourceImage.data());
    }
    if (rawFrame) {
        sendFrameImage(m_sourceImage, m_textureMatrix, input->pixelFormat());
    } else {
        sendFrameImage(m_sourceImage, QMatrix3x3(), QVideoFrame::Format_BGR32);
    }
    input->unmap();
    drawContent();
    m_context->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBOId);
    m_context->glFlush();
    m_context->glBindTexture(GL_TEXTURE_2D, m_frameFBO->texture());
    m_context->glGenerateMipmap(GL_TEXTURE_2D);
    m_context->glFlush();
    return QVideoFrame(new TextureVideoBuffer(m_frameFBO->texture()), QSize(m_sizeFrame.x, m_sizeFrame.y),
                       QVideoFrame::Format_BGR32);
}
