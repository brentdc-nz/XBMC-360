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

#include "GUIPythonWindow.h"
#include "pyutil.h"
#include "window.h"
#include "control.h"
#include "action.h"
#include "utils/log.h"

using namespace PYXBMC;

// TODO: Full implementation requires CGUIWindow, CGUIWindowManager, CAction
// These stubs provide the class skeleton matching xbmc4xbox

CGUIPythonWindow::CGUIPythonWindow(int id)
// TODO: : CGUIWindow(id, "")
{
	m_iWindowId = id;
	pCallbackWindow = NULL;
	m_actionEvent = NULL; // TODO: CreateEvent(NULL, true, false, NULL);
	m_loadOnDemand = false;
}

CGUIPythonWindow::~CGUIPythonWindow(void)
{
	// TODO: CloseHandle(m_actionEvent);
}

void CGUIPythonWindow::SetCallbackWindow(PyObject *object)
{
	pCallbackWindow = object;
}

void CGUIPythonWindow::WaitForActionEvent(unsigned int timeout)
{
	// TODO: WaitForSingleObject(m_actionEvent, timeout);
	// TODO: ResetEvent(m_actionEvent);
}

void CGUIPythonWindow::PulseActionEvent()
{
	// TODO: SetEvent(m_actionEvent);
}

// Callback dispatchers - called via Py_AddPendingCall
int Py_XBMC_Event_OnAction(void* arg)
{
	PyXBMCAction* action = (PyXBMCAction*)arg;
	if (action)
	{
		// TODO: Dispatch to Python callback window's onAction method
		// PyObject_CallMethod(action->pCallbackWindow, "onAction", "(O)", action->pObject);
		Py_XDECREF(action->pObject);
		delete action;
	}
	return 0;
}

int Py_XBMC_Event_OnControl(void* arg)
{
	PyXBMCAction* action = (PyXBMCAction*)arg;
	if (action)
	{
		// TODO: Dispatch to Python callback window's onClick method
		// PyObject_CallMethod(action->pCallbackWindow, "onClick", "(O)", action->pObject);
		Py_XDECREF(action->pObject);
		delete action;
	}
	return 0;
}
