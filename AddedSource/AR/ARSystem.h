#ifndef AR_ARSYSTEM_H
#define AR_ARSYSTEM_H

#include "TMath/TMath.h"
#include "Image.h"
#include "Camera.h"
#include "Map.h"
#include "MapInitializer.h"
#include "Tracker.h"
#include "MapProjector.h"
#include "LocationOptimizer.h"
#include "PreviewFrame.h"
#include "MapPointsDetector.h"
#include "MapResourcesManager.h"
#include "PerformanceMonitor.h"
#include "Configurations.h"
#include <memory>
#include <vector>
#include <utility>
#include <mutex>

namespace AR {

enum class TrackingState
{
    Undefining,
    CaptureFirstFrame,
    CaptureSecondFrame,
    Tracking,
    LostTracking
};

enum class TrackingQuality
{
    Good, Bad, Ugly
};

class ARSystem
{
public:
    ARSystem();
    ~ARSystem();

    InitConfiguration initConfiguration() const;
    void setInitConfiguration(const InitConfiguration& configuration);

    TrackingConfiguration trackingConfiguration() const;
    void setTrackingConfiguration(const TrackingConfiguration & configuration);

    MapPointsDetectorConfiguration candidatesDetectorConfiguration() const;
    void setMapPointsDetectorConfiguration(const MapPointsDetectorConfiguration & configuration);

    TMath::TVectord cameraParameters() const;
    void setCameraParameters(const TMath::TVectord& cameraParameters);

    double toleranceOfCreatingFrames() const;
    void setToleranceOfCreatingFrames(double toleranceOfCreatingFrames);

    int numberPointsForSructureOptimization() const;
    void setNumberPointsForSructureOptimization(int numberPointsForSructureOptimization);

    int numberIterationsForStructureOptimization() const;
    void setNumberIterationsForStructureOptimization(int numberIterationsForStructureOptimization);

    int maxCountKeyFrames() const;
    void setMaxCountKeyFrames(int count);

    ConstImage<uchar> lastImage() const;

    const std::vector<PreviewFrame::PreviewFeature> & currentFeatures() const;

    TrackingState trackingState() const;
    TrackingQuality  trackingQuality() const;

    TMath::TMatrixd currentRotation() const;
    TMath::TVectord currentTranslation() const;
    TMath::TMatrixd currentTransform() const;

    void reset();

    void nextTrackingState();

    void process(const ImageRef<Rgba> & frame);

    Map * map();
    const Map * map() const;

    MapResourcesManager * mapResourceManager();

    BuilderTypePoint builderTypeMapPoint() const;

    std::vector<std::pair<Point2f, Point2f>> debugTrackedMatches() const;

    std::shared_ptr<const PerformanceMonitor> performanceMonitor() const;
    std::shared_ptr<PerformanceMonitor> performanceMonitor();

private:
    mutable std::mutex m_mutex;

    TMath::TVectord m_cameraParameters;
    int m_numberPointsForSructureOptimization;
    int m_numberIterationsForStructureOptimization;
    double m_toleranceOfCreatingFrames;
    int m_minNumberTrackingPoints;
    int m_preferredNumberTrackingPoints;
    int m_maxCountKeyFrames;
    BuilderTypePoint m_builderTypeMapPoint;
    BuilderTypePoint m_builderTypeCandidatePoint;

    TrackingState m_trackingState;
    TrackingQuality m_trackingQuality;
    std::shared_ptr<const Camera> m_camera;
    PreviewFrame * m_lastFrame;
    std::vector<Image<uchar>> m_currentImagePyramid;
    Map m_map;
    MapResourcesManager m_mapResourceManager;
    MapInitializer m_initializer;
    Tracker m_trackerTransform;
    MapProjector m_mapProjector;
    LocationOptimizer m_locationOptimizer;
    Image<uchar> m_blackWhiteFrame;

    MapPointsDetector m_candidatesDetector;

    std::shared_ptr<PerformanceMonitor> m_performanceMonitor;

    void _converToBlackWhiteFrame(const ImageRef<Rgba> & frame);
    void _buildCurrentImagePyramid();
    void _optimizeMapPoints(PreviewFrame & frame);
    std::shared_ptr<KeyFrame> _createNewKeyFrame(PreviewFrame & previewFrame);
    void _incSuccessScore(PreviewFrame & frame);

    void _reset();
};

}

#endif // AR_ARSYSTEM_H
