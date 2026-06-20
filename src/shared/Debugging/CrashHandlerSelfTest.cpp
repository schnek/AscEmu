/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Debugging/CrashHandler.hpp"
#include "Debugging/StackTrace.hpp"

#include <cstdio>
#include <string>
#include <vector>

#if defined(_MSC_VER)
    #define AE_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
    #define AE_NOINLINE __attribute__((noinline))
#else
    #define AE_NOINLINE
#endif

namespace
{
    AE_NOINLINE std::vector<AscEmu::Debugging::StackFrame> traceLeaf()
    {
        return AscEmu::Debugging::StackTrace::capture(0, 32);
    }

    AE_NOINLINE std::vector<AscEmu::Debugging::StackFrame> traceMiddle()
    {
        return traceLeaf();
    }

    AE_NOINLINE std::vector<AscEmu::Debugging::StackFrame> traceRoot()
    {
        return traceMiddle();
    }
}

bool runCrashHandlerSelfTest()
{
    AscEmu::Debugging::CrashHandlerOptions options{};
    options.processKind = AscEmu::Debugging::ProcessKind::world;
    AscEmu::Debugging::installCrashHandler(options);

    const auto frames = traceRoot();
    if (frames.empty())
    {
        std::fprintf(stderr, "CrashHandler self-test failed: no frames captured.\n");
        return false;
    }

    const std::string formatted = AscEmu::Debugging::StackTrace::format(frames);
    if (formatted.empty())
    {
        std::fprintf(stderr, "CrashHandler self-test failed: formatted stack trace is empty.\n");
        return false;
    }

    std::fprintf(stderr, "CrashHandler self-test passed with %zu frames.\n", frames.size());
    return true;
}

[[noreturn]] void runCrashHandlerManualCrashTest()
{
    AscEmu::Debugging::CrashHandlerOptions options{};
    options.processKind = AscEmu::Debugging::ProcessKind::world;
    AscEmu::Debugging::installCrashHandler(options);

    volatile int* invalidPointer = nullptr;
    *invalidPointer = 42;

    std::abort();
}

#ifdef ASCEMU_CRASH_HANDLER_SELFTEST_STANDALONE
int main(int argc, char** argv)
{
    if (argc > 1 && std::string(argv[1]) == "--crash")
        runCrashHandlerManualCrashTest();

    return runCrashHandlerSelfTest() ? 0 : 1;
}
#endif
