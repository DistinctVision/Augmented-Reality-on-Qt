#ifndef AR_MAPRESOURCEOBJECT_H
#define AR_MAPRESOURCEOBJECT_H

#include <mutex>

namespace AR {

class Map;
class MapResourcesManager;
class MapResourceLocker;

class MapResourceObject
{
public:
    MapResourceObject(Map * map);

    const Map * map() const;
    Map * map();

protected:
    Map * m_map;

private:
    friend class MapResourcesManager;
    friend class MapResourceLocker;

    //mutable std::recursive_mutex m_mutex;
};

} // namespace AR

#endif // AR_MAPRESOURCE_H
