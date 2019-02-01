#pragma once

#include <stdio.h>

#ifdef _WIN32

#include <winsock2.h>
#include <winsock.h>
#include <string.h>
#include <sys/types.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <tchar.h>

#else

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#endif


#ifdef _WIN32

#define CLOSE(s) closesocket(s); 

#else

#define CLOSE(s) close(s); 

#endif

#ifndef _WIN32

#define LPSOCKADDR sockaddr*

#endif