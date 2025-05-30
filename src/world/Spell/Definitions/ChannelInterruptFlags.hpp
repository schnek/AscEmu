/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum ChannelInterruptFlags
{
    CHANNEL_INTERRUPT_NULL          = 0x0,
    CHANNEL_INTERRUPT_ON_1          = 0x1,
    CHANNEL_INTERRUPT_ON_DAMAGE     = 0x2,
    CHANNEL_INTERRUPT_ON_3          = 0x4,
    CHANNEL_INTERRUPT_ON_MOVEMENT   = 0x8,
    CHANNEL_INTERRUPT_ON_TURNING    = 0x10,
    CHANNEL_INTERRUPT_ON_6          = 0x20,
    CHANNEL_INTERRUPT_ON_7          = 0x40,
    CHANNEL_INTERRUPT_ON_8          = 0x80,
    CHANNEL_INTERRUPT_ON_9          = 0x100,
    CHANNEL_INTERRUPT_ON_10         = 0x200,
    CHANNEL_INTERRUPT_ON_11         = 0x400,
    CHANNEL_INTERRUPT_ON_12         = 0x800,
    CHANNEL_INTERRUPT_ON_13         = 0x1000,
    CHANNEL_INTERRUPT_ON_14         = 0x2000,
    CHANNEL_INTERRUPT_ON_15         = 0x4000,
    CHANNEL_INTERRUPT_ON_16         = 0x8000,
    CHANNEL_INTERRUPT_ON_17         = 0x10000,
    CHANNEL_INTERRUPT_ON_18         = 0x20000
};
