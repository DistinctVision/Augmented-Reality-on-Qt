#ifndef AR_TRACKER_H
#define AR_TRACKER_H

#include <vector>
#include <memory>
#include "Image.h"
#include "TMath/TVector.h"
#include "TMath/TMatrix.h"
#include "KeyFrame.h"
#include "PreviewFrame.h"
#include "MapResourcesManager.h"

namespace AR {

class Tracker
{
public:
    Tracker();
    ~Tracker();

    void setFirstFrame(const std::shared_ptr<KeyFrame> & keyFrame);
    void setFirstFrame(PreviewFrame & frame);
    void setSecondFrame(const std::shared_ptr<KeyFrame> & keyFrame);
    void setSecondFrame(const PreviewFrame & frame);

    double eps() const;
    void setEps(double eps);

    int numberIterations() const;
    void setNumberIterations(int numberIterations);

    int minLevel() const;
    int maxLevel() const;
    void setMinMaxLevel(int minLevel, int maxLevel);

    Point2i cursorSize() const;
    void setCursorSize(const Point2i & cursorSize);

    int countTrackedFeatures() const;

    void reset();

    void tracking();

    TMath::TMatrixd secondRotation() const;
    TMath::TVectord secondTranslation() const;

    MapResourcesManager * mapResourceManager();
    void setMapResourceManager(MapResourcesManager * mapResourceManager);

private:
    struct FeatureInfo {
        Point2f imagePosition;
        bool visible;
        float * pixels_cache;
        double* jacobian_cache;
        TMath::TVectord localPosition;
        TMath::TVectord J_x, J_y;
    };

    double m_eps;
    int m_numberIterations;
    int m_minLevel;
    int m_maxLevel;
    Point2i m_cursorSize;

    float * m_gaussian;
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

    MapResourcesManager * m_resourceManager;

    void _solveGaussian();
    void _setSigmaGaussian(float sigma);

    void _computeLevelInfo(const TMath::TMatrixd & deltaRotation, const TMath::TVectord & deltaTranslation, int level);
    double _optimize(TMath::TMatrixd & deltaRotation, TMath::TVectord & deltaTranslation, int level);
    bool _needToStop() const;
};

}

#endif // AR_TRACKER_H
