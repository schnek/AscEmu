/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Debugging/CrashHandler.hpp"
#include "BuildInfo.hpp"
#include "Debugging/StackTrace.hpp"
#include "Logging/Logger.hpp"

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <DbgHelp.h>
#include <Windows.h>
#else
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#endif

namespace
{
    using AscEmu::Debugging::CrashHandlerOptions;
    using AscEmu::Debugging::ProcessKind;

    std::atomic<bool> g_isInstalled{ false };

#ifdef _WIN32
    std::atomic<bool> g_isHandlingCrash{ false };
    std::mutex g_windowsCrashMutex;
#else
    volatile std::sig_atomic_t g_isHandlingSignalCrash = 0;
    stack_t g_alternateStack{};
    alignas(std::max_align_t) std::byte g_signalStack[64 * 1024];
#endif

    CrashHandlerOptions g_options{};
    char g_processName[16] = "world";
    char g_buildHash[128] = "unknown";
    char g_crashDumpDirectory[256] = "CrashDumps";
    char g_logDirectory[256] = "logs";

    void copyCString(char* destination, std::size_t destinationSize, const char* source)
    {
        if (destination == nullptr || destinationSize == 0)
            return;

        if (source == nullptr)
        {
            destination[0] = '\0';
            return;
        }

        const std::size_t sourceLength = std::strlen(source);
        const std::size_t copyLength = sourceLength < (destinationSize - 1) ? sourceLength : (destinationSize - 1);

        std::memcpy(destination, source, copyLength);
        destination[copyLength] = '\0';
    }

    [[nodiscard]] const char* processKindToString(ProcessKind processKind) noexcept
    {
        return processKind == ProcessKind::logon ? "logon" : "world";
    }

    void refreshStaticOptions(const CrashHandlerOptions& options)
    {
        g_options = options;
        copyCString(g_processName, sizeof(g_processName), processKindToString(options.processKind));
        copyCString(g_crashDumpDirectory, sizeof(g_crashDumpDirectory), options.crashDumpDirectory);
        copyCString(g_logDirectory, sizeof(g_logDirectory), options.logDirectory);
        copyCString(g_buildHash, sizeof(g_buildHash), BuildInfo::hash.data());
    }

    void ensureOutputDirectoriesExist()
    {
        std::error_code errorCode;
        std::filesystem::create_directories(g_crashDumpDirectory, errorCode);
        std::filesystem::create_directories(g_logDirectory, errorCode);
    }

    [[nodiscard]] std::string crashLogPath()
    {
        return std::string(g_logDirectory) + "/CrashLog.txt";
    }

    void appendCrashLogLine(std::string_view line)
    {
        std::ofstream stream(crashLogPath(), std::ios::out | std::ios::app);
        if (!stream.is_open())
            return;

        stream.write(line.data(), static_cast<std::streamsize>(line.size()));
    }

    void appendCrashLogText(std::string_view text)
    {
        appendCrashLogLine(text);
        if (text.empty() || text.back() != '\n')
            appendCrashLogLine("\n");
    }

    void logStackTraceToLoggerAndFile(std::string_view reason)
    {
        if (!reason.empty())
        {
            sLogger.failure("{}", reason);
            appendCrashLogText(reason);
        }

        const auto frames = AscEmu::Debugging::StackTrace::capture(1, 64);
        const std::string text = AscEmu::Debugging::StackTrace::format(frames);

        if (text.empty())
        {
            sLogger.failure("No stack trace available.");
            appendCrashLogText("No stack trace available.");
            return;
        }

        std::size_t lineStart = 0;
        while (lineStart < text.size())
        {
            const std::size_t lineEnd = text.find('\n', lineStart);
            const std::size_t count = (lineEnd == std::string::npos) ? (text.size() - lineStart) : (lineEnd - lineStart);

            if (count != 0)
            {
                const std::string_view line(text.data() + lineStart, count);
                sLogger.failure("{}", line);
                appendCrashLogText(line);
            }

            if (lineEnd == std::string::npos)
                break;

            lineStart = lineEnd + 1;
        }
    }

#ifdef _WIN32
    [[nodiscard]] std::string miniDumpPath()
    {
        SYSTEMTIME localTime{};
        ::GetLocalTime(&localTime);

        char path[MAX_PATH]{};
        std::snprintf(path, sizeof(path), "%s/%s-%s-%04u-%02u-%02u-%02u-%02u-%02u-%lu.dmp",
            g_crashDumpDirectory, g_processName, g_buildHash,
            static_cast<unsigned>(localTime.wYear),
            static_cast<unsigned>(localTime.wMonth),
            static_cast<unsigned>(localTime.wDay),
            static_cast<unsigned>(localTime.wHour),
            static_cast<unsigned>(localTime.wMinute),
            static_cast<unsigned>(localTime.wSecond),
            static_cast<unsigned long>(::GetCurrentProcessId()));
        return path;
    }

    void writeMiniDump(EXCEPTION_POINTERS* exceptionPointers)
    {
        if (!g_options.writeMiniDump)
            return;

        const std::string path = miniDumpPath();
        HANDLE dumpFile = ::CreateFileA(path.c_str(), GENERIC_WRITE, 0,
            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (dumpFile == INVALID_HANDLE_VALUE)
        {
            sLogger.failure("Failed to create minidump file: {}", path);
            appendCrashLogText(std::string("Failed to create minidump file: ") + path);
            return;
        }

        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo{};
        exceptionInfo.ThreadId = ::GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = exceptionPointers;
        exceptionInfo.ClientPointers = FALSE;

        const auto dumpType = static_cast<MINIDUMP_TYPE>( MiniDumpWithThreadInfo |
            MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

        const BOOL result = ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(),
            dumpFile, dumpType, exceptionPointers != nullptr ? &exceptionInfo : nullptr, nullptr, nullptr);

        ::CloseHandle(dumpFile);

        if (result == FALSE)
        {
            sLogger.failure("MiniDumpWriteDump failed.");
            appendCrashLogText("MiniDumpWriteDump failed.");
            return;
        }

        sLogger.failure("Wrote minidump to {}", path);
        appendCrashLogText(std::string("Wrote minidump to ") + path);
    }

    LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS* exceptionPointers)
    {
        std::unique_lock<std::mutex> lock(g_windowsCrashMutex, std::try_to_lock);
        if (!lock.owns_lock())
            ::TerminateProcess(::GetCurrentProcess(), EXIT_FAILURE);

        if (g_isHandlingCrash.exchange(true))
            ::TerminateProcess(::GetCurrentProcess(), EXIT_FAILURE);

        ensureOutputDirectoriesExist();
        appendCrashLogText("Unhandled Windows exception.");
        writeMiniDump(exceptionPointers);
        logStackTraceToLoggerAndFile("Unhandled Windows exception.");

        sLogger.finalize();
        return EXCEPTION_EXECUTE_HANDLER;
    }
#else
    void writeAll(int fileDescriptor, const char* text)
    {
        if (fileDescriptor < 0 || text == nullptr)
            return;

        const std::size_t totalLength = std::strlen(text);
        std::size_t written = 0;

        while (written < totalLength)
        {
            const ssize_t result = ::write(fileDescriptor, text + written, totalLength - written);
            if (result <= 0)
                return;

            written += static_cast<std::size_t>(result);
        }
    }

    [[noreturn]] void posixSignalHandler(int signalNumber, siginfo_t* /*signalInfo*/, void* /*context*/)
    {
        if (g_isHandlingSignalCrash != 0)
            ::_Exit(128 + signalNumber);

        g_isHandlingSignalCrash = 1;

        char path[512]{};
        std::snprintf(path, sizeof(path), "%s/%s-%s-pid-%ld-sig-%d.log",
            g_crashDumpDirectory, g_processName, g_buildHash,
            static_cast<long>(::getpid()), signalNumber);

        int fileDescriptor = ::open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fileDescriptor < 0)
            fileDescriptor = STDERR_FILENO;

        char header[256]{};
        std::snprintf(header, sizeof(header), "AscEmu caught signal %d in %s process (pid=%ld, build=%s)\n",
            signalNumber, g_processName, static_cast<long>(::getpid()), g_buildHash);

        writeAll(fileDescriptor, header);
        if (fileDescriptor != STDERR_FILENO)
            writeAll(STDERR_FILENO, header);

        std::array<void*, 64> frames{};
        const int frameCount = ::backtrace(frames.data(), static_cast<int>(frames.size()));

        ::backtrace_symbols_fd(frames.data(), frameCount, fileDescriptor);
        if (fileDescriptor != STDERR_FILENO)
        {
            ::backtrace_symbols_fd(frames.data(), frameCount, STDERR_FILENO);
            ::close(fileDescriptor);
        }

        ::_Exit(128 + signalNumber);
    }

    void installSignal(int signalNumber)
    {
        struct sigaction action {};
        std::memset(&action, 0, sizeof(action));
        action.sa_sigaction = &posixSignalHandler;
        action.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESETHAND;
        ::sigemptyset(&action.sa_mask);
        ::sigaction(signalNumber, &action, nullptr);
    }
#endif

    [[noreturn]] void terminateHandler()
    {
        AscEmu::Debugging::terminateWithDiagnostics("std::terminate was called.");
    }
}

void AscEmu::Debugging::installCrashHandler(const CrashHandlerOptions& options)
{
    bool expected = false;
    if (!g_isInstalled.compare_exchange_strong(expected, true))
        return;

    refreshStaticOptions(options);
    ensureOutputDirectoriesExist();

    if (options.installTerminateHandler)
        std::set_terminate(&terminateHandler);

#ifdef _WIN32
    ::SetUnhandledExceptionFilter(&unhandledExceptionFilter);
#else
    g_alternateStack.ss_sp = g_signalStack;
    g_alternateStack.ss_size = sizeof(g_signalStack);
    g_alternateStack.ss_flags = 0;
    ::sigaltstack(&g_alternateStack, nullptr);

    installSignal(SIGSEGV);
    installSignal(SIGABRT);
    installSignal(SIGBUS);
    installSignal(SIGILL);
    installSignal(SIGFPE);

    std::array<void*, 1> warmup{};
    ::backtrace(warmup.data(), static_cast<int>(warmup.size()));
#endif

    sLogger.info("Installed crash handler for {} process.", g_processName);
}

bool AscEmu::Debugging::isCrashHandlerInstalled() noexcept
{
    return g_isInstalled.load();
}

void AscEmu::Debugging::requestCrashDiagnostics(std::string_view reason)
{
    ensureOutputDirectoriesExist();
    logStackTraceToLoggerAndFile(reason);
}

[[noreturn]] void AscEmu::Debugging::terminateWithDiagnostics(std::string_view reason)
{
    ensureOutputDirectoriesExist();
    logStackTraceToLoggerAndFile(reason);

#ifdef _WIN32
    writeMiniDump(nullptr);
#endif

    sLogger.finalize();
    std::abort();
}
