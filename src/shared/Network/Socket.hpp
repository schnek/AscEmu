/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "SocketDefines.hpp"
#include "NetworkIncludes.hpp"
#include "BipBuffer.hpp"
#include "Logging/Log.hpp"
#include <string>
#include <mutex>
#include <atomic>
#include <map>
#include <set>

class SERVER_DECL Socket
{
public:
    Socket(SOCKET fd, uint32_t sendbuffersize, uint32_t recvbuffersize);
    virtual ~Socket();

    bool Connect(const char* Address, uint32_t Port);
    void Disconnect();
    void Accept(sockaddr_in* address);

    virtual void OnRead() {}
    virtual void OnConnect() {}
    virtual void OnDisconnect() {}

    bool Send(const uint8_t* Bytes, uint32_t Size);

    inline void BurstBegin() { m_writeMutex.lock(); }
    bool BurstSend(const uint8_t* Bytes, uint32_t Size);
    void BurstPush();
    inline void BurstEnd() { m_writeMutex.unlock(); }

    std::string GetRemoteIP();
    inline uint32_t GetRemotePort() { return ntohs(m_client.sin_port); }
    inline SOCKET GetFd() { return m_fd; }

    void SetupReadEvent();
    void ReadCallback(uint32_t len);
    void WriteCallback();

    inline bool IsDeleted()
    {
        return m_deleted;
    }
    inline bool IsConnected()
    {
        return m_connected;
    }
    inline sockaddr_in& GetRemoteStruct() { return m_client; }

    void Delete();

    inline in_addr GetRemoteAddress() { return m_client.sin_addr; }


    AscEmu::BipBuffer readBuffer;
    AscEmu::BipBuffer writeBuffer;

protected:
    void _OnConnect();

    SOCKET m_fd;

    std::recursive_mutex m_writeMutex;
    std::recursive_mutex m_readMutex;

    std::atomic<bool> m_connected;
    std::atomic<bool> m_deleted;

    sockaddr_in m_client;

    unsigned long m_BytesSent;
    unsigned long m_BytesRecieved;

public:

    inline void IncSendLock() { ++m_writeLock; }
    inline void DecSendLock() { --m_writeLock; }
    inline bool AcquireSendLock()
    {

        if (m_writeLock != 0)
            return false;

        m_writeLock = 1;
        return true;
    }

private:
    std::atomic<unsigned long> m_writeLock;

public:
    void PollTraffic(unsigned long* sent, unsigned long* recieved)
    {
        std::lock_guard lock{ m_writeMutex };

        *sent = m_BytesSent;
        *recieved = m_BytesRecieved;
        m_BytesSent = 0;
        m_BytesRecieved = 0;
    }

#ifdef CONFIG_USE_IOCP
public:
    inline void SetCompletionPort(HANDLE cp) { m_completionPort = cp; }

    OverlappedStruct m_readEvent;
    OverlappedStruct m_writeEvent;

private:
    HANDLE m_completionPort;

    void AssignToCompletionPort();
#endif

#ifdef CONFIG_USE_EPOLL
public:
    void PostEvent(uint32_t events);

    inline bool HasSendLock()
    {
        bool res;
        res = (m_writeLock.load() != 0);
        return res;
    }
#endif

#ifdef CONFIG_USE_KQUEUE
public:
    void PostEvent(int events, bool oneshot);
    inline bool HasSendLock()
    {
        bool res;
        res = (m_writeLock.load() != 0);
        return res;
    }
#endif
};


template<class T>
T* ConnectTCPSocket(const char* hostname, u_short port)
{
    T* s = new T(0);
    if (!s->Connect(hostname, port))
    {
        s->Delete();
        return nullptr;
    }

    return s;
}

#define SOCKET_GC_TIMEOUT 15

class SocketGarbageCollector
{
    std::map<Socket*, time_t> deletionQueue;
    std::mutex lock;
private:
    SocketGarbageCollector() = default;
    ~SocketGarbageCollector() = default;

public:

    static SocketGarbageCollector& getInstance()
    {
        static SocketGarbageCollector mInstance;
        return mInstance;
    }

    void finalize()
    {
        for (auto i : deletionQueue)
            delete i.first;
    }

    SocketGarbageCollector(SocketGarbageCollector&&) = delete;
    SocketGarbageCollector(SocketGarbageCollector const&) = delete;
    SocketGarbageCollector& operator=(SocketGarbageCollector&&) = delete;
    SocketGarbageCollector& operator=(SocketGarbageCollector const&) = delete;

    void Update()
    {
        std::map<Socket*, time_t>::iterator i, i2;
        time_t t = UNIXTIME;

        std::lock_guard guard{ lock };

        for (i = deletionQueue.begin(); i != deletionQueue.end();)
        {
            i2 = i++;
            if (i2->second <= t)
            {
                delete i2->first;
                deletionQueue.erase(i2);
            }
        }
    }

    void QueueSocket(Socket* s)
    {
        std::lock_guard guard{ lock };
        deletionQueue.insert(std::map<Socket*, time_t>::value_type(s, UNIXTIME + SOCKET_GC_TIMEOUT));
    }
};

#define sSocketGarbageCollector SocketGarbageCollector::getInstance()
