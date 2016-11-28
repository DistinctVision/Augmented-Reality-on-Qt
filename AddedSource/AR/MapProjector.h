#ifndef AR_MAPPROJECTOR_H
#define AR_MAPPROJECTOR_H

#include "TMath/TMatrix.h"
#include "Image.h"
#include "Feature.h"
#include "Frame.h"
#include "PreviewFrame.h"
#include "Camera.h"
#include "Map.h"
#include "MapResourcesManager.h"
#include "MapPoint.h"
#include "OpticalFlowCalculator.h"
#include "MapPointsDetector.h"
#include <vector>
#include <deque>
#include <memory>
#include <list>
#include <set>

namespace AR {

class MapProjector
{
public:
    struct Debug {
        Image<uchar> patch;
        Point2f p;
        float scale;
    };
    static std::vector<Debug> debug;

    MapProjector();

    Map * map() const;
    void setMap(Map * map);

    MapResourcesManager * mapResourceManager() const;
    void setMapResourceManager(MapResourcesManager * mapResourceManager);

    int frameBorder() const;
    void setFrameBorder(int frameBorder);

    Point2i gridSize() const;
    void setGridSize(const Point2i& gridSize);

    Point2i cursorSize() const;
    void setCursorSize(const Point2i& cursorSize);

    float pixelEps() const;
    void setPixelEps(float pixelEps);

    int maxNumberIterations() const;
    void setMaxNumberIterations(int value);

    std::size_t maxNumberOfUsedKeyFrames() const;
    void setMaxNumberOfUsedKeyFrames(const std::size_t & maxNumberOfUsedKeyFrames);

    std::size_t maxNumberOfFeaturesOnFrame() const;
    void setMaxNumberOfFeaturesOnFrame(const std::size_t & maxNumberOfFeaturesOnFrame);

    const BuilderTypePoint* builderTypeMapPoint() const;
    BuilderTypePoint* builderTypeMapPoint();
    void setBuilderTypeMapPoint(BuilderTypePoint * builderType);

    const BuilderTypePoint * builderTypeCandidatePoint() const;
    BuilderTypePoint * builderTypeCandidatePoint();
    void setBuilderTypeCandidatePoint(BuilderTypePoint * builderType);

    MapPointsDetector * mapPointsDetector() const;
    void setMapPointsDetector(MapPointsDetector * mapPointsDetector);

    void deleteFaildedMapPoints();

    void projectMapPoints(PreviewFrame & previewFrame,
                          const PreviewFrame & prevPreviewFrame);

    void createNewMapPointsFromCandidates(PreviewFrame & previewFrame);

    bool existCloseKeyFrame(PreviewFrame & previewFrame, double limitDistance);

    std::size_t lastCountVisibleKeyFrames() const;
    std::shared_ptr<KeyFrame> lastVisibleKeyFrame(std::size_t index);

    static void getWarpMatrixAffine(TMath::TMatrixf & warpMatrix,
                                    const Point2f & halfPatchSize,
                                    const std::shared_ptr<const Camera> & cameraA,
                                    const std::shared_ptr<const Camera> & cameraB,
                                    const Point2f & imagePointA,
                                    const TMath::TVectord & worldPointA,
                                    int level,
                                    const TMath::TMatrixd & rotation,
                                    const TMath::TVectord & translation);
    static int getBestSearchLevel(const TMath::TMatrixf & warpMatrix, int maxLevel);
    static void warpAffine(Image<uchar> patch, const TMath::TMatrixf & invWarpMatrix,
                            const ImageRef<uchar> & levelImage, const Point2f & imagePoint,
                            int level, int patchLevel);

private:
    struct VisibleKeyFrame
    {
        std::shared_ptr<KeyFrame> keyFrame;
        double distanceSquared;
        std::size_t countVisiblePoints;
    };

    struct ProjectedMapPoint
    {
        std::shared_ptr<MapPoint> mapPoint;
        Point2f projection;
    };

    struct ProjectedCandidateMapPoint
    {
        CandidateMapPoint * candidateMapPoint;
        Point2f projection;
    };

    struct SuccessCandidateMapPoint
    {
        CandidateMapPoint * candidateMapPoint;
        Point2f projection;
        int imageLevel;
    };

    Map * m_map;
    MapResourcesManager * m_resourceManager;

    std::size_t m_maxNumberOfUsedKeyFrames;
    int m_frameBorder;
    std::size_t m_maxNumberOfFeaturesOnFrame;

    Point2i m_gridSize;
    Point2f m_cellSize;
    std::vector<std::list<ProjectedMapPoint>> m_cells;
    std::vector<std::list<ProjectedCandidateMapPoint>> m_cells_candidates;
    std::deque<bool> m_cells_lock;
    std::vector<int> m_cellOrders;
    std::vector<VisibleKeyFrame> m_visibleKeyFrames;
    std::set<std::shared_ptr<MapPoint>> m_projectedMapPoints;
    Point2f m_targetFrameBegin, m_targetFrameEnd;
    TMath::TMatrixd m_targetFrame_invRotation;
    TMath::TVectord m_targetFrame_invTranslation;
    OpticalFlowCalculator m_matcher;
    Point2f m_border;
    std::vector<std::shared_ptr<MapPoint>> m_failedMapPoints;
    Point2f m_lastProjection;
    int m_lastSearchImageLevel;
    BuilderTypePoint * m_builderTypeMapPoint;
    BuilderTypePoint * m_builderTypeCandidatePoint;

    MapPointsDetector * m_mapPointsDetector;
    std::list<SuccessCandidateMapPoint> m_successCurrentCandidatePoints;

    void _resetGrid();
    void _findVisibleKeyFrames(const Frame & targetFrame);
    void _projectMapPointsOnGrid(const Frame & frame);
    bool _projectMapPointOnGrid(const Frame & frame, const std::shared_ptr<MapPoint> & mapPoint);
    void _projectCandidatesPointsOnGrid(const Frame & frame,
                                        CandidateMapPointsList * candidatesList);
    bool _processMapPointOnCell(PreviewFrame & targetFrame, int k);
    bool _projectMapPoint(PreviewFrame & targetFrame,
                          const std::shared_ptr<MapPoint> & mapPoint,
                          const Point2f & projection);
    bool _processCandidatePointOnCell(PreviewFrame & targetFrame, int k);
    bool _projectCandidatePoint(PreviewFrame & targetFrame,
                                CandidateMapPoint * candidateMapPoint,
                                const Point2f & projection);
};

}

#endif // AR_MAPPROJECTOR_H
