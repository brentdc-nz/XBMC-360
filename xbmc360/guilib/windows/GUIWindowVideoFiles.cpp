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

#include "GUIWindowVideoFiles.h"

#include "..\..\filesystem\HDDirectory.h" //TESTING
#include "..\..\Application.h"  //TESTING
#include "..\GUISpinControlEx.h"  //TESTING

CGUIWindowVideoFiles::CGUIWindowVideoFiles(void) : CGUIWindow(WINDOW_VIDEOS, "MyVideos.xml")
{
	m_loadOnDemand = false;
}

CGUIWindowVideoFiles::~CGUIWindowVideoFiles(void)
{
}

bool CGUIWindowVideoFiles::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
			CGUISpinControlEx *pOriginalSpin;
			pOriginalSpin = (CGUISpinControlEx*)GetControl(1);

			pOriginalSpin->SetPosition(420, 330);
			
			XFILE::CHDDirectory directory;
			
			//Find all video in test video folder!
			directory.GetDirectory("D:\\testvideos\\", m_items);
			
			for (int i = 0; i < m_items.Size(); ++i)
			{
				CFileItem* pItem = m_items[i];
				if (!pItem->m_bIsFolder)
				{
					CStdString strFileName;
					strFileName = pItem->GetLabel();

					pOriginalSpin->AddLabel(strFileName.c_str(), i);
					pOriginalSpin->SetNavigation(2,2,1,1);
				}
			}

			break;
		}
		case GUI_MSG_WINDOW_DEINIT:
		{
			m_items.Clear();

			CGUISpinControlEx *pOriginalSpin;
			pOriginalSpin = (CGUISpinControlEx*)GetControl(1);
			pOriginalSpin->Clear();

			break;
		}

		case GUI_MSG_CLICKED:
		{
			if(message.GetControlId() == 2)
			{
				//Play button clicked!
				CGUISpinControlEx *pOriginalSpin;
				pOriginalSpin = (CGUISpinControlEx*)GetControl(1);

				CStdString strFileName = pOriginalSpin->GetLabel();
				CStdString strPath = "D:\\testvideos\\";

				strPath += strFileName;

				g_application.PlayFile(strPath);

			}
			break;
		}
	}

	return CGUIWindow::OnMessage(message);
}