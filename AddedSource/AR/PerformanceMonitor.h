#ifndef AR_PERFORMANCEMONITOR_H
#define AR_PERFORMANCEMONITOR_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>

namespace AR {

class PerformanceMonitor
{
public:
    struct Timer
    {
        std::string name;
        std::size_t duration;
    };

    PerformanceMonitor();

    std::size_t countUsedTimes() const;
    void setCountUsedTimes(const std::size_t& countUsedTimes);

    std::size_t countTimers() const;
    Timer timer(std::size_t index) const;

    std::chrono::milliseconds commonTime() const;

    void startTimer(const std::string& name);
    void endTimer(const std::string& name);

    void start();
    void end();

private:
    struct TimerInfo
    {
        std::vector<std::chrono::milliseconds> durations;
        std::chrono::milliseconds last_start;
        std::size_t currentIndex;

        std::chrono::milliseconds getAvgDuration() const
        {
            if (durations.empty())
                return std::chrono::milliseconds(0);
            return std::accumulate(durations.begin(), durations.end(), std::chrono::milliseconds(0)) /
                                    durations.size();
        }
    };

    std::map<std::string, TimerInfo> m_running_timers;
    std::size_t m_currentIndex;
    std::vector<Timer> m_timers;
    std::vector<std::chrono::milliseconds> m_commonTimes;
    std::pair<std::chrono::milliseconds, std::chrono::milliseconds> m_currentCommonTime;
    std::size_t m_countUsedTimes;
};

}

#endif // AR_PERFORMANCEMONITOR_H
