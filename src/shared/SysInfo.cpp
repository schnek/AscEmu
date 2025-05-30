/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SysInfo.hpp"

#ifdef WIN32
#include <Windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi")
#endif

#if defined(linux) || defined(__linux)
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstdlib>
#endif

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#ifndef NULL
#define NULL 0
#endif
#endif

#ifdef __APPLE__
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/task.h>
#include <mach/mach.h>
#endif


namespace Ascemu
{

#ifdef WIN32

    long SysInfo::GetCPUCount()
    {
        SYSTEM_INFO info;
        GetSystemInfo(&info);

        return info.dwNumberOfProcessors;
    }

    unsigned long long SysInfo::GetCPUUsage()
    {
        FILETIME dummy;
        FILETIME kerneltime;
        FILETIME usertime;

        BOOL success = GetProcessTimes(GetCurrentProcess(), &dummy, &dummy, &kerneltime, &usertime);
        if(success == 0)
            return 0;

        ULARGE_INTEGER ktime;
        ULARGE_INTEGER utime;

        ktime.HighPart = kerneltime.dwHighDateTime;
        ktime.LowPart  = kerneltime.dwLowDateTime;
        utime.HighPart = usertime.dwHighDateTime;
        utime.LowPart  = usertime.dwLowDateTime;

        double totaltime = static_cast< double >(ktime.QuadPart + utime.QuadPart);

        // GetProcessTimes() returns the value in 100 nanosecond units and we need it in microseconds
        totaltime /= 10;

        return static_cast< unsigned long long >(totaltime);
    }

    unsigned long long SysInfo::GetRAMUsage()
    {
        PROCESS_MEMORY_COUNTERS pmcex;
        GetProcessMemoryInfo(GetCurrentProcess(), &pmcex, sizeof(pmcex));

        return pmcex.PagefileUsage;
    }

    unsigned long long SysInfo::GetTickCount()
    {
#ifndef _WIN64
        return ::GetTickCount();
#else
        return ::GetTickCount64();
#endif
    }

#endif

#if defined(linux) || defined(__linux)

    long SysInfo::GetCPUCount()
    {
        return sysconf(_SC_NPROCESSORS_ONLN);
    }

    unsigned long long SysInfo::GetCPUUsage()
    {
        rusage usage;

        if(getrusage(RUSAGE_SELF, &usage) != 0)
            return 0;

        unsigned long long k = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec;
        unsigned long long u = usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;

        return (k + u);
    }

    unsigned long long SysInfo::GetRAMUsage()
    {
        std::ifstream memf;
        std::string line;
        std::string value;
        std::stringstream fname;
        char line2[100];
        float memusage;
        unsigned long pos = 0;
        unsigned long pid = getpid();

        fname << "/proc/" << pid << "/status";

        memf.open(fname.str().c_str());

        do
        {
            pos = std::string::npos;
            memf.getline(line2, 100);
            line.assign(line2);
            pos = line.find("VmRSS");
        }
        while(pos == std::string::npos);

        pos = line.find(" ");
        ++pos;

        value = line.substr(pos, std::string::npos);

        memusage = static_cast<float>(atof(value.c_str()));

        memf.close();

        memusage *= 1024.0f;

        return memusage;
    }

    unsigned long long SysInfo::GetTickCount()
    {
        timeval Time;

        if(gettimeofday(&Time, NULL) != 0)
            return 0;

        return (Time.tv_sec * 1000) + (Time.tv_usec / 1000);
    }
#endif

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)

    long SysInfo::GetCPUCount()
    {
        int mib[4];
        int ncpu;
        size_t len = 4;

        sysctlnametomib("hw.ncpu", mib, &len);
        size_t len2 = sizeof(ncpu);
        sysctl(mib, len, &ncpu, &len2, NULL, 0);

        return ncpu;
    }

    unsigned long long SysInfo::GetCPUUsage()
    {
        rusage usage;

        if(getrusage(RUSAGE_SELF, &usage) != 0)
            return 0;

        unsigned long long k = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec;
        unsigned long long u = usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;

        return (k + u);
    }

    unsigned long long SysInfo::GetRAMUsage()
    {
        return 0;
    }

    unsigned long long SysInfo::GetTickCount()
    {
        timeval Time;

        if(gettimeofday(&Time, NULL) != 0)
            return 0;

        return (Time.tv_sec * 1000) + (Time.tv_usec / 1000);
    }

#endif

#ifdef __APPLE__

    long SysInfo::GetCPUCount()
    {
        int mib[4];
        int ncpu;
        size_t len = 4;

        sysctlnametomib("hw.ncpu", mib, &len);
        size_t len2 = sizeof(ncpu);
        sysctl(mib, len, &ncpu, &len2, NULL, 0);

        return ncpu;
    }

    unsigned long long SysInfo::GetCPUUsage()
    {
        rusage usage;

        if(getrusage(RUSAGE_SELF, &usage) != 0)
            return 0;

        unsigned long long k = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec;
        unsigned long long u = usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;

        return (k + u);
    }

    unsigned long long SysInfo::GetRAMUsage()
    {
        task_t task = MACH_PORT_NULL;
        task_basic_info t_info;
        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

        if(KERN_SUCCESS != task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count))
            return 0;

        return t_info.resident_size;
    }

    unsigned long long SysInfo::GetTickCount()
    {
        timeval Time;

        if(gettimeofday(&Time, NULL) != 0)
            return 0;

        return (Time.tv_sec * 1000) + (Time.tv_usec / 1000);
    }

#endif
}
