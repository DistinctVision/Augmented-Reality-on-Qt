#ifndef AR_MAPREADER_H
#define AR_MAPREADER_H

#include <memory>
#include <mutex>
#include "Image.h"
#include "TMath/TMatrix.h"
#include "TMath/TVector.h"
#include "Camera.h"

namespace AR {

class Map;
class MapPoint;
class Frame;
class KeyFrame;
class Feature;

class MapReader
{
public:
    MapReader(const Map* map);
    ~MapReader();

    const Map* map() const;
    std::size_t countMapPoints() const;
    std::shared_ptr<const MapPoint> mapPoint(std::size_t index) const;
    std::size_t countKeyFrames() const;
    std::shared_ptr<const KeyFrame> keyFrame(std::size_t index) const;

    std::shared_ptr<const KeyFrame> getNearestKeyFrame(const TMath::TVectord& position, const TMath::TVectord& dir) const;
    std::shared_ptr<const KeyFrame> getFurthestKeyFrame(const TMath::TVectord& position) const;

    int countImageLevels() const;
    int sizeOfSmallImage() const;

    Image<int> getSmallImage(const Frame& frame) const;

protected:
    friend class Map;

    MapReader(const MapReader& ) = delete;
    void operator = (const MapReader& ) = delete;

    Map* m_map;

    MapReader() = delete;
};

}

#endif // AR_MAPREADER_H
