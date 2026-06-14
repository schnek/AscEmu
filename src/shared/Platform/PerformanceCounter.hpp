/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace AscEmu::Platform
{
    class PerformanceCounter final
    {
    public:
        PerformanceCounter() noexcept;
        ~PerformanceCounter() = default;

        [[nodiscard]] float currentCpuUsage() noexcept;
        [[nodiscard]] float currentRamUsage() const noexcept;

    private:
        std::uint32_t m_cpuCount = 1;
        std::uint64_t m_lastUpdate = 0;   // milliseconds
        std::uint64_t m_lastCpuTime = 0;  // microseconds
    };
}