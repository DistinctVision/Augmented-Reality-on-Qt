#include "ARCameraCalibrator.h"
#include "QScrollEngine/QScrollEngineContext.h"
#include <QVector2D>
#include <QMatrix4x4>
#include <QStandardPaths>
#include <QSettings>
#include <QMutexLocker>
#include <QDebug>
#include <cassert>
#include "AR/ImageProcessing.h"

ARCameraCalibrator::ARCameraCalibrator():
    ARScene()
{
    m_imagePyramid.resize(1);
    m_pauseOnDetectGrid = false;
    m_imageReceiver = new ARCameraCalibrator_ImageReceiver(this);
    m_textureRenderer = nullptr;
    m_tempFrameTexture = nullptr;
    m_flagUpdateFrameTexture = false;
    m_fixedCameraParamters = TMath::TVector<bool>(5);
    m_fixedCameraParamters.fill(false);

    m_fixedCameraParamters(2) = true;
    m_fixedCameraParamters(3) = true;
    m_fixedCameraParamters(4) = true;
    TMath::TVectord cameraParamters = m_cameraCalibrator.camera()->cameraParameters();
    cameraParamters(1) = cameraParamters(0) * (m_cameraCalibrator.camera()->imageWidth() /
                                               m_cameraCalibrator.camera()->imageHeight());
    cameraParamters(2) = 0.5;
    cameraParamters(3) = 0.5;
    cameraParamters(4) = 0.0;
    m_cameraCalibrator.camera()->setCameraParameters(cameraParamters);

    m_config = new ARCalibrationConfig();
    connect(m_config, &ARCalibrationConfig::configChanged,
            this, &ARCameraCalibrator::updateCalibrationConfig,
            Qt::DirectConnection);
    updateCalibrationConfig();
}

ARCameraCalibrator::~ARCameraCalibrator()
{
    delete m_config;
}

void ARCameraCalibrator::initScene()
{
    scene()->setOrder(9999);
    m_textureRenderer = new TextureRenderer();
    m_tempFrameTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    connect(scene(), &QScrollEngine::QScene::deleting, this, [this]() {
        delete m_tempFrameTexture;
        delete m_textureRenderer;
    }, Qt::DirectConnection);
    connect(scene(), SIGNAL(endDrawing()), this, SLOT(drawCalibrationGrid()), Qt::DirectConnection);
}

AR::Point2i ARCameraCalibrator::cameraResolution() const
{
    AR::Point2d imageSize = m_cameraCalibrator.imageSize();
    return AR::Point2i(static_cast<int>(imageSize.x), static_cast<int>(imageSize.y));
}

TMath::TVectord ARCameraCalibrator::cameraParameters() const
{
    return m_cameraCalibrator.camera()->cameraParameters();
}

void ARCameraCalibrator::setCameraParameters(const TMath::TVectord& parameters)
{
    assert(parameters.size() == 5);
    for (int i = 0; i < 5; ++i) {
        if (std::isnan(parameters[i])) {
            qDebug() << "Camera parameters is nan!!!";
            return;
        }
    }
    m_cameraCalibrator.camera()->setCameraParameters(parameters);
    emit cameraParametersChanged();
}

QPoint ARCameraCalibrator::cameraResolutionQml() const
{
    AR::Point2d cameraRes = m_cameraCalibrator.imageSize();
    return QPoint(static_cast<int>(cameraRes.x), static_cast<int>(cameraRes.y));
}

QList<double> ARCameraCalibrator::cameraParametersQml() const
{
    TMath::TVectord parameters = cameraParameters();
    return QList<double>() << parameters(0) << parameters(1) << parameters(2) << parameters(3) << parameters(4);
}

void ARCameraCalibrator::setCameraParametersQml(const QList<double>& parameters)
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

bool ARCameraCalibrator::fixedOpticalCenter() const
{
    return m_fixedCameraParamters(2);
}

void ARCameraCalibrator::setFixedOpticalCenter(bool enabled)
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

bool ARCameraCalibrator::disabledRadialDistortion() const
{
    return m_fixedCameraParamters(4);
}

void ARCameraCalibrator::setDisabledRadialDistortion(bool disabled)
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

ARCalibrationConfig* ARCameraCalibrator::calibrationConfig()
{
    return m_config;
}

const ARCalibrationConfig* ARCameraCalibrator::calibrationConfig() const
{
    return m_config;
}

bool ARCameraCalibrator::pauseOnDetectGrid() const
{
    return m_pauseOnDetectGrid;
}

void ARCameraCalibrator::setPauseOnDetectGrid(bool pauseOnDetectGrid)
{
    m_pauseOnDetectGrid = pauseOnDetectGrid;
    emit pauseOnDetectGridChanged();
}

ARCameraCalibrator_ImageReceiver* ARCameraCalibrator::imageReceiver()
{
    return m_imageReceiver;
}

const ARCameraCalibrator_ImageReceiver* ARCameraCalibrator::imageReceiver() const
{
    return m_imageReceiver;
}

void ARCameraCalibrator::receiveFrame(const AR::ImageRef<AR::Rgba>& image,
                                      const QMatrix3x3& textureMatrix,
                                      const QVideoFrame::PixelFormat& pixelFormat)
{
    using namespace AR;
    m_textureMatrix = textureMatrix;
    QMutexLocker ml(&m_mutex);
    if (cameraResolution() != image.size()) {
        TMath::TVectord cameraParamters = m_cameraCalibrator.camera()->cameraParameters();
        m_cameraCalibrator.camera()->setImageSize(image.size());
        cameraParamters(1) = cameraParamters(0) * (m_cameraCalibrator.camera()->imageWidth() /
                                                   m_cameraCalibrator.camera()->imageHeight());
        setCameraParameters(cameraParamters);
        emit cameraResolutionChanged();
    }
    if (((int)m_imagePyramid.size() != m_calibrationConfig.countImageLevels) ||
            (m_imagePyramid[0].size() != image.size())) {
        m_imagePyramid.resize(m_calibrationConfig.countImageLevels);
        m_imagePyramid[0] = Image<uchar>(image.size());
        for (std::size_t i=1; i<m_imagePyramid.size(); ++i) {
            m_imagePyramid[i] = Image<uchar>(m_imagePyramid[i-1].size() / 2);
        }
    }
    const Rgba* rgbaPtr = image.data();
    uchar* bwPtr = m_imagePyramid[0].data();
    int area = image.area();
    for (int i=0; i<area; ++i) {
        *bwPtr = (rgbaPtr->red + rgbaPtr->green + rgbaPtr->blue) / 3;
        ++rgbaPtr;
        ++bwPtr;
    }
    for (std::size_t i = 1; i < m_imagePyramid.size(); ++i) {
        ImageProcessing::halfSample(m_imagePyramid[i], m_imagePyramid[i-1]);
    }
    m_lastCalibrationFrame = std::shared_ptr<CalibrationFrame>(
                new CalibrationFrame(m_cameraCalibrator.camera(),
                                     m_imagePyramid,
                                     m_calibrationConfig));
    if (m_lastCalibrationFrame->isCreated()) {
        if (m_pauseOnDetectGrid) {
            m_imageReceiver->setEnabled(false);
        }
        m_lastCalibrationFrame->guessInitialPose();
        emit detectGrid();
        m_lastPixelFormat = pixelFormat;
        if (m_lastImage.size() != image.size())
            m_lastImage = Image<Rgba>(image.size());
        Image<Rgba>::copyData(m_lastImage, image);
        //m_lastCalibrationFrame->drawGridCorners(m_lastImage);
        m_linesOfImageGrid = m_lastCalibrationFrame->debugLinesOfImageGrid();
        m_linesOf3DGrid = m_lastCalibrationFrame->debugLinesOf3DGrid();
        m_errors = m_lastCalibrationFrame->debugLineErrorsOfCalibration(1.0);
        m_flagUpdateFrameTexture = true;
    }
}

bool ARCameraCalibrator::grabGrid()
{
    QMutexLocker ml(&m_mutex);
    if (m_lastCalibrationFrame && m_lastCalibrationFrame->isCreated()) {
        AR::Point2i prevCameraResolution = cameraResolution();
        m_cameraCalibrator.addCalibrationFrame(m_lastCalibrationFrame);
        m_cameraCalibrator.forceOptimize(m_fixedCameraParamters);
        setCameraParameters(m_cameraCalibrator.camera()->cameraParameters());
        if (prevCameraResolution != cameraResolution()) {
            emit cameraResolutionChanged();
        }
        emit cameraParametersChanged();
        return true;
    }
    return false;
}

void ARCameraCalibrator::reset()
{
    m_flagUpdateFrameTexture = false;
    m_cameraCalibrator.reset();
    TMath::TVectord cameraParamters = m_cameraCalibrator.camera()->cameraParameters();
    cameraParamters(1) = cameraParamters(0) * (m_cameraCalibrator.camera()->imageWidth() /
                                               m_cameraCalibrator.camera()->imageHeight());
    if (m_fixedCameraParamters(2)) {
        cameraParamters(2) = 0.5;
        cameraParamters(3) = 0.5;
    }
    if (m_fixedCameraParamters(4)) {
        cameraParamters(4) = 0.0;
    }
    setCameraParameters(cameraParamters);
    emit cameraResolutionChanged();
    m_pauseOnDetectGrid = false;
    emit pauseOnDetectGridChanged();
    setFixedOpticalCenter(fixedOpticalCenter());
    setDisabledRadialDistortion(disabledRadialDistortion());
}

void ARCameraCalibrator::drawCalibrationGrid()
{
    using namespace QScrollEngine;
    QMutexLocker ml(&m_mutex);

    if (scene() == nullptr)
        return;

    if (m_lastCalibrationFrame->isCreated()) {
        QScrollEngineContext* context = scene()->parentContext();
        AR::Point2i frameSize = m_lastCalibrationFrame->imageSize();

        context->glDepthMask(GL_FALSE);
        context->glDisable(GL_DEPTH_TEST);
        context->glDisable(GL_BLEND);

        if (m_flagUpdateFrameTexture) {
            if (m_tempFrameTexture->isCreated()) {
                if ((m_tempFrameTexture->width() != m_lastImage.width()) ||
                        (m_tempFrameTexture->height() != m_lastImage.height())) {
                    m_tempFrameTexture->bind();
                    m_tempFrameTexture->destroy();
                    m_tempFrameTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
                    m_tempFrameTexture->setSize(m_lastImage.width(), m_lastImage.height(), 4);
                    m_tempFrameTexture->allocateStorage();
                } else {
                    m_tempFrameTexture->bind();
                }
            } else {
                m_tempFrameTexture->create();
                m_tempFrameTexture->bind();
                m_tempFrameTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
                m_tempFrameTexture->setSize(m_lastImage.width(), m_lastImage.height(), 4);
                m_tempFrameTexture->allocateStorage();
            }
            if (m_lastPixelFormat == QVideoFrame::Format_BGR32) {
                m_tempFrameTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, m_lastImage.data());
            } else {
                m_tempFrameTexture->setData(QOpenGLTexture::BGRA, QOpenGLTexture::UInt8, m_lastImage.data());
            }
            m_tempFrameTexture->generateMipMaps();
        }

        m_textureRenderer->draw(m_tempFrameTexture->textureId(), m_textureMatrix);

        QMatrix4x4 textureMatrix;
        textureMatrix.fill(0.0f);
        for (int i=0; i<2; ++i) {
            textureMatrix(i, 0) = m_textureMatrix(i, 0);
            textureMatrix(i, 1) = m_textureMatrix(i, 1);
            textureMatrix(i, 3) = m_textureMatrix(i, 2);
        }
        textureMatrix(3, 0) = m_textureMatrix(2, 0);
        textureMatrix(3, 1) = m_textureMatrix(2, 1);
        textureMatrix(3, 3) = m_textureMatrix(2, 2);

        QMatrix4x4 t;
        t.setToIdentity();
        t(0, 0) = 0.5f;
        t(1, 1) = -0.5f;
        t(0, 3) = 0.5f;
        t(1, 3) = 0.5f;

        QMatrix4x4 transformMatrix;
        transformMatrix.ortho(0.0f, (float)(frameSize.x),
                              0.0f, (float)(frameSize.y), 0.0f, 1.0f);
        transformMatrix = t.inverted() * textureMatrix * t * transformMatrix;

        int countLineVertices;
        QVector2D* lineVertices;

        context->glLineWidth(4.0f);

        countLineVertices = (int)(m_linesOfImageGrid.size() * 2);
        lineVertices = new QVector2D[countLineVertices];
        for (int i=0; i<(int)m_linesOfImageGrid.size(); ++i) {
            std::pair<AR::Point2f, AR::Point2f>& line = m_linesOfImageGrid[i];
            lineVertices[i * 2].setX(line.first.x);
            lineVertices[i * 2].setY(line.first.y);
            lineVertices[i * 2 + 1].setX(line.second.x);
            lineVertices[i * 2 + 1].setY(line.second.y);
        }
        context->drawLines(lineVertices, countLineVertices, QColor(0, 155, 255), transformMatrix);
        delete[] lineVertices;

        context->glEnable(GL_BLEND);

        /*countLineVertices = (int)(m_linesOf3DGrid.size() * 2);
        lineVertices = new QVector2D[countLineVertices];
        for (int i=0; i<(int)m_linesOf3DGrid.size(); ++i) {
            std::pair<AR::Point2f, AR::Point2f>& line = m_linesOf3DGrid[i];
            lineVertices[i * 2].setX(line.first.x);
            lineVertices[i * 2].setY(line.first.y);
            lineVertices[i * 2 + 1].setX(line.second.x);
            lineVertices[i * 2 + 1].setY(line.second.y);
        }
        context->drawLines(lineVertices, countLineVertices, QColor(155, 0, 255, 100), transformMatrix);
        delete[] lineVertices;*/

        context->glLineWidth(4.0f);

        countLineVertices = (int)(m_errors.size() * 2);
        lineVertices = new QVector2D[countLineVertices];
        for (int i=0; i<(int)m_errors.size(); ++i) {
            std::pair<AR::Point2f, AR::Point2f>& line = m_errors[i];
            lineVertices[i * 2].setX(line.first.x);
            lineVertices[i * 2].setY(line.first.y);
            lineVertices[i * 2 + 1].setX(line.second.x);
            lineVertices[i * 2 + 1].setY(line.second.y);
        }
        context->drawLines(lineVertices, countLineVertices, QColor(255, 255, 0, 200), transformMatrix);
        delete[] lineVertices;

        context->glDisable(GL_BLEND);
    }
}

void ARCameraCalibrator::updateCalibrationConfig()
{
    m_calibrationConfig = m_config->get();
    m_cameraCalibrator.setConfiguration(m_calibrationConfig);
}

ARCameraCalibrator_ImageReceiver::ARCameraCalibrator_ImageReceiver()
{}

ARCameraCalibrator_ImageReceiver::ARCameraCalibrator_ImageReceiver(ARCameraCalibrator* cameraCalibrator):
    FrameReceiver()
{
    assert(cameraCalibrator != nullptr);
    m_cameraCalibration = cameraCalibrator;
}

void ARCameraCalibrator_ImageReceiver::receiveFrame(const AR::ImageRef<AR::Rgba>& image,
                                                    const QMatrix3x3& textureMatrix,
                                                    const QVideoFrame::PixelFormat& pixelFormat)
{
    if (m_cameraCalibration == nullptr)
        return;
    m_cameraCalibration->receiveFrame(image, textureMatrix, pixelFormat);
}
