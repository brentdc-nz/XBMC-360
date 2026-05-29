/* Stub implementations for Xbox 360 platform - resolves linker errors
   for symbols not available on Xbox 360 or not compiled into python34.lib */

#include <xtl.h>
#include <math.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* === C math stubs === */

int finite(double x)
{
    return _finite(x);
}

double copysign(double x, double y)
{
    return _copysign(x, y);
}

double round(double x)
{
    return (x >= 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
}

int __control87_2(unsigned int newval, unsigned int mask,
                  unsigned int *x86_cw, unsigned int *sse2_cw)
{
    /* No FPU control word manipulation on Xbox 360 PPC */
    if (x86_cw) *x86_cw = 0;
    if (sse2_cw) *sse2_cw = 0;
    return 1;
}

/* === Win32 API stubs not available on Xbox 360 === */

unsigned int __stdcall GetACP(void)
{
    return 65001; /* UTF-8 */
}

unsigned int __stdcall GetConsoleCP(void)
{
    return 65001;
}

unsigned int __stdcall GetConsoleOutputCP(void)
{
    return 65001;
}

char* getenv(const char* name)
{
    return NULL;
}

unsigned long __stdcall GetFullPathNameW(const wchar_t* lpFileName,
    unsigned long nBufferLength, wchar_t* lpBuffer, wchar_t** lpFilePart)
{
    size_t len;
    if (!lpFileName) return 0;
    len = wcslen(lpFileName);
    if (lpBuffer && nBufferLength > len)
    {
        wcscpy(lpBuffer, lpFileName);
        if (lpFilePart) *lpFilePart = NULL;
        return (unsigned long)len;
    }
    return (unsigned long)(len + 1);
}

void* __stdcall GetModuleHandleW(const wchar_t* lpModuleName)
{
    return NULL;
}

int __stdcall GetProcessTimes(void* hProcess, void* lpCreationTime,
    void* lpExitTime, void* lpKernelTime, void* lpUserTime)
{
    if (lpCreationTime) memset(lpCreationTime, 0, 8);
    if (lpExitTime) memset(lpExitTime, 0, 8);
    if (lpKernelTime) memset(lpKernelTime, 0, 8);
    if (lpUserTime) memset(lpUserTime, 0, 8);
    return 1;
}

int __stdcall GetSystemTimeAdjustment(unsigned long* lpTimeAdjustment,
    unsigned long* lpTimeIncrement, int* lpTimeAdjustmentDisabled)
{
    if (lpTimeAdjustment) *lpTimeAdjustment = 0;
    if (lpTimeIncrement) *lpTimeIncrement = 10000;
    if (lpTimeAdjustmentDisabled) *lpTimeAdjustmentDisabled = 1;
    return 1;
}

int __stdcall IsDBCSLeadByteEx(unsigned int CodePage, unsigned char TestChar)
{
    return 0;
}

char* __stdcall CharPrevExA(unsigned short CodePage, const char* lpStart,
    const char* lpCurrentChar, unsigned long dwFlags)
{
    if (lpCurrentChar > lpStart)
        return (char*)(lpCurrentChar - 1);
    return (char*)lpStart;
}

wchar_t* _wgetcwd(wchar_t* buffer, int maxlen)
{
    if (buffer && maxlen > 3)
    {
        wcscpy(buffer, L"D:\\");
        return buffer;
    }
    return NULL;
}

/* === Python internal stubs (modules not compiled into lib) === */

typedef struct _object PyObject;

/* Debug-only consistency check - always report OK */
int _PyUnicode_CheckConsistency(PyObject *op, int check_content) { return 1; }

int PyErr_CheckSignals(void) { return 0; }
void PyErr_SetInterrupt(void) {}
void PyOS_InitInterrupts(void) {}
void PyOS_FiniInterrupts(void) {}
int PyOS_InterruptOccurred(void) { return 0; }
int _PyOS_IsMainThread(void) { return 1; }

/* _PyOS_SigintEvent - on Windows this is a HANDLE (event object) */
void* _PyOS_SigintEvent = NULL;

void _PyFaulthandler_Init(void) {}
void _PyFaulthandler_Fini(void) {}
int _PyTraceMalloc_Init(void) { return 0; }
void _PyTraceMalloc_Fini(void) {}
int _PyVerify_fd(int fd) { return 1; }

const char* Py_GetBuildInfo(void)
{
    return "3.4.10 (Xbox 360 static build)";
}

const char* _Py_hgidentifier(void) { return ""; }
const char* _Py_hgversion(void) { return ""; }

/* === GetCurrentProcessId - not available on Xbox 360 === */
unsigned long __stdcall GetCurrentProcessId(void)
{
    return 1;
}

/* === Expat LoadLibrary stub (pyexpat uses this for hash salt) === */
void* _Expat_LoadLibrary(const char* name)
{
    return NULL;
}

/* === sqlite3 functions that may be compiled out (SQLITE_OMIT_*) === */
int sqlite3_enable_shared_cache(int enable)
{
    return 0; /* SQLITE_OK */
}

void* sqlite3_trace(void* db, void (*xTrace)(void*,const char*), void* pArg)
{
    return NULL;
}

int sqlite3_enable_load_extension(void* db, int onoff)
{
    return 1; /* SQLITE_ERROR - not supported */
}

int sqlite3_load_extension(void* db, const char* zFile, const char* zProc, char** pzErrMsg)
{
    if (pzErrMsg) *pzErrMsg = NULL;
    return 1; /* SQLITE_ERROR - not supported */
}

int sqlite3_transfer_bindings(void* pFromStmt, void* pToStmt)
{
    return 21; /* SQLITE_MISUSE - deprecated function */
}

#ifdef __cplusplus
}
#endif
