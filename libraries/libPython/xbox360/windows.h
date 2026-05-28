/*
 * windows.h shim for Xbox 360
 *
 * Redirects to xtl.h which provides the Windows-like API on Xbox 360.
 * This allows code that includes <windows.h> to compile on Xbox 360.
 */

#ifndef _XBOX360_WINDOWS_H_SHIM
#define _XBOX360_WINDOWS_H_SHIM

#include <xtl.h>

/* Xbox 360 xtl.h should provide most Win32 threading APIs:
 * CreateThread, WaitForSingleObject, CRITICAL_SECTION, etc.
 * Plus HANDLE, DWORD, BOOL, TRUE, FALSE, etc.
 */

/* Ensure NTDDI_VERSION is not set to Vista+ on Xbox 360 */
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x05010000  /* XP level */
#endif

/* ====================================================================
 * Missing Win32 constants that Xbox 360 xtl.h doesn't define
 * ==================================================================== */

/* FormatMessage flags (errors.c) */
#ifndef FORMAT_MESSAGE_ALLOCATE_BUFFER
#define FORMAT_MESSAGE_ALLOCATE_BUFFER  0x00000100
#endif
#ifndef FORMAT_MESSAGE_FROM_SYSTEM
#define FORMAT_MESSAGE_FROM_SYSTEM      0x00001000
#endif
#ifndef FORMAT_MESSAGE_IGNORE_INSERTS
#define FORMAT_MESSAGE_IGNORE_INSERTS   0x00000200
#endif

/* File handle flags (fileutils.c) */
#ifndef HANDLE_FLAG_INHERIT
#define HANDLE_FLAG_INHERIT  0x00000001
#endif
#ifndef FILE_TYPE_CHAR
#define FILE_TYPE_CHAR  0x0002
#endif
#ifndef FILE_TYPE_PIPE
#define FILE_TYPE_PIPE  0x0003
#endif

/* Stub FormatMessageW if not available */
#ifndef FormatMessageW
#define FormatMessageW(flags, src, msgId, langId, buf, size, args) (0)
#endif

/* CryptoAPI stubs (random.c) */
#ifndef HCRYPTPROV
typedef unsigned long HCRYPTPROV;
#endif
#ifndef PROV_RSA_FULL
#define PROV_RSA_FULL  1
#endif
#ifndef CRYPT_VERIFYCONTEXT
#define CRYPT_VERIFYCONTEXT  0xF0000000
#endif
#ifndef CryptAcquireContext
#define CryptAcquireContext(a,b,c,d,e)  (FALSE)
#endif
#ifndef CryptGenRandom
#define CryptGenRandom(a,b,c)  (FALSE)
#endif
#ifndef CryptReleaseContext
#define CryptReleaseContext(a,b)  (FALSE)
#endif

/* Version info (sysmodule.c) */
#ifndef OSVERSIONINFOEX
typedef struct _OSVERSIONINFOEXA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    char szCSDVersion[128];
    WORD wServicePackMajor;
    WORD wServicePackMinor;
    WORD wSuiteMask;
    BYTE wProductType;
    BYTE wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;
typedef OSVERSIONINFOEXA OSVERSIONINFOEX;
typedef OSVERSIONINFOEXA *POSVERSIONINFOEX;
typedef OSVERSIONINFOEXA *LPOSVERSIONINFOEX;
#endif
#ifndef GetVersionEx
#define GetVersionEx(a)  (FALSE)
#endif
#ifndef VER_NT_WORKSTATION
#define VER_NT_WORKSTATION  1
#endif

/* SetHandleInformation stub */
#ifndef SetHandleInformation
#define SetHandleInformation(h, mask, flags)  (FALSE)
#endif
#ifndef GetHandleInformation
#define GetHandleInformation(h, flags)  (FALSE)
#endif

/* GetFileType stub */
#ifndef GetFileType
#define GetFileType(h)  (0)
#endif

/* Virtual memory constants (obmalloc.c) */
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

/* Code page constants (unicodeobject.c) */
#ifndef CP_UTF7
#define CP_UTF7  65000
#endif
#ifndef CP_UTF8
#define CP_UTF8  65001
#endif
#ifndef MB_ERR_INVALID_CHARS
#define MB_ERR_INVALID_CHARS  0x00000008
#endif
#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS  0x00000400
#endif

#endif /* _XBOX360_WINDOWS_H_SHIM */
