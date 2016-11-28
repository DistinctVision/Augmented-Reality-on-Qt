#ifndef AR_MAPWRITER_H
#define AR_MAPWRITER_H

#include <vector>
#include "Image.h"
#include "TMath/TMath.h"

namespace AR {

class Map;
class Frame;
class KeyFrame;
class MapPoint;
class Feature;
class Camera;

class MapWriter
{
public:
    MapWriter(Map* map);
    ~MapWriter();

    Map* map();
    std::size_t countMapPoints() const;
    std::shared_ptr<MapPoint> mapPoint(std::size_t index);
    std::size_t countKeyFrames() const;
    std::shared_ptr<KeyFrame> keyFrame(size_t index);

    std::shared_ptr<MapPoint> createMapPoint(const TMath::TVectord& position);
    void deleteMapPoint(const std::shared_ptr<MapPoint>& mapPoint);
    std::shared_ptr<KeyFrame> createKeyFrame(const std::shared_ptr<const Camera>& camera,
                                             const std::vector<Image<uchar>>& imagePyramid);
    std::shared_ptr<KeyFrame> createKeyFrame(const Frame& frame);
    void deleteKeyFrame(const std::shared_ptr<KeyFrame>& keyFrame);

    std::shared_ptr<KeyFrame> getNearestKeyFrame(const TMath::TVectord& position, const TMath::TVectord& dir);
    std::shared_ptr<KeyFrame> getFurthestKeyFrame(const TMath::TVectord& position);

    std::shared_ptr<Feature> createFeature(const std::shared_ptr<KeyFrame>& keyFrame,
                                           const Point2f& positionOnFrame, int imageLevel,
                                           const std::shared_ptr<MapPoint>& mapPoint);
    void deleteFeature(const std::shared_ptr<Feature>& feature);

    std::shared_ptr<MapPoint> accessWrite(const std::shared_ptr<const MapPoint>& mapPoint);
    std::shared_ptr<KeyFrame> accessWrite(const std::shared_ptr<const KeyFrame>& keyFrame);
    std::shared_ptr<Feature> accessWrite(const std::shared_ptr<const Feature>& feature);

    int countImageLevels() const;
    void setCountImageLevels(int count);
    int sizeOfSmallImage() const;
    void setSizeOfSmallImage(int size);

    void resetMap();

    void transform(const TMath::TMatrixd& rotation, const TMath::TVectord& translation);
    void scale(double scale);

    void deleteNullMapPoints();

    Image<int> getSmallImage(const Frame& frame) const;

protected:
    friend class Map;

    MapWriter(const MapWriter& ) = delete;
    void operator = (const MapWriter& ) = delete;

    Map* m_map;

    MapWriter() = delete;
};

}

#endif // AR_MAPWRITER_H
