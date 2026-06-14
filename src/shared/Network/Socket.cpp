/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Network.hpp"
#include "Network/Core/Resolver.hpp"
#include "Network/Core/SocketPlatformOps.hpp"

#pragma warning ( disable: 4996 )

Socket::Socket(SOCKET fd, uint32_t sendbuffersize, uint32_t recvbuffersize)
    : m_fd(fd), m_connected(false), m_deleted(false), m_writeLock(0)
{
    // Allocate Buffers
    readBuffer.Allocate(recvbuffersize);
    writeBuffer.Allocate(sendbuffersize);

    m_BytesSent = 0;
    m_BytesRecieved = 0;

    // IOCP Member Variables
#ifdef CONFIG_USE_IOCP
    m_completionPort = nullptr;
#endif

    if (m_fd == 0)
        m_fd = AscEmu::Network::SocketPlatformOps::createTcpSocket();

    sLogger.debug("Created Socket {}", m_fd);
}

Socket::~Socket()
{}

bool Socket::Connect(const char* Address, uint32_t Port)
{
    AscEmu::Network::SocketAddressIPv4 resolvedAddress;
    if (!AscEmu::Network::Resolver::resolveRemoteIPv4(Address, static_cast<uint16_t>(Port), resolvedAddress))
        return false;

    m_client = resolvedAddress.native();

    AscEmu::Network::SocketPlatformOps::setBlocking(m_fd);

    if (connect(m_fd, reinterpret_cast<const sockaddr*>(&m_client), sizeof(m_client)) == -1)
        return false;

#ifdef CONFIG_USE_IOCP
    m_completionPort = sSocketMgr.GetCompletionPort();
#endif

    _OnConnect();
    return true;
}

void Socket::Accept(sockaddr_in* address)
{
    memcpy(&m_client, address, sizeof(*address));
    _OnConnect();
}

void Socket::_OnConnect()
{
    AscEmu::Network::SocketPlatformOps::setNonBlocking(m_fd);
    AscEmu::Network::SocketPlatformOps::disableBuffering(m_fd);

    m_connected = true;

    // IOCP stuff
#ifdef CONFIG_USE_IOCP
    AssignToCompletionPort();
    SetupReadEvent();
#endif
    sSocketMgr.AddSocket(this);

    sLogger.info("Socket::_OnConnect called");
    // Call virtual onconnect
    OnConnect();
}

bool Socket::Send(const uint8_t* Bytes, uint32_t Size)
{
    bool rv;

    // This is really just a wrapper for all the burst stuff.
    BurstBegin();
    rv = BurstSend(Bytes, Size);
    if (rv)
        BurstPush();
    BurstEnd();

    return rv;
}

bool Socket::BurstSend(const uint8_t* Bytes, uint32_t Size)
{
    return writeBuffer.Write(Bytes, Size);
}

std::string Socket::GetRemoteIP()
{
    char* ip = (char*)inet_ntoa(m_client.sin_addr);
    if (ip != NULL)
        return std::string(ip);
    else
        return std::string("noip");
}

void Socket::Disconnect()
{
    //if returns false it means it's already disconnected
    if (!m_connected)
        return;

    m_connected = false;

    sLogger.info("Socket::Disconnect on socket {}", m_fd);

    // remove from mgr
    sSocketMgr.RemoveSocket(this);

    // Call virtual ondisconnect
    OnDisconnect();

    if (!IsDeleted())
        Delete();
}

void Socket::Delete()
{
    //if returns true it means it's already delete
    if (m_deleted)
        return;

    m_deleted = true;

    sLogger.debug("Socket::Delete() on socket {}", m_fd);

    if (IsConnected())
        Disconnect();

    AscEmu::Network::SocketPlatformOps::closeSocket(m_fd);

    sSocketGarbageCollector.QueueSocket(this);
}
