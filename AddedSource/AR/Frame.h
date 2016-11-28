#ifndef AR_FRAME_H
#define AR_FRAME_H

#include <mutex>
#include <vector>
#include <memory>
#include "Camera.h"
#include "TMath/TVector.h"
#include "TMath/TMatrix.h"

namespace AR {

class OpticalFlow;

class Frame
{
public:
    Frame(const std::shared_ptr<const Camera> & camera,
          const std::vector<Image<uchar>> & imagePyramid,
          const TMath::TMatrixd & rotation,
          const TMath::TVectord & translation);
    Frame(const std::shared_ptr<const Camera> & camera,
          const std::vector<Image<uchar>> & imagePyramid);
    Frame(const Frame & frame);
    Frame(Frame && frame);

    std::shared_ptr<const Camera> camera() const;

    int countImageLevels() const;
    ConstImage<uchar> imageLevel(int level) const;

    Point2i imageSize() const;

    TMath::TMatrixd rotation() const;
    void setRotation(const TMath::TMatrixd& rotation);

    TMath::TVectord translation() const;
    void setTranslation(const TMath::TVectord& translation);

    TMath::TVectord worldPosition() const;

    void transform(const TMath::TMatrixd& rotation, const TMath::TVectord& translation);

    bool imagePointInFrame(const Point2d & imagePoint) const;
    bool pointIsVisible(const TMath::TVectord & point) const;

    void copy(const Frame& frame);

    std::vector<Image<uchar>> getCopyOfImagePyramid() const;

    bool equals(const Frame& frame) const;

protected:
    friend class OpticalFlow;

    std::shared_ptr<const Camera> m_camera;
    std::vector<Image<uchar>> m_imagePyramid;
    TMath::TMatrixd m_rotation;
    TMath::TVectord m_translation;
};

}

#endif // AR_FRAME_H
