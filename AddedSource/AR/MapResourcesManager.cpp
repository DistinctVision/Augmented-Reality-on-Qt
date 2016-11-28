#include "MapResourcesManager.h"
#include <cassert>
#include "TMath/TMath.h"
#include "MapResourceObject.h"

namespace AR {

void MapResourcesManager::lock(const MapResourceObject * object)
{
    (void)object;
    /*using namespace std;
    if (!object->m_mutex.try_lock()) {
        for (auto it = m_used_objects.begin(); it != m_used_objects.end(); ++it) {
            (*it)->m_mutex.unlock();
        }
        for (;;) {
            object->m_mutex.lock();
            for (auto it = m_used_objects.begin(); it != m_used_objects.end(); ++it) {
                if (!(*it)->m_mutex.try_lock()) {
                    for (auto it2 = m_used_objects.begin(); it2 != it; ++it2) {
                        (*it2)->m_mutex.unlock();
                    }
                    goto nextTry;
                }
            }
            break;
            nextTry:
            object->m_mutex.unlock();
        }
    }
    m_used_objects.push_back(object);*/
}

void MapResourcesManager::unlock(const MapResourceObject * object)
{
    (void)object;
    /*for (auto it = m_used_objects.begin(); it != m_used_objects.end(); ++it) {
        if (*it == object) {
            m_used_objects.erase(it);
            object->m_mutex.unlock();
            return;
        }
    }
    assert(false);*/
}

} // namespace AR
