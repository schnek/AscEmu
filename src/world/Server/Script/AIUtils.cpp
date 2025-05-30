/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AIUtils.hpp"
#include "Debugging/Errors.h"

#include <random>

std::chrono::milliseconds SchedulerArgs::randtime(std::chrono::milliseconds min, std::chrono::milliseconds max)
{
    long long diff = max.count() - min.count();
    ASSERT(diff >= 0)
    ASSERT(diff <= (uint32_t)-1)

    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<long long> uni(0, diff);

    return min + std::chrono::milliseconds(uni(rng));
}
