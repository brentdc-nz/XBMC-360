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
#include "..\..\FileItem.h"

CGUIWindowVideoFiles::CGUIWindowVideoFiles(void) : CGUIMediaWindow(WINDOW_VIDEOS, "MyVideos.xml")
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
			//TEST
			if(!Update("D:\\testvideos\\"))
			{
			}

			m_viewControl.SetCurrentView(VIEW_METHOD_THUMBS);
			//TEST END

			return CGUIMediaWindow::OnMessage(message);
		}
		case GUI_MSG_WINDOW_DEINIT:
		{
			return CGUIMediaWindow::OnMessage(message);
		}

		break;
	}

	return CGUIMediaWindow::OnMessage(message);
}

bool CGUIWindowVideoFiles::OnClick(int iItem)
{
	if ( iItem < 0 || iItem >= (int)m_vecItems.Size() ) return true;
	CFileItem* pItem = m_vecItems[iItem];

	//TODO - Get info from file
/*	CStdString strExtension;
	CUtil::GetExtension(pItem->m_strPath, strExtension);

	if (strcmpi(strExtension.c_str(), ".nfo") == 0)
	{
		OnInfo(iItem);
		return true;
	}
*/
	return CGUIMediaWindow::OnClick(iItem);
}

bool CGUIWindowVideoFiles::Update(const CStdString &strDirectory)
{
	if (m_thumbLoader.IsLoading())
		m_thumbLoader.StopThread();

	if (!CGUIMediaWindow::Update(strDirectory))
		return false;

	m_thumbLoader.Load(m_vecItems);

	return true;
}