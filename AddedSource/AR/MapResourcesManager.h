#ifndef AR_MAPRESOURCEMANAGER_H
#define AR_MAPRESOURCEMANAGER_H

#include <vector>
#include <utility>
#include <thread>
#include <condition_variable>
#include <mutex>

namespace AR {

class MapResourceObject;
class MapResourceLocker;

class MapResourcesManager
{
public:
    void lock(const MapResourceObject * object);
    void unlock(const MapResourceObject * object);

private:
    friend class MapResourceLocker;

    //mutable std::mutex m_mutex;
    //std::vector<const MapResourceObject*> m_used_objects;
};

} // namespace AR

#endif // AR_MAPRESOURCEMANAGER_H
