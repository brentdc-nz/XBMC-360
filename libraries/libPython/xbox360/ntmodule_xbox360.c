/*
 * Minimal 'nt' module for Xbox 360 - provides just enough for
 * importlib bootstrap to succeed. Python 3's _frozen_importlib
 * requires either 'posix' or 'nt' as a builtin module.
 *
 * Functions needed by importlib: stat, getcwd, fspath, listdir
 */

#include "Python.h"
#include <xtl.h>
#include <string.h>

/* Minimal stat result - importlib checks st_mode for files/dirs */
static PyObject*
nt_stat(PyObject *self, PyObject *args, PyObject *kwargs)
{
    const char *path = NULL;
    PyObject *path_obj = NULL;
    static char *kwlist[] = {"path", "dir_fd", "follow_symlinks", NULL};
    PyObject *dir_fd = Py_None;
    int follow_symlinks = 1;
    DWORD attrs;
    unsigned int mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&|Oi", kwlist,
                                     PyUnicode_FSConverter, &path_obj,
                                     &dir_fd, &follow_symlinks))
        return NULL;

    path = PyBytes_AS_STRING(path_obj);
    attrs = GetFileAttributesA(path);

    if (attrs == (DWORD)-1) {
        PyErr_Format(PyExc_FileNotFoundError,
                     "[Errno 2] No such file or directory: '%s'", path);
        Py_DECREF(path_obj);
        return NULL;
    }
    Py_DECREF(path_obj);

    /* st_mode: 0x4000 (S_IFDIR) for dirs, 0x8000 (S_IFREG) for files */
    mode = (attrs & FILE_ATTRIBUTE_DIRECTORY) ? 0x4000 | 0x1FF : 0x8000 | 0x1FF;

    /* stat_result: (mode, ino, dev, nlink, uid, gid, size, atime, mtime, ctime) */
    return Py_BuildValue("(Ikkkkkkkkk)",
                         mode,
                         (unsigned long)0,
                         (unsigned long)0,
                         (unsigned long)1,
                         (unsigned long)0,
                         (unsigned long)0,
                         (unsigned long)0,
                         (unsigned long)0,
                         (unsigned long)0,
                         (unsigned long)0);
}

static PyObject*
nt_getcwd(PyObject *self, PyObject *noargs)
{
    return PyUnicode_FromString("D:\\");
}

static PyObject*
nt_listdir(PyObject *self, PyObject *args, PyObject *kwargs)
{
    /* Return empty list - no dynamic file import on Xbox 360 */
    return PyList_New(0);
}

static PyObject*
nt_fspath(PyObject *self, PyObject *path)
{
    if (PyUnicode_Check(path)) {
        Py_INCREF(path);
        return path;
    }
    if (PyBytes_Check(path)) {
        Py_INCREF(path);
        return path;
    }
    PyErr_Format(PyExc_TypeError,
                 "expected str or bytes, not %.200s",
                 Py_TYPE(path)->tp_name);
    return NULL;
}

static PyObject*
nt_urandom(PyObject *self, PyObject *args)
{
    Py_ssize_t size;
    PyObject *bytes;
    unsigned char *buf;
    LARGE_INTEGER counter;
    unsigned int state;
    Py_ssize_t i;

    if (!PyArg_ParseTuple(args, "n:urandom", &size))
        return NULL;
    if (size < 0) {
        PyErr_SetString(PyExc_ValueError, "negative argument not allowed");
        return NULL;
    }

    bytes = PyBytes_FromStringAndSize(NULL, size);
    if (bytes == NULL)
        return NULL;

    buf = (unsigned char*)PyBytes_AS_STRING(bytes);
    QueryPerformanceCounter(&counter);
    state = (unsigned int)(counter.QuadPart);
    if (state == 0) state = 0xDEADBEEF;

    for (i = 0; i < size; i++) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        buf[i] = (unsigned char)(state & 0xFF);
    }

    return bytes;
}

static PyMethodDef nt_methods[] = {
    {"stat",    (PyCFunction)nt_stat,    METH_VARARGS | METH_KEYWORDS, NULL},
    {"getcwd",  nt_getcwd,               METH_NOARGS,  NULL},
    {"listdir", (PyCFunction)nt_listdir, METH_VARARGS | METH_KEYWORDS, NULL},
    {"fspath",  nt_fspath,               METH_O,       NULL},
    {"urandom", nt_urandom,              METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef ntmodule = {
    PyModuleDef_HEAD_INIT,
    "nt",
    NULL,
    -1,
    nt_methods
};

PyObject*
PyInit_nt(void)
{
    PyObject *m;
    PyObject *env;

    m = PyModule_Create(&ntmodule);
    if (m == NULL)
        return NULL;

    /* Add path separator constants needed by importlib */
    PyModule_AddStringConstant(m, "sep", "\\");
    PyModule_AddStringConstant(m, "altsep", "/");
    PyModule_AddStringConstant(m, "extsep", ".");
    PyModule_AddStringConstant(m, "pathsep", ";");
    PyModule_AddStringConstant(m, "linesep", "\r\n");
    PyModule_AddStringConstant(m, "devnull", "nul");
    PyModule_AddStringConstant(m, "curdir", ".");
    PyModule_AddStringConstant(m, "pardir", "..");
    PyModule_AddStringConstant(m, "name", "nt");

    /* environ - empty dict */
    env = PyDict_New();
    if (env) {
        PyModule_AddObject(m, "environ", env);
    }

    return m;
}
