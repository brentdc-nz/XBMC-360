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

#include "GUIWindowScreensaver.h"
#include "..\GUIWindowManager.h"
#include "..\..\Application.h"

//Screensaver
#include "..\screensavers\ScreensaverPlasma.h"

CGUIWindowScreensaver::CGUIWindowScreensaver(void) : CGUIWindow(WINDOW_SCREENSAVER, "")
{
	m_loadOnDemand = false;
	m_pScreensaver = NULL;
}

CGUIWindowScreensaver::~CGUIWindowScreensaver(void)
{
}

void CGUIWindowScreensaver::Render()
{
	if(m_pScreensaver)
		m_pScreensaver->Render();

	CGUIWindow::Render();
}

bool CGUIWindowScreensaver::OnAction(const CAction &action)
{
	//Return if somthing is pressed
	g_application.ResetScreenSaverWindow(); //TODO Move call back to CApplication::OnKey()

	return CGUIWindow::OnAction(action);
}

bool CGUIWindowScreensaver::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_INIT:
		{
			m_pScreensaver = new CScreensaverPlasma; // TODO: Create a factory class - Only have Plasma screensaver atm tho
			m_pScreensaver->Initialize();

			return true;
		}

		case GUI_MSG_WINDOW_DEINIT:
		{
			m_pScreensaver->Close();
			delete m_pScreensaver;
			m_pScreensaver = NULL;

			return true;
		}
		break;
	}
	return CGUIWindow::OnMessage(message);
}
