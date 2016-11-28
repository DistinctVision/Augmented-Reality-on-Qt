#ifndef AR_LOCATIONOPTIMIZER_H
#define AR_LOCATIONOPTIMIZER_H

#include <vector>
#include "TMath/TVector.h"
#include "TMath/TMatrix.h"
#include "MapPoint.h"
#include "Map.h"
#include "PreviewFrame.h"
#include "MapProjector.h"
#include "MapResourcesManager.h"

namespace AR {

class LocationOptimizer
{
public:
    LocationOptimizer();

    int numberIterations() const;
    void setNumberIterations(int numberIterations);

    double maxSquarePixelError() const;
    void setMaxPixelError(double maxPixelError);

    double eps() const;
    void setEps(double eps);

    void optimize(PreviewFrame & previewFrame);

    const BuilderTypePoint * builderTypeMapPoint() const;
    BuilderTypePoint * builderTypeMapPoint();
    void setBuilderTypeMapPoint(BuilderTypePoint * builderTypeMapPoint);

    MapResourcesManager * mapResourceManager();
    void setMapResourceManager(MapResourcesManager * mapResourceManager);

    void deleteFaildedMapPoints(Map * map);

    inline static Point2d project2d(const TMath::TVectord & v)
    {
        return Point2d(v(0) / v(2), v(1) / v(2));
    }

    inline static void jacobian_xyz2uv(TMath::TVectord & J_x, TMath::TVectord & J_y, const TMath::TVectord & v)
    {
        TMath_assert((J_x.size() == 6) && (J_y.size() == 6));
        TMath_assert(v.size() == 3);

        double x = v(0);
        double y = v(1);
        double z_inv = 1.0 / v(2);
        double z_inv_squared = z_inv * z_inv;

        J_x(0) = - z_inv;                 // - 1 / z
        J_x(1) = 0.0;                     // 0
        J_x(2) = x * z_inv_squared;       // x / z^2
        J_x(3) = y * J_x(2);              // x * y / z^2
        J_x(4) = - (1.0 + x * J_x(2));    // -(1.0 + x^2 / z^2)
        J_x(5) = y * z_inv;               // y / z

        J_y(0) = 0.0;                     // 0
        J_y(1) = - z_inv;                 // - 1 / z
        J_y(2) = y * z_inv_squared;       // y / z^2
        J_y(3) = 1.0 + y * J_y(2);        // 1.0 + y^2 / z^2
        J_y(4) = - J_x(3);                // -x * y / z^2
        J_y(5) = - x * z_inv;             // - x / z
    }

    inline static void jacobian_xyz2uv(float * J_x, float * J_y, const float * v)
    {
        float x = v[0];
        float y = v[1];
        float z_inv = 1.0f / v[2];
        float z_inv_squared = z_inv * z_inv;

        J_x[0] = - z_inv;                 // - 1 / z
        J_x[1] = 0.0f;                    // 0
        J_x[2] = x * z_inv_squared;       // x / z^2
        J_x[3] = y * J_x[2];              // x * y / z^2
        J_x[4] = - (1.0f + x * J_x[2]);   // -(1.0 + x^2 / z^2)
        J_x[5] = y * z_inv;               // y / z

        J_y[0] = 0.0f;                    // 0
        J_y[1] = - z_inv;                 // - 1 / z
        J_y[2] = y * z_inv_squared;       // y / z^2
        J_y[3] = 1.0f + y * J_y[2];       // 1.0 + y^2 / z^2
        J_y[4] = - J_x[3];                // -x * y / z^2
        J_y[5] = - x * z_inv;             // - x / z
    }

private:
    int m_numberIterations;
    double m_maxSquarePixelError;
    double m_eps;

    BuilderTypePoint * m_builderTypeMapPoint;
    MapResourcesManager * m_mapResourceManager;
    std::vector<std::shared_ptr<MapPoint>> m_failedMapPoints;

    inline bool _needStop(const TMath::TVectord & dT) const
    {
        TMath_assert(dT.size() == 6);
        for (int i = 0; i < 6; ++i) {
            if (std::fabs(dT(i)) > m_eps)
                return false;
        }
        return true;
    }
};

}

#endif // AR_LOCATIONOPTIMIZER_H
