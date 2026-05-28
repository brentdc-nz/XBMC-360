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

#include "XBPyThread.h"
#include "XBPython.h"
#include "utils/log.h"

#include <string.h>

// Trace function used to interrupt scripts via KeyboardInterrupt
static int xbTrace(PyObject* /*obj*/, struct _frame* /*frame*/, int /*what*/, PyObject* /*arg*/)
{
	PyErr_SetString(PyExc_KeyboardInterrupt, "script interrupted by user\n");
	return -1;
}

XBPyThread::XBPyThread(XBPython* pExecuter, PyThreadState* mainThreadState, int id)
{
	CLog::Log(LOGDEBUG, "XBPyThread: new python thread created. id=%d", id);
	m_pExecuter = pExecuter;
	m_id = id;
	m_bDone = false;
	m_bStopping = false;
	m_argv = NULL;
	m_source = NULL;
	m_argc = 0;
	m_type = 0;

	// Restore the main thread state to acquire GIL, create new thread state
	PyEval_RestoreThread(mainThreadState);
	m_threadState = PyThreadState_New(mainThreadState->interp);
	PyEval_SaveThread();
}

XBPyThread::~XBPyThread()
{
	CLog::Log(LOGDEBUG, "XBPyThread: python thread %d destructed", m_id);
	delete[] m_source;
	if (m_argv)
	{
		for (unsigned int i = 0; i < m_argc; i++)
			delete[] m_argv[i];
		delete[] m_argv;
	}
}

int XBPyThread::evalFile(const char* src)
{
	m_type = 'F';
	m_source = new char[strlen(src) + 1];
	strcpy(m_source, src);
	Create();
	return 0;
}

int XBPyThread::evalString(const char* src)
{
	m_type = 'S';
	m_source = new char[strlen(src) + 1];
	strcpy(m_source, src);
	Create();
	return 0;
}

int XBPyThread::setArgv(unsigned int src_argc, const char** src)
{
	if (src == NULL)
		return 1;
	m_argc = src_argc;
	m_argv = new char*[m_argc];
	for (unsigned int i = 0; i < m_argc; i++)
	{
		m_argv[i] = new char[strlen(src[i]) + 1];
		strcpy(m_argv[i], src[i]);
	}
	return 0;
}

void XBPyThread::stop()
{
	m_bStopping = true;
	// Set the trace function to trigger KeyboardInterrupt
	PyEval_RestoreThread(m_threadState);
	PyEval_SetTrace(xbTrace, NULL);
	PyEval_SaveThread();
}

void XBPyThread::OnStartup() {}

void XBPyThread::Process()
{
	CLog::Log(LOGDEBUG, "XBPyThread: start processing");

	int m_Py_file_input = Py_file_input;

	// Acquire GIL and swap in our thread state
	PyEval_RestoreThread(m_threadState);

	CLog::Log(LOGDEBUG, "XBPyThread: source to load is %s", m_source);

	// Set sys.argv if provided (Python 3 uses wchar_t)
	if (m_argv != NULL)
	{
		wchar_t** wargv = new wchar_t*[m_argc];
		for (unsigned int i = 0; i < m_argc; i++)
		{
			size_t len = strlen(m_argv[i]) + 1;
			wargv[i] = new wchar_t[len];
			mbstowcs(wargv[i], m_argv[i], len);
		}
		PySys_SetArgv(m_argc, wargv);
		for (unsigned int i = 0; i < m_argc; i++)
			delete[] wargv[i];
		delete[] wargv;
	}

	if (m_type == 'F')
	{
		// Run script from file
		FILE* fp = fopen(m_source, "r");
		if (fp)
		{
			PyObject* module = PyImport_AddModule("__main__");
			PyObject* moduleDict = PyModule_GetDict(module);
			PyObject* f = PyUnicode_FromString(m_source);
			PyDict_SetItemString(moduleDict, "__file__", f);
			Py_DECREF(f);
			PyRun_File(fp, m_source, m_Py_file_input, moduleDict, moduleDict);
			fclose(fp);
		}
		else
		{
			CLog::Log(LOGERROR, "XBPyThread: %s not found!", m_source);
		}
	}
	else
	{
		// Run string
		PyObject* module = PyImport_AddModule("__main__");
		PyObject* moduleDict = PyModule_GetDict(module);
		PyRun_String(m_source, m_Py_file_input, moduleDict, moduleDict);
	}

	// Error reporting
	if (PyErr_Occurred())
	{
		LogPythonError();
	}
	else
	{
		CLog::Log(LOGINFO, "XBPyThread: Script result: Success");
	}

	// Release thread state and GIL
	PyEval_SaveThread();
}

void XBPyThread::OnExit()
{
	// Acquire GIL with our thread state
	PyEval_RestoreThread(m_threadState);

	// Wait for any sub-threads spawned by this script to finish
	PyInterpreterState* interp = m_threadState->interp;
	int waitCount = 0;
	while (waitCount < 50) // max ~5 seconds
	{
		PyThreadState* ts = PyInterpreterState_ThreadHead(interp);
		int threadCount = 0;
		while (ts)
		{
			threadCount++;
			ts = PyThreadState_Next(ts);
		}
		// Only our main thread state should remain (+ main interp thread)
		if (threadCount <= 2)
			break;
		CLog::Log(LOGDEBUG, "XBPyThread: waiting for %d sub-threads to exit", threadCount - 2);
		PyEval_SaveThread();
		Sleep(100);
		PyEval_RestoreThread(m_threadState);
		waitCount++;
	}

	// Run garbage collection before teardown
	PyGC_Collect();

	// Clean up thread state
	PyThreadState_Clear(m_threadState);
	PyThreadState_Swap(NULL);
	PyThreadState_Delete(m_threadState);
	PyEval_ReleaseLock();
	m_threadState = NULL;

	m_bDone = true;
	m_pExecuter->setDone(m_id);
}

void XBPyThread::LogPythonError()
{
	PyObject* exc_type = NULL;
	PyObject* exc_value = NULL;
	PyObject* exc_traceback = NULL;

	PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
	if (exc_type == NULL && exc_value == NULL && exc_traceback == NULL)
	{
		CLog::Log(LOGINFO, "XBPyThread: Strange: No Python exception occurred");
		return;
	}

	// Check for KeyboardInterrupt (user-initiated stop)
	if (exc_type != NULL && PyErr_GivenExceptionMatches(exc_type, PyExc_KeyboardInterrupt))
	{
		CLog::Log(LOGINFO, "XBPyThread: Script interrupted by user");
	}
	else
	{
		CLog::Log(LOGINFO, "-->Python script returned the following error<--");

		// Type
		if (exc_type != NULL)
		{
			PyObject* typeStr = PyObject_Str(exc_type);
			if (typeStr)
			{
				const char* s = PyUnicode_AsUTF8(typeStr);
				if (s) CLog::Log(LOGERROR, "Error Type: %s", s);
				Py_DECREF(typeStr);
			}
		}

		// Value
		if (exc_value != NULL)
		{
			PyObject* valStr = PyObject_Str(exc_value);
			if (valStr)
			{
				const char* s = PyUnicode_AsUTF8(valStr);
				if (s) CLog::Log(LOGERROR, "Error Contents: %s", s);
				Py_DECREF(valStr);
			}
		}

		// Traceback via traceback module
		PyObject* tbModule = PyImport_ImportModule("traceback");
		if (tbModule != NULL)
		{
			PyObject* tbList = PyObject_CallMethod(tbModule, "format_exception", "OOO",
				exc_type,
				exc_value ? exc_value : Py_None,
				exc_traceback ? exc_traceback : Py_None);
			if (tbList)
			{
				PyObject* emptyStr = PyUnicode_FromString("");
				PyObject* joined = PyObject_CallMethod(emptyStr, "join", "O", tbList);
				if (joined)
				{
					const char* s = PyUnicode_AsUTF8(joined);
					if (s) CLog::Log(LOGERROR, "%s", s);
					Py_DECREF(joined);
				}
				Py_DECREF(emptyStr);
				Py_DECREF(tbList);
			}
			Py_DECREF(tbModule);
		}

		CLog::Log(LOGINFO, "-->End of Python script error report<--");
	}

	Py_XDECREF(exc_type);
	Py_XDECREF(exc_value);
	Py_XDECREF(exc_traceback);
}
