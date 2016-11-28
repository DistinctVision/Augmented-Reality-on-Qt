#include "MapPoint.h"
#include "KeyFrame.h"
#include "Map.h"
#include "Feature.h"
#include "Frame.h"
#include "LocationOptimizer.h"
#include "TMath/TMath.h"
#include "MapResourceObject.h"
#include <limits>
#include <climits>

namespace AR {

std::mutex MapPoint::Statistic::m_mutex;

MapPoint::Statistic::Statistic():
    m_failedScore(0), m_successScore(0)
{
}

int MapPoint::Statistic::failedScore() const
{
    return m_failedScore;
}

void MapPoint::Statistic::incFailed()
{
    //std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    ++m_failedScore;
}

void MapPoint::Statistic::incFailed(int value)
{
    TMath_assert(value > 0);
    //std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    m_failedScore += value;
}

int MapPoint::Statistic::successScore() const
{
    return m_successScore;
}

void MapPoint::Statistic::incSuccess()
{
    //std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    ++m_successScore;
}

void MapPoint::Statistic::incSuccess(int value)
{
    TMath_assert(value > 0);
    //std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    m_successScore += value;
}

int MapPoint::Statistic::commonScore() const
{
    return (m_successScore - m_failedScore);
}

MapPoint::MapPoint(Map * map, std::size_t index, const TMath::TVectord & position):
    MapResourceObject(map),
    m_index(index),
    m_position(position)
{
    TMath_assert(position.size() == 3);
}

MapPoint::~MapPoint()
{
    _clearFeatures();
}

bool MapPoint::isDeleted() const
{
    return (m_index == std::numeric_limits<std::size_t>::max());
}

std::size_t MapPoint::index() const
{
    return m_index;
}

void MapPoint::setPosition(const TMath::TVectord & position)
{
    TMath_assert(position.size() == 3);
    m_position = position;
}

void MapPoint::transform(const TMath::TMatrixd & rotation, const TMath::TVectord & translation)
{
    TMath_assert((rotation.rows() == 3) && (rotation.cols() == 3));
    TMath_assert(translation.size() == 3);
    m_position = rotation * m_position + translation;
}

TMath::TVectord MapPoint::position() const
{
    return m_position;
}

std::size_t MapPoint::countFeatures() const
{
    return m_features.size();
}

std::shared_ptr<Feature> MapPoint::feature(std::size_t index)
{
    return m_features[index];
}

std::shared_ptr<const Feature> MapPoint::feature(std::size_t index) const
{
    return m_features[index];
}

std::shared_ptr<Feature> MapPoint::getBestCollinearFeature(const TMath::TVectord & dir, double limit)
{
    using namespace TMath;
    std::shared_ptr<Feature> result;
    double best = limit, d;
    for (auto it = m_features.begin(); it != m_features.end(); ++it) {
        d = dot((*it)->localDir(), dir);
        if (d > best) {
            best = d;
            result = *it;
        }
    }
    return result;
}

std::shared_ptr<const Feature> MapPoint::getBestCollinearFeature(const TMath::TVectord & dir, double limit) const
{
    using namespace TMath;
    std::shared_ptr<const Feature> result;
    double best = limit, d;
    for (auto it = m_features.cbegin(); it != m_features.cend(); ++it) {
        d = dot((*it)->localDir(), dir);
        if (d > best) {
            best = d;
            result = *it;
        }
    }
    return result;
}

void MapPoint::optimize(MapResourcesManager * manager, int numberIterations)
{
    using namespace TMath;

    TVectord oldPosition = m_position, v_f;
    double chi2 = std::numeric_limits<double>::max();
    TWLS<double> wls(3);
    TVectord J_x(3), J_y(3);
    Point2d e;

    for (auto it = m_features.cbegin(); it != m_features.cend(); ++it)
        manager->lock((*it)->keyFrame().get());

    for (int iteration = 0; iteration < numberIterations; ++iteration) {

        wls.clear();

        double new_chi2 = 0.0;

        // compute residuals
        auto it = m_features.cbegin();
        while (it != m_features.cend()) {

            std::shared_ptr<const KeyFrame> keyFrame = (*it)->keyFrame();

            TMath_assert(keyFrame && !keyFrame->isDeleted());

            TMatrixd rotation = keyFrame->rotation();

            v_f = rotation * m_position + keyFrame->translation();

            if (v_f(2) < std::numeric_limits<float>::epsilon()) {
                ++it;
                continue;
            }

            const double z_inv_squared = 1.0 / (v_f(2) * v_f(2));

            J_x(0) = (rotation(0, 0) * v_f(2) - rotation(2, 0) * v_f(0)) * z_inv_squared;
            J_x(1) = (rotation(0, 1) * v_f(2) - rotation(2, 1) * v_f(0)) * z_inv_squared;
            J_x(2) = (rotation(0, 2) * v_f(2) - rotation(2, 2) * v_f(0)) * z_inv_squared;

            J_y(0) = (rotation(1, 0) * v_f(2) - rotation(2, 0) * v_f(1)) * z_inv_squared;
            J_y(1) = (rotation(1, 1) * v_f(2) - rotation(2, 1) * v_f(1)) * z_inv_squared;
            J_y(2) = (rotation(1, 2) * v_f(2) - rotation(2, 2) * v_f(1)) * z_inv_squared;

            e = LocationOptimizer::project2d((*it)->localDir()) - LocationOptimizer::project2d(v_f);

            wls.addMeasurement(e.x, J_x);
            wls.addMeasurement(e.y, J_y);

            new_chi2 += e.lengthSquared();

            ++it;
        }

        wls.compute();

        // check if error increased
        if ((new_chi2 > chi2) || (std::isnan(wls.X()(0)))) {
            m_position = oldPosition; // roll-back
            break;
        }

        oldPosition = m_position;
        m_position += wls.X();

        chi2 = new_chi2;
    }

    for (auto it = m_features.cbegin(); it != m_features.cend(); ++it)
        manager->unlock((*it)->keyFrame().get());
}

void MapPoint::optimize(MapResourcesManager * manager,
                        int numberIterations, const Frame & frame, const Point2f & positionOnFrame)
{
    using namespace TMath;

    TVectord oldPosition = m_position, v_f;
    double chi2 = std::numeric_limits<double>::max();
    TWLS<double> wls(3);
    TVectord J_x(3), J_y(3);
    Point2d e;
    TMatrixd rotation(3, 3);

    for (auto it = m_features.cbegin(); it != m_features.cend(); ++it)
        manager->lock((*it)->keyFrame().get());

    Point2d localPoint = frame.camera()->unproject(positionOnFrame);

    for (int iteration = 0; iteration < numberIterations; ++iteration) {

        wls.clear();

        double new_chi2 = 0.0;

        // compute residuals
        {
            rotation = frame.rotation();

            v_f = rotation * m_position + frame.translation();

            if (v_f(2) > std::numeric_limits<float>::epsilon()) {
                const double z_inv_squared = 1.0 / (v_f(2) * v_f(2));

                J_x(0) = (rotation(0, 0) * v_f(2) - rotation(2, 0) * v_f(0)) * z_inv_squared;
                J_x(1) = (rotation(0, 1) * v_f(2) - rotation(2, 1) * v_f(0)) * z_inv_squared;
                J_x(2) = (rotation(0, 2) * v_f(2) - rotation(2, 2) * v_f(0)) * z_inv_squared;

                J_y(0) = (rotation(1, 0) * v_f(2) - rotation(2, 0) * v_f(1)) * z_inv_squared;
                J_y(1) = (rotation(1, 1) * v_f(2) - rotation(2, 1) * v_f(1)) * z_inv_squared;
                J_y(2) = (rotation(1, 2) * v_f(2) - rotation(2, 2) * v_f(1)) * z_inv_squared;

                e = localPoint - LocationOptimizer::project2d(v_f);

                wls.addMeasurement(e.x, J_x);
                wls.addMeasurement(e.y, J_y);

                new_chi2 += e.lengthSquared();
            }
        }
        auto it = m_features.cbegin();
        while (it != m_features.cend()) {

            std::shared_ptr<const KeyFrame> keyFrame = (*it)->keyFrame();
            TMath_assert(keyFrame && !keyFrame->isDeleted());

            rotation = keyFrame->rotation();

            v_f = rotation * m_position + keyFrame->translation();

            if (v_f(2) < std::numeric_limits<float>::epsilon()) {
                ++it;
                continue;
            }

            const double z_inv_squared = 1.0 / (v_f(2) * v_f(2));

            J_x(0) = (rotation(0, 0) * v_f(2) - rotation(2, 0) * v_f(0)) * z_inv_squared;
            J_x(1) = (rotation(0, 1) * v_f(2) - rotation(2, 1) * v_f(0)) * z_inv_squared;
            J_x(2) = (rotation(0, 2) * v_f(2) - rotation(2, 2) * v_f(0)) * z_inv_squared;

            J_y(0) = (rotation(1, 0) * v_f(2) - rotation(2, 0) * v_f(1)) * z_inv_squared;
            J_y(1) = (rotation(1, 1) * v_f(2) - rotation(2, 1) * v_f(1)) * z_inv_squared;
            J_y(2) = (rotation(1, 2) * v_f(2) - rotation(2, 2) * v_f(1)) * z_inv_squared;

            e = LocationOptimizer::project2d((*it)->localDir()) - LocationOptimizer::project2d(v_f);

            wls.addMeasurement(e.x, J_x);
            wls.addMeasurement(e.y, J_y);

            new_chi2 += e.lengthSquared();

            ++it;
        }

        wls.compute();

        // check if error increased
        if ((new_chi2 > chi2) || (std::isnan(wls.X()(0)))) {
            m_position = oldPosition; // roll-back
            break;
        }

        oldPosition = m_position;
        m_position += wls.X();

        chi2 = new_chi2;
    }

    for (auto it = m_features.cbegin(); it != m_features.cend(); ++it)
        manager->unlock((*it)->keyFrame().get());
}

MapPoint::Statistic & MapPoint::statistic() const
{
    return m_statistic;
}

void MapPoint::_freeFeature(Feature * feature)
{
    TMath_assert(feature->m_indexInMapPoint >= 0);
    int lastIndex = m_features.size() - 1;
    if (feature->m_indexInMapPoint < lastIndex) {
        std::shared_ptr<Feature> lastFeature = m_features[lastIndex];
        m_features[feature->m_indexInMapPoint] = lastFeature;
        lastFeature->m_indexInMapPoint = feature->m_indexInMapPoint;
    }
    m_features.resize(lastIndex);
    feature->m_indexInMapPoint = -1;
    feature->m_mapPoint.reset();
}

void MapPoint::_clearFeatures()
{
    for (auto it = m_features.begin(); it != m_features.end(); ++it) {
        (*it)->m_indexInMapPoint = -1;
        (*it)->m_mapPoint.reset();
        (*it)->_releaseInKeyFrame();
    }
    m_features.clear();
}

BuilderTypePoint::BuilderTypePoint()
{
    m_goodSuccessLimit = 10;
    m_failedLimit = 15;
}

int BuilderTypePoint::goodSuccessLimit() const
{
    return m_goodSuccessLimit;
}

void BuilderTypePoint::setGoodSuccessLimit(int limit)
{
    m_goodSuccessLimit = limit;
}

int BuilderTypePoint::failedLimit() const
{
    return m_failedLimit;
}

void BuilderTypePoint::setFailedLimit(int limit)
{
    m_failedLimit = limit;
}

TypeMapPoint BuilderTypePoint::getType(const MapPoint::Statistic & statistic) const
{
    if (statistic.successScore() >= m_goodSuccessLimit)
        return TypeMapPoint::Good;
    return (statistic.failedScore() > m_failedLimit) ? TypeMapPoint::Unknown : TypeMapPoint::Failed;
}

}
