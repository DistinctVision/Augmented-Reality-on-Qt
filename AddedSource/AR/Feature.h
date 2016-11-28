#ifndef AR_FEATURE_H
#define AR_FEATURE_H

#include <vector>
#include <memory>
#include "Image.h"
#include "TMath/TVector.h"

namespace AR {

class Map;
class KeyFrame;
class MapPoint;

class Feature
{
public:
    ~Feature();

    bool isDeleted() const;

    std::shared_ptr<KeyFrame> keyFrame();
    std::shared_ptr<const KeyFrame> keyFrame() const;

    std::shared_ptr<MapPoint> mapPoint();
    std::shared_ptr<const MapPoint> mapPoint() const;

    int imageLevel() const;

    Point2f positionOnFrame() const;

    TMath::TVectord localDir() const;

private:
    friend class Map;
    friend class KeyFrame;
    friend class MapPoint;

    Feature(const Feature & ) = delete;
    void operator = (const Feature & ) = delete;

    std::weak_ptr<KeyFrame> m_keyFrame;
    int m_indexInKeyFrame;
    std::weak_ptr<MapPoint> m_mapPoint;
    int m_indexInMapPoint;

    int m_imageLevel;
    Point2f m_positionOnFrame;
    TMath::TVectord m_localDir;

    Feature(const std::shared_ptr<KeyFrame> & keyFrame,
            const Point2f & positionOnFrame, int imageLevel,
            const std::shared_ptr<MapPoint> & mapPoint);

    void _releaseInKeyFrame();
    void _releaseInMapPoint();
};

}

#endif // AR_FEATURE_H
