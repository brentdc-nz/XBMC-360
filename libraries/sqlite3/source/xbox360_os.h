/*
** Xbox 360 OS layer header for SQLite
**
** This replaces os_win.h on the Xbox 360 platform. It includes the
** XDK header (xtl.h), provides missing Win32 types and constants,
** and supplies stub implementations for APIs that the XDK does not
** provide but os_win.c requires (as function pointers in the SYSCALL
** table, so these must be real functions, not macros).
**
** Included by os_win.c, mutex_w32.c, and threads.c when _XBOX is defined.
*/
#ifndef SQLITE_XBOX360_OS_H
#define SQLITE_XBOX360_OS_H

/* ================================================================ */
/* Platform SDK                                                     */
/* ================================================================ */
#include <xtl.h>
#include <string.h>  /* memset, strlen, strcpy, strrchr */

/* Suppress "benign redefinition of type" for types that may already
** exist in XDK headers with identical definitions.                 */
#pragma warning(push)
#pragma warning(disable: 4142)

/* ================================================================ */
/* Platform detection macros (matching os_win.h)                    */
/* ================================================================ */

#if SQLITE_OS_WIN && !defined(SQLITE_OS_WINNT)
# define SQLITE_OS_WINNT 1
#endif

/* Xbox 360 is not WinCE */
#define SQLITE_OS_WINCE 0

#define SQLITE_WIN32_VOLATILE volatile

/* Xbox 360 supports threads */
#if SQLITE_OS_WIN && SQLITE_THREADSAFE>0
# define SQLITE_OS_WIN_THREADS 1
#else
# define SQLITE_OS_WIN_THREADS 0
#endif

/* ================================================================ */
/* Types missing from Xbox 360 XDK                                  */
/* ================================================================ */

/* OSVERSIONINFOA - not in Xbox 360 XDK */
typedef struct _SQLITE_OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR  szCSDVersion[128];
} OSVERSIONINFOA, *LPOSVERSIONINFOA;

/* SYSTEM_INFO - not in Xbox 360 title-mode headers
** (only DM_SYSTEM_INFO in xbdm.h for debug builds) */
typedef struct _SQLITE_SYSTEM_INFO {
    union {
        DWORD dwOemId;
        struct {
            WORD wProcessorArchitecture;
            WORD wReserved;
        };
    };
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

/* ================================================================ */
/* Constants missing from Xbox 360 XDK                              */
/* ================================================================ */

#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT 2
#endif

#ifndef CP_OEMCP
#define CP_OEMCP 1
#endif

#ifndef CP_ACP
#define CP_ACP 0
#endif

#ifndef FORMAT_MESSAGE_ALLOCATE_BUFFER
#define FORMAT_MESSAGE_ALLOCATE_BUFFER  0x00000100
#endif
#ifndef FORMAT_MESSAGE_FROM_SYSTEM
#define FORMAT_MESSAGE_FROM_SYSTEM      0x00001000
#endif
#ifndef FORMAT_MESSAGE_IGNORE_INSERTS
#define FORMAT_MESSAGE_IGNORE_INSERTS   0x00000200
#endif

#ifndef LOCKFILE_FAIL_IMMEDIATELY
#define LOCKFILE_FAIL_IMMEDIATELY  0x00000001
#endif
#ifndef LOCKFILE_EXCLUSIVE_LOCK
#define LOCKFILE_EXCLUSIVE_LOCK    0x00000002
#endif

/* ================================================================ */
/* Function stubs for missing Win32 APIs                            */
/* These are real functions so os_win.c can take their address      */
/* for the SYSCALL function-pointer table.                          */
/* ================================================================ */

/* AreFileApisANSI - Xbox 360 always ANSI */
static BOOL WINAPI AreFileApisANSI(void)
{
    return TRUE;
}

/* GetCurrentProcessId - Xbox 360 is single-process */
static DWORD WINAPI GetCurrentProcessId(void)
{
    return (DWORD)1;
}

/* GetVersionExA - report as NT-family OS */
static BOOL WINAPI GetVersionExA(LPOSVERSIONINFOA lpVersionInfo)
{
    if (lpVersionInfo) {
        memset(lpVersionInfo, 0, sizeof(*lpVersionInfo));
        lpVersionInfo->dwOSVersionInfoSize = sizeof(*lpVersionInfo);
        lpVersionInfo->dwMajorVersion = 5;
        lpVersionInfo->dwMinorVersion = 0;
        lpVersionInfo->dwPlatformId = VER_PLATFORM_WIN32_NT;
    }
    return TRUE;
}

/* GetFullPathNameA - pass through, Xbox 360 paths are already absolute */
static DWORD WINAPI GetFullPathNameA(
    LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, LPSTR *lpFilePart
){
    DWORD len;
    if (!lpFileName) return 0;
    len = (DWORD)strlen(lpFileName);
    if (lpBuffer && nBufferLength > len) {
        strcpy(lpBuffer, lpFileName);
        if (lpFilePart) {
            char *p = strrchr(lpBuffer, '\\');
            char *p2 = strrchr(lpBuffer, '/');
            if (p2 > p) p = p2;
            *lpFilePart = p ? p + 1 : lpBuffer;
        }
    }
    return len;
}

/* GetTempPathA - fixed temp path for Xbox 360 (cache: partition for temp files) */
static DWORD WINAPI GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer)
{
    static const char tempPath[] = "cache:\\temp\\";
    DWORD len = (DWORD)(sizeof(tempPath) - 1);
    if (lpBuffer && nBufferLength > len) {
        strcpy(lpBuffer, tempPath);
    }
    return len;
}

/* GetDiskFreeSpaceA - stub: report generous defaults */
static BOOL WINAPI GetDiskFreeSpaceA(
    LPCSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
){
    (void)lpRootPathName;
    if (lpSectorsPerCluster)      *lpSectorsPerCluster = 8;
    if (lpBytesPerSector)         *lpBytesPerSector = 512;
    if (lpNumberOfFreeClusters)   *lpNumberOfFreeClusters = 1024 * 1024;
    if (lpTotalNumberOfClusters)  *lpTotalNumberOfClusters = 2 * 1024 * 1024;
    return TRUE;
}

/* GetSystemInfo - not in Xbox 360 title headers */
static void WINAPI GetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
    if (lpSystemInfo) {
        memset(lpSystemInfo, 0, sizeof(*lpSystemInfo));
        lpSystemInfo->dwPageSize = 4096;
        lpSystemInfo->dwAllocationGranularity = 65536;
        lpSystemInfo->dwNumberOfProcessors = 6;  /* 3 cores x 2 HW threads */
    }
}

/* GetNativeSystemInfo - forward to GetSystemInfo */
static void WINAPI GetNativeSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
    GetSystemInfo(lpSystemInfo);
}

/* FormatMessageA - stub: return empty string */
static DWORD WINAPI FormatMessageA(
    DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId,
    DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list *Arguments
){
    (void)dwFlags; (void)lpSource; (void)dwMessageId;
    (void)dwLanguageId; (void)Arguments;
    if (lpBuffer && nSize > 0) lpBuffer[0] = '\0';
    return 0;
}

/* FormatMessageW - stub: return empty string */
static DWORD WINAPI FormatMessageW(
    DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId,
    DWORD dwLanguageId, LPWSTR lpBuffer, DWORD nSize, va_list *Arguments
){
    (void)dwFlags; (void)lpSource; (void)dwMessageId;
    (void)dwLanguageId; (void)Arguments;
    if (lpBuffer && nSize > 0) lpBuffer[0] = L'\0';
    return 0;
}

/* ================================================================ */
/* File locking stubs                                               */
/* Xbox 360 is single-title, so file locking is unnecessary.        */
/* ================================================================ */

static BOOL WINAPI LockFile(
    HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToLockLow, DWORD nNumberOfBytesToLockHigh
){
    (void)hFile; (void)dwFileOffsetLow; (void)dwFileOffsetHigh;
    (void)nNumberOfBytesToLockLow; (void)nNumberOfBytesToLockHigh;
    return TRUE;
}

static BOOL WINAPI UnlockFile(
    HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToUnlockLow, DWORD nNumberOfBytesToUnlockHigh
){
    (void)hFile; (void)dwFileOffsetLow; (void)dwFileOffsetHigh;
    (void)nNumberOfBytesToUnlockLow; (void)nNumberOfBytesToUnlockHigh;
    return TRUE;
}

static BOOL WINAPI LockFileEx(
    HANDLE hFile, DWORD dwFlags, DWORD dwReserved,
    DWORD nNumberOfBytesToLockLow, DWORD nNumberOfBytesToLockHigh,
    LPOVERLAPPED lpOverlapped
){
    (void)hFile; (void)dwFlags; (void)dwReserved;
    (void)nNumberOfBytesToLockLow; (void)nNumberOfBytesToLockHigh;
    (void)lpOverlapped;
    return TRUE;
}

static BOOL WINAPI UnlockFileEx(
    HANDLE hFile, DWORD dwReserved,
    DWORD nNumberOfBytesToUnlockLow, DWORD nNumberOfBytesToUnlockHigh,
    LPOVERLAPPED lpOverlapped
){
    (void)hFile; (void)dwReserved;
    (void)nNumberOfBytesToUnlockLow; (void)nNumberOfBytesToUnlockHigh;
    (void)lpOverlapped;
    return TRUE;
}

/* ================================================================ */
/* Heap validation/compaction stubs                                 */
/* Xbox 360 XDK does not provide HeapValidate or HeapCompact.       */
/* ================================================================ */

static BOOL WINAPI HeapValidate(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem)
{
    (void)hHeap; (void)dwFlags; (void)lpMem;
    return TRUE;
}

static UINT WINAPI HeapCompact(HANDLE hHeap, DWORD dwFlags)
{
    (void)hHeap; (void)dwFlags;
    return 0;
}

/* ================================================================ */
/* Memory-mapped files - not available on Xbox 360                  */
/* Disabled via SQLITE_MAX_MMAP_SIZE=0; stubs as safety net         */
/* ================================================================ */

static HANDLE WINAPI CreateFileMappingA(
    HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttr, DWORD flProtect,
    DWORD dwMaxHigh, DWORD dwMaxLow, LPCSTR lpName
){
    (void)hFile; (void)lpAttr; (void)flProtect;
    (void)dwMaxHigh; (void)dwMaxLow; (void)lpName;
    return NULL;
}

static HANDLE WINAPI CreateFileMappingW(
    HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttr, DWORD flProtect,
    DWORD dwMaxHigh, DWORD dwMaxLow, LPCWSTR lpName
){
    (void)hFile; (void)lpAttr; (void)flProtect;
    (void)dwMaxHigh; (void)dwMaxLow; (void)lpName;
    return NULL;
}

static LPVOID WINAPI MapViewOfFile(
    HANDLE hMapping, DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap
){
    (void)hMapping; (void)dwDesiredAccess;
    (void)dwFileOffsetHigh; (void)dwFileOffsetLow; (void)dwNumberOfBytesToMap;
    return NULL;
}

static BOOL WINAPI UnmapViewOfFile(LPCVOID lpBaseAddress)
{
    (void)lpBaseAddress;
    return FALSE;
}

#pragma warning(pop)

#endif /* SQLITE_XBOX360_OS_H */
