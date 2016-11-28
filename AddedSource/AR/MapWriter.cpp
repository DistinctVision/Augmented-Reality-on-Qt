#include "MapWriter.h"
#include "Map.h"
#include "Feature.h"
#include "Frame.h"
#include "KeyFrame.h"
#include "MapPoint.h"
#include "Camera.h"

namespace AR {

MapWriter::MapWriter(Map* map)
{
    TMath_assert(map != nullptr);
    m_map = map;
    //volatile int countTries = 0;
    m_map->m_mutex.lock();
    /*while ((m_map->m_mapWriter != nullptr) && (!m_map->m_mapReaders.empty())) {
        m_map->m_mutex.unlock();
        ++countTries;
        m_map->m_mutex.lock();
    }
    m_map->m_mapWriter = this;
    m_map->m_mutex.unlock();*/
}

MapWriter::~MapWriter()
{
    /*m_map->m_mutex.lock();
    assert(m_map->m_mapWriter == this);
    m_map->m_mapWriter = nullptr;*/
    m_map->m_mutex.unlock();
}

Map* MapWriter::map()
{
    return m_map;
}

std::size_t MapWriter::countMapPoints() const
{
    TMath_assert(m_map != nullptr);
    return m_map->_countMapPoints();
}

std::shared_ptr<MapPoint> MapWriter::mapPoint(std::size_t index)
{
    TMath_assert(m_map != nullptr);
    return m_map->_mapPoint(index);
}

std::size_t MapWriter::countKeyFrames() const
{
    TMath_assert(m_map != nullptr);
    return m_map->_countKeyFrames();
}

std::shared_ptr<KeyFrame> MapWriter::keyFrame(size_t index)
{
    TMath_assert(m_map != nullptr);
    return m_map->_keyFrame(index);
}

std::shared_ptr<MapPoint> MapWriter::createMapPoint(const TMath::TVectord& position)
{
    TMath_assert(m_map != nullptr);
    return m_map->_createMapPoint(position);
}

void MapWriter::deleteMapPoint(const std::shared_ptr<MapPoint>& mapPoint)
{
    TMath_assert(m_map != nullptr);
    m_map->_deleteMapPoint(mapPoint);
}

std::shared_ptr<KeyFrame> MapWriter::createKeyFrame(const std::shared_ptr<const Camera>& camera,
                                                    const std::vector<Image<uchar>>& imagePyramid)
{
    TMath_assert(m_map != nullptr);
    return m_map->_createKeyFrame(camera, imagePyramid);
}

std::shared_ptr<KeyFrame> MapWriter::createKeyFrame(const Frame& frame)
{
    TMath_assert(m_map != nullptr);
    return m_map->_createKeyFrame(frame);
}

void MapWriter::deleteKeyFrame(const std::shared_ptr<KeyFrame>& keyFrame)
{
    TMath_assert(m_map != nullptr);
    m_map->_deleteKeyFrame(keyFrame);
}

std::shared_ptr<KeyFrame> MapWriter::getNearestKeyFrame(const TMath::TVectord& position, const TMath::TVectord& dir)
{
    TMath_assert(m_map != nullptr);
    return m_map->_getNearestKeyFrame(position, dir);
}

std::shared_ptr<KeyFrame> MapWriter::getFurthestKeyFrame(const TMath::TVectord& position)
{
    TMath_assert(m_map != nullptr);
    return m_map->_getFurthestKeyFrame(position);
}

std::shared_ptr<Feature> MapWriter::createFeature(const std::shared_ptr<KeyFrame>& keyFrame,
                                                  const Point2f& positionOnFrame, int imageLevel,
                                                  const std::shared_ptr<MapPoint>& mapPoint)
{
    TMath_assert(m_map != nullptr);
    return m_map->_createFeature(keyFrame, positionOnFrame, imageLevel, mapPoint);
}

void MapWriter::deleteFeature(const std::shared_ptr<Feature>& feature)
{
    TMath_assert(m_map != nullptr);
    m_map->_deleteFeature(feature);
}

std::shared_ptr<MapPoint> MapWriter::accessWrite(const std::shared_ptr<const MapPoint>& mapPoint)
{
    TMath_assert(mapPoint->map() == m_map);
    if (mapPoint->isDeleted())
        return std::shared_ptr<MapPoint>(nullptr);
    return std::const_pointer_cast<MapPoint>(mapPoint);
}

std::shared_ptr<KeyFrame> MapWriter::accessWrite(const std::shared_ptr<const KeyFrame>& keyFrame)
{
    TMath_assert(keyFrame->map() == m_map);
    if (keyFrame->isDeleted())
        return std::shared_ptr<KeyFrame>(nullptr);
    return std::const_pointer_cast<KeyFrame>(keyFrame);
}

std::shared_ptr<Feature> MapWriter::accessWrite(const std::shared_ptr<const Feature>& feature)
{
    TMath_assert(m_map != nullptr);
    if (feature->isDeleted())
        return std::shared_ptr<Feature>(nullptr);
    TMath_assert(feature->mapPoint().lock()->map() == m_map);
    TMath_assert(feature->keyFrame().lock()->map() == m_map);
    return std::const_pointer_cast<Feature>(feature);
}

int MapWriter::countImageLevels() const
{
    TMath_assert(m_map != nullptr);
    return m_map->_countImageLevels();
}

void MapWriter::setCountImageLevels(int count)
{
    TMath_assert(m_map != nullptr);
    m_map->_setCountImageLevels(count);
}

int MapWriter::sizeOfSmallImage() const
{
    TMath_assert(m_map != nullptr);
    return m_map->_sizeOfSmallImage();
}

void MapWriter::setSizeOfSmallImage(int size)
{
    TMath_assert(m_map != nullptr);
    m_map->_setSizeOfSmallImage(size);
}

void MapWriter::resetMap()
{
    TMath_assert(m_map != nullptr);
    m_map->_resetMap();
}

void MapWriter::transform(const TMath::TMatrixd& rotation, const TMath::TVectord& translation)
{
    TMath_assert(m_map != nullptr);
    m_map->_transform(rotation, translation);
}

void MapWriter::scale(double scale)
{
    TMath_assert(m_map != nullptr);
    m_map->_scale(scale);
}

void MapWriter::deleteNullMapPoints()
{
    TMath_assert(m_map != nullptr);
    m_map->_deleteNullMapPoints();
}

Image<int> MapWriter::getSmallImage(const Frame &frame) const
{
    TMath_assert(m_map != nullptr);
    return m_map->_getSmallImage(frame);
}

}
