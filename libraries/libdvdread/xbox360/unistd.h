/* Minimal unistd.h for the Xbox 360 (XDK CRT) build of libdvd. */
#ifndef LIBDVD_X360_UNISTD_H
#define LIBDVD_X360_UNISTD_H

#include <io.h>
#include <sys/types.h>
#ifndef _OFF_T_DEFINED
typedef __int64 off_t; /* 64-bit: DVD ISO images exceed the 32-bit XDK default */
#define _OFF_T_DEFINED
#endif
#include <fcntl.h>
#include <process.h>
#include <direct.h>

#define open   _open
#define read   _read
#define write  _write
#define lseek  _lseeki64 /* 64-bit: ISO images exceed the 32-bit _lseek range */
#define close _close
#define getpid _getpid

#ifndef O_BINARY
#define O_BINARY _O_BINARY
#endif

#endif /* LIBDVD_X360_UNISTD_H */
