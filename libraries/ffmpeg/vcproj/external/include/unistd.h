/* unistd.h stub for Xbox 360 XDK / MSVC builds */
/* This file provides minimal compatibility for code that includes unistd.h */
#ifndef _UNISTD_H_STUB
#define _UNISTD_H_STUB

#ifdef _MSC_VER

#include <io.h>
#include <process.h>
#include <direct.h>

/* Common unistd.h definitions */
#ifndef STDIN_FILENO
#define STDIN_FILENO  0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

/* access() mode flags */
#ifndef R_OK
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
#endif

#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef int ssize_t;
#endif

#endif /* _MSC_VER */

#endif /* _UNISTD_H_STUB */
