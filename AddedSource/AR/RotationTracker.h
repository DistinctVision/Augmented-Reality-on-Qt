#ifndef AR_ROTATIONTRACKER_H
#define AR_ROTATIONTRACKER_H

#include <vector>

#include "TMath/TMath.h"
#include "AR/Frame.h"
#include "AR/FeatureDetector.h"
#include "AR/Camera.h"

namespace AR {


class RotationTracker
{
public:
    RotationTracker();
    ~RotationTracker();

    void setFirstFrame(const Frame& frame, std::vector<Point2f>& features);
    void setSecondFrame(const Frame& frame);

    double eps() const;
    void setEps(double eps);

    int numberIterations() const;
    void setNumberIterations(int numberIterations);

    int minLevel() const;
    int maxLevel() const;
    void setMinMaxLevel(int minLevel, int maxLevel);

    Point2i cursorSize() const;
    void setCursorSize(const Point2i& cursorSize);

    int countTrackedFeatures() const;

    void reset();

    bool tracking();

    float computeDisparity() const;

    TMath::TMatrixd secondRotation() const;
    TMath::TVectord secondTranslation() const;

    inline static void jacobian_xyz2uv(TMath::TVectord& J_x, TMath::TVectord& J_y, const TMath::TVectord& v)
    {
        TMath_assert((J_x.size() == 3) && (J_y.size() == 3));
        TMath_assert(v.size() == 3);

        double z_inv = 1.0 / v(2);
        double x_z = v(0) * z_inv;
        double y_z = v(1) * z_inv;

        J_x(0) = x_z * y_z;              // x * y / z^2
        J_x(1) = - (1.0 + x_z * x_z);    // -(1.0 + x^2 / z^2)
        J_x(2) = y_z;                    // y / z

        J_y(0) = 1.0 + y_z * y_z;        // 1.0 + y^2 / z^2
        J_y(1) = - x_z * y_z;            // -x * y / z^2
        J_y(2) = - x_z;                  // - x / z
    }

private:
    struct FeatureInfo {
        Point2f imagePosition;
        bool visible;
        float* pixels_cache;
        double* jacobian_cache;
        TMath::TVectord localPosition;
        TMath::TVectord J_x, J_y;
    };

    double m_eps;
    int m_numberIterations;
    int m_minLevel;
    int m_maxLevel;
    Point2i m_cursorSize;

    float* m_gaussian;
    float m_sigmaGaussian;
    float m_invMaxErrorSquared;

    Image<float> m_pixels_cashe;
    Image<double> m_jacobian_cashe;

    std::vector<ConstImage<uchar>> m_firstFrameImagePyramid;
    TMath::TMatrixd m_firstRotation;
    TMath::TVectord m_firstTranslation;
    std::shared_ptr<const Camera> m_firstCamera;

    std::vector<ConstImage<uchar>> m_secondFrameImagePyramid;
    TMath::TMatrixd m_secondRotation;
    TMath::TVectord m_secondTranslation;
    std::shared_ptr<const Camera> m_secondCamera;

    std::vector<FeatureInfo> m_featuresInfo;

    TMath::TVectord m_last_X;
    std::size_t m_countTrackedFeatures;

    std::vector<float> m_square_errors;
    float m_sigmaSquared;

    void _solveGaussian();
    void _setSigmaGaussian(float sigma);

    void _computeLevelInfo(const TMath::TMatrixd& deltaRotation, const TMath::TVectord& deltaTranslation, int level);
    double _optimize(TMath::TMatrixd& deltaRotation, TMath::TVectord& deltaTranslation, int level);
    bool _needToStop() const;
};

} // namespace AR

#endif // AR_ROTATIONTRACKER_H
