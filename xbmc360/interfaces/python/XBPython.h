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

// XBPython - Python scripting manager for XBMC-360
// Manages interpreter lifecycle, script threads, and player callbacks.

#include "XBPyThread.h"
#include "cores/IPlayer.h"
#include "utils/CriticalSection.h"

#include <string>
#include <vector>

typedef struct {
	int id;
	bool bDone;
	std::string strFile;
	XBPyThread* pyThread;
} PyElem;

typedef std::vector<PyElem> PyList;

class XBPython
{
public:
	XBPython();
	virtual ~XBPython();

	void Initialize();
	void Finalize();
	void FinalizeImmediate();
	void FreeResources();
	void Process();

	int  ScriptsSize();
	int  GetPythonScriptId(int scriptPosition);
	int  evalFile(const char* src);
	int  evalFile(const char* src, unsigned int argc, const char** argv);
	int  evalString(const char* src, unsigned int argc = 0, const char** argv = NULL);

	bool isRunning(int scriptId);
	bool isStopping(int scriptId);
	void setDone(int id);
	void stopScript(int scriptId);

	// Returns NULL if script doesn't exist or has no filename
	const char* getFileName(int scriptId);
	// Returns -1 if no scripts exist with specified filename
	int  getScriptId(const char* strFile);

	PyThreadState* getMainThreadState() { return m_mainThreadState; }

	// Player callback registration (called from CPythonPlayer)
	void RegisterPythonPlayerCallBack(IPlayerCallback* pCallback);
	void UnregisterPythonPlayerCallBack(IPlayerCallback* pCallback);
	void PulseGlobalEvent();

	// Player event forwarding (called from CApplication)
	void OnPlayBackStarted();
	void OnPlayBackEnded();
	void OnPlayBackStopped();
	void OnPlayBackPaused();
	void OnPlayBackResumed();

	bool bStartup;
	bool bLogin;

private:
	bool FileExist(const char* strFile);

	int              m_nextId;
	PyThreadState*   m_mainThreadState;
	bool             m_bInitialized;
	int              m_iDllScriptCounter;
	unsigned int     m_finalizeTime;
	bool             m_bFinalizeScheduled;

	CCriticalSection m_critSection;
	PyList           m_vecPyList;
	std::vector<IPlayerCallback*> m_vecPlayerCallbackList;
};

extern XBPython g_pythonParser;
