/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string_view>

namespace AscEmu::Debugging
{
    enum class ProcessKind
    {
        world,
        logon
    };

    struct CrashHandlerOptions
    {
        ProcessKind processKind = ProcessKind::world;
        bool writeMiniDump = true;
        bool installTerminateHandler = true;
        const char* crashDumpDirectory = "CrashDumps";
        const char* logDirectory = "logs";
    };

    void installCrashHandler(const CrashHandlerOptions& options = {});
    [[nodiscard]] bool isCrashHandlerInstalled() noexcept;

    void requestCrashDiagnostics(std::string_view reason = {});
    [[noreturn]] void terminateWithDiagnostics(std::string_view reason = {});
}

