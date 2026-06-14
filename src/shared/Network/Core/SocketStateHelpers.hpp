/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/Socket.hpp"

namespace AscEmu::Network
{
    inline bool isSocketUsable(Socket* socket)
    {
        return socket != nullptr && !socket->isDeleted();
    }

    inline bool hasPendingWrite(Socket& socket)
    {
        return socket.writeBuffer.GetSize() > 0;
    }

#if defined(CONFIG_USE_EPOLL) || defined(CONFIG_USE_KQUEUE)
    inline bool shouldArmWriteAfterRead(Socket& socket)
    {
        return socket.writeBuffer.GetSize() > 0 && socket.isConnected() && !socket.hasSendLock();
    }
#endif

    inline void disconnectSocket(Socket& socket)
    {
        if (!socket.isDeleted())
            socket.disconnect();
    }
}
