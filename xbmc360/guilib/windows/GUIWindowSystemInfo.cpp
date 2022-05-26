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

#include "GUIWindowSystemInfo.h"
#include "guilib\LocalizeStrings.h"
#include "guilib\GUIInfoManager.h"

CGUIWindowSystemInfo::CGUIWindowSystemInfo(void)
:CGUIWindow(WINDOW_SYSTEM_INFORMATION, "SettingsSystemInfo.xml")
{
//	iControl = CONTROL_BT_DEFAULT;
}

CGUIWindowSystemInfo::~CGUIWindowSystemInfo(void)
{
}

void CGUIWindowSystemInfo::Render()
{
	if(/*iControl == CONTROL_BT_DEFAULT*/1) //TODO
	{
		SET_CONTROL_LABEL(40, "General Info");
		int i = 2;
		SetControlLabel(i++, "%s %s", 16111, SYSTEM_FPS);
		SetControlLabel(i++, "%s %s", 140, SYSTEM_CPU_TEMPERATURE);
		SetControlLabel(i++, "%s %s", 141, SYSTEM_GPU_TEMPERATURE);
		SetControlLabel(i++, "%s %s", 158, SYSTEM_FREE_MEMORY);
	}

	CGUIWindow::Render();
}

bool CGUIWindowSystemInfo::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_INIT:
		{
			CGUIWindow::OnMessage(message);
			SetLabelDummy();
			return true;
		}
		break;
		
		case GUI_MSG_CLICKED:
		{
			iControl = message.GetSenderId();
		}
		break;
	}

	return CGUIWindow::OnMessage(message);
}

void CGUIWindowSystemInfo::SetLabelDummy()
{
	// Set Label Dummy Entry! ""
	for (int i=2; i<12; i++)
	{
		SET_CONTROL_LABEL(i, "");
	}
}

void CGUIWindowSystemInfo::SetControlLabel(int id, const char *format, int label, int info)
{
	CStdString tmpStr;
	tmpStr.Format(format, g_localizeStrings.Get(label).c_str(), g_infoManager.GetLabel(info).c_str());
	SET_CONTROL_LABEL(id, tmpStr);
}