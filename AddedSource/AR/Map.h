#ifndef AR_MAP_H
#define AR_MAP_H

#include <vector>
#include <memory>
#include <mutex>
#include "TMath/TVector.h"
#include "TMath/TMatrix.h"
#include "Image.h"
#include "Camera.h"

namespace AR {

class Frame;
class KeyFrame;
class MapPoint;
class Feature;
class MapResourceObject;
class MapResourcesManager;

class Map
{
public:
    class MapListener
    {
    public:
        virtual void onCreateKeyFrame(const std::shared_ptr<KeyFrame> & keyFrame)
        { (void)keyFrame; }
        virtual void onDeleteKeyFrame(const std::shared_ptr<KeyFrame> & keyFrame)
        { (void)keyFrame; }
        virtual void onCreateMapPoint(const std::shared_ptr<MapPoint> & mapPoint)
        { (void)mapPoint; }
        virtual void onDeleteMapPoint(const std::shared_ptr<MapPoint> & mapPoint)
        { (void)mapPoint; }
        virtual void onCreateFeature(const std::shared_ptr<const Feature> & feature)
        { (void)feature; }
        virtual void onDeleteFeature(const std::shared_ptr<const Feature> & feature)
        { (void)feature; }
        virtual void onTransformMap(const TMath::TMatrixd & rotation, const TMath::TVectord & translation)
        { (void)rotation; (void)translation; }
        virtual void onScaleMap(double scale) { (void)scale; }
        virtual void onResetMap() {}
    };

    Map(int countImageLevels, int sizeOfSmallImage);
    ~Map();

    void lock();
    void unlock();

    MapListener * listener();
    void setListener(MapListener * listener);
    void resetListener();

    std::shared_ptr<MapPoint> createMapPoint(const TMath::TVectord & position);
    void deleteMapPoint(const std::shared_ptr<MapPoint> & mapPoint);
    std::size_t countMapPoints() const;
    std::shared_ptr<MapPoint> mapPoint(std::size_t index);
    std::shared_ptr<const MapPoint> mapPoint(std::size_t index) const;

    std::shared_ptr<KeyFrame> createKeyFrame(const std::shared_ptr<const Camera> & camera,
                                             const std::vector<Image<uchar>> & imagePyramid);
    std::shared_ptr<KeyFrame> createKeyFrame(const std::shared_ptr<const Camera> & camera,
                                             const std::vector<Image<uchar>> & imagePyramid,
                                             const TMath::TMatrixd & rotation,
                                             const TMath::TVectord & translation);
    std::shared_ptr<KeyFrame> createKeyFrame(const Frame & frame);
    void deleteKeyFrame(const std::shared_ptr<KeyFrame> & keyFrame);
    std::size_t countKeyFrames() const;
    std::shared_ptr<KeyFrame> keyFrame(std::size_t index);
    const std::shared_ptr<const KeyFrame> keyFrame(std::size_t index) const;

    std::shared_ptr<KeyFrame> getNearestKeyFrame(MapResourcesManager * manager,
                                                 const TMath::TVectord & position,
                                                 const TMath::TVectord & dir);
    std::shared_ptr<const KeyFrame> getNearestKeyFrame(MapResourcesManager * manager,
                                                 const TMath::TVectord & position,
                                                 const TMath::TVectord & dir) const;
    std::shared_ptr<KeyFrame> getFurthestKeyFrame(MapResourcesManager * manager,
                                                  const TMath::TVectord & position);
    std::shared_ptr<const KeyFrame> getFurthestKeyFrame(MapResourcesManager * manager,
                                                  const TMath::TVectord & position) const;

    std::shared_ptr<Feature> createFeature(const std::shared_ptr<KeyFrame> & keyFrame,
                                           const Point2f& positionOnFrame, int imageLevel,
                                           const std::shared_ptr<MapPoint> & mapPoint);
    void deleteFeature(const std::shared_ptr<Feature> & feature);

    int countImageLevels() const;
    void setCountImageLevels(MapResourcesManager * manager, int count);
    int sizeOfSmallImage() const;
    void setSizeOfSmallImage(MapResourcesManager * manager, int size);

    Image<int> getSmallImage(const Frame & frame) const;

    void transform(MapResourcesManager * manager,
                   const TMath::TMatrixd & rotation,
                   const TMath::TVectord & translation);
    void scale(MapResourcesManager * manager, double scale);

    void resetMap(MapResourcesManager * manager);

    void deleteNullMapPoints(MapResourcesManager * manager);

private:
    friend class MapPoint;
    friend class KeyFrame;

    Map(const Map & ) = delete;
    void operator = (const Map & ) = delete;

    mutable std::mutex m_mutex;
    mutable std::mutex m_mutex_mapPoints;
    mutable std::mutex m_mutex_keyFrames;

    int m_countImageLevels;
    mutable Image<uchar> m_casheSmallImageH;
    mutable Image<uchar> m_casheSmallImageV;

    std::vector<std::shared_ptr<MapPoint>> m_mapPoints;
    std::vector<std::shared_ptr<KeyFrame>> m_keyFrames;

    MapListener * m_listener;

    static MapListener _static_null_map_listener;
};

}

#endif // AR_MAP_H
