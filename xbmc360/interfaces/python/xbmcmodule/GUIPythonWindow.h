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

// TODO: #include "GUIWindow.h" - not yet available on Xbox 360

class PyXBMCAction
{
public:
	int param;
	PyObject* pCallbackWindow;
	PyObject* pObject;
	int controlId;
	PyXBMCAction() : param(0), pCallbackWindow(NULL), pObject(NULL), controlId(0) { }
};

int Py_XBMC_Event_OnAction(void* arg);
int Py_XBMC_Event_OnControl(void* arg);

// TODO: CGUIPythonWindow requires CGUIWindow base class (not yet ported)
// Stubbed interface matching xbmc4xbox's class
class CGUIPythonWindow // : public CGUIWindow
{
public:
	CGUIPythonWindow(int id);
	virtual ~CGUIPythonWindow(void);
	// TODO: virtual bool OnMessage(CGUIMessage& message);
	// TODO: virtual bool OnAction(const CAction &action);
	// TODO: virtual bool OnBack(int actionID);
	void SetCallbackWindow(PyObject *object);
	void WaitForActionEvent(unsigned int timeout);
	void PulseActionEvent();
	int GetID() const { return m_iWindowId; }

protected:
	PyObject* pCallbackWindow;
	void* m_actionEvent; // TODO: HANDLE m_actionEvent;
	int m_iWindowId;
	bool m_loadOnDemand;
};
