#ifndef MAPRESOURCELOCKER_H
#define MAPRESOURCELOCKER_H

#include "MapResourceObject.h"
#include "MapResourcesManager.h"

namespace AR {

class MapResourceLocker
{
public:
    MapResourceLocker(MapResourcesManager * manager, const MapResourceObject * object);
    ~MapResourceLocker();

private:
    //MapResourcesManager * m_manager;
    //const MapResourceObject * m_object;
};

} // AR

#endif // MAPRESOURCELOCKER_H
