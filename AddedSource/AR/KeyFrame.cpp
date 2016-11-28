#include "KeyFrame.h"
#include "Feature.h"
#include "ImageProcessing.h"
#include "Map.h"
#include "MapResourcesManager.h"
#include "MapPoint.h"
#include "MapResourceLocker.h"
#include "TMath/TMath.h"
#include <limits>
#include <climits>

namespace AR {

KeyFrame::KeyFrame(Map * map, size_t index, const std::shared_ptr<const Camera> & camera,
                   const std::vector<Image<uchar>> & imagePyramid,
                   const TMath::TMatrixd & rotation,
                   const TMath::TVectord & translation):
    Frame(camera, imagePyramid, rotation, translation),
    MapResourceObject(map),
    m_index(index)
{
    TMath_assert(imagePyramid.size() > 0);
    TMath_assert((int)imagePyramid.size() == m_map->countImageLevels());
    m_smallImage = m_map->getSmallImage(*this);
}

KeyFrame::KeyFrame(Map * map, size_t index, const std::shared_ptr<const Camera> & camera,
                   const std::vector<Image<uchar>> & imagePyramid):
    Frame(camera, imagePyramid),
    MapResourceObject(map),
    m_index(index)
{
    TMath_assert(imagePyramid.size() > 0);
    TMath_assert((int)imagePyramid.size() == m_map->countImageLevels());
    m_smallImage = m_map->getSmallImage(*this);
}

KeyFrame::KeyFrame(Map * map, std::size_t index, const Frame & frame):
    Frame(frame.camera(), frame.getCopyOfImagePyramid(), frame.rotation(), frame.translation()),
    MapResourceObject(map),
    m_index(index)
{
    TMath_assert(frame.countImageLevels() == m_map->countImageLevels());
    m_smallImage = m_map->getSmallImage(*this);
}

KeyFrame::~KeyFrame()
{
    _clearFeatures();
}

bool KeyFrame::isDeleted() const
{
    return (m_index == std::numeric_limits<std::size_t>::max());
}

std::size_t KeyFrame::index() const
{
    return m_index;
}

int KeyFrame::countFeatures() const
{
    return (int)m_features.size();
}

std::shared_ptr<Feature> KeyFrame::feature(int index)
{
    return m_features[index];
}

std::shared_ptr<const Feature> KeyFrame::feature(int index) const
{
    return m_features[index];
}

ConstImage<int> KeyFrame::smallImage() const
{
    return m_smallImage;
}

void KeyFrame::_freeFeature(Feature * feature)
{
    TMath_assert(feature->m_indexInKeyFrame >= 0);
    int lastIndex = m_features.size() - 1;
    if (feature->m_indexInKeyFrame < lastIndex) {
        std::shared_ptr<Feature> lastFeature = m_features[lastIndex];
        m_features[feature->m_indexInKeyFrame] = lastFeature;
        lastFeature->m_indexInKeyFrame = feature->m_indexInKeyFrame;
    }
    m_features.resize(lastIndex);
    feature->m_indexInKeyFrame = -1;
    feature->m_keyFrame.reset();
}

void KeyFrame::_clearFeatures()
{
    for (auto it = m_features.begin(); it != m_features.end(); ++it) {
        (*it)->m_indexInKeyFrame = -1;
        (*it)->m_keyFrame.reset();
        (*it)->_releaseInMapPoint();
    }
    m_features.clear();
}

//static
void KeyFrame::getDepth(MapResourcesManager * manager, double & depthMean, double & depthMin,
                        const std::shared_ptr<const KeyFrame> & frame)
{
    using namespace TMath;

    if (frame->countFeatures() == 0) {
        depthMean = depthMin = 0.0;
        return;
    }

    TMatrixd rotation = frame->rotation();
    TVectord translation = frame->translation();

    std::vector<double> vectorDepth;
    vectorDepth.reserve(frame->countFeatures());
    depthMin = std::numeric_limits<double>::max();
    for (int i = 0; i < frame->countFeatures(); ++i) {
        std::shared_ptr<const MapPoint> mapPoint = frame->feature(i)->mapPoint();
        if (!mapPoint)
            continue;
        MapResourceLocker lockerR(manager, mapPoint.get()); (void)lockerR;
        if (mapPoint->isDeleted())
            continue;
        TVectord localPos = rotation * mapPoint->position() + translation;
        if (depthMin > localPos(2))
            depthMin = localPos(2);
        vectorDepth.push_back(localPos(2));
    }
    if (vectorDepth.empty()) {
        depthMean = depthMin = 0.0;
        return;
    }
    std::size_t offset = vectorDepth.size() / 2;
    std::nth_element(vectorDepth.begin(), vectorDepth.begin() + offset, vectorDepth.end());
    depthMean = vectorDepth[offset];
}

}
