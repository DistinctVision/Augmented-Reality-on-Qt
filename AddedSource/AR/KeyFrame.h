#ifndef AR_KEYFRAME_H
#define AR_KEYFRAME_H

#include <mutex>
#include <vector>
#include <memory>
#include "Camera.h"
#include "TMath/TVector.h"
#include "TMath/TMatrix.h"
#include "Frame.h"
#include "PreviewFrame.h"
#include "MapResourceObject.h"

namespace AR {

class Map;
class Feature;

class KeyFrame:
        public Frame,
        public MapResourceObject
{
public:
    ~KeyFrame();

    bool isDeleted() const;
    std::size_t index() const;

    int countFeatures() const;
    std::shared_ptr<Feature> feature(int index);
    std::shared_ptr<const Feature> feature(int index) const;

    ConstImage<int> smallImage() const;

    static void getDepth(MapResourcesManager * manager, double & depthMean, double & depthMin,
                         const std::shared_ptr<const KeyFrame> & frame);

private:
    friend class Map;
    friend class Feature;

    KeyFrame(const KeyFrame & ) = delete;
    void operator = (const KeyFrame & ) = delete;

    mutable std::recursive_mutex m_mutex;
    std::size_t m_index;
    std::vector<std::shared_ptr<Feature>> m_features;
    ConstImage<int> m_smallImage;

    KeyFrame(Map * map, std::size_t index,
             const std::shared_ptr<const Camera> & camera,
             const std::vector<Image<uchar>> & imagePyramid,
             const TMath::TMatrixd & rotation,
             const TMath::TVectord & translation);

    KeyFrame(Map * map, std::size_t index,
             const std::shared_ptr<const Camera> & camera,
             const std::vector<Image<uchar>> & imagePyramid);

    KeyFrame(Map * map, size_t index, const Frame & frame);

    void _freeFeature(Feature * feature);
    void _clearFeatures();
};

}

#endif // AR_KEYFRAME_H
