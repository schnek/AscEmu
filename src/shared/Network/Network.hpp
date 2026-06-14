/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Logging/Logger.hpp"
#include "SocketDefines.hpp"
#include "Socket.hpp"
#include "SocketMgr.hpp"

#ifdef CONFIG_USE_IOCP
#include "Backends/IOCP/ListenSocketWin32.hpp"
#endif

#if defined(CONFIG_USE_EPOLL) || defined(CONFIG_USE_KQUEUE)
#include "Core/PollListenSocket.hpp"
#endif
