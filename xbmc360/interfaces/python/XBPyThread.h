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

#include "Python.h"
#include "utils/Thread.h"

#include <string>

class XBPython; // forward

class XBPyThread : public CThread
{
public:
	XBPyThread(XBPython* pExecuter, PyThreadState* mainThreadState, int id);
	virtual ~XBPyThread();

	int  evalFile(const char* src);
	int  evalString(const char* src);
	int  setArgv(unsigned int argc, const char** argv);
	bool isDone() const  { return m_bDone; }
	bool isStopping() const { return m_bStopping; }
	void stop();

protected:
	virtual void OnStartup();
	virtual void Process();
	virtual void OnExit();

private:
	void LogPythonError();

	XBPython*       m_pExecuter;
	PyThreadState*  m_threadState;

	char   m_type;         // 'F' = file, 'S' = string
	char*  m_source;
	char** m_argv;
	unsigned int m_argc;
	bool   m_bDone;
	bool   m_bStopping;
	int    m_id;
};
