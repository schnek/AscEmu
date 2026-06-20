/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Debugging/StackTrace.hpp"
#include "Logging/Logger.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <sstream>
#include <string>
#include <string_view>

#ifdef _WIN32
    #if !defined(__cpp_lib_stacktrace) || (__cpp_lib_stacktrace < 202011L)
        #error "Windows builds require std::stacktrace support for AscEmu::Debugging::StackTrace."
    #endif

    #include <stacktrace>
#else
    #include <cxxabi.h>
    #include <dlfcn.h>
    #include <execinfo.h>
    #include <memory>
#endif

namespace
{
#ifndef _WIN32
    [[nodiscard]] std::string demangle(const char* symbolName)
    {
        if (symbolName == nullptr || *symbolName == '\0')
            return {};

        int status = 0;
        std::unique_ptr<char, decltype(&std::free)> demangled(
            abi::__cxa_demangle(symbolName, nullptr, nullptr, &status),
            &std::free);

        if (status != 0 || demangled == nullptr)
            return symbolName;

        return demangled.get();
    }
#endif
}

std::vector<AscEmu::Debugging::StackFrame> AscEmu::Debugging::StackTrace::capture(std::size_t skipFrames, std::size_t maxFrames)
{
    std::vector<StackFrame> frames;

#ifdef _WIN32
    const std::stacktrace trace = std::stacktrace::current(skipFrames + 1, maxFrames);
    frames.reserve(trace.size());

    for (const auto& entry : trace)
    {
        StackFrame frame{};
        frame.symbol = entry.description();
        frame.file = entry.source_file();
        frame.line = static_cast<std::uint32_t>(entry.source_line());
        frames.push_back(std::move(frame));
    }
#else
    std::vector<void*> addresses(maxFrames + skipFrames + 1, nullptr);
    const int addressCount = ::backtrace(addresses.data(), static_cast<int>(addresses.size()));

    if (addressCount <= 0)
        return frames;

    const std::size_t beginIndex = std::min<std::size_t>(skipFrames + 1, static_cast<std::size_t>(addressCount));
    frames.reserve(static_cast<std::size_t>(addressCount) - beginIndex);

    for (std::size_t index = beginIndex; index < static_cast<std::size_t>(addressCount); ++index)
    {
        StackFrame frame{};
        frame.address = reinterpret_cast<std::uintptr_t>(addresses[index]);

        Dl_info info{};
        if (::dladdr(addresses[index], &info) != 0)
        {
            if (info.dli_sname != nullptr)
                frame.symbol = demangle(info.dli_sname);

            if (info.dli_fname != nullptr)
                frame.module = info.dli_fname;
        }

        if (frame.symbol.empty())
        {
            char fallback[32]{};
            std::snprintf(fallback, sizeof(fallback), "0x%zx", frame.address);
            frame.symbol = fallback;
        }

        frames.push_back(std::move(frame));
    }
#endif

    return frames;
}

std::string AscEmu::Debugging::StackTrace::format(std::span<const StackFrame> frames)
{
    std::ostringstream out;

    for (std::size_t index = 0; index < frames.size(); ++index)
    {
        const StackFrame& frame = frames[index];
        out << '#' << index << ' ';

        if (!frame.symbol.empty())
            out << frame.symbol;
        else
            out << "(unknown)";

        if (!frame.file.empty())
        {
            out << " [" << frame.file;
            if (frame.line != 0)
                out << ':' << frame.line;

            out << ']';
        }
        else if (!frame.module.empty())
        {
            out << " [" << frame.module << ']';
        }

        if (frame.address != 0)
            out << " @ 0x" << std::hex << frame.address << std::dec;

        out << '\n';
    }

    return out.str();
}

void AscEmu::Debugging::StackTrace::logCurrent(std::string_view header, std::size_t skipFrames, std::size_t maxFrames)
{
    if (!header.empty())
        sLogger.failure("{}", header);

    const auto frames = capture(skipFrames + 1, maxFrames);
    const std::string text = format(frames);

    if (text.empty())
    {
        sLogger.failure("No stack trace available.");
        return;
    }

    std::size_t lineStart = 0;
    while (lineStart < text.size())
    {
        const std::size_t lineEnd = text.find('\n', lineStart);
        const std::size_t count = (lineEnd == std::string::npos) ? (text.size() - lineStart) : (lineEnd - lineStart);

        if (count != 0)
            sLogger.failure("{}", std::string_view(text.data() + lineStart, count));

        if (lineEnd == std::string::npos)
            break;

        lineStart = lineEnd + 1;
    }
}
