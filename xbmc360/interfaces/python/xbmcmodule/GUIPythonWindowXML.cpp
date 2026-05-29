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

#include "GUIPythonWindowXML.h"
#include "utils/log.h"

using namespace std;

// TODO: Full implementation requires CGUIMediaWindow, CGUIWindowManager, CFileItem

CGUIPythonWindowXML::CGUIPythonWindowXML(int id, string strXML, string strFallBackPath)
: CGUIPythonWindow(id)
{
	pCallbackWindow = NULL;
	m_actionEvent = NULL; // TODO: CreateEvent(NULL, true, false, NULL);
	m_bRunning = false;
	m_strXML = strXML;
	m_strFallBackPath = strFallBackPath;
	m_currentPosition = -1;
}

CGUIPythonWindowXML::~CGUIPythonWindowXML(void)
{
	// TODO: CloseHandle(m_actionEvent);
}

void CGUIPythonWindowXML::SetCallbackWindow(PyObject *object)
{
	pCallbackWindow = object;
}

void CGUIPythonWindowXML::WaitForActionEvent(unsigned int timeout)
{
	// TODO: WaitForSingleObject(m_actionEvent, timeout);
	// TODO: ResetEvent(m_actionEvent);
}

void CGUIPythonWindowXML::PulseActionEvent()
{
	// TODO: SetEvent(m_actionEvent);
}

void CGUIPythonWindowXML::AddItem(void* fileItem, int itemPosition)
{
	// TODO: Add CFileItemPtr to m_vecItems at position
}

void CGUIPythonWindowXML::RemoveItem(int itemPosition)
{
	// TODO: Remove item from m_vecItems at position
}

void CGUIPythonWindowXML::ClearList()
{
	// TODO: m_vecItems->Clear();
}

int CGUIPythonWindowXML::GetListSize()
{
	// TODO: return m_vecItems->Size();
	return 0;
}

int CGUIPythonWindowXML::GetCurrentListPosition()
{
	return m_currentPosition;
}

void CGUIPythonWindowXML::SetCurrentListPosition(int item)
{
	m_currentPosition = item;
	// TODO: Update control selection
}

void CGUIPythonWindowXML::SetProperty(const string &strProperty, const string &strValue)
{
	// TODO: Implement via CGUIWindow::SetProperty
}

// Callback dispatchers
int Py_XBMC_Event_OnClick(void* arg)
{
	PyXBMCAction* action = (PyXBMCAction*)arg;
	if (action)
	{
		// TODO: Dispatch onClick to Python callback
		delete action;
	}
	return 0;
}

int Py_XBMC_Event_OnFocus(void* arg)
{
	PyXBMCAction* action = (PyXBMCAction*)arg;
	if (action)
	{
		// TODO: Dispatch onFocus to Python callback
		delete action;
	}
	return 0;
}

int Py_XBMC_Event_OnInit(void* arg)
{
	PyXBMCAction* action = (PyXBMCAction*)arg;
	if (action)
	{
		// TODO: Dispatch onInit to Python callback
		delete action;
	}
	return 0;
}
