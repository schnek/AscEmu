/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace AscEmu::Debugging
{
    struct StackFrame
    {
        std::uintptr_t address = 0;
        std::string symbol;
        std::string module;
        std::string file;
        std::uint32_t line = 0;
    };

    class StackTrace
    {
    public:
        [[nodiscard]] static std::vector<StackFrame> capture(std::size_t skipFrames = 0, std::size_t maxFrames = 64);
        [[nodiscard]] static std::string format(std::span<const StackFrame> frames);
        static void logCurrent(std::string_view header = {}, std::size_t skipFrames = 0, std::size_t maxFrames = 64);
    };
}
