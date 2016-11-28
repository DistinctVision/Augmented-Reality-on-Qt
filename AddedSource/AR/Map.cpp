#include "Map.h"
#include "TMath/TMath.h"
#include "Frame.h"
#include "KeyFrame.h"
#include "MapPoint.h"
#include "Feature.h"
#include "ImageProcessing.h"
#include "Painter.h"
#include <chrono>
#include <thread>
#include <limits>
#include <climits>
#include <cmath>
#include "MapResourceObject.h"
#include "MapResourcesManager.h"
#include "MapResourceLocker.h"

namespace AR {

Map::MapListener Map::_static_null_map_listener;

Map::Map(int countImageLevels, int sizeOfSmallImage)
{
    m_countImageLevels = countImageLevels;
    m_casheSmallImageH = Image<uchar>(Point2i(sizeOfSmallImage, sizeOfSmallImage));
    m_listener = &_static_null_map_listener;
}

Map::~Map()
{
}

void Map::lock()
{
    m_mutex.lock();
}

void Map::unlock()
{
    m_mutex.unlock();
}

Map::MapListener * Map::listener()
{
    //std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    return m_listener;
}

void Map::setListener(MapListener * listener)
{
    TMath_assert(listener != nullptr);
    //std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    m_listener = listener;
}

void Map::resetListener()
{
    //std::lock_guard<std::mutex> lock(m_mutex); (void)lock;
    m_listener = &_static_null_map_listener;
}

std::shared_ptr<MapPoint> Map::createMapPoint(const TMath::TVectord & position)
{
    std::shared_ptr<MapPoint> newMapPoint(new MapPoint(this, m_mapPoints.size(), position));
    {
        //std::lock_guard<std::mutex> locker(m_mutex_mapPoints); (void)locker;
        m_mapPoints.push_back(newMapPoint);
    }
    m_listener->onCreateMapPoint(newMapPoint);
    return newMapPoint;
}

void Map::deleteMapPoint(const std::shared_ptr<MapPoint> & mapPoint)
{
    if (mapPoint->isDeleted())
        return;
    {
        //std::lock_guard<std::mutex> locker(m_mutex_mapPoints); (void)locker;
        TMath_assert(mapPoint);
        TMath_assert(mapPoint->map() == this);
        TMath_assert(!mapPoint->isDeleted());
        TMath_assert(mapPoint == m_mapPoints[mapPoint->m_index]);
        std::size_t lastIndex = m_mapPoints.size() - 1;
        if (mapPoint->m_index < lastIndex) {
            std::shared_ptr<MapPoint> lastMapPoint = m_mapPoints[lastIndex];
            m_mapPoints[mapPoint->m_index] = lastMapPoint;
            lastMapPoint->m_index = mapPoint->m_index;
        }
        m_mapPoints.resize(lastIndex);
        mapPoint->_clearFeatures();
        mapPoint->m_index = std::numeric_limits<std::size_t>::max();
    }
    m_listener->onDeleteMapPoint(mapPoint);
}

std::size_t Map::countMapPoints() const
{
    //std::lock_guard<std::mutex> locker(m_mutex_mapPoints); (void)locker;
    return m_mapPoints.size();
}

std::shared_ptr<MapPoint> Map::mapPoint(std::size_t index)
{
    //std::lock_guard<std::mutex> locker(m_mutex_mapPoints); (void)locker;
    return m_mapPoints[index];
}

std::shared_ptr<const MapPoint> Map::mapPoint(std::size_t index) const
{
    //std::lock_guard<std::mutex> locker(m_mutex_mapPoints); (void)locker;
    return m_mapPoints[index];
}

std::shared_ptr<KeyFrame> Map::createKeyFrame(const std::shared_ptr<const Camera> & camera,
                                              const std::vector<Image<uchar>> & imagePyramid)
{
    std::shared_ptr<KeyFrame> newKeyFrame(new KeyFrame(this, m_keyFrames.size(), camera, imagePyramid));
    {
        //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
        m_keyFrames.push_back(newKeyFrame);
    }
    m_listener->onCreateKeyFrame(newKeyFrame);
    return newKeyFrame;
}

std::shared_ptr<KeyFrame> Map::createKeyFrame(const std::shared_ptr<const Camera> & camera,
                                              const std::vector<Image<uchar>> & imagePyramid,
                                              const TMath::TMatrixd & rotation,
                                              const TMath::TVectord & translation)
{
    std::shared_ptr<KeyFrame> newKeyFrame(new KeyFrame(this, m_keyFrames.size(), camera, imagePyramid,
                                                       rotation, translation));
    {
        //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
        m_keyFrames.push_back(newKeyFrame);
    }
    m_listener->onCreateKeyFrame(newKeyFrame);
    return newKeyFrame;
}

std::shared_ptr<KeyFrame> Map::createKeyFrame(const Frame & frame)
{
    std::shared_ptr<KeyFrame> newKeyFrame(new KeyFrame(this, m_keyFrames.size(), frame));
    {
        //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
        m_keyFrames.push_back(newKeyFrame);
    }
    m_listener->onCreateKeyFrame(newKeyFrame);
    return newKeyFrame;
}

void Map::deleteKeyFrame(const std::shared_ptr<KeyFrame> & keyFrame)
{
    {
        //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
        TMath_assert(keyFrame);
        TMath_assert(keyFrame->map() == this);
        TMath_assert(!keyFrame->isDeleted());
        keyFrame->_clearFeatures();
        std::size_t lastIndex = m_keyFrames.size() - 1;
        if (keyFrame->m_index < lastIndex) {
            std::shared_ptr<KeyFrame> lastKeyFrame = m_keyFrames[lastIndex];
            m_keyFrames[keyFrame->m_index] = lastKeyFrame;
            lastKeyFrame->m_index = keyFrame->m_index;
        }
        m_keyFrames.resize(lastIndex);
        keyFrame->m_index = std::numeric_limits<std::size_t>::max();
    }
    m_listener->onDeleteKeyFrame(keyFrame);
}

std::size_t Map::countKeyFrames() const
{
    //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
    return m_keyFrames.size();
}

std::shared_ptr<KeyFrame> Map::keyFrame(std::size_t index)
{
    //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
    return m_keyFrames[index];
}

const std::shared_ptr<const KeyFrame> Map::keyFrame(std::size_t index) const
{
    //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
    return m_keyFrames[index];
}

std::shared_ptr<KeyFrame> Map::getNearestKeyFrame(MapResourcesManager * manager,
                                                  const TMath::TVectord & position,
                                                  const TMath::TVectord & dir)
{
    TMath_assert(position.size() == 3);
    std::shared_ptr<KeyFrame> result;
    {
        //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
        double maxDistanceSquared = 0.0, d;
        for (auto it = m_keyFrames.begin(); it != m_keyFrames.end(); ++it) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            d = ((*it)->worldPosition() - position).lengthSquared();
            d *= std::min(0.0, TMath::dot((*it)->rotation().getColumn(2), dir));
            if (d > maxDistanceSquared) {
                maxDistanceSquared = d;
                result = *it;
            }
        }
    }
    return result;
}

std::shared_ptr<const KeyFrame> Map::getNearestKeyFrame(MapResourcesManager * manager,
                                          const TMath::TVectord & position,
                                          const TMath::TVectord & dir) const
{
    TMath_assert(position.size() == 3);
    std::shared_ptr<const KeyFrame> result;
    {
        //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
        double maxDistanceSquared = 0.0, d;
        for (auto it = m_keyFrames.cbegin(); it != m_keyFrames.cend(); ++it) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            d = ((*it)->worldPosition() - position).lengthSquared();
            d *= std::min(0.0, TMath::dot((*it)->rotation().getColumn(2), dir));
            if (d > maxDistanceSquared) {
                maxDistanceSquared = d;
                result = *it;
            }
        }
    }
    return result;
}

std::shared_ptr<KeyFrame> Map::getFurthestKeyFrame(MapResourcesManager * manager,
                                                   const TMath::TVectord & position)
{
    TMath_assert(position.size() == 3);
    std::shared_ptr<KeyFrame> result;
    {
        //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
        double maxDistanceSquared = 0.0, d;
        for (auto it = m_keyFrames.begin(); it != m_keyFrames.end(); ++it) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            d = ((*it)->worldPosition() - position).lengthSquared();
            if (d > maxDistanceSquared) {
                maxDistanceSquared = d;
                result = *it;
            }
        }
    }
    return result;
}

std::shared_ptr<const KeyFrame> Map::getFurthestKeyFrame(MapResourcesManager * manager,
                                                         const TMath::TVectord & position) const
{
    TMath_assert(position.size() == 3);
    std::shared_ptr<const KeyFrame> result;
    {
        //std::lock_guard<std::mutex> locker(m_mutex_keyFrames); (void)locker;
        double maxDistanceSquared = 0.0, d;
        for (auto it = m_keyFrames.cbegin(); it != m_keyFrames.cend(); ++it) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            d = ((*it)->worldPosition() - position).lengthSquared();
            if (d > maxDistanceSquared) {
                maxDistanceSquared = d;
                result = *it;
            }
        }
    }
    return result;
}

std::shared_ptr<Feature> Map::createFeature(const std::shared_ptr<KeyFrame> & keyFrame,
                                            const Point2f & positionOnFrame, int imageLevel,
                                            const std::shared_ptr<MapPoint> & mapPoint)
{
    std::shared_ptr<Feature> f(new Feature(keyFrame, positionOnFrame, imageLevel, mapPoint));
    f->m_indexInKeyFrame = (int)keyFrame->m_features.size();
    keyFrame->m_features.push_back(f);
    f->m_indexInMapPoint = (int)mapPoint->m_features.size();
    mapPoint->m_features.push_back(f);
    m_listener->onCreateFeature(f);
    return f;
}

void Map::deleteFeature(const std::shared_ptr<Feature> & feature)
{
    feature->_releaseInKeyFrame();
    feature->_releaseInMapPoint();
    m_listener->onDeleteFeature(feature);
}

int Map::countImageLevels() const
{
    //std::lock_guard<std::mutex> locker(m_mutex); (void)locker;
    return m_countImageLevels;
}

void Map::setCountImageLevels(MapResourcesManager * manager, int count)
{
    TMath_assert(count > 0);
    //std::lock_guard<std::mutex> locker(m_mutex); (void)locker;
    //std::lock_guard<std::mutex> lockerKeyFrames(m_mutex_keyFrames); (void)lockerKeyFrames;
    m_countImageLevels = count;
    for (auto it = m_keyFrames.begin(); it != m_keyFrames.end(); ++it) {
        std::shared_ptr<KeyFrame> k = *it;
        MapResourceLocker lockerR(manager, k.get()); (void)lockerR;
        if (m_countImageLevels <= k->countImageLevels()) {
            k->m_imagePyramid.resize(m_countImageLevels);
        } else {
            int prevSize = (int)k->m_imagePyramid.size();
            k->m_imagePyramid.resize(m_countImageLevels);
            for (int i = prevSize; i < m_countImageLevels; ++i) {
                k->m_imagePyramid[i] = Image<uchar>(k->m_imagePyramid[i - 1].size());
                ImageProcessing::halfSample(k->m_imagePyramid[i], k->m_imagePyramid[i - 1]);
            }
        }
    }
}

int Map::sizeOfSmallImage() const
{
    //std::lock_guard<std::mutex> locker(m_mutex); (void)locker;
    return m_casheSmallImageH.width();
}

void Map::setSizeOfSmallImage(MapResourcesManager * manager, int size)
{
    TMath_assert(size > 0);
    //std::lock_guard<std::mutex> lockerKeyFrames(m_mutex_keyFrames); (void)lockerKeyFrames;
    {
        //std::lock_guard<std::mutex> locker(m_mutex); (void)locker;
        m_casheSmallImageH = Image<uchar>(Point2i(size, size));
        m_casheSmallImageV = Image<uchar>(m_casheSmallImageH.size());
    }
    for (auto it = m_keyFrames.begin(); it != m_keyFrames.end(); ++it) {
        std::shared_ptr<KeyFrame> keyFrame = *it;
        MapResourceLocker lockerR(manager, keyFrame.get()); (void)lockerR;
        keyFrame->m_smallImage = getSmallImage(*keyFrame);
    }
}

Image<int> Map::getSmallImage(const Frame & frame) const
{
    //std::lock_guard<std::mutex> locker(m_mutex); (void)locker;
    int level = 0;
    Point2i levelSize = frame.imageLevel(level).size();
    int nextLevel = level + 1;
    Point2i nextLevelSize;
    while (nextLevel < frame.countImageLevels()) {
        nextLevelSize = frame.imageLevel(nextLevel).size();
        if ((nextLevelSize.x >= m_casheSmallImageH.width()) && (nextLevelSize.y >= m_casheSmallImageH.height())) {
            levelSize = nextLevelSize;
        } else {
            break;
        }
        ++nextLevel;
    }
    Painter::drawImage(m_casheSmallImageV, frame.imageLevel(level), Point2f(0.0f, 0.0f), levelSize.cast<float>());
    int sizeBlur = 3;
    ImageProcessing::gaussianBlurX(m_casheSmallImageH, m_casheSmallImageV, sizeBlur / 2, sizeBlur / 6.0f);
    ImageProcessing::gaussianBlurY(m_casheSmallImageV, m_casheSmallImageH, sizeBlur / 2, sizeBlur / 6.0f);
    const uchar* data = m_casheSmallImageH.data();
    int mean = 0, i, area = m_casheSmallImageH.area();
    for (i = 0; i < area; ++i)
        mean += data[i];
    mean /= area;
    Image<int> smallImage(m_casheSmallImageH.size());
    int * smallImageData = smallImage.data();
    for (i = 0; i < area; ++i)
        smallImageData[i] = data[i] - mean;
    return smallImage;
}

void Map::transform(MapResourcesManager * manager,
                    const TMath::TMatrixd & rotation,
                    const TMath::TVectord & translation)
{
    {
        //std::lock_guard<std::mutex> lockerKeyFrames(m_mutex_keyFrames); (void)lockerKeyFrames;
        //std::lock_guard<std::mutex> lockerMapPoints(m_mutex_mapPoints); (void)lockerMapPoints;
        for (std::vector<std::shared_ptr<MapPoint>>::iterator it = m_mapPoints.begin(); it != m_mapPoints.end(); ++it) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            (*it)->transform(rotation, translation);
        }
        TMath::TMatrixd invRotation = TMath::TTools::matrix3x3Inverted(rotation);
        TMath::TVectord invTranslation = - (invRotation * translation);
        for (std::vector<std::shared_ptr<KeyFrame>>::iterator it = m_keyFrames.begin(); it != m_keyFrames.end(); ++it) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            (*it)->transform(invRotation, invTranslation);
        }
    }
    m_listener->onTransformMap(rotation, translation);
}

void Map::scale(MapResourcesManager * manager, double scale)
{
    {
        std::lock_guard<std::mutex> lockerKeyFrames(m_mutex_keyFrames); (void)lockerKeyFrames;
        std::lock_guard<std::mutex> lockerMapPoints(m_mutex_mapPoints); (void)lockerMapPoints;
        for (std::vector<std::shared_ptr<MapPoint>>::iterator it = m_mapPoints.begin(); it != m_mapPoints.end(); ++it) {
            std::shared_ptr<MapPoint> mapPoint = *it;
            MapResourceLocker lockerR(manager, mapPoint.get()); (void)lockerR;
            mapPoint->setPosition(mapPoint->position() * scale);
        }
        for (std::vector<std::shared_ptr<KeyFrame>>::iterator it = m_keyFrames.begin(); it != m_keyFrames.end(); ++it) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            (*it)->setTranslation((*it)->translation() * scale);
        }
    }
    m_listener->onScaleMap(scale);
}

void Map::resetMap(MapResourcesManager * manager)
{
    {
        //std::lock_guard<std::mutex> lockerKeyFrames(m_mutex_keyFrames); (void)lockerKeyFrames;
        //std::lock_guard<std::mutex> lockerMapPoints(m_mutex_mapPoints); (void)lockerMapPoints;
        for (auto it = m_mapPoints.begin(); it != m_mapPoints.end(); ++it) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            (*it)->m_index = std::numeric_limits<std::size_t>::max();
        }
        m_mapPoints.clear();
        for (auto it = m_keyFrames.begin(); it != m_keyFrames.end(); ++it) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            (*it)->m_index = std::numeric_limits<std::size_t>::max();
        }
        m_keyFrames.clear();
    }
    m_listener->onResetMap();
}

void Map::deleteNullMapPoints(MapResourcesManager * manager)
{
    {
        //std::lock_guard<std::mutex> lockerMapPoints(m_mutex_mapPoints); (void)lockerMapPoints;
        for (auto it = m_mapPoints.begin(); it != m_mapPoints.end(); ) {
            MapResourceLocker lockerR(manager, it->get()); (void)lockerR;
            if ((*it)->countFeatures() == 0) {
                deleteMapPoint(*it);
                it = m_mapPoints.erase(it);
            } else {
                ++it;
            }
        }
    }
}

}
