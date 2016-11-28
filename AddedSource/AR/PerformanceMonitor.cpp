#include "PerformanceMonitor.h"
#include <algorithm>
#include <numeric>
#include <cassert>
#include <climits>
#include <limits>

namespace AR {

PerformanceMonitor::PerformanceMonitor()
{
    m_countUsedTimes = 10;
    m_currentCommonTime.first = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
    m_currentCommonTime.second = m_currentCommonTime.first;
}

std::size_t PerformanceMonitor::countUsedTimes() const
{
    return m_countUsedTimes;
}

void PerformanceMonitor::setCountUsedTimes(const std::size_t& countUsedTimes)
{
    m_countUsedTimes = countUsedTimes;
}

std::size_t PerformanceMonitor::countTimers() const
{
    return m_timers.size();
}

PerformanceMonitor::Timer PerformanceMonitor::timer(std::size_t index) const
{
    return m_timers[index];
}

std::chrono::milliseconds PerformanceMonitor::commonTime() const
{
    if (m_commonTimes.empty())
        return m_currentCommonTime.second - m_currentCommonTime.first;
    return std::accumulate(m_commonTimes.begin(), m_commonTimes.end(), std::chrono::milliseconds(0)) /
                            m_commonTimes.size();
}

void PerformanceMonitor::startTimer(const std::string& name)
{
    auto it = m_running_timers.find(name);
    if (it == m_running_timers.end()) {
        m_running_timers.insert(std::pair<std::string, TimerInfo>(
                                    name,
                                    { std::vector<std::chrono::milliseconds>(),
                                      std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch()),
                                      m_currentIndex }));
    } else {
        it->second.last_start = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch());
        it->second.currentIndex = m_currentIndex;
    }
    ++m_currentIndex;
}

void PerformanceMonitor::endTimer(const std::string& name)
{
    auto it = m_running_timers.find(name);
    if (it != m_running_timers.end()) {
        if (it->second.durations.size() > m_countUsedTimes) {
            it->second.durations.erase(it->second.durations.begin(),
                                       it->second.durations.begin() +
                                         ((it->second.durations.size() + 1) - m_countUsedTimes));
        }
        it->second.durations.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::system_clock::now().time_since_epoch()) - it->second.last_start);
    } else {
        assert(false);
    }
}

void PerformanceMonitor::start()
{
    m_currentIndex = 0;
    m_timers.resize(0);
    for (auto it = m_running_timers.begin(); it != m_running_timers.end(); ++it) {
        it->second.currentIndex = std::numeric_limits<std::size_t>::max();
    }
    m_currentCommonTime.first = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
}

void PerformanceMonitor::end()
{
    m_currentCommonTime.second = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
    if (m_commonTimes.size() >= m_countUsedTimes) {
        m_commonTimes.erase(m_commonTimes.begin(), m_commonTimes.begin() + ((m_commonTimes.size() + 1) - m_countUsedTimes));
    }
    m_commonTimes.push_back(m_currentCommonTime.second - m_currentCommonTime.first);
    std::vector<Timer> timers;
    std::vector<std::pair<std::size_t, std::size_t>> indices;
    for (auto it = m_running_timers.begin(); it != m_running_timers.end(); ) {
        if (it->second.currentIndex == std::numeric_limits<std::size_t>::max()) {
            it = m_running_timers.erase(it);
        } else {
            indices.push_back({ timers.size(), it->second.currentIndex });
            timers.push_back({ it->first, (std::size_t)it->second.getAvgDuration().count() });
            ++it;
        }
    }
    std::sort(indices.begin(), indices.end(), [] (const std::pair<std::size_t, std::size_t>& a,
                                                  const std::pair<std::size_t, std::size_t>& b) {
        return (a.second < b.second);
    });
    m_timers.resize(timers.size());
    for (std::size_t i = 0; i < timers.size(); ++i) {
        m_timers[i] = timers[indices[i].first];
    }
}

}
