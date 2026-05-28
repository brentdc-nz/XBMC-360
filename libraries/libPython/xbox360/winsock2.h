/*
 * winsock2.h shim for Xbox 360
 *
 * Python only uses winsock2.h for socket error codes in exceptions.c.
 * Provide minimal definitions without pulling in the full Xbox networking stack.
 */

#ifndef _XBOX360_WINSOCK2_H_SHIM
#define _XBOX360_WINSOCK2_H_SHIM

/* Include the base windows shim for types */
#include "windows.h"

/* Winsock error codes that Python uses */
#ifndef WSABASEERR
#define WSABASEERR          10000
#endif
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK      10035L
#endif
#ifndef WSAEINPROGRESS
#define WSAEINPROGRESS      10036L
#endif
#ifndef WSAEALREADY
#define WSAEALREADY         10037L
#endif
#ifndef WSAENOTSOCK
#define WSAENOTSOCK         10038L
#endif
#ifndef WSAECONNREFUSED
#define WSAECONNREFUSED     10061L
#endif
#ifndef WSAECONNRESET
#define WSAECONNRESET       10054L
#endif
#ifndef WSAECONNABORTED
#define WSAECONNABORTED     10053L
#endif
#ifndef WSAETIMEDOUT
#define WSAETIMEDOUT        10060L
#endif
#ifndef WSAHOST_NOT_FOUND
#define WSAHOST_NOT_FOUND   11001L
#endif

/* WSAGetLastError stub */
#ifndef WSAGetLastError
#define WSAGetLastError()   (0)
#endif

#endif /* _XBOX360_WINSOCK2_H_SHIM */
