/*
 * getpath_xbox360.c
 *
 * Minimal path resolution for Python on Xbox 360.
 * On Xbox 360 all paths are relative to the game content directory.
 */

#include "Python.h"
#include "osdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Default paths for Xbox 360 */
static wchar_t prefix[256] = L"game:\\python";
static wchar_t exec_prefix[256] = L"game:\\python";
static wchar_t progpath[256] = L"game:\\default.xex";
static wchar_t module_search_path[1024] = L"game:\\python\\Lib";

/* Only provide functions NOT already in pythonrun.c */

wchar_t *
Py_GetPrefix(void)
{
    return prefix;
}

wchar_t *
Py_GetExecPrefix(void)
{
    return exec_prefix;
}

wchar_t *
Py_GetPath(void)
{
    return module_search_path;
}

wchar_t *
Py_GetProgramFullPath(void)
{
    return progpath;
}

void
Py_SetPath(const wchar_t *path)
{
    if (path) {
        wcsncpy(module_search_path, path, 1023);
        module_search_path[1023] = L'\0';
    }
}

#ifdef __cplusplus
}
#endif
