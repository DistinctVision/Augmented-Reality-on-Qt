#include "AndroidCameraView.h"
#include <QList>
#include <limits>
#include <climits>
#include <memory>
#include <cstring>
#include <QtGui/qopengl.h>
#include <QScreen>
#include <QtMath>
#include <QMutexLocker>
#include "AR/ImageProcessing.h"
#include <QTime>
#include <QDebug>

#if defined(Q_OS_ANDROID)

AndroidCameraView::AndroidCameraView()
{
    setFlag(QQuickItem::ItemHasContents);
    m_arFrameHandler = nullptr;
    m_camera = nullptr;
    m_surfaceTexture = nullptr;
    m_renderer = nullptr;
    m_selectedCameraId = -1;
    m_targetPreviewSize = QSize(320, 0);
    m_surfaceTextureMatrix.setToIdentity();
    m_surfaceTextureMatrix(1, 1) = - 1.0f;
    m_surfaceTextureMatrix(1, 2) = 1.0f;
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));
}

AndroidCameraView::~AndroidCameraView()
{
    m_mutex.lock();
    if (m_arFrameHandler != nullptr)
        m_arFrameHandler->setParent(nullptr);
    m_mutex.unlock();
}

int AndroidCameraView::selectedCameraId() const
{
    QMutexLocker ml(&m_mutex);
    return m_selectedCameraId;
}

void AndroidCameraView::setSelectedCameraId(int selectedCameraId)
{
    if (selectedCameraId == m_selectedCameraId)
        return;
    m_mutex.lock();
    m_selectedCameraId = selectedCameraId;
    m_mutex.unlock();
    emit selectedCameraIdChanged();
}

AndroidJniCamera::AndroidCameraInfo AndroidCameraView::getCameraInfo() const
{
    QMutexLocker ml(&m_mutex);
    if (m_camera == nullptr)
        return AndroidJniCamera::AndroidCameraInfo();
    AndroidJniCamera::AndroidCameraInfo cameraInfo;
    AndroidJniCamera::getCameraInfo(m_camera->cameraId(), &cameraInfo);
    return cameraInfo;
}

QList<QString> AndroidCameraView::getAviableCameras() const
{
    int countDevices = AndroidJniCamera::getNumberOfCameras();
    AndroidJniCamera::AndroidCameraInfo cameraInfo;
    QList<QString> devices;
    for (int i = 0; i < countDevices; ++i) {
        AndroidJniCamera::getCameraInfo(i, &cameraInfo);
        devices.push_back(cameraInfo.description);
    }
    return devices;
}

void AndroidCameraView::handleWindowChanged(QQuickWindow* win)
{
    if (win) {
        connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
        connect(win, SIGNAL(sceneGraphInvalidated()), this, SLOT(cleanup()), Qt::DirectConnection);
        win->setClearBeforeRendering(false);
    }
}

QMatrix3x3 AndroidCameraView::surfaceTextureMatrix() const
{
    QMutexLocker ml(&m_mutex);
    return m_surfaceTextureMatrix;
}

ARFrameHandler* AndroidCameraView::arFrameHandler() const
{
    QMutexLocker ml(&m_mutex);
    return m_arFrameHandler;
}

void AndroidCameraView::setArFrameHandler(ARFrameHandler* arFrameHandler)
{
    if (m_arFrameHandler == arFrameHandler)
        return;
    m_mutex.lock();
    if (m_arFrameHandler != nullptr)
        m_arFrameHandler->setParent(nullptr);
    m_arFrameHandler = arFrameHandler;
    if (m_arFrameHandler != nullptr)
        m_arFrameHandler->setParent(this);
    if (m_renderer)
        m_renderer->setArFrameHandler(m_arFrameHandler);
    m_mutex.unlock();
    emit arFrameHandlerChanged();
}

bool AndroidCameraView::cameraRunning() const
{
    QMutexLocker ml(&m_mutex);
    return (m_camera != nullptr);
}

AndroidJniSurfaceTexture* AndroidCameraView::surfaceTexture() const
{
    QMutexLocker ml(&m_mutex);
    return m_surfaceTexture;
}

QSize AndroidCameraView::previewSize() const
{
    QMutexLocker ml(&m_mutex);
    return m_previewSize;
}

QSize AndroidCameraView::targetPreviewSize() const
{
    QMutexLocker ml(&m_mutex);
    return m_targetPreviewSize;
}

void AndroidCameraView::setTargetPreviewSize(const QSize& targetPreviewSize)
{
    if (m_targetPreviewSize == targetPreviewSize)
        return;
    m_mutex.lock();
    m_targetPreviewSize = targetPreviewSize;
    _updatePreviewSize();
    m_mutex.unlock();
    emit targetPreviewSizeChanged();
}

void AndroidCameraView::_updatePreviewSize()
{
    if (m_camera == nullptr)
        return;
    QList<QSize> listPreviewSizes = m_camera->getSupportedPreviewSizes();
    if (!listPreviewSizes.isEmpty()) {
        QSize targetSize = m_targetPreviewSize;
        if (targetSize.width() <= 0)
            targetSize.setWidth(std::numeric_limits<int>::max());
        if (targetSize.height() <= 0)
            targetSize.setHeight(std::numeric_limits<int>::max());
        int targetItem = 0;
        for (int i=1; i<listPreviewSizes.size(); ++i) {
            if ((listPreviewSizes[i].width() <= targetSize.width()) &&
                    (listPreviewSizes[i].height() <= targetSize.height())) {
                targetItem = i;
            } else {
                break;
            }
        }
        m_previewSize = listPreviewSizes[targetItem];
        m_camera->setPreviewSize(m_previewSize);
    } else {
        m_previewSize = m_camera->previewSize();
    }
}

void AndroidCameraView::sync()
{
    if (window() == nullptr) {
        cleanup();
        return;
    }
    m_mutex.lock();
    if (m_selectedCameraId < 0) {
        m_mutex.unlock();
        cleanup();
    } else {
        if (!m_renderer) {
            m_renderer = new AndroidCameraViewRenderer(this);
            if (m_arFrameHandler)
                m_renderer->setArFrameHandler(m_arFrameHandler);
            connect(window(), SIGNAL(beforeRendering()), m_renderer, SLOT(render()), Qt::DirectConnection);
        }
        if (m_camera) {
            if (m_camera && m_camera->cameraId() != m_selectedCameraId) {
                m_camera->stopPreview();
                //m_camera->unlock();
                delete m_camera;
                m_camera = nullptr;
            }
        }
        m_mutex.unlock();
    }
}

void AndroidCameraView::cleanup()
{
    m_mutex.lock();
    if (m_renderer) {
        if (m_surfaceTexture) {
            GLuint textureId = (GLuint)m_surfaceTexture->textureID();
            delete m_surfaceTexture;
            m_renderer->glDeleteTextures(1, &textureId);
        }
        delete m_renderer;
        m_renderer = nullptr;
    }
    if (m_camera) {
        m_camera->stopPreview();
        //m_camera->unlock();
        delete m_camera;
        m_camera = nullptr;
    }
    m_mutex.unlock();
}

QRectF AndroidCameraView::boundingRect() const
{
    QMutexLocker ml(&m_mutex);
    if ((m_previewSize.width() == 0) || (m_previewSize.height() == 0))
        return QRectF(0.0f, 0.0f, 0.0f, 0.0f);
    float cameraRatio = m_previewSize.width() / (float)m_previewSize.height();
    float w = width(), h = height();
    float tw = h * cameraRatio;
    if (tw <= w) {
        return QRectF(x() + (w - tw) * 0.5f, y(), tw, h);
    }
    float th = w / cameraRatio;
    return QRectF(x(), y() + (h - th) * 0.5f, w, th);
}

bool AndroidCameraView::_startPreview(QOpenGLFunctions* glFunctions)
{
    if (m_camera != nullptr)
        return false;
    m_camera = AndroidJniCamera::open(m_selectedCameraId);
    if (m_camera == nullptr) {
        return false;
    }
    /*if (!m_camera->lock()) {
        delete m_camera;
        m_camera = nullptr;
        return false;
    }*/
    _updatePreviewSize();
    GLuint textureId;
    glFunctions->glGenTextures(1, &textureId);
    m_surfaceTexture = new AndroidJniSurfaceTexture(textureId);
    connect(m_surfaceTexture, SIGNAL(frameAvailable()), m_renderer, SLOT(frameAvailable()), Qt::DirectConnection);
    if (!m_camera->setPreviewTexture(m_surfaceTexture)) {
        //m_camera->unlock();
        delete m_surfaceTexture;
        delete m_camera;
        glFunctions->glDeleteTextures(1, &textureId);
        m_camera = nullptr;
        return false;
    }
    m_camera->setRecordingHint(true);
    m_camera->set("orientation", "landscape");
    m_camera->setRotation(0);
    m_camera->setFocusMode("infinity");
    //m_camera->setFlashMode("torch");
    m_camera->startPreview();
    emit previewSizeChanged();
    return true;
}

const GLfloat AndroidCameraViewRenderer::gl_vertex_data[8] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    -1.0f, 1.0f,
    1.0f, 1.0f
};

const GLfloat AndroidCameraViewRenderer::gl_texCoord_data[8] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};

AndroidCameraViewRenderer::AndroidCameraViewRenderer(AndroidCameraView* parentView)
{
    m_view = parentView;
    m_frameAvailable = true;
    m_shaderProgramRGB = nullptr;
    m_shaderProgramBW = nullptr;
    m_arFrameHandler = nullptr;
    m_arFrameHandlerRunnable = nullptr;
    m_frameBufferObject = nullptr;
    m_pixelsReader = nullptr;
}

AndroidCameraViewRenderer::~AndroidCameraViewRenderer()
{
    QMutexLocker ml(&m_mutex);
    if (m_frameBufferObject != nullptr)
        delete m_frameBufferObject;
    if (m_pixelsReader != 0)
        delete m_pixelsReader;
    if (m_shaderProgramRGB) {
        delete m_shaderProgramRGB;
        delete m_shaderProgramBW;
    }
    if (m_arFrameHandlerRunnable)
        delete m_arFrameHandlerRunnable;
}

void AndroidCameraViewRenderer::frameAvailable()
{
    QMutexLocker ml(&m_mutex);
    m_frameAvailable = true;
}

ARFrameHandler* AndroidCameraViewRenderer::arFrameHandler() const
{
    QMutexLocker ml(&m_mutex);
    return m_arFrameHandler;
}

void AndroidCameraViewRenderer::setArFrameHandler(ARFrameHandler* arFrameHandler)
{
    QMutexLocker ml(&m_mutex);
    if (m_arFrameHandler == arFrameHandler)
        return;
    m_arFrameHandler = arFrameHandler;
}

void AndroidCameraViewRenderer::_initGL()
{
    initializeOpenGLFunctions();

    QOpenGLShader vertexShader(QOpenGLShader::Vertex);
    vertexShader.compileSourceCode("attribute highp vec2 vPosition;\n" \
                                   "attribute highp vec2 vTexCoords;\n" \
                                   "uniform mat3 textureMatrix;\n" \
                                   "varying highp vec2 textureCoords;\n" \
                                   "void main() {\n" \
                                   "    gl_Position = vec4(vPosition, 0.5, 1);\n" \
                                   "    textureCoords = (textureMatrix * vec3(vTexCoords, 1)).xy;\n" \
                                   "}");

    m_shaderProgramRGB = new QOpenGLShaderProgram();
    m_shaderProgramRGB->addShader(&vertexShader);
    m_shaderProgramRGB->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       "#extension GL_OES_EGL_image_external : require\n" \
                                       "varying highp vec2 textureCoords;\n" \
                                       "uniform samplerExternalOES frameTexture;\n" \
                                       "void main() {\n" \
                                       "    gl_FragColor = texture2D(frameTexture, textureCoords);\n" \
                                       "}");
    m_shaderProgramRGB->link();
    m_shaderProgramRGB->bindAttributeLocation("vPosition", 0);
    m_shaderProgramRGB->bindAttributeLocation("vTexCoords", 1);

    m_shaderProgramBW = new QOpenGLShaderProgram();
    m_shaderProgramBW->addShader(&vertexShader);
    m_shaderProgramBW->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                               "varying highp vec2 textureCoords;\n" \
                                               "uniform sampler2D frameTexture;\n" \
                                               "void main() {\n" \
                                               "    gl_FragColor = texture2D(frameTexture, textureCoords);\n" \
                                               "}");

    m_shaderProgramBW->link();
    m_shaderProgramBW->bindAttributeLocation("vPosition", 0);
    m_shaderProgramBW->bindAttributeLocation("vTexCoords", 1);
}

void AndroidCameraViewRenderer::render()
{
    using namespace AR;

    QMutexLocker ml(&m_mutex);
    if (!m_shaderProgramRGB) {
        _initGL();
    }

    bool draw = true;
    if (!m_view->cameraRunning()) {
        if (!m_view->_startPreview(this)) {
            draw = false;
        }
        m_frameAvailable = false;
    }

    AndroidJniSurfaceTexture* surfaceTexture = m_view->surfaceTexture();

    if (m_frameAvailable) {
        surfaceTexture->updateTexImage();
        m_frameAvailable = false;
    }

    QQuickWindow* window = m_view->window();

    if (draw) {
        if (m_arFrameHandlerRunnable) {
            if (m_arFrameHandlerRunnable->parentItem() != m_arFrameHandler) {
                delete m_arFrameHandlerRunnable;
                m_arFrameHandlerRunnable = nullptr;
            }
        }
        if (!m_arFrameHandlerRunnable) {
            if (m_arFrameHandler) {
                 m_arFrameHandlerRunnable = dynamic_cast<ARFrameHandlerRunnable*>(m_arFrameHandler->createFilterRunnable());
                 m_arFrameHandlerRunnable->setFrameTexture(surfaceTexture->textureID(), m_view->surfaceTextureMatrix(), true);
            }
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        if (m_arFrameHandlerRunnable) {
            m_arFrameHandlerRunnable->preDraw();
            QSize previewSize = m_view->previewSize();
            Point2i arFrameSize = m_arFrameHandlerRunnable->getFrameSize(AR::Point2i(previewSize.width(),
                                                                                     previewSize.height()));

            if ((m_pixelsReader == nullptr) || (m_pixelsReader->size() != arFrameSize) ||
                    (m_frameBufferObject == nullptr)) {
                if (m_frameBufferObject != nullptr)
                    delete m_frameBufferObject;
                m_frameBufferObject = new QOpenGLFramebufferObject(arFrameSize.x, arFrameSize.y);
                if (m_pixelsReader == nullptr)
                    m_pixelsReader = new PixelsReader(arFrameSize);
                else
                    m_pixelsReader->resize(arFrameSize);
            }

            m_frameBufferObject->bind();

            glViewport(0, 0, arFrameSize.x, arFrameSize.y);

            m_shaderProgramRGB->bind();

            m_shaderProgramRGB->enableAttributeArray(0);
            m_shaderProgramRGB->enableAttributeArray(1);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, gl_vertex_data);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, gl_texCoord_data);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, surfaceTexture->textureID());
            m_shaderProgramRGB->setUniformValue("frameTexture", 0);
            m_shaderProgramRGB->setUniformValue("textureMatrix", m_view->surfaceTextureMatrix());

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            m_pixelsReader->read();

            m_shaderProgramRGB->disableAttributeArray(1);
            m_shaderProgramRGB->disableAttributeArray(0);
            //m_shaderProgramRGB->release();

            m_frameBufferObject->release();

            m_arFrameHandlerRunnable->sendFrameImage(m_pixelsReader->image(), QMatrix3x3(), QVideoFrame::Format_BGR32);
        }

        glViewport(0, 0, window->width(), window->height());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        QRectF rect = m_view->boundingRect();
        glViewport(qRound(rect.x()), qRound((window->height() - (rect.y() + rect.height()))),
                   qRound(rect.width()), qRound(rect.height()));

        m_shaderProgramRGB->bind();

        m_shaderProgramRGB->enableAttributeArray(0);
        m_shaderProgramRGB->enableAttributeArray(1);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, gl_vertex_data);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, gl_texCoord_data);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, surfaceTexture->textureID());
        m_shaderProgramRGB->setUniformValue("frameTexture", 0);
        m_shaderProgramRGB->setUniformValue("textureMatrix", m_view->surfaceTextureMatrix());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_shaderProgramRGB->disableAttributeArray(1);
        m_shaderProgramRGB->disableAttributeArray(0);
        m_shaderProgramRGB->release();

        if (m_arFrameHandlerRunnable)
            m_arFrameHandlerRunnable->drawContent();
    } else {
        glViewport(0, 0, window->width(), window->height());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    window->update();
}

#endif
