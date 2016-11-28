#ifndef ARTRACKER_H
#define ARTRACKER_H

#include <vector>
#include <utility>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix3x3>
#include <QList>
#include <QVideoFrame>
#include "AR/Image.h"
#include "AR/Point2.h"
#include "AR/ARSystem.h"
#include "TMath/TVector.h"
#include "FrameReceiver.h"
#include "ARInitConfig.h"
#include "ARTrackerConfig.h"
#include "ARMapPointsDetectorConfig.h"

class ARTracker: public FrameReceiver
{
    Q_OBJECT

    Q_ENUMS(TrackingState)
    Q_ENUMS(TrackingQuality)

    Q_PROPERTY(QList<double> cameraParameters READ cameraParametersQml WRITE setCameraParametersQml NOTIFY cameraParametersChanged)
    Q_PROPERTY(ARInitConfig* initConfig READ initConfig CONSTANT)
    Q_PROPERTY(ARMapPointsDetectorConfig* mapPointsDetectorConfig READ mapPointsDetectorConfig CONSTANT)
    Q_PROPERTY(ARTrackerConfig* trackerConfig READ trackerConfig CONSTANT)
    Q_PROPERTY(TrackingState trackingState READ trackingState NOTIFY trackingStateChanged)
    Q_PROPERTY(TrackingQuality trackingQuality READ trackingQuality NOTIFY trackingQualityChanged)
public:
    enum TrackingState: int {
        Undefining = (int)AR::TrackingState::Undefining,
        CaptureFirstFrame = (int)AR::TrackingState::CaptureFirstFrame,
        CaptureSecondFrame = (int)AR::TrackingState::CaptureSecondFrame,
        Tracking = (int)AR::TrackingState::Tracking,
        LostTracking = (int)AR::TrackingState::LostTracking
    };

    enum TrackingQuality: int {
        Good = (int)AR::TrackingQuality::Good,
        Bad = (int)AR::TrackingQuality::Bad,
        Ugly = (int)AR::TrackingQuality::Ugly
    };

    ARTracker();
    ~ARTracker();

    const ARInitConfig* initConfig() const;
    ARInitConfig* initConfig();

    const ARTrackerConfig* trackerConfig() const;
    ARTrackerConfig* trackerConfig();

    const ARMapPointsDetectorConfig* mapPointsDetectorConfig() const;
    ARMapPointsDetectorConfig* mapPointsDetectorConfig();

    const AR::ARSystem& arSystem() const;
    AR::ARSystem& arSystem();

    TMath::TVectord cameraParameters() const;
    void setCameraParameters(TMath::TVectord parameters);

    QList<double> cameraParametersQml() const;
    void setCameraParametersQml(const QList<double>& parameters);

    QQuaternion orientation() const;
    QVector3D position() const;

    AR::Point2i imageSize() const;
    std::vector<std::pair<AR::Point2f, AR::Point2f>> debugTrackedMatches() const;

    QMatrix3x3 textureMatrix();

    TrackingState trackingState() const;
    TrackingQuality trackingQuality() const;

    void receiveFrame(const AR::ImageRef<AR::Rgba>& image,
                      const QMatrix3x3& textureMatrix,
                      const QVideoFrame::PixelFormat& pixelFormat) override;

public slots:
   void nextTrackingState();
   void updateInitConfig();
   void updateTrackerConfig();
   void updateMapPointsDetectorConfig();

signals:
    void cameraParametersChanged();
    void trackingStateChanged();
    void trackingQualityChanged();

private:
    AR::ARSystem m_arSystem;
    QMatrix3x3 m_textureMatrix;
    AR::Point2i m_imageSize;
    ARInitConfig* m_initConfig;
    ARTrackerConfig* m_trackerConfig;
    ARMapPointsDetectorConfig* m_mapPointsDetectorConfig;
    TrackingState m_trackingState;
    TrackingQuality m_trackingQuality;
};


#endif // ARTRACKER_H
