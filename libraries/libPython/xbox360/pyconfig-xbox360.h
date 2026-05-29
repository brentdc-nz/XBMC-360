/*
 * pyconfig-xbox360.h
 *
 * Python 3.4.10 configuration for Xbox 360 (PowerPC, big-endian)
 * Static library build targeting the Xbox 360 XDK.
 */

#ifndef Py_PYCONFIG_XBOX360_H
#define Py_PYCONFIG_XBOX360_H

/* Define the same guard as PC/pyconfig.h so it won't override our settings */
#define Py_CONFIG_H

/* ======================================================================
 * Platform identification
 * ====================================================================== */
#define _XBOX              1
#define MS_WIN32           1
#define MS_WINDOWS         1
#define HAVE_STDARG_PROTOTYPES 1

/* Xbox 360 is PowerPC big-endian */
#define WORDS_BIGENDIAN    1
#define PY_BIG_ENDIAN      1
#define PY_LITTLE_ENDIAN   0
#define DOUBLE_IS_BIG_ENDIAN_IEEE754 1

/* Static build - do NOT define Py_ENABLE_SHARED */
#define Py_BUILD_CORE      1
#define Py_NO_ENABLE_SHARED 1

/* ======================================================================
 * Compiler / CRT settings (MSVC-like for Xbox 360 compiler)
 * ====================================================================== */
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

/* Disable iterator debugging in STL */
#define _ITERATOR_DEBUG_LEVEL 0
#define _HAS_ITERATOR_DEBUGGING 0

/* ======================================================================
 * Type sizes (Xbox 360 is ILP32 - 32-bit pointers, 32-bit long)
 * ====================================================================== */
#define SIZEOF_VOID_P      4
#define SIZEOF_SHORT       2
#define SIZEOF_INT         4
#define SIZEOF_LONG        4
#define SIZEOF_LONG_LONG   8
#define SIZEOF_DOUBLE      8
#define SIZEOF_FLOAT       4
#define SIZEOF_SIZE_T      4
#define SIZEOF_WCHAR_T     2
#define SIZEOF_FPOS_T      8
#define SIZEOF_OFF_T       4
#define SIZEOF_TIME_T      4
#define SIZEOF_PID_T       4
#define SIZEOF_UINTPTR_T   4

/* Exact-width integer types for dtoa.c and others */
#define HAVE_UINT32_T      1
#define HAVE_UINT64_T      1
#define HAVE_INT32_T       1
#define HAVE_INT64_T       1
typedef unsigned int       uint32_t;
typedef unsigned __int64   uint64_t;
typedef int                int32_t;
typedef __int64            int64_t;
#define PY_UINT32_T        uint32_t
#define PY_UINT64_T        uint64_t
#define PY_INT32_T         int32_t
#define PY_INT64_T         int64_t

#define LONG_BIT           32
#define WORD_BIT           32

/* pid_t for signalmodule.c */
#ifndef _PID_T_DEFINED
typedef int pid_t;
#define _PID_T_DEFINED
#endif

/* Py_ssize_t will be defined by pyport.h using Py_intptr_t (long on 32-bit) */
/* Do NOT define HAVE_SSIZE_T - ssize_t doesn't exist in Xbox 360 SDK */
/* PY_SSIZE_T_MAX/MIN are defined by pyport.h automatically */

/* ======================================================================
 * Available headers
 * ====================================================================== */
#define HAVE_LONG_LONG     1
#define HAVE_ERRNO_H       1
#define HAVE_FCNTL_H       1
#define HAVE_LIMITS_H      1
#define HAVE_SIGNAL_H      1
#define HAVE_STDDEF_H      1
#define HAVE_STDLIB_H      1
#define HAVE_STRING_H      1
#define HAVE_MATH_H        1
#define HAVE_FLOAT_H       1
#define HAVE_DIRECT_H      1
#define HAVE_IO_H          1
#define HAVE_PROCESS_H     1
#define HAVE_CONIO_H       0
#define HAVE_SYS_TYPES_H  1
#define HAVE_SYS_STAT_H   1

/* Headers Xbox 360 does NOT have */
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_DIRENT_H */
/* #undef HAVE_SYS_UTIME_H */
/* #undef HAVE_SYS_TIME_H */
/* #undef HAVE_SYS_WAIT_H */
/* #undef HAVE_SYS_FILE_H */
/* #undef HAVE_SYS_RESOURCE_H */
/* #undef HAVE_SYS_SOCKET_H */
/* #undef HAVE_NETINET_IN_H */
/* #undef HAVE_DLFCN_H */
/* #undef HAVE_LANGINFO_H */
/* #undef HAVE_LIBINTL_H */
/* #undef HAVE_POLL_H */
/* #undef HAVE_PTHREAD_H */
/* #undef HAVE_TERMIOS_H */
/* #undef HAVE_UTIME_H */
/* #undef HAVE_WCHAR_H -- actually Xbox 360 has limited wchar support */
#define HAVE_WCHAR_H       1

/* ======================================================================
 * Available functions
 * ====================================================================== */
#define HAVE_HYPOT         1
#define HAVE_STRFTIME      1
#define HAVE_STRERROR      1
#define HAVE_CLOCK         1
#define HAVE_COPYSIGN      1
#define HAVE_ROUND         1
#define HAVE_ISINF         1
#define HAVE_ISNAN         1
#define HAVE_FINITE        1
#define HAVE_SNPRINTF      1
#define HAVE_MEMMOVE       1
#define HAVE_GETCWD        1

/* Functions Xbox 360 does NOT have */
#define DONT_HAVE_SIG_ALARM  1
#define DONT_HAVE_SIG_PAUSE  1
/* #undef HAVE_ALARM */
/* #undef HAVE_CHOWN */
/* #undef HAVE_FORK */
/* #undef HAVE_FLOCK */
/* #undef HAVE_GETPID */
/* #undef HAVE_KILL */
/* #undef HAVE_LINK */
/* #undef HAVE_LSTAT */
/* #undef HAVE_MKFIFO */
/* #undef HAVE_MKNOD */
/* #undef HAVE_OPENPTY */
/* #undef HAVE_PIPE */
/* #undef HAVE_POPEN */
/* #undef HAVE_READLINK */
/* #undef HAVE_SETUID */
/* #undef HAVE_SETGID */
/* #undef HAVE_SETSID */
/* #undef HAVE_SYMLINK */
/* #undef HAVE_TCGETPGRP */
/* #undef HAVE_TCSETPGRP */
/* #undef HAVE_TIMES */
/* #undef HAVE_TRUNCATE */
/* #undef HAVE_UNAME */
/* #undef HAVE_WAIT */
/* #undef HAVE_WAITPID */
/* #undef HAVE_TMPFILE */
/* #undef HAVE_TMPNAM */
/* #undef HAVE_TEMPNAM */
/* #undef HAVE_EXECV */
/* #undef HAVE_SPAWNV */
/* #undef HAVE_SYSTEM */
/* #undef HAVE_GETLOGIN */
/* #undef HAVE_CTERMID */
/* #undef HAVE_FCHDIR */
/* #undef HAVE_FDATASYNC */
/* #undef HAVE_FSYNC */
/* #undef HAVE_PUTENV */
/* #undef HAVE_SETENV */
/* #undef HAVE_UNSETENV */

/* ======================================================================
 * Dynamic loading - DISABLED on Xbox 360
 * ====================================================================== */
/* #undef HAVE_DYNAMIC_LOADING */

/* ======================================================================
 * Threading - Xbox 360 has native threads
 * ====================================================================== */
#define WITH_THREAD        1
#define NT_THREADS         1
/* Use Xbox 360 threading primitives (similar to Win32 threads) */

/* ======================================================================
 * Unicode / wchar
 * ====================================================================== */
/* Py_UNICODE_SIZE is computed by unicodeobject.h from SIZEOF_WCHAR_T.
   Do NOT define it here to avoid C4005 redefinition warnings. */
#define PY_UNICODE_TYPE    wchar_t
#define HAVE_USABLE_WCHAR_T 1

/* ======================================================================
 * File system encoding
 * ====================================================================== */
#define PYTHONPATH         L".\\Lib"
#define LANDMARK           L"os.py"
/* Note: PREFIX and EXEC_PREFIX are NOT defined here because they conflict
 * with expat's internal typedef 'PREFIX'. getpath_xbox360.c provides
 * Py_GetPrefix() and Py_GetExecPrefix() directly. */
#define VERSION            "3.4"
#define VPATH              ""
#define PYLONG_BITS_IN_DIGIT 15

/* ======================================================================
 * Misc settings
 * ====================================================================== */
/* No console I/O on Xbox 360 */
/* #undef HAVE_GETC_UNLOCKED */

/* Math */
#define Py_OVERFLOWED(X) ((X) != 0.0 && (errno == ERANGE || (X) == Py_HUGE_VAL || (X) == -Py_HUGE_VAL))
#define Py_FORCE_DOUBLE(X) (X)

/* Floating-point control word macros - Xbox 360 PPC doesn't have x86 FPU control.
   The Xbox 360 SDK float.h defines _controlfp as a function, so do NOT
   redefine it here. Just provide the control word constants if missing. */
#ifndef _MCW_PC
#define _MCW_PC   0x00030000
#endif
#ifndef _PC_53
#define _PC_53    0x00010000
#endif
#ifndef _MCW_RC
#define _MCW_RC   0x00000300
#endif
#ifndef _RC_NEAR
#define _RC_NEAR  0x00000000
#endif

/* Hash algorithm - use FNV (simpler, no external deps) */
#define Py_HASH_ALGORITHM  Py_HASH_FNV

/* No /dev/urandom on Xbox 360 */
/* #undef HAVE_GETRANDOM */
/* #undef HAVE_GETENTROPY */

/* Disable features that need significant OS support */
/* #undef HAVE_DLOPEN */
/* #undef HAVE_MMAP */
/* #undef MS_COREDLL */

/* Xbox 360 path separator */
#define DELIM              L';'
#define SEP                L'\\'
#define ALTSEP             L'/'

/* ======================================================================
 * Virtual Memory constants (obmalloc.c)
 * These must be available even if xtl.h doesn't define them in C mode.
 * ====================================================================== */
#ifndef MEM_COMMIT
#define MEM_COMMIT      0x1000
#endif
#ifndef MEM_RESERVE
#define MEM_RESERVE     0x2000
#endif
#ifndef MEM_RELEASE
#define MEM_RELEASE     0x8000
#endif
#ifndef PAGE_READWRITE
#define PAGE_READWRITE  0x04
#endif

/* ======================================================================
 * Function declarations for Xbox 360 stubs
 * These functions are implemented in python_xbox360_stubs.c but need
 * prototypes visible to the C89 compiler to avoid implicit-int warnings.
 * Win32 API prototypes are NOT declared here because they conflict with
 * the SDK's winbase.h; C4047 for those is suppressed at project level.
 * ====================================================================== */
#ifdef __cplusplus
extern "C" {
#endif

/* C standard library stubs */
char* getenv(const char* name);

/* Math function used by mathmodule.c FUNC2(copysign,...) */
#ifndef _XBOX_COPYSIGN_DECLARED
#define _XBOX_COPYSIGN_DECLARED
double copysign(double x, double y);
#endif

#ifdef __cplusplus
}
#endif

#endif /* Py_PYCONFIG_XBOX360_H */
