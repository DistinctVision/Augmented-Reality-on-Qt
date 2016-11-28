#ifndef AR_MAPRESOURCEMANAGER_H
#define AR_MAPRESOURCEMANAGER_H

#include <vector>
#include <utility>
#include <thread>
#include <condition_variable>
#include <mutex>

namespace AR {

class MapResourceObject;

class MapResourcesManager
{
public:
    void lock(MapResourceObject * object);
    void unlock(MapResourceObject * object);

private:
    std::mutex m_mutex;
    std::vector<std::pair<MapResourceObject*, int>> m_used_objects;

};

} // namespace AR

#endif // AR_MAPRESOURCEMANAGER_H
