#include "MapResourceLocker.h"

namespace AR {

MapResourceLocker::MapResourceLocker(MapResourcesManager * manager, const MapResourceObject * object)
{
    (void)manager;
    (void)object;
    /*m_manager = manager;
    m_object = object;
    m_manager->lock(m_object);*/
}

MapResourceLocker::~MapResourceLocker()
{
    //m_manager->unlock(m_object);
}

} // namespace AR
