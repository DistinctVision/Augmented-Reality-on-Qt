#include "MapProjector.h"
#include "ImageProcessing.h"
#include "KeyFrame.h"
#include "MapPoint.h"
#include "MapResourceLocker.h"
#include "TMath/TMath.h"
#include <algorithm>
#include <cmath>
#include <climits>
#include <limits>
#include <iostream>

namespace AR {

std::vector<MapProjector::Debug> MapProjector::debug;

MapProjector::MapProjector()
{
    m_map = nullptr;
    m_resourceManager = nullptr;
    m_builderTypeMapPoint = nullptr;
    m_builderTypeCandidatePoint = nullptr;
    m_mapPointsDetector = nullptr;
    m_maxNumberOfUsedKeyFrames = 10;
    m_frameBorder = 5;
    m_maxNumberOfFeaturesOnFrame = 60;
    setGridSize(Point2i(20, 20));
    setCursorSize(Point2i(4, 4));
    setPixelEps(1e-3f);
}

Map * MapProjector::map() const
{
    return m_map;
}

void MapProjector::setMap(Map * map)
{
    m_map = map;
}

MapResourcesManager * MapProjector::mapResourceManager() const
{
    return m_resourceManager;
}

void MapProjector::setMapResourceManager(MapResourcesManager * mapResourceManager)
{
    m_resourceManager = mapResourceManager;
}

Point2i MapProjector::gridSize() const
{
    return m_gridSize;
}

void MapProjector::setGridSize(const Point2i & gridSize)
{
    m_gridSize = gridSize;
    int size = gridSize.x * gridSize.y;
    m_cells.resize(size);
    m_cells_candidates.resize(size);
    m_cells_lock.resize(size);
    m_cellOrders.resize(size);
    for (int i = 0; i < size; ++i)
        m_cellOrders[i] = i;
    std::random_shuffle(m_cellOrders.begin(), m_cellOrders.end());
}

Point2i MapProjector::cursorSize() const
{
    return m_matcher.cursorSize();
}

void MapProjector::setCursorSize(const Point2i & cursorSize)
{
    m_matcher.setCursorSize(cursorSize);
}

float MapProjector::pixelEps() const
{
    return m_matcher.pixelEps();
}

void MapProjector::setPixelEps(float eps)
{
    m_matcher.setPixelEps(eps);
}

int MapProjector::maxNumberIterations() const
{
    return m_matcher.numberIterations();
}

void MapProjector::setMaxNumberIterations(int value)
{
    m_matcher.setNumberIterations(value);
}

int MapProjector::frameBorder() const
{
    return m_frameBorder;
}

void MapProjector::setFrameBorder(int frameBorder)
{
    m_frameBorder = frameBorder;
}

std::size_t MapProjector::maxNumberOfFeaturesOnFrame() const
{
    return m_maxNumberOfFeaturesOnFrame;
}

void MapProjector::setMaxNumberOfFeaturesOnFrame(const std::size_t & maxNumberOfFeaturesOnFrame)
{
    m_maxNumberOfFeaturesOnFrame = maxNumberOfFeaturesOnFrame;
}

std::size_t MapProjector::maxNumberOfUsedKeyFrames() const
{
    return m_maxNumberOfUsedKeyFrames;
}

void MapProjector::setMaxNumberOfUsedKeyFrames(const std::size_t & maxNumberOfUsedKeyFrames)
{
    m_maxNumberOfUsedKeyFrames = maxNumberOfUsedKeyFrames;
}

BuilderTypePoint* MapProjector::builderTypeMapPoint()
{
    return m_builderTypeMapPoint;
}

const BuilderTypePoint* MapProjector::builderTypeMapPoint() const
{
    return m_builderTypeMapPoint;
}

void MapProjector::setBuilderTypeMapPoint(BuilderTypePoint * builderType)
{
    m_builderTypeMapPoint = builderType;
}

const BuilderTypePoint* MapProjector::builderTypeCandidatePoint() const
{
    return m_builderTypeCandidatePoint;
}

BuilderTypePoint* MapProjector::builderTypeCandidatePoint()
{
    return m_builderTypeCandidatePoint;
}

void MapProjector::setBuilderTypeCandidatePoint(BuilderTypePoint * builderType)
{
    m_builderTypeCandidatePoint = builderType;
}

MapPointsDetector* MapProjector::mapPointsDetector() const
{
    return m_mapPointsDetector;
}

void MapProjector::setMapPointsDetector(MapPointsDetector* mapPointsDetector)
{
    m_mapPointsDetector = mapPointsDetector;
}

void MapProjector::projectMapPoints(PreviewFrame & previewFrame,
                                    const PreviewFrame & prevPreviewFrame)
{
    //debug.clear();
    TMath_assert(m_builderTypeMapPoint != nullptr);
    TMath_assert(m_map != nullptr);
    Point2i targetImageSize = previewFrame.imageSize();
    m_targetFrameBegin = m_matcher.cursorSize().cast<float>() + Point2f(m_frameBorder, m_frameBorder);
    m_targetFrameEnd = targetImageSize.cast<float>() - (m_targetFrameBegin + Point2f(1.0f, 1.0f));
    m_cellSize.set(targetImageSize.x / (float)m_gridSize.x, targetImageSize.y / (float)m_gridSize.y);
    m_border = (m_matcher.cursorSize() + Point2i(1, 1)).cast<float>();
    m_targetFrame_invRotation = TMath::TTools::matrix3x3Inverted(previewFrame.rotation());
    m_targetFrame_invTranslation = - m_targetFrame_invRotation * previewFrame.translation();

    _resetGrid();
    std::size_t currentCountTrackingPoints = 0;
    m_successCurrentCandidatePoints.clear();

    m_projectedMapPoints.clear();
    (void)prevPreviewFrame;
    /*std::shared_ptr<const Camera> prevCamera = prevPreviewFrame.camera();
    const std::vector<PreviewFrame::PreviewFeature>& oldFeatures = prevPreviewFrame.previewFeatures();
    for (auto it = oldFeatures.cbegin(); it != oldFeatures.cend(); ++it) {
        std::shared_ptr<const MapPoint> mapPoint = it->mapPoint;
        m_indiciesOfProjectedMapPoints.insert(mapPoint->index());
        TMath::TVectord position = previewFrame.rotation() * mapPoint->position() + previewFrame.translation();
        m_lastProjection = prevCamera->project(Point2d(position(0) / position(2),
                                                       position(1) / position(2))).cast<float>();
        if ((m_lastProjection.x > m_targetFrameBegin.x) && (m_lastProjection.y > m_targetFrameBegin.y) &&
                (m_lastProjection.x < m_targetFrameEnd.x) && (m_lastProjection.y < m_targetFrameEnd.y)) {
            int k = ((int)(m_lastProjection.y / m_cellSize.y)) * m_gridSize.x +
                     (int)(m_lastProjection.x / m_cellSize.x);
            if (m_cells_lock[k]) {
                continue;
            }
            if (_projectMapPoint(previewFrame, mapPoint, m_lastProjection)) {
                previewFrame.addPreviewFeature({ it->mapPoint, m_lastProjection, m_lastSearchImageLevel });
                m_cells_lock[k] = true;
                ++currentCountTrackingPoints;
            }
        }
    }*/

    _findVisibleKeyFrames(previewFrame);
    _projectMapPointsOnGrid(previewFrame);
    std::srand((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch()).count());
    std::random_shuffle(m_cellOrders.begin(), m_cellOrders.end());
    std::size_t i;
    for (i = 0; (i < m_cellOrders.size()) &&
         (currentCountTrackingPoints < m_maxNumberOfFeaturesOnFrame);
         ++i) {
        if (_processMapPointOnCell(previewFrame, m_cellOrders[i])) {
            ++currentCountTrackingPoints;
        }
    }
    if (currentCountTrackingPoints < m_maxNumberOfFeaturesOnFrame) {
        CandidatesReader candidatesReader(m_mapPointsDetector);
        _projectCandidatesPointsOnGrid(previewFrame, candidatesReader.candidateMapPointsList());
        for (i = 0;
             (i < m_cellOrders.size()) && (currentCountTrackingPoints < m_maxNumberOfFeaturesOnFrame);
             ++i) {
            if (_processCandidatePointOnCell(previewFrame, m_cellOrders[i])) {
                ++currentCountTrackingPoints;
            }
        }
    }
}

void MapProjector::createNewMapPointsFromCandidates(PreviewFrame & previewFrame)
{
    TMath_assert(m_map != nullptr);
    for (auto it = m_successCurrentCandidatePoints.begin(); it != m_successCurrentCandidatePoints.end(); ++it) {
        CandidateMapPoint * candidateMapPoint = it->candidateMapPoint;
        std::shared_ptr<KeyFrame> keyFrame = candidateMapPoint->keyFrame;
        MapResourceLocker lockerKeyFrame(m_resourceManager, keyFrame.get()); (void)lockerKeyFrame;
        if (keyFrame->isDeleted())
            continue;
        std::shared_ptr<MapPoint> mapPoint = m_map->createMapPoint(candidateMapPoint->position);
        MapResourceLocker lockerMapPoint(m_resourceManager, mapPoint.get()); (void)lockerMapPoint;
        m_map->createFeature(keyFrame, candidateMapPoint->projection, candidateMapPoint->imageLevel, mapPoint);
        previewFrame.addPreviewFeature(mapPoint, it->projection, it->imageLevel);
    }
    m_successCurrentCandidatePoints.clear();
}

bool MapProjector::existCloseKeyFrame(PreviewFrame & previewFrame, double limitDistance)
{
    using namespace TMath;

    double depthMin = 0.0, depthMean = 0.0;
    PreviewFrame::getDepth(m_resourceManager, depthMean, depthMin, previewFrame);
    if (depthMean < std::numeric_limits<float>::epsilon())
        return true;

    TMatrixd rotation = previewFrame.rotation();
    TVectord translation = previewFrame.translation();

    for (auto it = m_visibleKeyFrames.begin(); it != m_visibleKeyFrames.end(); ++it) {
        std::shared_ptr<const KeyFrame> keyFrame = it->keyFrame;
        TVectord localPos = rotation * keyFrame->worldPosition() + translation;
        if (((std::fabs(localPos(0)) / depthMean) < limitDistance) &&
            ((std::fabs(localPos(1)) / depthMean) < limitDistance) &&
            ((std::fabs(localPos(2)) / depthMean) < limitDistance)) {
            return true;
        }
    }
    return false;
}

std::size_t MapProjector::lastCountVisibleKeyFrames() const
{
    return m_visibleKeyFrames.size();
}

std::shared_ptr<KeyFrame> MapProjector::lastVisibleKeyFrame(std::size_t index)
{
    return m_visibleKeyFrames[index].keyFrame;
}

void MapProjector::deleteFaildedMapPoints()
{
    for (auto it = m_failedMapPoints.begin(); it != m_failedMapPoints.end(); ++it) {
        std::shared_ptr<MapPoint> mapPoint = *it;
        MapResourceLocker locker(m_resourceManager, mapPoint.get()); (void)locker;
        m_map->deleteMapPoint(mapPoint);
    }
    m_failedMapPoints.resize(0);
}

void MapProjector::_resetGrid()
{
    for (auto it = m_cells.begin(); it != m_cells.end(); ++it)
        it->clear();
    for (auto it = m_cells_candidates.begin(); it != m_cells_candidates.end(); ++it)
        it->clear();
    for (auto it = m_cells_lock.begin(); it != m_cells_lock.end(); ++it)
        *it = false;
}

void MapProjector::_findVisibleKeyFrames(const Frame & targetFrame)
{
    TMath_assert(m_map != nullptr);
    m_visibleKeyFrames.resize(0);
    TMath::TVectord targetTranslation = targetFrame.worldPosition();
    std::size_t countKeyFrames = m_map->countKeyFrames();
    for (std::size_t i = 0; i < countKeyFrames; ++i) {
        std::shared_ptr<KeyFrame> keyFrame = m_map->keyFrame(i);
        MapResourceLocker lockerKeyFrame(m_resourceManager, keyFrame.get()); (void)lockerKeyFrame;
        std::size_t countFeatures = keyFrame->countFeatures();
        for (std::size_t j = 0; j < countFeatures; ++j) {
            std::shared_ptr<MapPoint> mapPoint = keyFrame->feature(j)->mapPoint();
            m_resourceManager->lock(mapPoint.get());
            TMath::TVectord v = targetFrame.rotation() * mapPoint->position() + targetFrame.translation();
            m_resourceManager->unlock(mapPoint.get());
            if (v(2) < std::numeric_limits<float>::epsilon())
                continue;
            Point2f p = targetFrame.camera()->project(Point2d(v(0) / v(2), v(1) / v(2))).cast<float>();
            if ((p.x > m_targetFrameBegin.x) && (p.y > m_targetFrameBegin.y) &&
                    (p.x < m_targetFrameEnd.x) && (p.y < m_targetFrameEnd.y)) {
                m_visibleKeyFrames.push_back({ keyFrame,
                                               (targetTranslation - targetFrame.translation()).lengthSquared(),
                                               0 });
                break;
            }
        }
    }
    std::size_t size = std::min(m_maxNumberOfUsedKeyFrames, m_visibleKeyFrames.size());
    std::partial_sort(m_visibleKeyFrames.begin(), m_visibleKeyFrames.begin() + size, m_visibleKeyFrames.end(),
              [] (const VisibleKeyFrame & a, const VisibleKeyFrame & b) -> bool {
                  return (a.distanceSquared < b.distanceSquared);
              });
    m_visibleKeyFrames.resize(size);
}

void MapProjector::_projectMapPointsOnGrid(const Frame & frame)
{
    for (auto it = m_visibleKeyFrames.begin(); it != m_visibleKeyFrames.end(); ++it) {
        std::shared_ptr<KeyFrame> keyFrame = it->keyFrame;
        MapResourceLocker lockerKeyFrame(m_resourceManager, keyFrame.get()); (void)lockerKeyFrame;
        std::size_t countFeatures = keyFrame->countFeatures();
        for (std::size_t j = 0; j < countFeatures; ++j) {
            std::shared_ptr<MapPoint> mapPoint = keyFrame->feature(j)->mapPoint();
            MapResourceLocker lockerMapPoint(m_resourceManager, mapPoint.get()); (void)lockerMapPoint;
            if (m_projectedMapPoints.find(mapPoint) == m_projectedMapPoints.end()) {
                if (_projectMapPointOnGrid(frame, mapPoint)) {
                    ++it->countVisiblePoints;
                }
                m_projectedMapPoints.insert(mapPoint);
            }
        }
    }
}

bool MapProjector::_projectMapPointOnGrid(const Frame & frame, const std::shared_ptr<MapPoint> & mapPoint)
{
    TMath::TVectord v = frame.rotation() * mapPoint->position() + frame.translation();
    if (v(2) >= std::numeric_limits<float>::epsilon()) {
        Point2f p = frame.camera()->project(Point2d(v(0) / v(2), v(1) / v(2))).cast<float>();
        if ((p.x > m_targetFrameBegin.x) && (p.y > m_targetFrameBegin.y) &&
                (p.x < m_targetFrameEnd.x) && (p.y < m_targetFrameEnd.y)) {
            int k = ((int)(p.y / m_cellSize.y)) * m_gridSize.x + (int)(p.x / m_cellSize.x);
            if (m_cells_lock[k])
                return false;
            m_cells[k].push_back({ mapPoint, p });
            return true;
        }
    }
    if (mapPoint->statistic().successScore() == 0) { // Candidate point
        mapPoint->statistic().incFailed(3);
        if (m_builderTypeMapPoint->getType(mapPoint->statistic()) == TypeMapPoint::Failed)
            m_failedMapPoints.push_back(mapPoint);
    }
    return false;
}

void MapProjector::_projectCandidatesPointsOnGrid(const Frame & frame,
                                                  CandidateMapPointsList * candidatesList)
{
    using namespace TMath;

    if (m_mapPointsDetector == nullptr)
        return;

    TVectord v(3);
    CandidateMapPoint * e = candidatesList->head();
    CandidateMapPoint * e_next;
    while (e != nullptr) {
        e_next = e->next();
        v = frame.rotation() * e->position + frame.translation();
        Point2f p = frame.camera()->project(Point2d(v(0) / v(2), v(1) / v(2))).cast<float>();
        if ((p.x > m_targetFrameBegin.x) && (p.y > m_targetFrameBegin.y) &&
                (p.x < m_targetFrameEnd.x) && (p.y < m_targetFrameEnd.y)) {
            int k = ((int)(p.y / m_cellSize.y)) * m_gridSize.x + (int)(p.x / m_cellSize.x);
            if (m_cells_lock[k]) {
                e->statistic.incFailed(3);
                if (m_builderTypeCandidatePoint->getType(e->statistic) == TypeMapPoint::Failed) {
                     delete e;
                }
            } else {
                m_cells_candidates[k].push_back({ e, p });
            }
        }
        e = e_next;
    }
}

bool MapProjector::_processMapPointOnCell(PreviewFrame & targetFrame, int k)
{
    if (m_cells_lock[k])
        return false;

    std::list<ProjectedMapPoint> & mapPoints = m_cells[k];
    mapPoints.sort([](const ProjectedMapPoint & lhs,
                      const ProjectedMapPoint & rhs) -> bool {
           return (lhs.mapPoint->statistic().commonScore() > rhs.mapPoint->statistic().commonScore());
    });
    for (std::list<ProjectedMapPoint>::iterator it = mapPoints.begin(); it != mapPoints.end(); ++it) {
        std::shared_ptr<MapPoint> mapPoint = it->mapPoint;
        MapResourceLocker lockerMapPoint(m_resourceManager, mapPoint.get()); (void)lockerMapPoint;
        if (_projectMapPoint(targetFrame, mapPoint, it->projection)) {
            targetFrame.addPreviewFeature({ mapPoint, m_lastProjection, m_lastSearchImageLevel });
            m_cells_lock[k] = true;
            return true;
        }
    }
    return false;
}

bool MapProjector::_projectMapPoint(PreviewFrame & targetFrame,
                                    const std::shared_ptr<MapPoint> & mapPoint,
                                    const Point2f & projection)
{
    using namespace TMath;

    std::shared_ptr<const Feature> f = mapPoint->getBestCollinearFeature(
                (targetFrame.rotation() * mapPoint->position() + targetFrame.translation()).normalized());
    if (f == nullptr)
        return false;
    std::shared_ptr<const KeyFrame> f_keyFrame = f->keyFrame();
    MapResourceLocker locker_f_keyFrame(m_resourceManager, f_keyFrame.get()); (void)locker_f_keyFrame;

    Point2f oldImagePoint = f->positionOnFrame() / (float)(1 << f->imageLevel());
    ConstImage<uchar> oldImage = f_keyFrame->imageLevel(f->imageLevel());
    if ((oldImagePoint.x < m_border.x) || (oldImagePoint.y < m_border.y) ||
            (oldImagePoint.x >= (oldImage.width() - (m_border.x + 1.0f))) ||
            (oldImagePoint.y >= (oldImage.height() - (m_border.y + 1.0f)))) {
        return false;
    }

    TMatrixd f_keyFrame_worldRotation = f_keyFrame->rotation();
    if (!TTools::matrix3x3Invert(f_keyFrame_worldRotation)) {
        mapPoint->statistic().incFailed(100);
        m_failedMapPoints.push_back(mapPoint);
        return false;
    }
    TVectord f_keyFrame_worldPosition = - f_keyFrame_worldRotation * f_keyFrame->translation();
    TMatrixd deltaRotation = targetFrame.rotation() * f_keyFrame_worldRotation;
    TVectord deltaTranslation = targetFrame.rotation() * f_keyFrame_worldPosition + targetFrame.translation();
    TVectord localMapPoint = f->localDir() * (mapPoint->position() - f_keyFrame_worldPosition).length();
    if (localMapPoint(2) < std::numeric_limits<float>::epsilon()) {
        return false;
    }
    TMatrixf w(2, 2);
    getWarpMatrixAffine(w,
                        m_matcher.cursorSize().cast<float>(),
                        f_keyFrame->camera(), targetFrame.camera(), f->positionOnFrame(),
                        localMapPoint, f->imageLevel(),
                        deltaRotation, deltaTranslation);
    m_lastSearchImageLevel = getBestSearchLevel(w, targetFrame.countImageLevels() - 1);
    if (!TTools::matrix2x2Invert(w)) {
        mapPoint->statistic().incFailed(3);
        if (m_builderTypeMapPoint->getType(mapPoint->statistic()) == TypeMapPoint::Failed)
            m_failedMapPoints.push_back(mapPoint);
        return false;
    }
    warpAffine(m_matcher.patch(), w, oldImage, f->positionOnFrame(),
               f->imageLevel(), m_lastSearchImageLevel);
    m_matcher.setSecondImage(targetFrame.imageLevel(m_lastSearchImageLevel));
    float scale = (float)(1 << m_lastSearchImageLevel);
    m_lastProjection = projection / scale;
    Point2f d = m_lastProjection;
    if (m_matcher.tracking2d_patch(m_lastProjection) == TrackingResult::Fail) {
        mapPoint->statistic().incFailed();
        if (m_builderTypeMapPoint->getType(mapPoint->statistic()) == TypeMapPoint::Failed)
            m_failedMapPoints.push_back(mapPoint);
        return false;
    }
    d -= m_lastProjection;
    if ((std::fabs(d.x) > m_matcher.cursorWidth()) || (std::fabs(d.y) > m_matcher.cursorHeight())) {
        mapPoint->statistic().incFailed();
        if (m_builderTypeMapPoint->getType(mapPoint->statistic()) == TypeMapPoint::Failed)
            m_failedMapPoints.push_back(mapPoint);
        return false;
    }
    m_lastProjection *= scale;
    //debug.push_back({ m_matcher.patch().copy(), projection, scale });
    return true;
}

bool MapProjector::_processCandidatePointOnCell(PreviewFrame & targetFrame, int k)
{
    if (m_cells_lock[k])
        return false;

    std::list<ProjectedCandidateMapPoint> & cell_candidates = m_cells_candidates[k];

    cell_candidates.sort([](const ProjectedCandidateMapPoint & lhs,
                            const ProjectedCandidateMapPoint & rhs) -> bool {
                                return (lhs.candidateMapPoint->statistic.commonScore() >
                                        rhs.candidateMapPoint->statistic.commonScore());
                            });
    auto it = cell_candidates.begin();
    while (it != cell_candidates.end()) {
        if (_projectCandidatePoint(targetFrame, it->candidateMapPoint, it->projection)) {
            m_successCurrentCandidatePoints.push_back({ it->candidateMapPoint, m_lastProjection, m_lastSearchImageLevel });
            //m_cells_lock[k] = true;
            return true;
        } else {
            if (m_builderTypeCandidatePoint->getType(it->candidateMapPoint->statistic) == TypeMapPoint::Failed) {
                delete it->candidateMapPoint;
                it = cell_candidates.erase(it);
            } else {
                ++it;
            }
        }
    }
    return false;
}

bool MapProjector::_projectCandidatePoint(PreviewFrame & targetFrame,
                                          CandidateMapPoint * candidateMapPoint,
                                          const Point2f & projection)
{
    using namespace TMath;

    std::shared_ptr<KeyFrame> keyFrame = candidateMapPoint->keyFrame;
    MapResourceLocker lockerKeyFrame(m_resourceManager, keyFrame.get()); (void)lockerKeyFrame;

    if (keyFrame->isDeleted()) {
        candidateMapPoint->statistic.incFailed(100);
        return false;
    }

    Point2f oldImagePoint = candidateMapPoint->projection / (float)(1 << candidateMapPoint->imageLevel);
    ConstImage<uchar> oldImage = keyFrame->imageLevel(candidateMapPoint->imageLevel);
    if ((oldImagePoint.x < m_border.x) ||
            (oldImagePoint.y < m_border.y) ||
            (oldImagePoint.x >= (oldImage.width() - (m_border.x + 1.0f))) ||
            (oldImagePoint.y >= (oldImage.height() - (m_border.y + 1.0f)))) {
        candidateMapPoint->statistic.incFailed();
        return false;
    }

    TMatrixd f_keyFrame_worldRotation = keyFrame->rotation();
    if (!TTools::matrix3x3Invert(f_keyFrame_worldRotation)) {
        candidateMapPoint->statistic.incFailed(100);
        return false;
    }
    TVectord f_keyFrame_worldPosition = - f_keyFrame_worldRotation * keyFrame->translation();
    TMatrixd deltaRotation = targetFrame.rotation() * f_keyFrame_worldRotation;
    TVectord deltaTranslation = targetFrame.rotation() * f_keyFrame_worldPosition + targetFrame.translation();
    //TVectord v = candidateMapPoint->keyFrame->rotation() * candidateMapPoint->position + candidateMapPoint->keyFrame->translation();
    Point2d p = keyFrame->camera()->unproject(projection);
    TVectord localMapPoint = TVectord::create(p.x, p.y, 1.0).normalized() *
                                    (candidateMapPoint->position - f_keyFrame_worldPosition).length();
    if (localMapPoint(2) < std::numeric_limits<float>::epsilon()) {
        candidateMapPoint->statistic.incFailed(3);
        return false;
    }

    TMatrixf w(2, 2);
    getWarpMatrixAffine(w,
                        m_matcher.cursorSize().cast<float>(),
                        keyFrame->camera(), targetFrame.camera(), candidateMapPoint->projection,
                        localMapPoint, candidateMapPoint->imageLevel,
                        deltaRotation, deltaTranslation);
    m_lastSearchImageLevel = getBestSearchLevel(w, targetFrame.countImageLevels() - 1);
    if (!TTools::matrix2x2Invert(w)) {
        candidateMapPoint->statistic.incFailed(100);
        return false;
    }
    warpAffine(m_matcher.patch(), w, oldImage, candidateMapPoint->projection,
               candidateMapPoint->imageLevel, m_lastSearchImageLevel);
    m_matcher.setSecondImage(targetFrame.imageLevel(m_lastSearchImageLevel));
    float scale = (float)(1 << m_lastSearchImageLevel);
    m_lastProjection = projection / scale;
    Point2f d = m_lastProjection;
    if (m_matcher.tracking2d_patch(m_lastProjection) == TrackingResult::Fail) {
        candidateMapPoint->statistic.incFailed();
        return false;
    }

    d -= m_lastProjection;
    if ((std::fabs(d.x) > m_matcher.cursorWidth()) || (std::fabs(d.y) > m_matcher.cursorHeight())) {
        candidateMapPoint->statistic.incFailed(3);
        return false;
    }

    m_lastProjection *= scale;
    //debug.push_back({ m_matcher.patch().copy(), projection, scale });
    return true;
}

void MapProjector::getWarpMatrixAffine(TMath::TMatrixf & warpMatrix,
                                       const Point2f & halfPatchSize,
                                       const std::shared_ptr<const Camera> & cameraA,
                                       const std::shared_ptr<const Camera> & cameraB,
                                       const Point2f & imagePointA, const TMath::TVectord & worldPointA,
                                       int level,
                                       const TMath::TMatrixd & rotation, const TMath::TVectord & translation)
{
    using namespace TMath;
    TMath_assert((warpMatrix.rows() == 2) && (warpMatrix.cols() == 2));
    TMath_assert((rotation.rows() == 3) && (rotation.cols() == 3));
    TMath_assert(translation.size() == 3);
    float scale = (float)(1 << level);
    Point2d du = cameraA->unproject(Point2f(imagePointA.x + halfPatchSize.x * scale, imagePointA.y));
    du *= worldPointA(2);
    Point2d dv = cameraA->unproject(Point2f(imagePointA.x, imagePointA.y + halfPatchSize.y * scale));
    dv *= worldPointA(2);
    TVectord p = rotation * worldPointA + translation;
    Point2d imagePointB(cameraB->project(Point2d(p(0) / p(2), p(1) / p(2))));
    p = rotation * TVectord::create(du.x, du.y, worldPointA(2)) + translation;
    du.set(p(0) / p(2), p(1) / p(2));
    du = cameraB->project(du);
    p = rotation * TVectord::create(dv.x, dv.y, worldPointA(2)) + translation;
    dv.set(p(0) / p(2), p(1) / p(2));
    dv = cameraB->project(dv);
    warpMatrix(0, 0) = (float)(du.x - imagePointB.x) / halfPatchSize.x;
    warpMatrix(0, 1) = (float)(dv.x - imagePointB.x) / halfPatchSize.y;
    warpMatrix(1, 0) = (float)(du.y - imagePointB.y) / halfPatchSize.x;
    warpMatrix(1, 1) = (float)(dv.y - imagePointB.y) / halfPatchSize.y;
}

int MapProjector::getBestSearchLevel(const TMath::TMatrixf & warpMatrix, int maxLevel)
{
    using namespace TMath;

    TMath_assert((warpMatrix.rows() == 2) && (warpMatrix.cols() == 2));
    // Compute patch level in other image
    int searchLevel = 0;
    float D = TTools::matrix2x2Determinant(warpMatrix);
    while((D > 3.0) && (searchLevel < maxLevel)) {
        searchLevel += 1;
        D *= 0.25;
    }
    return searchLevel;
}

void MapProjector::warpAffine(Image<uchar> patch, const TMath::TMatrixf & warpMatrix,
                              const ImageRef<uchar> & levelImage, const Point2f & imagePoint,
                              int level, int patchLevel)
{
    TMath_assert((warpMatrix.rows() == 2) && (warpMatrix.cols() == 2));
    Point2f centerPatch((patch.width() - 1) * 0.5f, (patch.height() - 1) * 0.5f);
    // Perform the warp on a larger patch.
    uchar* patch_ptr = patch.data();

    float patchScale = (float)(1 << patchLevel);
    Point2f levelImagePoint = imagePoint / (float)(1 << level), p_match, localP;
    Point2i p;
    for (p.y = 0; p.y < patch.height(); ++p.y) {
        for (p.x = 0; p.x < patch.width(); ++p.x, ++patch_ptr) {
            p_match.set((p.x - centerPatch.x) * patchScale,
                        (p.y - centerPatch.y) * patchScale);
            localP.set(warpMatrix(0, 0) * p_match.x + warpMatrix(0, 1) * p_match.y + levelImagePoint.x,
                       warpMatrix(1, 0) * p_match.x + warpMatrix(1, 1) * p_match.y + levelImagePoint.y);
            if ((localP.x < 0.0f) || (localP.y < 0.0f) ||
                (localP.x >= (levelImage.width() - 1.0f)) || (localP.y >= (levelImage.height() - 1.0f))) {
                *patch_ptr = 128;
            } else {
                *patch_ptr = ImageProcessing::interpolate<uchar>(levelImage, localP);
            }
        }
    }
}

}
