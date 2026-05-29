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

#include "GUIPythonWindow.h"
#include <string>
#include <vector>

// TODO: #include "windows/GUIMediaWindow.h" - not yet available

int Py_XBMC_Event_OnClick(void* arg);
int Py_XBMC_Event_OnFocus(void* arg);
int Py_XBMC_Event_OnInit(void* arg);

// TODO: Should inherit from CGUIMediaWindow when available
class CGUIPythonWindowXML : public CGUIPythonWindow
{
public:
	CGUIPythonWindowXML(int id, std::string strXML, std::string strFallBackPath);
	virtual ~CGUIPythonWindowXML(void);
	// TODO: virtual bool OnMessage(CGUIMessage& message);
	// TODO: virtual bool OnAction(const CAction &action);
	// TODO: virtual bool OnBack(int actionID);
	// TODO: virtual void AllocResources(bool forceLoad = false);
	// TODO: virtual void FreeResources(bool forceUnLoad = false);
	// TODO: virtual void Render();
	void WaitForActionEvent(unsigned int timeout);
	void PulseActionEvent();
	void AddItem(/*CFileItemPtr*/ void* fileItem, int itemPosition);
	void RemoveItem(int itemPosition);
	void ClearList();
	// TODO: CFileItemPtr GetListItem(int position);
	int GetListSize();
	int GetCurrentListPosition();
	void SetCurrentListPosition(int item);
	void SetCallbackWindow(PyObject *object);
	// TODO: virtual bool OnClick(int iItem);
	void SetProperty(const std::string &strProperty, const std::string &strValue);

protected:
	// TODO: virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
	// TODO: virtual bool LoadXML(const std::string &strPath, const std::string &strPathLower);
	// TODO: unsigned int LoadScriptStrings();
	// TODO: void ClearScriptStrings();
	// TODO: virtual bool Update(const std::string &strPath);
	// TODO: void SetupShares();
	PyObject* pCallbackWindow;
	void* m_actionEvent; // TODO: HANDLE
	bool m_bRunning;
	std::string m_scriptPath;
	std::string m_mediaDir;
	std::string m_strXML;
	std::string m_strFallBackPath;
	int m_currentPosition;
};
