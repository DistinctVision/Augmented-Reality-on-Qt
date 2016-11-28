#ifndef ARCAMERACALIBRATOR_H
#define ARCAMERACALIBRATOR_H

#include "ARScene.h"
#include "FrameReceiver.h"
#include "TextureRenderer.h"
#include <qopengl.h>
#include <QOpenGLTexture>
#include <QMatrix3x3>
#include "AR/Image.h"
#include "AR/CameraCalibrator.h"
#include "AR/CalibrationFrame.h"
#include <QMutex>
#include <vector>
#include <QPoint>
#include <utility>
#include "ARCalibrationConfig.h"

class ARCameraCalibrator_ImageReceiver;

class ARCameraCalibrator: public ARScene
{
    Q_OBJECT

    Q_PROPERTY(ARCameraCalibrator_ImageReceiver* imageReceiver READ imageReceiver NOTIFY imageReceiverChaged)
    Q_PROPERTY(bool pauseOnDetectGrid READ pauseOnDetectGrid WRITE setPauseOnDetectGrid NOTIFY pauseOnDetectGridChanged)
    Q_PROPERTY(QPoint cameraResolution READ cameraResolutionQml NOTIFY cameraResolutionChanged)
    Q_PROPERTY(QList<double> cameraParameters READ cameraParametersQml WRITE setCameraParametersQml NOTIFY cameraParametersChanged)
    Q_PROPERTY(bool fixedOpticalCenter READ fixedOpticalCenter WRITE setFixedOpticalCenter NOTIFY fixedOpticalCenterChanged)
    Q_PROPERTY(bool disabledRadialDistortion READ disabledRadialDistortion WRITE setDisabledRadialDistortion
               NOTIFY disabledRadialDistortionChanged)
    Q_PROPERTY(ARCalibrationConfig* calibrationConfig READ calibrationConfig CONSTANT)

public:
    ARCameraCalibrator();
    ~ARCameraCalibrator();

    ARCameraCalibrator_ImageReceiver* imageReceiver();
    const ARCameraCalibrator_ImageReceiver* imageReceiver() const;

    void receiveFrame(const AR::ImageRef<AR::Rgba>& image,
                      const QMatrix3x3& textureMatrix,
                      const QVideoFrame::PixelFormat& pixelFormat);

    bool pauseOnDetectGrid() const;
    void setPauseOnDetectGrid(bool pauseOnDetectGrid);

    Q_INVOKABLE bool grabGrid();
    Q_INVOKABLE void reset();

    AR::Point2i cameraResolution() const;
    TMath::TVectord cameraParameters() const;
    void setCameraParameters(const TMath::TVectord& parameters);

    QPoint cameraResolutionQml() const;
    QList<double> cameraParametersQml() const;
    void setCameraParametersQml(const QList<double>& parameters);

    bool fixedOpticalCenter() const;
    void setFixedOpticalCenter(bool enabled);

    bool disabledRadialDistortion() const;
    void setDisabledRadialDistortion(bool disabled);

    ARCalibrationConfig* calibrationConfig();
    const ARCalibrationConfig* calibrationConfig() const;

public slots:
    void drawCalibrationGrid();
    void updateCalibrationConfig();

signals:
    void cameraResolutionChanged();
    void cameraParametersChanged();
    void imageReceiverChaged();
    void pauseOnDetectGridChanged();
    void fixedOpticalCenterChanged();
    void disabledRadialDistortionChanged();
    void detectGrid();

protected:
    void initScene() override;

private:
    QMutex m_mutex;
    bool m_pauseOnDetectGrid;
    TextureRenderer* m_textureRenderer;
    QOpenGLTexture* m_tempFrameTexture;
    bool m_flagUpdateFrameTexture;
    QMatrix3x3 m_textureMatrix;
    QVideoFrame::PixelFormat m_lastPixelFormat;
    AR::Image<AR::Rgba> m_lastImage;
    std::vector<AR::Image<uchar>> m_imagePyramid;
    AR::CameraCalibrator m_cameraCalibrator;
    std::shared_ptr<AR::CalibrationFrame> m_lastCalibrationFrame;
    AR::CalibrationConfiguration m_calibrationConfig;
    TMath::TVector<bool> m_fixedCameraParamters;
    ARCameraCalibrator_ImageReceiver* m_imageReceiver;
    ARCalibrationConfig* m_config;

    std::vector<std::pair<AR::Point2f, AR::Point2f>> m_linesOfImageGrid;
    std::vector<std::pair<AR::Point2f, AR::Point2f>> m_linesOf3DGrid;
    std::vector<std::pair<AR::Point2f, AR::Point2f>> m_errors;
};

class ARCameraCalibrator_ImageReceiver: public FrameReceiver
{
    Q_OBJECT

public:
    ARCameraCalibrator_ImageReceiver();
    ARCameraCalibrator_ImageReceiver(ARCameraCalibrator* cameraCalibrator);

    void receiveFrame(const AR::ImageRef<AR::Rgba>& image,
                      const QMatrix3x3& textureMatrix,
                      const QVideoFrame::PixelFormat& pixelFormat) override;

private:
    ARCameraCalibrator* m_cameraCalibration;
};

#endif // ARCAMERACALIBRATOR_H
