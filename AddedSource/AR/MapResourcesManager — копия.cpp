#include "MapResourcesManager.h"
#include <cassert>
#include "TMath/TMath.h"
#include "MapResourceObject.h"

namespace AR {

void MapResourcesManager::lock(MapResourceObject * object)
{
    using namespace std;
    unique_lock<mutex> locker(m_mutex); (void)locker;
    unique_lock<mutex> lockerObject(object->m_mutex); (void)lockerObject;
    if (object->m_usedCounter > 0) {
        if (object->m_usedResourcesManager == this) {
            for (auto it = m_used_objects.begin(); it != m_used_objects.end(); ++it) {
                if (it->first == object) {
                    it->second++;
                    object->m_usedCounter++;
                    break;
                }
            }
        } else {
            for (auto it = m_used_objects.begin(); it != m_used_objects.end(); ++it) {
                it->first->m_mutex.lock();
                it->first->m_usedCounter = 0;
                it->first->m_mutex.unlock();
                it->first->m_notifer.notify_one();
            }
            MapResourceObject * lockObject = object;
            for (;;) {
                lockerObject.unlock();
                lockObject->m_notifer.wait(locker);
                lockerObject.lock();
                if (object->m_usedCounter > 0) {
                    lockObject = object;
                    continue;
                }
                lockObject = nullptr;
                for (auto it = m_used_objects.begin(); it != m_used_objects.end(); ++it) {
                    it->first->m_mutex.lock();
                    if (it->first->m_usedCounter > 0) {
                        for (auto itB = m_used_objects.begin(); itB != it; ++itB) {
                            itB->first->m_mutex.lock();
                            itB->first->m_usedCounter = 0;
                            itB->first->m_mutex.unlock();
                            itB->first->m_notifer.notify_one();
                        }
                        lockObject = it->first;
                        it->first->m_mutex.unlock();
                        break;
                    } else {
                        it->first->m_usedResourcesManager = this;
                        it->first->m_usedCounter = it->second;
                        it->first->m_mutex.unlock();
                    }
                }
                if (lockObject == nullptr)
                    break;
            }
            object->m_usedResourcesManager = this;
            object->m_usedCounter = 1;
            m_used_objects.push_back({ object, 1 });
        }
    } else {
        object->m_usedResourcesManager = this;
        object->m_usedCounter = 1;
        m_used_objects.push_back({ object, 1 });
    }
}

void MapResourcesManager::unlock(MapResourceObject * object)
{
    std::lock_guard<std::mutex> locker(m_mutex); (void)locker;
    for (auto it = m_used_objects.begin(); it != m_used_objects.end(); ++it) {
        if (it->first == object) {
            object->m_mutex.lock();
            object->m_usedCounter--;
            object->m_mutex.unlock();
            it->second--;
            if (it->second <= 0) {
                m_used_objects.erase(it);
                object->m_notifer.notify_one();
            }
            return;
        }
    }
    assert(false);
}

} // namespace AR
