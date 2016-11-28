#include "Feature.h"
#include "KeyFrame.h"
#include "MapPoint.h"
#include "Map.h"
#include "MapResourcesManager.h"

namespace AR {

Feature::Feature(const std::shared_ptr<KeyFrame> & keyFrame,
                 const Point2f & positionOnFrame, int imageLevel,
                 const std::shared_ptr<MapPoint> & mapPoint)
{
    TMath_assert(keyFrame);
    TMath_assert(mapPoint);
    TMath_assert(keyFrame->map() == mapPoint->map());
    TMath_assert((imageLevel >= 0) && (imageLevel < keyFrame->countImageLevels()));
    m_keyFrame = keyFrame;
    m_mapPoint = mapPoint;
    m_imageLevel = imageLevel;
    m_positionOnFrame = positionOnFrame;
    Point2d p = keyFrame->camera()->unproject(m_positionOnFrame);
    m_localDir = TMath::TVectord::create(p.x, p.y, 1.0).normalized();
}

Feature::~Feature()
{
    _releaseInKeyFrame();
    _releaseInMapPoint();
}

bool Feature::isDeleted() const
{
    return ((m_indexInKeyFrame < 0) || (m_indexInMapPoint < 0));
}

std::shared_ptr<KeyFrame> Feature::keyFrame()
{
    return m_keyFrame.lock();
}

std::shared_ptr<const KeyFrame> Feature::keyFrame() const
{
    return m_keyFrame.lock();
}

std::shared_ptr<MapPoint> Feature::mapPoint()
{
    return m_mapPoint.lock();
}

std::shared_ptr<const MapPoint> Feature::mapPoint() const
{
    return m_mapPoint.lock();
}

int Feature::imageLevel() const
{
    return m_imageLevel;
}

Point2f Feature::positionOnFrame() const
{
    return m_positionOnFrame;
}

TMath::TVectord Feature::localDir() const
{
    return m_localDir;
}

void Feature::_releaseInKeyFrame()
{
    if (m_indexInKeyFrame >= 0) {
        m_keyFrame.lock()->_freeFeature(this);
    }
}

void Feature::_releaseInMapPoint()
{
    if (m_indexInMapPoint >= 0) {
        m_mapPoint.lock()->_freeFeature(this);
    }
}

}
