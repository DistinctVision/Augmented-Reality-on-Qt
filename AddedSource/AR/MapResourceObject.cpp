#include "MapResourceObject.h"
#include "MapResourcesManager.h"
#include "Map.h"

namespace AR {

MapResourceObject::MapResourceObject(Map * map)
{
    TMath_assert(map != nullptr);
    m_map = map;
}

const Map * MapResourceObject::map() const
{
    return m_map;
}

Map * MapResourceObject::map()
{
    return m_map;
}

} // namespace AR
