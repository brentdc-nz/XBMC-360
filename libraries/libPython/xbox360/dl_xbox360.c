/*
 * dl_xbox360.c
 *
 * Stub dynamic loading for Xbox 360.
 * Xbox 360 does not support dynamic module loading,
 * so all modules must be statically linked.
 */

#include "Python.h"

/* Dynamic loading is not supported on Xbox 360 */
const char *_PyImport_DynLoadFiletab[] = {NULL};

PyObject *
_PyImport_LoadDynamicModule(char *name, char *pathname, FILE *fp)
{
    PyErr_SetString(PyExc_ImportError,
        "dynamic module loading is not supported on Xbox 360");
    return NULL;
}
