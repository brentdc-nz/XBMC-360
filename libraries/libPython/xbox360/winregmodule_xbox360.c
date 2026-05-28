/*
 * Minimal winreg module for Xbox 360.
 * importlib bootstrap imports winreg when 'nt' is the OS module.
 * All registry operations raise OSError - no registry on Xbox 360.
 */

#include "Python.h"

/* Registry key constants - importlib references these */
#define HKEY_CURRENT_USER 0x80000001
#define HKEY_LOCAL_MACHINE 0x80000002

static PyObject*
winreg_OpenKey(PyObject *self, PyObject *args)
{
    PyErr_SetString(PyExc_OSError, "registry not available on Xbox 360");
    return NULL;
}

static PyObject*
winreg_QueryValue(PyObject *self, PyObject *args)
{
    PyErr_SetString(PyExc_OSError, "registry not available on Xbox 360");
    return NULL;
}

static PyObject*
winreg_CloseKey(PyObject *self, PyObject *args)
{
    Py_RETURN_NONE;
}

static PyMethodDef winreg_methods[] = {
    {"OpenKey",     winreg_OpenKey,    METH_VARARGS, NULL},
    {"OpenKeyEx",   winreg_OpenKey,    METH_VARARGS, NULL},
    {"QueryValue",  winreg_QueryValue, METH_VARARGS, NULL},
    {"CloseKey",    winreg_CloseKey,   METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef winregmodule = {
    PyModuleDef_HEAD_INIT,
    "winreg",
    NULL,
    -1,
    winreg_methods
};

PyObject*
PyInit_winreg(void)
{
    PyObject *m;

    m = PyModule_Create(&winregmodule);
    if (m == NULL)
        return NULL;

    PyModule_AddIntConstant(m, "HKEY_CURRENT_USER", (long)HKEY_CURRENT_USER);
    PyModule_AddIntConstant(m, "HKEY_LOCAL_MACHINE", (long)HKEY_LOCAL_MACHINE);

    return m;
}
