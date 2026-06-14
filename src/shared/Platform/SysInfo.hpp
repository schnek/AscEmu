/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace AscEmu::Platform
{
    class SysInfo final
    {
    public:
        [[nodiscard]] static std::uint32_t cpuCount() noexcept;
        [[nodiscard]] static std::uint64_t processCpuTime() noexcept;
        [[nodiscard]] static std::uint64_t processMemoryUsage() noexcept;
        [[nodiscard]] static std::uint64_t tickCount() noexcept;
    };
}
