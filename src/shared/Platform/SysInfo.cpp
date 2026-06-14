/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SysInfo.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <thread>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <psapi.h>
#if defined(_MSC_VER)
#pragma comment(lib, "Psapi.lib")
#endif
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/sysctl.h>
#endif
#if defined(__linux__)
#include <cstdlib>
#include <fstream>
#include <string>
#endif
#endif

namespace AscEmu::Platform
{
    std::uint32_t SysInfo::cpuCount() noexcept
    {
#if defined(_WIN32)
        SYSTEM_INFO info{};
        GetSystemInfo(&info);
        return std::max<std::uint32_t>(1u, info.dwNumberOfProcessors);
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__APPLE__)
        int ncpu = 0;
        size_t size = sizeof(ncpu);
        if (sysctlbyname("hw.ncpu", &ncpu, &size, nullptr, 0) == 0 && ncpu > 0)
            return static_cast<std::uint32_t>(ncpu);
        return 1;
#elif defined(__linux__)
        const long count = sysconf(_SC_NPROCESSORS_ONLN);
        return count > 0 ? static_cast<std::uint32_t>(count) : 1;
#else
        const auto count = std::thread::hardware_concurrency();
        return count == 0 ? 1 : count;
#endif
    }

    std::uint64_t SysInfo::processCpuTime() noexcept
    {
#if defined(_WIN32)
        FILETIME creationTime{};
        FILETIME exitTime{};
        FILETIME kernelTime{};
        FILETIME userTime{};

        if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime) == 0)
            return 0;

        ULARGE_INTEGER kernel{};
        kernel.HighPart = kernelTime.dwHighDateTime;
        kernel.LowPart = kernelTime.dwLowDateTime;

        ULARGE_INTEGER user{};
        user.HighPart = userTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;

        // GetProcessTimes returns 100 ns units; expose microseconds.
        return static_cast<std::uint64_t>((kernel.QuadPart + user.QuadPart) / 10ULL);
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__APPLE__)
        rusage usage{};
        if (getrusage(RUSAGE_SELF, &usage) != 0)
            return 0;

        const auto user = static_cast<std::uint64_t>(usage.ru_utime.tv_sec) * 1'000'000ULL +
            static_cast<std::uint64_t>(usage.ru_utime.tv_usec);
        const auto system = static_cast<std::uint64_t>(usage.ru_stime.tv_sec) * 1'000'000ULL +
            static_cast<std::uint64_t>(usage.ru_stime.tv_usec);
        return user + system;
#else
        return 0;
#endif
    }

    std::uint64_t SysInfo::processMemoryUsage() noexcept
    {
#if defined(_WIN32)
        PROCESS_MEMORY_COUNTERS_EX counters{};
        if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters), sizeof(counters)) == 0)
            return 0;

        return static_cast<std::uint64_t>(counters.WorkingSetSize);
#elif defined(__APPLE__)
        mach_task_basic_info info{};
        mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS)
            return 0;

        return static_cast<std::uint64_t>(info.resident_size);
#elif defined(__linux__)
        std::ifstream status("/proc/self/status");
        std::string key;

        while (status >> key)
        {
            if (key == "VmRSS:")
            {
                std::uint64_t valueKb = 0;
                status >> valueKb;
                return valueKb * 1024ULL;
            }

            status.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        return 0;
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
        rusage usage{};
        if (getrusage(RUSAGE_SELF, &usage) != 0)
            return 0;

        // ru_maxrss is KiB on FreeBSD.
        return static_cast<std::uint64_t>(usage.ru_maxrss) * 1024ULL;
#else
        return 0;
#endif
    }

    std::uint64_t SysInfo::tickCount() noexcept
    {
        using namespace std::chrono;
        return static_cast<std::uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
    }
}