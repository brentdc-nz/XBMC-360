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

#include "GUIWindowMusicFiles.h"
#include "..\..\FileItem.h"

CGUIWindowMusicFiles::CGUIWindowMusicFiles(void) : CGUIMediaWindow(WINDOW_MUSIC, "MyMusic.xml")
{
	m_loadOnDemand = false;
}

CGUIWindowMusicFiles::~CGUIWindowMusicFiles(void)
{
}

bool CGUIWindowMusicFiles::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
			//TEST
			if (!Update("D:\\testmusic\\"))
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

bool CGUIWindowMusicFiles::OnClick(int iItem)
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

bool CGUIWindowMusicFiles::Update(const CStdString &strDirectory)
{
	if (m_thumbLoader.IsLoading())
		m_thumbLoader.StopThread();

	if (!CGUIMediaWindow::Update(strDirectory))
		return false;

	m_thumbLoader.Load(m_vecItems);

	return true;
}