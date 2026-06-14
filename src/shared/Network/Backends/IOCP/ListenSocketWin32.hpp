/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#ifdef CONFIG_USE_IOCP

#include "Network/SocketMgr.hpp"
#include "Network/SocketDefines.hpp"
#include "Network/NetworkIncludes.hpp"
#include "Network/Core/ListenCommon.hpp"
#include "Network/Core/SocketPlatformOps.hpp"

template <class T>
class SERVER_DECL ListenSocket
{
public:
    ListenSocket(const char* listenAddress, uint32_t port)
    {
        m_socket = AscEmu::Network::SocketPlatformOps::createTcpSocket();
        if (m_socket == INVALID_SOCKET)
            return;

        AscEmu::Network::SocketPlatformOps::setReuseAddress(m_socket);
        AscEmu::Network::SocketPlatformOps::setBlocking(m_socket);
        AscEmu::Network::SocketPlatformOps::setTimeout(m_socket, 60);

        if (!AscEmu::Network::resolveListenAddress(listenAddress, static_cast<uint16_t>(port), m_address))
            return;

        if (!AscEmu::Network::bindAndListenSocket(m_socket, m_address, port))
            return;

        m_opened = true;
        m_tempLength = sizeof(sockaddr_in);
        m_completionPort = sSocketMgr.GetCompletionPort();
    }

    ~ListenSocket()
    {
        Close();
    }

    bool runThread()
    {
        while (m_opened)
        {
            m_tempLength = sizeof(sockaddr_in);
            SOCKET acceptedSocket = WSAAccept(m_socket, reinterpret_cast<sockaddr*>(&m_tempAddress), &m_tempLength, nullptr, 0);
            if (acceptedSocket == INVALID_SOCKET)
                continue;

            auto* accepted = new T(acceptedSocket);
            accepted->setCompletionPort(m_completionPort);
            accepted->accept(&m_tempAddress);
        }

        return false;
    }

    void Close()
    {
        const bool wasOpened = m_opened;
        m_opened = false;

        if (wasOpened)
            AscEmu::Network::SocketPlatformOps::closeSocket(m_socket);
    }

    bool IsOpen() const
    {
        return m_opened;
    }

private:
    SOCKET m_socket = INVALID_SOCKET;
    AscEmu::Network::SocketAddressIPv4 m_address{};
    sockaddr_in m_tempAddress{};
    bool m_opened = false;
    int m_tempLength = 0;
    HANDLE m_completionPort = nullptr;
};

#endif
