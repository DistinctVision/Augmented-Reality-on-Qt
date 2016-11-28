#ifndef AR_MAPRESOURCEOBJECT_H
#define AR_MAPRESOURCEOBJECT_H

#include <mutex>
#include <condition_variable>

namespace AR {

class Map;
class MapResourcesManager;
class KeyFramePtr;
class MapPointPtr;

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
    friend class KeyFramePtr;
    friend class MapPointPtr;

    std::condition_variable m_notifer;
    std::mutex m_mutex;
    MapResourcesManager * m_usedResourcesManager;
    int m_usedCounter;

    void _lock(MapResourcesManager * manager);
    void _unlock(MapResourcesManager * manager);
};

} // namespace AR

#endif // AR_MAPRESOURCE_H
