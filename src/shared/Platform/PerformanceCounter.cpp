/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "PerformanceCounter.hpp"
#include "SysInfo.hpp"

#include <algorithm>

namespace AscEmu::Platform
{
    PerformanceCounter::PerformanceCounter() noexcept : m_cpuCount(std::max<std::uint32_t>(1u, SysInfo::cpuCount())),
        m_lastUpdate(SysInfo::tickCount()), m_lastCpuTime(SysInfo::processCpuTime())
    {
    }

    float PerformanceCounter::currentCpuUsage() noexcept
    {
        const auto now = SysInfo::tickCount();
        const auto nowCpuTime = SysInfo::processCpuTime();

        const auto elapsedMs = now - m_lastUpdate;
        const auto cpuTimeUs = nowCpuTime - m_lastCpuTime;

        m_lastUpdate = now;
        m_lastCpuTime = nowCpuTime;

        if (elapsedMs == 0 || m_cpuCount == 0)
            return 0.0f;

        const double elapsedUs = static_cast<double>(elapsedMs) * 1000.0;
        const double usage = (static_cast<double>(cpuTimeUs) / elapsedUs) * 100.0 / static_cast<double>(m_cpuCount);

        return static_cast<float>(std::clamp(usage, 0.0, 100.0));
    }

    float PerformanceCounter::currentRamUsage() const noexcept
    {
        return static_cast<float>(static_cast<double>(SysInfo::processMemoryUsage()) / (1024.0 * 1024.0));
    }
}