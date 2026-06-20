/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Server/Master.hpp"

#include "Debugging/CrashHandler.hpp"
#include "Debugging/StackTrace.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#ifndef _WIN32
#include <sys/resource.h>
#endif

namespace
{
    void installLogonCrashHandler()
    {
        AscEmu::Debugging::CrashHandlerOptions crashHandlerOptions{};
        crashHandlerOptions.processKind = AscEmu::Debugging::ProcessKind::logon;
        AscEmu::Debugging::installCrashHandler(crashHandlerOptions);
    }

    int runLogonCrashTestStack()
    {
        const auto frames = AscEmu::Debugging::StackTrace::capture(0, 32);
        if (frames.empty())
        {
            std::fprintf(stderr, "[logon] crash test failed: no frames captured\n");
            return EXIT_FAILURE;
        }

        const std::string formatted = AscEmu::Debugging::StackTrace::format(frames);
        if (formatted.empty())
        {
            std::fprintf(stderr, "[logon] crash test failed: formatted stack trace is empty\n");
            return EXIT_FAILURE;
        }

        std::fprintf(stderr, "[logon] stack trace smoke test passed\n%s\n", formatted.c_str());
        AscEmu::Debugging::requestCrashDiagnostics("logon manual stack smoke test");
        return EXIT_SUCCESS;
    }

    [[noreturn]] void runLogonCrashTestTerminate()
    {
        AscEmu::Debugging::terminateWithDiagnostics("logon manual terminate test");
    }

    [[noreturn]] void runLogonCrashTestSegv()
    {
        volatile int* invalidPointer = nullptr;
        *invalidPointer = 42;
        std::abort();
    }

    int tryRunLogonCrashTests(std::string test)
    {
        if (test == "--crash-test-stack")
            return runLogonCrashTestStack();

        if (test == "--crash-test-terminate")
            runLogonCrashTestTerminate();

        if (test == "--crash-test-segv")
            runLogonCrashTestSegv();

        return -1;
    }
}

int main(int argc, char** argv)
{
    installLogonCrashHandler();

//#define ASCEMU_ENABLE_CRASH_TESTS 1
#ifdef ASCEMU_ENABLE_CRASH_TESTS
    std::string test = "--crash-test-stack";
    if (const int crashTestResult = tryRunLogonCrashTests(test); crashTestResult >= 0)
    {
        fmt::println("CrashHandler {} ended {}", test, crashTestResult == 0 ? "success" : "failed");
    }
#endif

#ifndef _WIN32
    rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1)
        fmt::println("getrlimit failed. This could be problem.");
    else
    {
        rl.rlim_cur = rl.rlim_max;
        if (setrlimit(RLIMIT_CORE, &rl) == -1)
            fmt::println("setrlimit failed. Server may not save core.dump files.");
    }
#endif

    sMasterLogon.Run(argc, argv);
}
