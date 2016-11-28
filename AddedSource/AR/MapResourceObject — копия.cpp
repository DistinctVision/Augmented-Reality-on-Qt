#include "MapResourceObject.h"
#include "MapResourcesManager.h"
#include "Map.h"
#include "KeyFramePtr.h"
#include "MapPointPtr.h"

namespace AR {

MapResourceObject::MapResourceObject(Map * map)
{
    TMath_assert(map != nullptr);
    m_map = map;
    m_usedCounter = 0;
    m_usedResourcesManager = nullptr;
}

const Map * MapResourceObject::map() const
{
    return m_map;
}

Map * MapResourceObject::map()
{
    return m_map;
}

void MapResourceObject::_lock(MapResourcesManager * manager)
{
    manager->lock(this);
}

void MapResourceObject::_unlock(MapResourcesManager * manager)
{
    manager->unlock(this);
}

} // namespace AR
