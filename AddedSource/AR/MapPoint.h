#ifndef AR_MAPPOINT_H
#define AR_MAPPOINT_H

#include <vector>
#include <mutex>
#include <memory>
#include "Point2.h"
#include "TMath/TMath.h"
#include "MapResourceObject.h"
#include "MapResourcesManager.h"

namespace AR {

class Feature;
class Map;
class Frame;

class MapPoint:
        public MapResourceObject
{
public:
    class Statistic {
    public:
        Statistic();

        int failedScore() const;
        void incFailed();
        void incFailed(int value);

        int successScore() const;
        void incSuccess();
        void incSuccess(int value);

        int commonScore() const;
    private:
        int m_failedScore;
        int m_successScore;

        static std::mutex m_mutex;
    };

    ~MapPoint();

    bool isDeleted() const;

    std::size_t index() const;

    void setPosition(const TMath::TVectord & position);
    TMath::TVectord position() const;

    void transform(const TMath::TMatrixd & rotation, const TMath::TVectord & translation);

    std::size_t countFeatures() const;
    std::shared_ptr<Feature> feature(std::size_t index);
    std::shared_ptr<const Feature> feature(std::size_t index) const;

    std::shared_ptr<Feature> getBestCollinearFeature(const TMath::TVectord & dir, double limit = 0.0);
    std::shared_ptr<const Feature> getBestCollinearFeature(const TMath::TVectord & dir, double limit = 0.0) const;

    void optimize(MapResourcesManager * manager, int numberIterations);
    void optimize(MapResourcesManager * manager, int numberIterations, const Frame & frame, const Point2f & positionOnFrame);

    Statistic & statistic() const;

private:
    friend class Map;
    friend class Feature;

    MapPoint(const MapPoint & ) = delete;
    void operator = (const MapPoint & ) = delete;

    std::size_t m_index;
    TMath::TVectord m_position;
    std::vector<std::shared_ptr<Feature>> m_features;
    mutable Statistic m_statistic;

    MapPoint(Map * map, size_t index, const TMath::TVectord & position);

    void _freeFeature(Feature * feature);
    void _clearFeatures();
};

enum class TypeMapPoint: int
{
    Failed = 0,
    Unknown = 1,
    Good = 2
};

class BuilderTypePoint
{
public:
    BuilderTypePoint();

    TypeMapPoint getType(const MapPoint::Statistic & statistic) const;

    int goodSuccessLimit() const;
    void setGoodSuccessLimit(int limit);

    int failedLimit() const;
    void setFailedLimit(int limit);

private:
    int m_goodSuccessLimit;
    int m_failedLimit;
};

}

#endif // AR_MAPPOINT_H
