/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

// Static-library implementation of CLibPython for Xbox 360.
// All methods forward directly to the CPython 3.4 C API (statically linked).

#include "LibPython.h"
#include "utils/log.h"

#include <stdarg.h>

bool CLibPython::m_bLoaded = false;

// ---------------------------------------------------------------------------
// Load / Unload (trivial for static lib â€” kept for interface parity)
// ---------------------------------------------------------------------------

bool CLibPython::Load()
{
	if (m_bLoaded)
		return true;

	CLog::Log(LOGINFO, "CLibPython: Static library ready (Python %s)", PY_VERSION);
	m_bLoaded = true;
	return true;
}

void CLibPython::Unload()
{
	m_bLoaded = false;
}

// ---------------------------------------------------------------------------
// Interpreter lifecycle
// ---------------------------------------------------------------------------

void CLibPython::Initialize()
{
	Py_Initialize();
}

void CLibPython::Finalize()
{
	Py_Finalize();
}

int CLibPython::IsInitialized()
{
	return Py_IsInitialized();
}

// ---------------------------------------------------------------------------
// Thread state
// ---------------------------------------------------------------------------

void CLibPython::InitThreads()
{
	PyEval_InitThreads();
}

PyThreadState* CLibPython::SaveThread()
{
	return PyEval_SaveThread();
}

void CLibPython::RestoreThread(PyThreadState* tstate)
{
	PyEval_RestoreThread(tstate);
}

PyThreadState* CLibPython::GetThreadState()
{
	return PyThreadState_Get();
}

PyThreadState* CLibPython::SwapThreadState(PyThreadState* tstate)
{
	return PyThreadState_Swap(tstate);
}

PyThreadState* CLibPython::NewThreadState(PyInterpreterState* interp)
{
	return PyThreadState_New(interp);
}

void CLibPython::ClearThreadState(PyThreadState* tstate)
{
	PyThreadState_Clear(tstate);
}

void CLibPython::DeleteThreadState(PyThreadState* tstate)
{
	PyThreadState_Delete(tstate);
}

PyGILState_STATE CLibPython::GILStateEnsure()
{
	return PyGILState_Ensure();
}

void CLibPython::GILStateRelease(PyGILState_STATE state)
{
	PyGILState_Release(state);
}

// ---------------------------------------------------------------------------
// Execution
// ---------------------------------------------------------------------------

int CLibPython::RunSimpleString(const char* command)
{
	return PyRun_SimpleString(command);
}

int CLibPython::RunSimpleFile(FILE* fp, const char* filename)
{
	return PyRun_SimpleFile(fp, filename);
}

PyObject* CLibPython::RunString(const char* str, int start, PyObject* globals, PyObject* locals)
{
	return PyRun_String(str, start, globals, locals);
}

PyObject* CLibPython::RunFile(FILE* fp, const char* filename, int start, PyObject* globals, PyObject* locals)
{
	return PyRun_File(fp, filename, start, globals, locals);
}

// ---------------------------------------------------------------------------
// Module / Import
// ---------------------------------------------------------------------------

PyObject* CLibPython::ImportModule(const char* name)
{
	return PyImport_ImportModule(name);
}

PyObject* CLibPython::ImportAddModule(const char* name)
{
	return PyImport_AddModule(name);
}

PyObject* CLibPython::ModuleGetDict(PyObject* module)
{
	return PyModule_GetDict(module);
}

int CLibPython::ModuleAddObject(PyObject* module, const char* name, PyObject* value)
{
	return PyModule_AddObject(module, name, value);
}

int CLibPython::ModuleAddIntConstant(PyObject* module, const char* name, long value)
{
	return PyModule_AddIntConstant(module, name, value);
}

int CLibPython::ModuleAddStringConstant(PyObject* module, const char* name, const char* value)
{
	return PyModule_AddStringConstant(module, name, value);
}

// ---------------------------------------------------------------------------
// Object protocol
// ---------------------------------------------------------------------------

PyObject* CLibPython::ObjectStr(PyObject* o)
{
	return PyObject_Str(o);
}

PyObject* CLibPython::ObjectRepr(PyObject* o)
{
	return PyObject_Repr(o);
}

PyObject* CLibPython::ObjectCallFunction(PyObject* callable, const char* format, ...)
{
	va_list va;
	va_start(va, format);
	PyObject* result = PyObject_CallFunction(callable, format, va);
	va_end(va);
	return result;
}

PyObject* CLibPython::ObjectCallMethod(PyObject* o, const char* method, const char* format, ...)
{
	va_list va;
	va_start(va, format);
	PyObject* result = PyObject_CallMethod(o, method, format, va);
	va_end(va);
	return result;
}

int CLibPython::ObjectRichCompareBool(PyObject* o1, PyObject* o2, int opid)
{
	return PyObject_RichCompareBool(o1, o2, opid);
}

// ---------------------------------------------------------------------------
// Error handling
// ---------------------------------------------------------------------------

void CLibPython::ErrSetString(PyObject* type, const char* message)
{
	PyErr_SetString(type, message);
}

PyObject* CLibPython::ErrOccurred()
{
	return PyErr_Occurred();
}

void CLibPython::ErrPrint()
{
	PyErr_Print();
}

void CLibPython::ErrFetch(PyObject** ptype, PyObject** pvalue, PyObject** ptraceback)
{
	PyErr_Fetch(ptype, pvalue, ptraceback);
}

void CLibPython::ErrClear()
{
	PyErr_Clear();
}

// ---------------------------------------------------------------------------
// Sys
// ---------------------------------------------------------------------------

void CLibPython::SysSetPath(const wchar_t* path)
{
	PySys_SetPath(path);
}

void CLibPython::SysSetArgv(int argc, wchar_t** argv)
{
	PySys_SetArgv(argc, argv);
}

// ---------------------------------------------------------------------------
// Build values
// ---------------------------------------------------------------------------

PyObject* CLibPython::BuildValue(const char* format, ...)
{
	va_list va;
	va_start(va, format);
	PyObject* result = Py_VaBuildValue(format, va);
	va_end(va);
	return result;
}

// ---------------------------------------------------------------------------
// List
// ---------------------------------------------------------------------------

PyObject* CLibPython::ListNew(Py_ssize_t len)
{
	return PyList_New(len);
}

Py_ssize_t CLibPython::ListSize(PyObject* list)
{
	return PyList_Size(list);
}

PyObject* CLibPython::ListGetItem(PyObject* list, Py_ssize_t index)
{
	return PyList_GetItem(list, index);
}

int CLibPython::ListAppend(PyObject* list, PyObject* item)
{
	return PyList_Append(list, item);
}

// ---------------------------------------------------------------------------
// Dict
// ---------------------------------------------------------------------------

PyObject* CLibPython::DictNew()
{
	return PyDict_New();
}

int CLibPython::DictSetItemString(PyObject* dp, const char* key, PyObject* item)
{
	return PyDict_SetItemString(dp, key, item);
}

PyObject* CLibPython::DictGetItemString(PyObject* dp, const char* key)
{
	return PyDict_GetItemString(dp, key);
}

Py_ssize_t CLibPython::DictSize(PyObject* dp)
{
	return PyDict_Size(dp);
}

int CLibPython::DictNext(PyObject* dp, Py_ssize_t* ppos, PyObject** pkey, PyObject** pvalue)
{
	return PyDict_Next(dp, ppos, pkey, pvalue);
}

// ---------------------------------------------------------------------------
// Long
// ---------------------------------------------------------------------------

PyObject* CLibPython::LongFromLong(long v)
{
	return PyLong_FromLong(v);
}

long CLibPython::LongAsLong(PyObject* obj)
{
	return PyLong_AsLong(obj);
}

long long CLibPython::LongAsLongLong(PyObject* obj)
{
	return PyLong_AsLongLong(obj);
}

// ---------------------------------------------------------------------------
// Float
// ---------------------------------------------------------------------------

PyObject* CLibPython::FloatFromDouble(double v)
{
	return PyFloat_FromDouble(v);
}

double CLibPython::FloatAsDouble(PyObject* obj)
{
	return PyFloat_AsDouble(obj);
}

// ---------------------------------------------------------------------------
// Unicode (Python 3 â€” replaces PyString)
// ---------------------------------------------------------------------------

PyObject* CLibPython::UnicodeFromString(const char* u)
{
	return PyUnicode_FromString(u);
}

const char* CLibPython::UnicodeAsUTF8(PyObject* unicode)
{
	return PyUnicode_AsUTF8(unicode);
}

// ---------------------------------------------------------------------------
// Tuple
// ---------------------------------------------------------------------------

PyObject* CLibPython::TupleNew(Py_ssize_t len)
{
	return PyTuple_New(len);
}

int CLibPython::TupleSetItem(PyObject* p, Py_ssize_t pos, PyObject* o)
{
	return PyTuple_SetItem(p, pos, o);
}

PyObject* CLibPython::TupleGetItem(PyObject* p, Py_ssize_t pos)
{
	return PyTuple_GetItem(p, pos);
}

// ---------------------------------------------------------------------------
// Type
// ---------------------------------------------------------------------------

int CLibPython::TypeReady(PyTypeObject* type)
{
	return PyType_Ready(type);
}

int CLibPython::TypeIsSubtype(PyTypeObject* a, PyTypeObject* b)
{
	return PyType_IsSubtype(a, b);
}

// ---------------------------------------------------------------------------
// Misc
// ---------------------------------------------------------------------------

int CLibPython::AddPendingCall(int (*func)(void*), void* arg)
{
	return Py_AddPendingCall(func, arg);
}

int CLibPython::MakePendingCalls()
{
	return Py_MakePendingCalls();
}
