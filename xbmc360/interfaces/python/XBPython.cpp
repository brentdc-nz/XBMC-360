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

#include "XBPython.h"
#include "LibPython.h"
#include "utils/log.h"
#include "utils/SingleLock.h"

// Module init functions (Python 3 style)
extern "C" PyObject* PyInit_xbmc(void);
extern "C" PyObject* PyInit_xbmcgui(void);
extern "C" PyObject* PyInit_xbmcplugin(void);

XBPython g_pythonParser;

// Python path on Xbox 360 game partition
#define PYTHON_PATH L"game:\\system\\python\\lib"

XBPython::XBPython()
{
	m_bInitialized = false;
	m_nextId = 0;
	m_mainThreadState = NULL;
	m_iDllScriptCounter = 0;
	m_finalizeTime = 0;
	m_bFinalizeScheduled = false;
	bStartup = false;
	bLogin = false;
}

XBPython::~XBPython()
{
}

bool XBPython::FileExist(const char* strFile)
{
	if (!strFile)
		return false;

	FILE* fp = fopen(strFile, "r");
	if (!fp)
	{
		CLog::Log(LOGERROR, "Python: Cannot find '%s'", strFile);
		return false;
	}
	fclose(fp);
	return true;
}

void XBPython::Initialize()
{
	CLog::Log(LOGINFO, "XBPython: Initializing Python engine");
	CSingleLock lock(m_critSection);
	m_iDllScriptCounter++;

	if (!m_bInitialized)
	{
		// Load the static library wrapper
		if (!CLibPython::Load())
		{
			CLog::Log(LOGERROR, "XBPython: Failed to load CLibPython");
			Finalize();
			return;
		}

		// Register built-in XBMC modules before Py_Initialize()
		PyImport_AppendInittab("xbmc", PyInit_xbmc);
		PyImport_AppendInittab("xbmcgui", PyInit_xbmcgui);
		PyImport_AppendInittab("xbmcplugin", PyInit_xbmcplugin);

		// Initialize Python interpreter (no site module on Xbox 360)
		Py_NoSiteFlag = 1;
		Py_Initialize();
		PyEval_InitThreads();

		// Set up sys.path
		PySys_SetPath(PYTHON_PATH);

		// Set empty argv
		wchar_t* python_argv[] = { L"" };
		PySys_SetArgv(1, python_argv);

		// Redirect stdout/stderr to xbmc.log()
		if (PyRun_SimpleString(
			"import xbmc\n"
			"class XBMCOutput:\n"
			"    def write(self, data):\n"
			"        if data and data.strip():\n"
			"            xbmc.log(data.rstrip(), xbmc.LOGINFO)\n"
			"    def close(self):\n"
			"        xbmc.log('.', xbmc.LOGINFO)\n"
			"    def flush(self):\n"
			"        pass\n"
			"\n"
			"import sys\n"
			"sys.stdout = XBMCOutput()\n"
			"sys.stderr = XBMCOutput()\n"
			"print('-->Python Initialized<--')\n"
			"") == -1)
		{
			CLog::Log(LOGERROR, "XBPython: Python Initialize Error");
		}

		m_mainThreadState = PyThreadState_Get();
		if (!m_mainThreadState)
			CLog::Log(LOGERROR, "XBPython: threadstate is NULL");

		// Release the GIL so other threads can acquire it
		PyEval_SaveThread();

		m_bInitialized = true;
		CLog::Log(LOGINFO, "XBPython: Python %s initialized successfully", PY_VERSION);
	}
}

void XBPython::Finalize()
{
	CSingleLock lock(m_critSection);
	m_iDllScriptCounter--;

	if (m_iDllScriptCounter == 0 && m_bInitialized)
	{
		// Delay finalization by 10 seconds to avoid costly re-init
		m_bFinalizeScheduled = true;
		m_finalizeTime = GetTickCount() + 10000;
		CLog::Log(LOGDEBUG, "XBPython: Finalize scheduled in 10 seconds");
	}
}

void XBPython::FinalizeImmediate()
{
	CSingleLock lock(m_critSection);
	if (m_bInitialized)
	{
		m_bInitialized = false;
		m_bFinalizeScheduled = false;
		CLog::Log(LOGINFO, "XBPython: Finalizing Python - no scripts running");

		PyEval_RestoreThread(m_mainThreadState);
		Py_Finalize();

		CLibPython::Unload();
		m_mainThreadState = NULL;
	}
}

void XBPython::FreeResources()
{
	CSingleLock lock(m_critSection);
	if (m_bInitialized)
	{
		PyList::iterator it = m_vecPyList.begin();
		while (it != m_vecPyList.end())
		{
			lock.Leave();
			delete it->pyThread;
			lock.Enter();
			it = m_vecPyList.erase(it);
			Finalize();
		}
	}
}

void XBPython::Process()
{
	if (bStartup)
	{
		bStartup = false;

		// autoexec.py - system
		const char* strAutoExecPy = "D:\\scripts\\autoexec.py";

		if (FileExist(strAutoExecPy))
		{
			CLog::Log(LOGINFO, "XBPython: Running autoexec.py");
			evalFile(strAutoExecPy);
		}
		else
		{
			CLog::Log(LOGDEBUG, "XBPython: No system autoexec.py found, skipping");
		}
	}

	if (bLogin)
	{
		bLogin = false;

		// autoexec.py - profile
		const char* strProfileAutoExec = "D:\\scripts\\profile_autoexec.py";

		if (FileExist(strProfileAutoExec))
			evalFile(strProfileAutoExec);
		else
			CLog::Log(LOGDEBUG, "XBPython: No profile autoexec.py found, skipping");
	}

	CSingleLock lock(m_critSection);
	if (!m_bInitialized)
		return;

	// Clean up finished scripts
	PyList::iterator it = m_vecPyList.begin();
	while (it != m_vecPyList.end())
	{
		if (it->bDone)
		{
			delete it->pyThread;
			it = m_vecPyList.erase(it);
			Finalize();
		}
		else
		{
			it++;
		}
	}

	// Handle delayed finalization
	if (m_bFinalizeScheduled && m_iDllScriptCounter == 0)
	{
		if (GetTickCount() >= m_finalizeTime)
		{
			FinalizeImmediate();
		}
	}
	else if (m_bFinalizeScheduled && m_iDllScriptCounter > 0)
	{
		// New script started, cancel scheduled finalize
		m_bFinalizeScheduled = false;
	}
}

int XBPython::ScriptsSize()
{
	CSingleLock lock(m_critSection);
	return (int)m_vecPyList.size();
}

int XBPython::GetPythonScriptId(int scriptPosition)
{
	CSingleLock lock(m_critSection);
	if (scriptPosition >= 0 && scriptPosition < (int)m_vecPyList.size())
		return m_vecPyList[scriptPosition].id;
	return -1;
}

int XBPython::evalFile(const char* src)
{
	return evalFile(src, 0, NULL);
}

int XBPython::evalFile(const char* src, unsigned int argc, const char** argv)
{
	CSingleLock lock(m_critSection);
	Initialize();

	if (!m_bInitialized)
		return -1;

	if (!FileExist(src))
		return -1;

	int id = m_nextId++;
	XBPyThread* pyThread = new XBPyThread(this, m_mainThreadState, id);

	if (argv && argc > 0)
		pyThread->setArgv(argc, argv);

	pyThread->evalFile(src);

	PyElem elem;
	elem.id = id;
	elem.bDone = false;
	elem.strFile = src;
	elem.pyThread = pyThread;
	m_vecPyList.push_back(elem);

	return id;
}

int XBPython::evalString(const char* src, unsigned int argc, const char** argv)
{
	CSingleLock lock(m_critSection);
	Initialize();

	if (!m_bInitialized)
		return -1;

	int id = m_nextId++;
	XBPyThread* pyThread = new XBPyThread(this, m_mainThreadState, id);

	if (argv && argc > 0)
		pyThread->setArgv(argc, argv);

	pyThread->evalString(src);

	PyElem elem;
	elem.id = id;
	elem.bDone = false;
	elem.strFile = "";
	elem.pyThread = pyThread;
	m_vecPyList.push_back(elem);

	return id;
}

bool XBPython::isRunning(int scriptId)
{
	CSingleLock lock(m_critSection);
	for (PyList::iterator it = m_vecPyList.begin(); it != m_vecPyList.end(); it++)
	{
		if (it->id == scriptId)
			return !it->bDone;
	}
	return false;
}

bool XBPython::isStopping(int scriptId)
{
	CSingleLock lock(m_critSection);
	for (PyList::iterator it = m_vecPyList.begin(); it != m_vecPyList.end(); it++)
	{
		if (it->id == scriptId)
			return it->pyThread->isStopping();
	}
	return false;
}

void XBPython::setDone(int id)
{
	CSingleLock lock(m_critSection);
	for (PyList::iterator it = m_vecPyList.begin(); it != m_vecPyList.end(); it++)
	{
		if (it->id == id)
		{
			it->bDone = true;
			break;
		}
	}
}

void XBPython::stopScript(int scriptId)
{
	CSingleLock lock(m_critSection);
	for (PyList::iterator it = m_vecPyList.begin(); it != m_vecPyList.end(); it++)
	{
		if (it->id == scriptId)
		{
			it->pyThread->stop();
			break;
		}
	}
}

const char* XBPython::getFileName(int scriptId)
{
	CSingleLock lock(m_critSection);
	for (PyList::iterator it = m_vecPyList.begin(); it != m_vecPyList.end(); it++)
	{
		if (it->id == scriptId)
		{
			if (it->strFile.empty())
				return NULL;
			return it->strFile.c_str();
		}
	}
	return NULL;
}

int XBPython::getScriptId(const char* strFile)
{
	CSingleLock lock(m_critSection);
	for (PyList::iterator it = m_vecPyList.begin(); it != m_vecPyList.end(); it++)
	{
		if (it->strFile == strFile)
			return it->id;
	}
	return -1;
}

// =========================================================================
// Player callback management
// =========================================================================
void XBPython::RegisterPythonPlayerCallBack(IPlayerCallback* pCallback)
{
	CSingleLock lock(m_critSection);
	m_vecPlayerCallbackList.push_back(pCallback);
}

void XBPython::UnregisterPythonPlayerCallBack(IPlayerCallback* pCallback)
{
	CSingleLock lock(m_critSection);
	std::vector<IPlayerCallback*>::iterator it = m_vecPlayerCallbackList.begin();
	while (it != m_vecPlayerCallbackList.end())
	{
		if (*it == pCallback)
		{
			it = m_vecPlayerCallbackList.erase(it);
			break;
		}
		else
			++it;
	}
}

void XBPython::PulseGlobalEvent()
{
	// Currently a no-op. Could signal an event to wake sleeping threads.
}

void XBPython::OnPlayBackStarted()
{
	CSingleLock lock(m_critSection);
	for (size_t i = 0; i < m_vecPlayerCallbackList.size(); i++)
		m_vecPlayerCallbackList[i]->OnPlayBackStarted();
}

void XBPython::OnPlayBackEnded()
{
	CSingleLock lock(m_critSection);
	for (size_t i = 0; i < m_vecPlayerCallbackList.size(); i++)
		m_vecPlayerCallbackList[i]->OnPlayBackEnded();
}

void XBPython::OnPlayBackStopped()
{
	CSingleLock lock(m_critSection);
	for (size_t i = 0; i < m_vecPlayerCallbackList.size(); i++)
		m_vecPlayerCallbackList[i]->OnPlayBackStopped();
}

void XBPython::OnPlayBackPaused()
{
	CSingleLock lock(m_critSection);
	for (size_t i = 0; i < m_vecPlayerCallbackList.size(); i++)
		m_vecPlayerCallbackList[i]->OnPlayBackPaused();
}

void XBPython::OnPlayBackResumed()
{
	CSingleLock lock(m_critSection);
	for (size_t i = 0; i < m_vecPlayerCallbackList.size(); i++)
		m_vecPlayerCallbackList[i]->OnPlayBackResumed();
}
