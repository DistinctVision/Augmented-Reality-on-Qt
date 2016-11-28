#ifndef AR_PREVIEWFRAME_H
#define AR_PREVIEWFRAME_H

#include <memory>
#include <vector>
#include "TMath/TVector.h"
#include "Point2.h"
#include "Frame.h"
#include "MapResourcesManager.h"

namespace AR {

class MapPoint;
class KeyFrame;

class PreviewFrame:
        public Frame
{
public:
    struct PreviewFeature {
        std::shared_ptr<MapPoint> mapPoint;
        Point2f positionOnFrame;
        int imageLevel;
    };

public:
    PreviewFrame(const std::shared_ptr<const Camera> & camera,
                 const std::vector<Image<uchar>> & imagePyramid,
                 const TMath::TMatrixd & rotation,
                 const TMath::TVectord & translation);

    std::size_t countPreviewFeatures() const;

    PreviewFeature & previewFeature(std::size_t index);
    const PreviewFeature & previewFeature(std::size_t index) const;

    std::vector<PreviewFeature> & previewFeatures();
    const std::vector<PreviewFeature> & previewFeatures() const;

    void addPreviewFeature(const PreviewFeature & previewFeature);
    void addPreviewFeature(const std::shared_ptr<MapPoint> & mapPoint,
                           const Point2f & positionOnFrame,
                           int imageLevel);

    void deletePreviewFeature(std::size_t index);

    void copy(const std::shared_ptr<KeyFrame> & keyFrame);

    void copy(const PreviewFrame & frame);

    static void getDepth(MapResourcesManager * manager, double & depthMean, double & depthMin, PreviewFrame & frame);

private:
    std::vector<PreviewFeature> m_previewFeatures;
};

}

#endif
