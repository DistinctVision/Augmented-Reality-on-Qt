#ifndef AR_MAPPOINTSDETECTOR_H
#define AR_MAPPOINTSDETECTOR_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <vector>
#include <list>
#include "TMath/TVector.h"
#include "TMath/TSVD.h"
#include "Point2.h"
#include "FeatureDetector.h"
#include "Camera.h"
#include "Frame.h"
#include "KeyFrame.h"
#include "Map.h"
#include "MapPoint.h"
#include "MapResourcesManager.h"
#include "OpticalFlowCalculator.h"
#include "Configurations.h"

namespace AR {

class CandidatesReader;
class CandidateMapPoint;
class CandidateMapPointsList;

class MapPointsDetector
{
public:

    MapPointsDetector(Map * map);
    ~MapPointsDetector();

    MapPointsDetectorConfiguration configuration() const;
    void setConfiguration(const MapPointsDetectorConfiguration & configuration);

    int maxNumberOfUsedFrames() const;
    void setMaxNumberOfUsedFrames(int maxNumberOfUsedFrames);

    int maxNumberOfSearchSteps() const;
    void setMaxNumberOfSearchSteps(int maxNumberOfSearchSteps);

    float seedConvergenceSquaredSigmaThresh() const;
    void setSeedConvergenceSquaredSigmaThresh(float seedConvergenceSquaredSigmaThresh);

    float pixelNoise() const;
    void setPixelNoise(float pixelNoise);

    int sizeOfCommitMapPoints() const;
    void setSizeOfCommitMapPoints(int sizeOfCommitMapPoints);

    int countImageLevels() const;
    void setCountImageLevels(int count);

    bool threadIsRunning() const;
    void startThread();
    void stopThread();

    void addKeyFrame(const std::shared_ptr<KeyFrame> & keyFrame);
    void addFrame(Frame && frame);

    void loop();

private:
    friend class CandidatesReader;

    struct ProjectionPoint {
        std::shared_ptr<const Camera> camera;
        Point2f projection;
        TMath::TMatrixd rotation;
        TMath::TVectord translation;
    };

    struct Seed {
        std::weak_ptr<KeyFrame> keyFrame;
        TMath::TVectord localDir;
        int imageLevel;
        float a;                   //!< a of Beta distribution: When high, probability of inlier is large.
        float b;                   //!< b of Beta distribution: When high, probability of outlier is large.
        float mu;                  //!< Mean of normal distribution.
        float z_range;             //!< Max range of the possible depth.
        float sigmaSquared;        //!< Variance of normal distribution.
        std::vector<ProjectionPoint> projections;
    };

    Map * m_map;
    MapResourcesManager m_resourceManager;
    mutable std::mutex m_thread_mutex;
    mutable std::mutex m_config_mutex;
    bool m_thread_is_running;
    std::thread * m_thread;
    mutable std::mutex m_framesQueueMutex;
    std::condition_variable m_frameQueue_condition;
    std::vector<std::shared_ptr<Frame>> m_framesQueue;
    std::weak_ptr<KeyFrame> m_keyFrame;
    bool m_isNewKeyFrame;
    bool * m_cellsLock;
    Point2f m_lastProjection;
    std::list<Seed> m_seeds;
    std::mutex m_seedMutex;
    bool m_needToStopSeedUpdating;

    FeatureDetector m_featureDetector;
    OpticalFlowCalculator m_matcher;

    CandidateMapPointsList * m_preparedCandidates;
    CandidateMapPointsList * m_finalCandidates;
    std::mutex m_mutex_finalCandidates;

    int m_maxNumberOfUsedFrames;
    int m_maxNumberOfSearchSteps;
    float m_seedConvergenceSquaredSigmaThresh;
    int m_sizeOfCommitMapPoints;
    int m_maxCountCandidatePoints;
    float m_pixelNoise;

    mutable TMath::TSVD<double> m_svd;

    void _initializeSeed(const std::shared_ptr<KeyFrame> & keyFrame);
    void _updateSeeds(const Frame & frame);
    bool _findEpipolarMatch(double & depth,
                            const std::shared_ptr<KeyFrame> & keyFrame,
                            Seed & seed, const Frame & secondFrame,
                            const TMath::TMatrixd & deltaRotation, const TMath::TVectord & deltaTranslation,
                            const double d_estimate, const double d_min, const double d_max);
    double _computeTau(const TMath::TVectord & t, const TMath::TVectord & localDir, double z, double px_error_angle);
    void _updateSeed(Seed & seed, float x, float tauSquared);
    float _pdf(float x, float mean, float scale);
    void _addPreparedPoint(Seed & seed, const TMath::TVectord & worldPoint);
    void _commitPreparedPoints();
    double _getPointInWorld(TMath::TVectord & result, const std::vector<ProjectionPoint> & projections) const;
};

class CandidateMapPointsList
{
public:
    CandidateMapPointsList();
    ~CandidateMapPointsList();
    bool isEmpty() const;
    void clear();
    CandidateMapPoint * head();
    int size() const;
    void splice(CandidateMapPointsList * list);

private:
    friend class CandidateMapPoint;

    CandidateMapPoint * m_head;
    int m_size;
};

class CandidateMapPoint
{
public:
    CandidateMapPoint(CandidateMapPointsList * list);
    CandidateMapPoint(CandidateMapPointsList * list,
                      const std::shared_ptr<KeyFrame> & keyFrame,
                      const Point2f & projection,
                      int imageLevel,
                      const TMath::TVectord & position);
    ~CandidateMapPoint();
    CandidateMapPointsList * list();
    CandidateMapPoint * next();
    CandidateMapPoint * prev();

    std::shared_ptr<KeyFrame> keyFrame;
    Point2f projection;
    int imageLevel;
    TMath::TVectord position;
    MapPoint::Statistic statistic;

private:
    friend class CandidateMapPointsList;

    CandidateMapPoint * m_prev;
    CandidateMapPoint * m_next;
    CandidateMapPointsList * m_list;
};

class CandidatesReader
{
public:
    CandidatesReader(MapPointsDetector * mapPointsDetector);
    CandidateMapPointsList * candidateMapPointsList();

private:
    mutable MapPointsDetector * m_mapPointsDetector;
    mutable std::lock_guard<std::mutex> m_locker;
};

}

#endif // AR_MAPPOINTSDETECTOR_H
