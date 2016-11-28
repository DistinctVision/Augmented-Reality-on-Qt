#include "ARTracker.h"
#include "QScrollEngine/QOtherMathFunctions.h"

ARTracker::ARTracker():
        FrameReceiver()
{
    m_imageSize.setZero();
    m_initConfig = new ARInitConfig();
    m_trackerConfig = new ARTrackerConfig();
    m_mapPointsDetectorConfig = new ARMapPointsDetectorConfig();
    connect(m_initConfig, &ARInitConfig::configChanged, this, &ARTracker::updateInitConfig);
    connect(m_trackerConfig, &ARTrackerConfig::configChanged, this, &ARTracker::updateTrackerConfig);
    connect(m_mapPointsDetectorConfig, &ARMapPointsDetectorConfig::configChanged, this, &ARTracker::updateMapPointsDetectorConfig);
    updateInitConfig();
    updateTrackerConfig();
    updateMapPointsDetectorConfig();
    m_trackingState = Undefining;
    m_trackingQuality = Ugly;
}

ARTracker::~ARTracker()
{
    delete m_initConfig;
    delete m_trackerConfig;
    delete m_mapPointsDetectorConfig;
}

ARInitConfig* ARTracker::initConfig()
{
    return m_initConfig;
}

const ARInitConfig* ARTracker::initConfig() const
{
    return m_initConfig;
}

ARTrackerConfig* ARTracker::trackerConfig()
{
    return m_trackerConfig;
}

const ARTrackerConfig* ARTracker::trackerConfig() const
{
    return m_trackerConfig;
}

ARMapPointsDetectorConfig* ARTracker::mapPointsDetectorConfig()
{
    return m_mapPointsDetectorConfig;
}

const ARMapPointsDetectorConfig* ARTracker::mapPointsDetectorConfig() const
{
    return m_mapPointsDetectorConfig;
}

void ARTracker::updateInitConfig()
{
    m_arSystem.setInitConfiguration(m_initConfig->get());
}

void ARTracker::updateTrackerConfig()
{
    m_arSystem.setTrackingConfiguration(m_trackerConfig->get());
}

void ARTracker::updateMapPointsDetectorConfig()
{
    m_arSystem.setMapPointsDetectorConfiguration(m_mapPointsDetectorConfig->get());
}

const AR::ARSystem& ARTracker::arSystem() const
{
    return m_arSystem;
}

AR::ARSystem& ARTracker::arSystem()
{
    return m_arSystem;
}

void ARTracker::nextTrackingState()
{
    m_arSystem.nextTrackingState();
    TrackingState trackingState = TrackingState((int)m_arSystem.trackingState());
    TrackingQuality trackingQuality = TrackingQuality((int)m_arSystem.trackingQuality());
    if (trackingState != m_trackingState) {
        m_trackingState = trackingState;
        emit trackingStateChanged();
    }
    if (trackingQuality != m_trackingQuality) {
        m_trackingQuality = trackingQuality;
        emit trackingQualityChanged();
    }
}

ARTracker::TrackingState ARTracker::trackingState() const
{
    return m_trackingState;
}

ARTracker::TrackingQuality ARTracker::trackingQuality() const
{
    return m_trackingQuality;
}

TMath::TVectord ARTracker::cameraParameters() const
{
    TMath::TVectord parameters = m_arSystem.cameraParameters();
    parameters(0) = - parameters(0);
    return parameters;
}

void ARTracker::setCameraParameters(TMath::TVectord parameters)
{
    assert(parameters.size() == 5);

    //for opengl camera
    parameters(0) = - parameters(0);

    m_arSystem.setCameraParameters(parameters);
    emit cameraParametersChanged();
}

QList<double> ARTracker::cameraParametersQml() const
{
    TMath::TVectord parameters = cameraParameters();
    return QList<double>() << parameters(0) << parameters(1) << parameters(2) << parameters(3) << parameters(4);
}

void ARTracker::setCameraParametersQml(const QList<double>& parameters)
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

QQuaternion ARTracker::orientation() const
{
    using namespace TMath;
    using namespace QScrollEngine;

    QQuaternion orientation;
    TMatrixd rotation = m_arSystem.currentRotation();
    rotation.setRow(0, - rotation.getRow(0));
    rotation.setRow(2, - rotation.getRow(2));
    QOtherMathFunctions::matrixToQuaternion(TTools::convert<QMatrix3x3>(rotation), orientation);
    return orientation;
}

QVector3D ARTracker::position() const
{
    using namespace TMath;

    TVectord translation = m_arSystem.currentTranslation();
    translation(0) = - translation(0);
    translation(2) = - translation(2);
    return TTools::convert<QVector3D>(translation);
}

AR::Point2i ARTracker::imageSize() const
{
    return m_imageSize;
}

std::vector<std::pair<AR::Point2f, AR::Point2f>> ARTracker::debugTrackedMatches() const
{
    return m_arSystem.debugTrackedMatches();
}

QMatrix3x3 ARTracker::textureMatrix()
{
    return m_textureMatrix;
}

void ARTracker::receiveFrame(const AR::ImageRef<AR::Rgba>& image,
                             const QMatrix3x3& textureMatrix,
                             const QVideoFrame::PixelFormat& pixelFormat)
{
    Q_UNUSED(pixelFormat);
    m_arSystem.process(image);
    m_imageSize = image.size();
    m_textureMatrix = textureMatrix;
    TrackingState trackingState = TrackingState((int)m_arSystem.trackingState());
    TrackingQuality trackingQuality = TrackingQuality((int)m_arSystem.trackingQuality());
    if (trackingState != m_trackingState) {
        m_trackingState = trackingState;
        emit trackingStateChanged();
    }
    if (trackingQuality != m_trackingQuality) {
        m_trackingQuality = trackingQuality;
        emit trackingQualityChanged();
    }
}
