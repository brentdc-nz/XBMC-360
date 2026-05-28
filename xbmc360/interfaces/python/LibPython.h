#pragma once

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

// Static-library wrapper for CPython 3.4 on Xbox 360.
// Replaces the DLL-loading shim (XBPythonDll) used in xbmc4xbox.
// All calls go directly to the statically-linked python34.lib.

#include "Python.h"

class CLibPython
{
public:
	static bool Load();
	static void Unload();
	static bool IsLoaded() { return m_bLoaded; }

	// --- Interpreter lifecycle ---
	static void          Initialize();
	static void          Finalize();
	static int           IsInitialized();

	// --- Thread state ---
	static void          InitThreads();
	static PyThreadState* SaveThread();
	static void          RestoreThread(PyThreadState* tstate);
	static PyThreadState* GetThreadState();
	static PyThreadState* SwapThreadState(PyThreadState* tstate);
	static PyThreadState* NewThreadState(PyInterpreterState* interp);
	static void          ClearThreadState(PyThreadState* tstate);
	static void          DeleteThreadState(PyThreadState* tstate);
	static PyGILState_STATE GILStateEnsure();
	static void          GILStateRelease(PyGILState_STATE state);

	// --- Execution ---
	static int           RunSimpleString(const char* command);
	static int           RunSimpleFile(FILE* fp, const char* filename);
	static PyObject*     RunString(const char* str, int start, PyObject* globals, PyObject* locals);
	static PyObject*     RunFile(FILE* fp, const char* filename, int start, PyObject* globals, PyObject* locals);

	// --- Module / Import ---
	static PyObject*     ImportModule(const char* name);
	static PyObject*     ImportAddModule(const char* name);
	static PyObject*     ModuleGetDict(PyObject* module);
	static int           ModuleAddObject(PyObject* module, const char* name, PyObject* value);
	static int           ModuleAddIntConstant(PyObject* module, const char* name, long value);
	static int           ModuleAddStringConstant(PyObject* module, const char* name, const char* value);

	// --- Object protocol ---
	static PyObject*     ObjectStr(PyObject* o);
	static PyObject*     ObjectRepr(PyObject* o);
	static PyObject*     ObjectCallFunction(PyObject* callable, const char* format, ...);
	static PyObject*     ObjectCallMethod(PyObject* o, const char* method, const char* format, ...);
	static int           ObjectRichCompareBool(PyObject* o1, PyObject* o2, int opid);

	// --- Error handling ---
	static void          ErrSetString(PyObject* type, const char* message);
	static PyObject*     ErrOccurred();
	static void          ErrPrint();
	static void          ErrFetch(PyObject** ptype, PyObject** pvalue, PyObject** ptraceback);
	static void          ErrClear();

	// --- Sys ---
	static void          SysSetPath(const wchar_t* path);
	static void          SysSetArgv(int argc, wchar_t** argv);

	// --- Build values / Parse args ---
	static PyObject*     BuildValue(const char* format, ...);

	// --- List ---
	static PyObject*     ListNew(Py_ssize_t len);
	static Py_ssize_t    ListSize(PyObject* list);
	static PyObject*     ListGetItem(PyObject* list, Py_ssize_t index);
	static int           ListAppend(PyObject* list, PyObject* item);

	// --- Dict ---
	static PyObject*     DictNew();
	static int           DictSetItemString(PyObject* dp, const char* key, PyObject* item);
	static PyObject*     DictGetItemString(PyObject* dp, const char* key);
	static Py_ssize_t    DictSize(PyObject* dp);
	static int           DictNext(PyObject* dp, Py_ssize_t* ppos, PyObject** pkey, PyObject** pvalue);

	// --- Long ---
	static PyObject*     LongFromLong(long v);
	static long          LongAsLong(PyObject* obj);
	static long long     LongAsLongLong(PyObject* obj);

	// --- Float ---
	static PyObject*     FloatFromDouble(double v);
	static double        FloatAsDouble(PyObject* obj);

	// --- Unicode (replaces PyString in Python 3) ---
	static PyObject*     UnicodeFromString(const char* u);
	static const char*   UnicodeAsUTF8(PyObject* unicode);

	// --- Tuple ---
	static PyObject*     TupleNew(Py_ssize_t len);
	static int           TupleSetItem(PyObject* p, Py_ssize_t pos, PyObject* o);
	static PyObject*     TupleGetItem(PyObject* p, Py_ssize_t pos);

	// --- Type ---
	static int           TypeReady(PyTypeObject* type);
	static int           TypeIsSubtype(PyTypeObject* a, PyTypeObject* b);

	// --- Misc ---
	static int           AddPendingCall(int (*func)(void*), void* arg);
	static int           MakePendingCalls();

private:
	static bool m_bLoaded;
};
