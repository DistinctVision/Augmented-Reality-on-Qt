#include "LocationOptimizer.h"
#include "Feature.h"
#include "MapPoint.h"
#include "MapResourceLocker.h"
#include "Image.h"
#include "TukeyRobustCost.h"
#include "TMath/TMath.h"
#include <vector>

namespace AR {

LocationOptimizer::LocationOptimizer()
{
    m_builderTypeMapPoint = nullptr;
    m_mapResourceManager = nullptr;
    m_numberIterations = 15;
    m_eps = 3e-5;
    setMaxPixelError(5.0);
}

int LocationOptimizer::numberIterations() const
{
    return m_numberIterations;
}

void LocationOptimizer::setNumberIterations(int numberIterations)
{
    m_numberIterations = numberIterations;
}

double LocationOptimizer::maxSquarePixelError() const
{
    return m_maxSquarePixelError;
}

void LocationOptimizer::setMaxPixelError(double maxPixelError)
{
    m_maxSquarePixelError = maxPixelError;
}

double LocationOptimizer::eps() const
{
    return m_eps;
}

void LocationOptimizer::setEps(double eps)
{
    m_eps = eps;
}

BuilderTypePoint* LocationOptimizer::builderTypeMapPoint()
{
    return m_builderTypeMapPoint;
}

const BuilderTypePoint* LocationOptimizer::builderTypeMapPoint() const
{
    return m_builderTypeMapPoint;
}

void LocationOptimizer::setBuilderTypeMapPoint(BuilderTypePoint * builderTypeMapPoint)
{
    m_builderTypeMapPoint = builderTypeMapPoint;
}

MapResourcesManager * LocationOptimizer::mapResourceManager()
{
    return m_mapResourceManager;
}

void LocationOptimizer::setMapResourceManager(MapResourcesManager * mapResourceManager)
{
    m_mapResourceManager = mapResourceManager;
}

void LocationOptimizer::optimize(PreviewFrame & previewFrame)
{
    TMath_assert(m_builderTypeMapPoint != nullptr);
    using namespace TMath;

    std::vector<PreviewFrame::PreviewFeature> & features = previewFrame.previewFeatures();

    TWLS<double> wls(6);

    // compute the scale of the error for robust estimation
    std::vector<float> squareErrors;
    squareErrors.resize(features.size());
    std::vector<Point2d> f_points;
    f_points.resize(features.size());

    Point2d e;

    std::shared_ptr<const Camera> camera = previewFrame.camera();

    TMatrixd rotation = previewFrame.rotation();
    TVectord translation = previewFrame.translation();

    std::size_t i = 0;
    for (auto it = features.begin(); it != features.end(); ++it, ++i) {
        std::shared_ptr<MapPoint> mapPoint = it->mapPoint;
        m_mapResourceManager->lock(mapPoint.get());
        f_points[i] = camera->unproject(it->positionOnFrame);
        float scale = (float)(1 << it->imageLevel);
        e = f_points[i] - project2d(rotation * mapPoint->position() + translation);
        squareErrors[i] = (float)(e.lengthSquared() / (scale * scale));
    }

    if (squareErrors.empty())
        return;

    double squareSigma = (double)TukeyRobustCost::findSquareSigma(squareErrors);

    TMatrixd dRotation(3, 3);
    TVectord dTranslation(3);

    TVectord v_f(3), J_x(6), J_y(6);

    for (int iteration = 0; iteration < m_numberIterations; ++iteration) {

        /*if (iteration == m_numberIterations / 2)
            squareSigma *= 0.5f;*/

        wls.clear();

        i = 0;
        for (auto it = features.begin(); it != features.end(); ++it, ++i) {
            std::shared_ptr<MapPoint> mapPoint = it->mapPoint;

            v_f = rotation * mapPoint->position() + translation;
            e = f_points[i] - project2d(v_f);
            double sqrt_cov = (double)(1 << it->imageLevel);
            sqrt_cov *= sqrt_cov;

            jacobian_xyz2uv(J_x, J_y, v_f);

            double weight = (TukeyRobustCost::weight(e.lengthSquared(), squareSigma)) / sqrt_cov;

            wls.addMeasurement(e.x, J_x, weight);
            wls.addMeasurement(e.y, J_y, weight);
        }

        wls.compute();

        TVectord dT = - wls.X();
        TTools::exp_transform(dRotation, dTranslation, dT);

        rotation = dRotation * rotation;
        translation = dRotation * translation + dTranslation;

        if (_needStop(dT))
            break;
    }

    previewFrame.setRotation(rotation);
    previewFrame.setTranslation(translation);

    for (auto it = features.begin(); it != features.end(); ) {
        std::shared_ptr<MapPoint> mapPoint = it->mapPoint;
        e = it->positionOnFrame.cast<double>() - camera->project(project2d(rotation * mapPoint->position() + translation));
        float scale = (float)(1 << it->imageLevel);
        if (e.lengthSquared() / (scale * scale) > m_maxSquarePixelError) {
            mapPoint->statistic().incFailed();
            if (m_builderTypeMapPoint->getType(mapPoint->statistic()) == TypeMapPoint::Failed)
                m_failedMapPoints.push_back(mapPoint);
            it = features.erase(it);
        } else {
            ++it;
        }
        m_mapResourceManager->unlock(mapPoint.get());
    }
}

void LocationOptimizer::deleteFaildedMapPoints(Map * map)
{
    for (auto it = m_failedMapPoints.begin(); it != m_failedMapPoints.end(); ++it) {
        std::shared_ptr<MapPoint> mapPoint = *it;
        MapResourceLocker lockerR(m_mapResourceManager, mapPoint.get()); (void)lockerR;
        map->deleteMapPoint(mapPoint);
    }
    m_failedMapPoints.resize(0);
}
}
