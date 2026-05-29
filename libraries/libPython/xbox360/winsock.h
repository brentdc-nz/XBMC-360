/*
 * winsock.h shim for Xbox 360
 *
 * The select module includes <winsock.h> on Windows.
 * On Xbox 360, Winsock is provided via xtl.h which includes winsockx.h.
 * This shim redirects to the Xbox 360 networking headers.
 */

#ifndef _XBOX360_WINSOCK_H_SHIM
#define _XBOX360_WINSOCK_H_SHIM

#include <xtl.h>

/* Xbox 360 xtl.h provides:
 * - select(), fd_set, FD_SET, FD_CLR, FD_ISSET, FD_ZERO
 * - SOCKET, INVALID_SOCKET, SOCKET_ERROR
 * - struct timeval
 * - closesocket(), ioctlsocket()
 */

/* Ensure SOCKET type is defined */
#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (SOCKET)(~0)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR    (-1)
#endif

#endif /* _XBOX360_WINSOCK_H_SHIM */
