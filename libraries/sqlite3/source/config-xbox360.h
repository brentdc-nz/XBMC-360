#ifndef SQLITE_CONFIG_XBOX360_H
#define SQLITE_CONFIG_XBOX360_H
/***************************************************************************
 * Hand crafted config file for Xbox 360 XDK
 * SQLite 3.54.0 ported for Xbox 360 PowerPC (big-endian)
 *
 * This header must be force-included before any SQLite source file,
 * or included at the top of sqliteInt.h.
 ***************************************************************************/

/* ---------------------------------------------------------------- */
/*                    Xbox 360 platform identity                    */
/* ---------------------------------------------------------------- */
#ifndef _XBOX
#define _XBOX 1
#endif

/* Xbox 360 XDK defines _WIN32 so SQLITE_OS_WIN is auto-detected.
** The Xbox 360 kernel is NT-based, so set SQLITE_OS_WINNT=1 to
** enable the NT code paths in os_win.c (CreateFile, etc.)
*/
#ifndef SQLITE_OS_WINNT
#define SQLITE_OS_WINNT 1
#endif

/* ---------------------------------------------------------------- */
/*             Features to DISABLE on Xbox 360                      */
/* ---------------------------------------------------------------- */

/* No dynamic library loading on Xbox 360 */
#define SQLITE_OMIT_LOAD_EXTENSION 1

/* Compile-option diagnostics require generated ctime.c - omit */
#define SQLITE_OMIT_COMPILEOPTION_DIAGS 1

/* No memory-mapped I/O (Xbox 360 doesn't expose MapViewOfFile to title code) */
#ifndef SQLITE_MAX_MMAP_SIZE
#define SQLITE_MAX_MMAP_SIZE 0
#endif

/* No localtime_r / localtime_s on Xbox 360 */
#define SQLITE_OMIT_LOCALTIME 1

/* No GetTempPath on Xbox 360 - temp files go to a fixed location */
#define SQLITE_TEMP_STORE 2  /* Use memory for temp by default, file if needed */

/* No deprecated APIs needed */
#define SQLITE_OMIT_DEPRECATED 1

/* Disable shared cache (not needed for single-app console) */
#define SQLITE_OMIT_SHARED_CACHE 1

/* Disable WAL mode - requires advanced NT kernel file-locking that 
** may not be fully available in the Xbox 360 title environment. 
** Can be re-enabled later if testing shows it works. */
#define SQLITE_OMIT_WAL 1

/* No POSIX advisory locking */
#define SQLITE_OMIT_POSIX_ADVISORY_LOCKING 1

/* ---------------------------------------------------------------- */
/*             Features to ENABLE on Xbox 360                       */
/* ---------------------------------------------------------------- */

/* Thread safety: Xbox 360 supports threads via XDK threading APIs */
#define SQLITE_THREADSAFE 1

/* Use the Win32 mutex implementation (mutex_w32.c) */
/* This is automatic when SQLITE_OS_WIN=1 */

/* Enable FTS3/FTS4 full-text search (used by XBMC for library search) */
#define SQLITE_ENABLE_FTS3 1
#define SQLITE_ENABLE_FTS3_PARENTHESIS 1

/* Enable R-Tree (spatial indexing) */
#define SQLITE_ENABLE_RTREE 1

/* Enable column metadata (used by dataset wrapper) */
#define SQLITE_ENABLE_COLUMN_METADATA 1

/* Enable the unlock-notify API */
#define SQLITE_ENABLE_UNLOCK_NOTIFY 1

/* Enable JSON support */
#define SQLITE_ENABLE_JSON1 1

/* ---------------------------------------------------------------- */
/*              Win32 API configuration for Xbox 360                */
/* ---------------------------------------------------------------- */

/* Xbox 360 uses ANSI (narrow char) Win32 APIs, not Unicode */
#define SQLITE_WIN32_NO_WIDE 1

/* GetVersionEx is available on Xbox 360 */
#define SQLITE_WIN32_GETVERSIONEX 1

/* Xbox 360 doesn't have FormatMessage (use simple error codes instead) */
/* The os_win.c code falls back gracefully when the SYSCALL table entry is 0 */

/* ---------------------------------------------------------------- */
/*              Memory / performance tuning                         */
/* ---------------------------------------------------------------- */

/* Default page size - 4096 is good for Xbox 360 HDD */
#define SQLITE_DEFAULT_PAGE_SIZE 4096

/* Default cache size (in pages): ~2MB */
#define SQLITE_DEFAULT_CACHE_SIZE -2000

/* Maximum variable number in SQL (default 999, keep reasonable) */
/* #define SQLITE_MAX_VARIABLE_NUMBER 999 */

/* ---------------------------------------------------------------- */
/*              Byte order - Xbox 360 is big-endian PPC             */
/* ---------------------------------------------------------------- */

/* SQLite auto-detects endianness at runtime via sqlite3IsNativelyBigEndian().
** On Xbox 360 PowerPC, we can hint for slightly better code:
*/
#define SQLITE_BYTEORDER 4321  /* big-endian */
#define SQLITE_BIGENDIAN 1
#define SQLITE_LITTLEENDIAN 0
#define SQLITE_UTF8_BIDI 0

/* ---------------------------------------------------------------- */
/*              Pointer size                                        */
/* ---------------------------------------------------------------- */

/* Xbox 360 has 32-bit pointers despite being a 64-bit PPC CPU.
** SQLite's auto-detection falls through to PTRSIZE=8, which makes
** the uptr typedef u64 and causes C4826 sign-extension warnings
** on every pointer-to-integer cast. */
#define SQLITE_PTRSIZE 4

/* ---------------------------------------------------------------- */
/*              Compiler compatibility                              */
/* ---------------------------------------------------------------- */

/* Xbox 360 XDK does not ship intrin.h - disable MSVC intrinsics */
#define SQLITE_DISABLE_INTRINSIC 1

/* Xbox 360 XDK does not support SEH (__try/__except) */
#define SQLITE_OMIT_SEH 1

/* MSVC is used via the XDK, but some CRT functions may differ */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

/* Xbox 360 XDK provides <windows.h> via xtl.h */
/* The XDK compiler defines _MSC_VER, _WIN32, etc. */

#endif /* SQLITE_CONFIG_XBOX360_H */
