#ifndef IMAGETRACKER_H
#define IMAGETRACKER_H

#include <vector>
#include <memory>
#include "Image.h"
#include "TMath/TVector.h"
#include "TMath/TMatrix.h"
#include "Frame.h"

namespace AR {

class ImageTracker
{
public:
    ImageTracker();
    ~ImageTracker();

    void setFirstFrame(const Frame& frame,
                       const std::vector<ConstImage<float>>& depthImagePyramid,
                       const std::vector<ConstImage<float>>& weightImagePyramid);
    void setSecondFrame(const Frame& frame);

    double eps() const;
    void setEps(double eps);

    int numberIterations() const;
    void setNumberIterations(int numberIterations);

    int minLevel() const;
    int maxLevel() const;
    void setMinMaxLevel(int minLevel, int maxLevel);

    void reset();
    void tracking();

    TMath::TMatrixd secondRotation() const;
    TMath::TVectord secondTranslation() const;

private:
    struct CashePixelInfo {
        float localPos[3];
        float J_x[6];
        float J_y[6];
        float J[6];
    };

    double m_eps;
    int m_numberIterations;
    int m_minLevel;
    int m_maxLevel;

    float m_sigmaSquared;

    std::vector<ConstImage<CashePixelInfo>> m_tempCashePyramid;
    std::vector<ConstImage<float>> m_firstWeightImagePyramid;
    std::vector<ConstImage<float>> m_firstDepthImagePyramid;
    std::vector<ConstImage<uchar>> m_firstFrameImagePyramid;
    TMath::TMatrixd m_firstRotation;
    TMath::TVectord m_firstTranslation;
    std::shared_ptr<const Camera> m_firstCamera;

    std::vector<ConstImage<uchar>> m_secondFrameImagePyramid;
    TMath::TMatrixd m_secondRotation;
    TMath::TVectord m_secondTranslation;
    std::shared_ptr<const Camera> m_secondCamera;

    TMath::TVectord m_last_X;

    void _computeLevelInfo(const TMath::TMatrixd& deltaRotation, const TMath::TVectord& deltaTranslation, int level);
    double _optimize(TMath::TMatrixd& deltaRotation, TMath::TVectord& deltaTranslation, int level);
    bool _needToStop() const;
};

}

#endif // IMAGETRACKER_H
