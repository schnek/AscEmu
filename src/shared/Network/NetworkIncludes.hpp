/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/time.h>
    #include <sys/types.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <signal.h>
    #include <netdb.h>
#endif
