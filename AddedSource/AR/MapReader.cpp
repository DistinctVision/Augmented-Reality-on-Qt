#include "MapReader.h"
#include "Map.h"
#include "KeyFrame.h"
#include "MapPoint.h"
#include "Feature.h"
#include <iostream>

namespace AR {

MapReader::MapReader(const Map* map)
{
    TMath_assert(map != nullptr);
    m_map = const_cast<Map*>(map);
    //volatile int countTries = 0;
    m_map->m_mutex.lock();
    /*while (m_map->m_mapWriter != nullptr) {
        m_map->m_mutex.unlock();
        ++countTries;
        m_map->m_mutex.lock();
    }
    m_map->m_mapReaders.push_back(this);
    m_map->m_mutex.unlock();*/
}

MapReader::~MapReader()
{
    /*m_map->m_mutex.lock();
    for (auto it = m_map->m_mapReaders.begin(); it != m_map->m_mapReaders.end(); ++it) {
        if (*it == this) {
            m_map->m_mapReaders.erase(it);
            m_map->m_mutex.unlock();
            return;
        }
    }
    assert(false);
    std::cout << "Not founded map reader";*/
    m_map->m_mutex.unlock();
}

const Map* MapReader::map() const
{
    return m_map;
}

std::size_t MapReader::countMapPoints() const
{
    TMath_assert(m_map != nullptr);
    return m_map->_countMapPoints();
}

std::shared_ptr<const MapPoint> MapReader::mapPoint(std::size_t index) const
{
    TMath_assert(m_map != nullptr);
    return m_map->_mapPoint(index);
}

std::size_t MapReader::countKeyFrames() const
{
    TMath_assert(m_map != nullptr);
    return m_map->_countKeyFrames();
}

std::shared_ptr<const KeyFrame> MapReader::keyFrame(std::size_t index) const
{
    TMath_assert(m_map != nullptr);
    return m_map->_keyFrame(index);
}

std::shared_ptr<const KeyFrame> MapReader::getNearestKeyFrame(const TMath::TVectord& position, const TMath::TVectord& dir) const
{
    TMath_assert(m_map != nullptr);
    return m_map->_getNearestKeyFrame(position, dir);
}

std::shared_ptr<const KeyFrame> MapReader::getFurthestKeyFrame(const TMath::TVectord& position) const
{
    TMath_assert(m_map != nullptr);
    return m_map->_getFurthestKeyFrame(position);
}

int MapReader::countImageLevels() const
{
    return m_map->_countImageLevels();
}

int MapReader::sizeOfSmallImage() const
{
    return m_map->_sizeOfSmallImage();
}

Image<int> MapReader::getSmallImage(const Frame& frame) const
{
    return m_map->_getSmallImage(frame);
}

}
