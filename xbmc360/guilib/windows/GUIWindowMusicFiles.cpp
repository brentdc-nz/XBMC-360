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
#include "FileItem.h"
#include "utils\log.h"
#include "utils\Util.h"
#include "guilib\LocalizeStrings.h"
#include "guilib\dialogs\GUIDialogMediaSource.h"

CGUIWindowMusicFiles::CGUIWindowMusicFiles(void) : CGUIMediaWindow(WINDOW_MUSIC, "MyMusic.xml")
{
	m_loadOnDemand = false;
}

CGUIWindowMusicFiles::~CGUIWindowMusicFiles(void)
{
}

bool CGUIWindowMusicFiles::OnMessage(CGUIMessage& message)
{
	switch(message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
			// Check for a passed destination path
			CStdString strDestination = message.GetStringParam();
			if(!strDestination.IsEmpty())
			{
				message.SetStringParam("");
//				g_stSettings.m_iVideoStartWindow = GetID();
				CLog::Log(LOGINFO, "Attempting to quickpath to: %s", strDestination.c_str());

				// Reset directory path, as we have effectively cleared it here
				m_history.ClearPathHistory();
			}

			// Is this the first time accessing this window?
			// A quickpath overrides the a default parameter
			if(m_vecItems.m_strPath == "?" && strDestination.IsEmpty())
			{
				m_vecItems.m_strPath = strDestination;// = g_stSettings.m_szDefaultVideos; //CHECK ME
				CLog::Log(LOGINFO, "Attempting to default to: %s", strDestination.c_str());
			}

			// Try to open the destination path
			if(!strDestination.IsEmpty())
			{
				// Open root
				if(strDestination.Equals("$ROOT"))
				{
					m_vecItems.m_strPath = "";
					CLog::Log(LOGINFO, "  Success! Opening root listing.");
				}
				else
				{
					// Default parameters if the jump fails
					m_vecItems.m_strPath = "";

					bool bIsBookmarkName = false;

					SetupShares();
					VECSOURCES shares;
					m_rootDir.GetShares(shares);

					int iIndex = -1; // CUtil::GetMatchingShare(strDestination, shares, bIsBookmarkName);// CHECK ME

					if(iIndex > -1)
					{
						if(bIsBookmarkName)
							m_vecItems.m_strPath=shares[iIndex].strPath;
						else
							m_vecItems.m_strPath=strDestination;

						CUtil::RemoveSlashAtEnd(m_vecItems.m_strPath);
						CLog::Log(LOGINFO, "  Success! Opened destination path: %s", strDestination.c_str());
					}
					else
					{
						CLog::Log(LOGERROR, "  Failed! Destination parameter (%s) does not match a valid share!", strDestination.c_str());
					}
				}
				SetHistoryForPath(m_vecItems.m_strPath);
			}
			return CGUIMediaWindow::OnMessage(message);
		}
		break;


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
	if(iItem < 0 || iItem >= (int)m_vecItems.Size()) return true;
	CFileItem* pItem = m_vecItems[iItem];

	return CGUIMediaWindow::OnClick(iItem);
}

bool CGUIWindowMusicFiles::Update(const CStdString &strDirectory)
{
	if(m_thumbLoader.IsLoading())
		m_thumbLoader.StopThread();

	if(!CGUIMediaWindow::Update(strDirectory))
		return false;

	m_thumbLoader.Load(m_vecItems);

	return true;
}

bool CGUIWindowMusicFiles::OnPlayMedia(int iItem)
{
	if( iItem < 0 || iItem >= (int)m_vecItems.Size()) return false;
	CFileItem* pItem = m_vecItems[iItem];

//	if(pItem->m_bIsShareOrDrive)
//		return false;

	if(pItem->GetPath() == "add" && pItem->GetLabel() == /*g_localizeStrings.Get(1026)*/"Add source") // 'add source button' in empty root
	{
		if(CGUIDialogMediaSource::ShowAndAddMediaSource("music"))
		{
			Update("");
			return true;
		}
		return false;
	}

	return CGUIMediaWindow::OnPlayMedia(iItem);
}