#include "ARSystem.h"
#include "MapPoint.h"
#include "MapResourceLocker.h"
#include "ImageProcessing.h"
#include "TMath/TTools.h"
#include "ZMSSD.h"
#include <algorithm>
#include <iostream>

namespace AR {

ARSystem::ARSystem():
    m_map(3, 32),
    m_candidatesDetector(&m_map)
{
    using namespace TMath;

    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;

    m_mapProjector.setMap(&m_map);
    m_mapProjector.setMapResourceManager((&m_mapResourceManager));
    m_mapProjector.setMapPointsDetector(&m_candidatesDetector);
    m_mapProjector.setBuilderTypeMapPoint(&m_builderTypeMapPoint);
    m_mapProjector.setBuilderTypeCandidatePoint(&m_builderTypeCandidatePoint);
    m_locationOptimizer.setBuilderTypeMapPoint(&m_builderTypeMapPoint);
    m_locationOptimizer.setMapResourceManager(&m_mapResourceManager);
    m_trackerTransform.setMapResourceManager(&m_mapResourceManager);
    m_numberPointsForSructureOptimization = 30;
    m_numberIterationsForStructureOptimization = 10;
    m_toleranceOfCreatingFrames = 0.15;
    m_minNumberTrackingPoints = 15;
    m_preferredNumberTrackingPoints = 25;
    m_maxCountKeyFrames = 10;
    m_trackingState = TrackingState::Undefining;
    m_trackingQuality = TrackingQuality::Ugly;
    m_cameraParameters = Camera::defaultCameraParameters;
    //m_candidatesDetector.startThread();
    m_currentImagePyramid.resize(m_map.countImageLevels());
    m_lastFrame = new PreviewFrame(m_camera, m_currentImagePyramid, TMatrixd::Identity(3), TVectord::create(0.0, 0.0, 0.0));
    m_performanceMonitor = std::shared_ptr<PerformanceMonitor>(new PerformanceMonitor());
}

ARSystem::~ARSystem()
{
    m_candidatesDetector.stopThread();
    {
        std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
        delete m_lastFrame;
    }
}

InitConfiguration ARSystem::initConfiguration() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_initializer.configuration();
}

void ARSystem::setInitConfiguration(const InitConfiguration & configuration)
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    m_initializer.setConfiguration(configuration);
}

TrackingConfiguration ARSystem::trackingConfiguration() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;

    TrackingConfiguration configuration;

    configuration.countImageLevels = m_map.countImageLevels();
    configuration.sizeOfSmallImage = m_map.sizeOfSmallImage();
    configuration.mapPoint_goodSuccessLimit = m_builderTypeMapPoint.goodSuccessLimit();
    configuration.mapPoint_failedLimit = m_builderTypeMapPoint.failedLimit();
    configuration.candidate_failedLimit = m_builderTypeCandidatePoint.failedLimit();
    configuration.maxNumberOfUsedKeyFrames = m_mapProjector.maxNumberOfUsedKeyFrames();
    configuration.frameBorder = m_mapProjector.frameBorder();
    configuration.maxNumberOfFeaturesOnFrame = m_mapProjector.maxNumberOfFeaturesOnFrame();
    configuration.frameGridSize = m_mapProjector.gridSize();
    configuration.featureCursorSize = m_mapProjector.cursorSize();
    configuration.pixelEps = m_mapProjector.pixelEps();
    configuration.locationEps = m_locationOptimizer.eps();
    configuration.locationMaxPixelError = std::sqrt(m_locationOptimizer.maxSquarePixelError());
    configuration.locationNumberIterations = m_locationOptimizer.numberIterations();
    configuration.numberPointsForSructureOptimization = m_numberPointsForSructureOptimization;
    configuration.numberIterationsForStructureOptimization = m_numberIterationsForStructureOptimization;
    configuration.toleranceOfCreatingFrames = m_toleranceOfCreatingFrames;
    configuration.minNumberTrackingPoints = m_minNumberTrackingPoints;
    configuration.preferredNumberTrackingPoints = m_preferredNumberTrackingPoints;
    configuration.maxCountKeyFrames = m_maxCountKeyFrames;
    configuration.featureMaxNumberIterations = m_mapProjector.maxNumberIterations();
    configuration.tracker_eps = m_trackerTransform.eps();
    configuration.tracker_numberIterations = m_trackerTransform.numberIterations();
    configuration.tracker_minImageLevel = m_trackerTransform.minLevel();
    configuration.tracker_maxImageLevel = m_trackerTransform.maxLevel();
    configuration.tracker_cursorSize = m_trackerTransform.cursorSize();

    return configuration;
}

void ARSystem::setTrackingConfiguration(const TrackingConfiguration & configuration)
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
\
    m_currentImagePyramid.resize(configuration.countImageLevels);
    m_candidatesDetector.setCountImageLevels(configuration.countImageLevels);
    m_map.setCountImageLevels(&m_mapResourceManager, configuration.countImageLevels);
    m_map.setSizeOfSmallImage(&m_mapResourceManager, configuration.sizeOfSmallImage);
    m_builderTypeMapPoint.setGoodSuccessLimit(configuration.mapPoint_goodSuccessLimit);
    m_builderTypeMapPoint.setFailedLimit(configuration.mapPoint_failedLimit);
    m_builderTypeCandidatePoint.setGoodSuccessLimit(100);
    m_builderTypeCandidatePoint.setFailedLimit(configuration.candidate_failedLimit);
    m_mapProjector.setMaxNumberOfUsedKeyFrames(configuration.maxNumberOfUsedKeyFrames);
    m_mapProjector.setFrameBorder(configuration.frameBorder);
    m_mapProjector.setMaxNumberOfFeaturesOnFrame(configuration.maxNumberOfFeaturesOnFrame);
    m_mapProjector.setGridSize(configuration.frameGridSize);
    m_mapProjector.setCursorSize(configuration.featureCursorSize);
    m_mapProjector.setPixelEps(configuration.pixelEps);
    m_locationOptimizer.setEps(configuration.locationEps);
    m_locationOptimizer.setMaxPixelError(configuration.locationMaxPixelError);
    m_locationOptimizer.setNumberIterations(configuration.locationNumberIterations);
    m_numberPointsForSructureOptimization = configuration.numberPointsForSructureOptimization;
    m_numberIterationsForStructureOptimization = configuration.numberIterationsForStructureOptimization;
    m_toleranceOfCreatingFrames = configuration.toleranceOfCreatingFrames;
    m_minNumberTrackingPoints = configuration.minNumberTrackingPoints;
    m_preferredNumberTrackingPoints = configuration.preferredNumberTrackingPoints;
    m_maxCountKeyFrames = configuration.maxCountKeyFrames;
    m_mapProjector.setMaxNumberIterations(configuration.featureMaxNumberIterations);
    m_trackerTransform.setEps(configuration.tracker_eps);
    m_trackerTransform.setNumberIterations(configuration.tracker_numberIterations);
    m_trackerTransform.setMinMaxLevel(configuration.tracker_minImageLevel, configuration.tracker_maxImageLevel);
    m_trackerTransform.setCursorSize(configuration.tracker_cursorSize);
}

MapPointsDetectorConfiguration ARSystem::candidatesDetectorConfiguration() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_candidatesDetector.configuration();
}

void ARSystem::setMapPointsDetectorConfiguration(const MapPointsDetectorConfiguration & configuration)
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    m_candidatesDetector.setConfiguration(configuration);
}

ConstImage<uchar> ARSystem::lastImage() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_currentImagePyramid[0];
}

const std::vector<PreviewFrame::PreviewFeature> & ARSystem::currentFeatures() const
{
    return m_lastFrame->previewFeatures();
}

TrackingState ARSystem::trackingState() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_trackingState;
}

TrackingQuality ARSystem::trackingQuality() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_trackingQuality;
}

TMath::TVectord ARSystem::cameraParameters() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_cameraParameters;
}

void ARSystem::setCameraParameters(const TMath::TVectord & cameraParameters)
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;

    m_cameraParameters = cameraParameters;
    Camera* camera = new Camera();
    camera->setCameraParameters(m_cameraParameters);
    m_camera = std::shared_ptr<const Camera>(camera);
}

double ARSystem::toleranceOfCreatingFrames() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_toleranceOfCreatingFrames;
}

void ARSystem::setToleranceOfCreatingFrames(double toleranceOfCreatingFrames)
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    m_toleranceOfCreatingFrames = toleranceOfCreatingFrames;
}

int ARSystem::numberPointsForSructureOptimization() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_numberPointsForSructureOptimization;
}

void ARSystem::setNumberPointsForSructureOptimization(int numberPointsForSructureOptimization)
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    m_numberPointsForSructureOptimization = numberPointsForSructureOptimization;
}

int ARSystem::numberIterationsForStructureOptimization() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_numberIterationsForStructureOptimization;
}

void ARSystem::setNumberIterationsForStructureOptimization(int numberIterationsForStructureOptimization)
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    m_numberIterationsForStructureOptimization = numberIterationsForStructureOptimization;
}

int ARSystem::maxCountKeyFrames() const
{
    return m_maxCountKeyFrames;
}

void ARSystem::setMaxCountKeyFrames(int count)
{
    m_maxCountKeyFrames = count;
}

TMath::TMatrixd ARSystem::currentRotation() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_lastFrame->rotation();
}

TMath::TVectord ARSystem::currentTranslation() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_lastFrame->translation();
}

TMath::TMatrixd ARSystem::currentTransform() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;

    TMath::TMatrixd result(4, 4);
    result.setToIdentity();
    result.fill(0, 0, m_lastFrame->rotation());
    result.setColumn(3, m_lastFrame->translation());
    return result;
}

BuilderTypePoint ARSystem::builderTypeMapPoint() const
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_builderTypeMapPoint;
}

void ARSystem::_reset()
{
    m_initializer.reset();
    m_map.resetMap(&m_mapResourceManager);
    m_trackingState = TrackingState::Undefining;
    m_trackingQuality = TrackingQuality::Ugly;
}

void ARSystem::reset()
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;

    _reset();
}

void ARSystem::nextTrackingState()
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    switch (m_trackingState) {
    case TrackingState::Undefining:
        m_trackingState = TrackingState::CaptureFirstFrame;
        break;
    case TrackingState::CaptureFirstFrame:
        break;
    case TrackingState::CaptureSecondFrame: {
        m_initializer.setSecondFrame(m_camera, m_blackWhiteFrame);
        MapInitializer::InitializationResult initializationResult = m_initializer.compute(&m_map, &m_mapResourceManager, true);
        switch (initializationResult) {
            case MapInitializer::InitializationResult::Success:
                m_trackingState = TrackingState::Tracking;
                m_trackingQuality = TrackingQuality::Good;
                m_initializer.reset();
                break;
            case MapInitializer::InitializationResult::Failure:
                break;
            case MapInitializer::InitializationResult::Reset:
                _reset();
                break;
        }
    } break;
    case TrackingState::Tracking:
        _reset();
        break;
    case TrackingState::LostTracking:
        _reset();
        break;
    }
}

void ARSystem::process(const ImageRef<Rgba> & frame)
{
    std::lock_guard<std::mutex> lock(m_mutex); (void)lock;

    m_performanceMonitor->start();

    m_map.lock();

    if (m_camera->imageSize() != frame.size().cast<double>()) {
        Camera * camera = new Camera();
        camera->setImageSize(frame.size());
        camera->setCameraParameters(m_cameraParameters);
        m_camera = std::shared_ptr<Camera>(camera);
    }

    bool needNewFrame = false;

    m_performanceMonitor->startTimer("Creation of image pyramid");
    _converToBlackWhiteFrame(frame);
    _buildCurrentImagePyramid();
    m_performanceMonitor->endTimer("Creation of image pyramid");

    PreviewFrame newFrame(m_camera, m_currentImagePyramid,
                          m_lastFrame->rotation(), m_lastFrame->translation());

    switch (m_trackingState) {
    case TrackingState::Tracking: {

        m_performanceMonitor->startTimer("Calculating motion of camera");
        m_trackerTransform.setFirstFrame(*m_lastFrame);
        m_trackerTransform.setSecondFrame(newFrame);
        m_trackerTransform.tracking();
        newFrame.setRotation(m_trackerTransform.secondRotation());
        newFrame.setTranslation(m_trackerTransform.secondTranslation());
        m_performanceMonitor->endTimer("Calculating motion of camera");

        needNewFrame = true;

        m_performanceMonitor->startTimer("Search of map points");
        m_mapProjector.projectMapPoints(newFrame, *m_lastFrame);
        m_performanceMonitor->endTimer("Search of map points");

        m_performanceMonitor->startTimer("Rectification of camera location");
        m_locationOptimizer.optimize(newFrame);
        m_performanceMonitor->endTimer("Rectification of camera location");

        if (m_mapProjector.existCloseKeyFrame(newFrame, m_toleranceOfCreatingFrames)) {
            needNewFrame = false;
        }
        _incSuccessScore(newFrame);
        if ((int)newFrame.countPreviewFeatures() < m_minNumberTrackingPoints) {
            m_trackingQuality = TrackingQuality::Ugly;
            m_trackingState = TrackingState::LostTracking;
            newFrame.setRotation(m_lastFrame->rotation());
            newFrame.setTranslation(m_lastFrame->translation());
            needNewFrame = false;
            m_mapProjector.deleteFaildedMapPoints();
            break;
        } else if ((int)newFrame.countPreviewFeatures() < m_preferredNumberTrackingPoints) {
            m_trackingQuality = TrackingQuality::Bad;
            needNewFrame = false;
        } else {
            m_trackingQuality = TrackingQuality::Good;
        }
        {
            m_performanceMonitor->startTimer("Rectification of positions of map points");
            m_mapProjector.deleteFaildedMapPoints();
            m_locationOptimizer.deleteFaildedMapPoints(&m_map);
            m_map.deleteNullMapPoints(&m_mapResourceManager);
            _optimizeMapPoints(newFrame);
            m_mapProjector.createNewMapPointsFromCandidates(newFrame);
            m_performanceMonitor->endTimer("Rectification of positions of map points");

            if (m_trackingQuality == TrackingQuality::Good) {
                if (needNewFrame) {
                    if ((int)m_map.countKeyFrames() > m_maxCountKeyFrames) {
                        m_map.deleteKeyFrame(m_map.getFurthestKeyFrame(&m_mapResourceManager, newFrame.worldPosition()));
                    }
                    std::shared_ptr<KeyFrame> newKeyFrame = _createNewKeyFrame(newFrame);
                    m_candidatesDetector.addKeyFrame(newKeyFrame);
                }
            }
        }
    } break;
    case TrackingState::CaptureSecondFrame: {
        m_performanceMonitor->startTimer("Tracking points and initialization");
        m_initializer.setSecondFrame(m_camera, m_blackWhiteFrame);
        MapInitializer::InitializationResult initializationResult = m_initializer.compute(&m_map, &m_mapResourceManager, false);
        m_performanceMonitor->endTimer("Tracking points and initialization");

        switch (initializationResult) {
        case MapInitializer::InitializationResult::Success: {
            m_trackingState = TrackingState::Tracking;
            m_trackingQuality = TrackingQuality::Good;
            std::shared_ptr<KeyFrame> keyFrame = m_map.keyFrame(m_map.countKeyFrames() - 1);
            MapResourceLocker lockerR(&m_mapResourceManager, keyFrame.get()); (void)lockerR;
            newFrame.copy(keyFrame);
            m_initializer.reset();
        } break;
        case MapInitializer::InitializationResult::Failure:
            break;
        case MapInitializer::InitializationResult::Reset:
            _reset();
            break;
        }
    } break;
    case TrackingState::CaptureFirstFrame:
        m_initializer.setFirstFrame(m_camera, m_blackWhiteFrame.copy());
        m_trackingState = TrackingState::CaptureSecondFrame;
        break;
    case TrackingState::LostTracking: {
        bool foundNearestKeyFrame = false;
        {
            std::weak_ptr<KeyFrame> nearestKeyFrame;
            int bestScore = std::numeric_limits<int>::max(), score;
            m_performanceMonitor->startTimer("Find nearest image");
            Image<int> targetSmallImage = m_map.getSmallImage(newFrame);
            for (std::size_t i = 0; i < m_map.countKeyFrames(); ++i) {
                std::shared_ptr<KeyFrame> k = m_map.keyFrame(i);
                if (!k)
                    continue;
                MapResourceLocker lockerR(&m_mapResourceManager, k.get()); (void)lockerR;
                if (!k->isDeleted()) {
                    ConstImage<int> smallImage = k->smallImage();
                    score = ZMSSD::compare(smallImage, Point2i(0, 0),
                                           targetSmallImage, Point2i(0, 0),
                                           targetSmallImage.size());
                    if (score < bestScore) {
                        bestScore = score;
                        nearestKeyFrame = k;
                    }
                }
            }
            m_performanceMonitor->endTimer("Find nearest image");
            std::shared_ptr<KeyFrame> keyFrame = nearestKeyFrame.lock();
            if (keyFrame) {
                foundNearestKeyFrame = true;
                if (bestScore > (int)(targetSmallImage.area() * ((255 * 0.18) * (255 * 0.18)))) {
                    keyFrame.reset();
                }
            }
            if (keyFrame) {
                MapResourceLocker lockerR(&m_mapResourceManager, keyFrame.get()); (void)lockerR;
                newFrame.setRotation(keyFrame->rotation());
                newFrame.setTranslation(keyFrame->translation());
                m_performanceMonitor->startTimer("Calculating motion of camera");
                m_trackerTransform.setFirstFrame(keyFrame);
                m_trackerTransform.setSecondFrame(newFrame);
                m_trackerTransform.tracking();
                newFrame.setRotation(m_trackerTransform.secondRotation());
                newFrame.setTranslation(m_trackerTransform.secondTranslation());
                m_performanceMonitor->endTimer("Calculating motion of camera");

                if (m_trackerTransform.countTrackedFeatures() < m_minNumberTrackingPoints) {
                    newFrame.setRotation(m_lastFrame->rotation());
                    newFrame.setTranslation(m_lastFrame->translation());
                    break;
                }

                m_performanceMonitor->startTimer("Search of map points");
                m_mapProjector.projectMapPoints(newFrame, *m_lastFrame);
                m_performanceMonitor->endTimer("Search of map points");

                m_performanceMonitor->startTimer("Rectification of camera location");
                m_locationOptimizer.optimize(newFrame);
                m_performanceMonitor->endTimer("Rectification of camera location");
            }
        }
        {
            m_mapProjector.deleteFaildedMapPoints();
            m_locationOptimizer.deleteFaildedMapPoints(&m_map);
            m_map.deleteNullMapPoints(&m_mapResourceManager);
            m_mapProjector.createNewMapPointsFromCandidates(newFrame);
        }
        if (!foundNearestKeyFrame) {
            _reset();
            break;
        }
        if ((int)newFrame.countPreviewFeatures() < m_minNumberTrackingPoints) {
            m_trackingQuality = TrackingQuality::Ugly;
            m_trackingState = TrackingState::LostTracking;
            newFrame.setRotation(m_lastFrame->rotation());
            newFrame.setTranslation(m_lastFrame->translation());
            break;
        } else if ((int)newFrame.countPreviewFeatures() < m_preferredNumberTrackingPoints) {
            m_trackingState = TrackingState::Tracking;
            m_trackingQuality = TrackingQuality::Bad;
        } else {
            m_trackingState = TrackingState::Tracking;
            m_trackingQuality = TrackingQuality::Good;
        }
    } break;
    case TrackingState::Undefining:
        break;
    }

    m_lastFrame->copy(newFrame);
    if (m_trackingQuality == TrackingQuality::Good) {
        if (!needNewFrame) {
            m_candidatesDetector.addFrame(std::move(newFrame));
        }
    }
    m_performanceMonitor->end();

    m_map.unlock();
}

AR::Map * ARSystem::map()
{
    return &m_map;
}

const AR::Map * ARSystem::map() const
{
    return &m_map;
}

MapResourcesManager * ARSystem::mapResourceManager()
{
    return &m_mapResourceManager;
}

std::shared_ptr<const PerformanceMonitor> ARSystem::performanceMonitor() const
{
    return m_performanceMonitor;
}

std::shared_ptr<PerformanceMonitor> ARSystem::performanceMonitor()
{
    return m_performanceMonitor;
}

std::vector<std::pair<Point2f, Point2f>> ARSystem::debugTrackedMatches() const
{
    std::vector<std::pair<Point2f, Point2f>> result;
    switch (m_trackingState) {
        case TrackingState::CaptureSecondFrame: {
            const std::vector<Point2f>& firstFeatures = m_initializer.firstFeatures();
            const std::vector<Point2f>& secondFeatures = m_initializer.secondFeatures();
            if (firstFeatures.size() != secondFeatures.size())
                break;
            result.resize(firstFeatures.size());
            for (std::size_t i = 0; i < result.size(); ++i) {
                result[i] = std::make_pair(firstFeatures[i], secondFeatures[i]);
            }
        } break;
        default:
            break;
    }
    return result;
}

void ARSystem::_converToBlackWhiteFrame(const ImageRef<Rgba> & frame)
{
    if (frame.size() != m_blackWhiteFrame.size())
        m_blackWhiteFrame = Image<uchar>(frame.size());
    const int area = m_blackWhiteFrame.area();
    uchar * bwPtr = m_blackWhiteFrame.data();
    const Rgba * rgbaPtr = frame.data();
    for (int i = 0; i < area; ++i, ++bwPtr, ++rgbaPtr)
        *bwPtr = (rgbaPtr->red + rgbaPtr->green + rgbaPtr->blue) / 3;
}

void ARSystem::_buildCurrentImagePyramid()
{
    m_currentImagePyramid[0] = m_blackWhiteFrame;
    Point2i imageSize = m_blackWhiteFrame.size();
    for (std::size_t i = 1; i < m_currentImagePyramid.size(); ++i) {
        imageSize /= 2;
        if (m_currentImagePyramid[i].size() != imageSize)
            m_currentImagePyramid[i] = Image<uchar>(imageSize);
        ImageProcessing::halfSample(m_currentImagePyramid[i], m_currentImagePyramid[i - 1]);
    }
}

void ARSystem::_optimizeMapPoints(PreviewFrame & frame)
{
    std::vector<PreviewFrame::PreviewFeature> & features = frame.previewFeatures();
    std::srand((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch()).count());
    std::random_shuffle(features.begin(), features.end());
    int countProcessedPoints = 0;
    auto it = features.begin();
    for (; it != features.end(); ) {
        std::shared_ptr<MapPoint> mapPoint = it->mapPoint;
        MapResourceLocker locker(&m_mapResourceManager, mapPoint.get()); (void)locker;
        if (!mapPoint->isDeleted()) {
            mapPoint->optimize(&m_mapResourceManager, m_numberIterationsForStructureOptimization, frame, it->positionOnFrame);
            ++countProcessedPoints;
            if (countProcessedPoints > m_numberPointsForSructureOptimization)
                break;
            ++it;
        } else {
            it = features.erase(it);
        }
    }
}

std::shared_ptr<KeyFrame> ARSystem::_createNewKeyFrame(PreviewFrame & previewFrame)
{
    std::shared_ptr<KeyFrame> keyFrame = m_map.createKeyFrame(previewFrame);
    MapResourceLocker lockerKeyFrame(&m_mapResourceManager, keyFrame.get()); (void)lockerKeyFrame;

    std::vector<PreviewFrame::PreviewFeature> & features = previewFrame.previewFeatures();
    for (auto it = features.begin(); it != features.end(); ++it) {
        std::shared_ptr<MapPoint> mapPoint = it->mapPoint;
        MapResourceLocker lockerMapPoint(&m_mapResourceManager, mapPoint.get()); (void)lockerMapPoint;
        if (!mapPoint->isDeleted()) {
            m_map.createFeature(keyFrame, it->positionOnFrame, it->imageLevel, mapPoint);
        }
    }

    return keyFrame;
}

void ARSystem::_incSuccessScore(PreviewFrame & frame)
{
    std::vector<PreviewFrame::PreviewFeature> & features = frame.previewFeatures();
    for (auto it = features.begin(); it != features.end(); ++it) {
        if (m_builderTypeMapPoint.getType(it->mapPoint->statistic()) != TypeMapPoint::Good)
            it->mapPoint->statistic().incSuccess();
    }
}

}
