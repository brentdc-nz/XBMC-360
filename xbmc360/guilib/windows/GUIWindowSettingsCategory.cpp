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

#include "GUIWindowSettingsCategory.h"

CGUIWindowSettimgsCategory::CGUIWindowSettimgsCategory(void) : CGUIWindow(WINDOW_SETTINGS_MYPICTURES, "SettingsCategory.xml")
{
	m_loadOnDemand = false;
	
	// set the correct ID range...
	m_dwIDRange = 8;
	m_iScreen = 0;
}

CGUIWindowSettimgsCategory::~CGUIWindowSettimgsCategory(void)
{
}

bool CGUIWindowSettimgsCategory::OnMessage(CGUIMessage &message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
			if (message.GetParam1() != WINDOW_INVALID)
			{	
				// coming to this window first time (ie not returning back from some other window)
				// so we reset our section and control states
			}
			m_iScreen = (int)message.GetParam2() - (int)iWindowID;
			return CGUIWindow::OnMessage(message);
		}
		break;
  
		case GUI_MSG_WINDOW_DEINIT:
		{

			return true;
		}
		break;	
	}

	return CGUIWindow::OnMessage(message);
}