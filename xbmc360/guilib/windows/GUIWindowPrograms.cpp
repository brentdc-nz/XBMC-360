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

#include "GUIWindowPrograms.h"
#include "utils\log.h"
#include "utils\Util.h"
#include "utils\XEXUtils.h"
#include "guilib\dialogs\GUIDialogMediaSource.h"

CGUIWindowPrograms::CGUIWindowPrograms(void) : CGUIMediaWindow(WINDOW_PROGRAMS, "MyPrograms.xml")
{
	m_loadOnDemand = false;
}

CGUIWindowPrograms::~CGUIWindowPrograms(void)
{
}

bool CGUIWindowPrograms::OnMessage(CGUIMessage& message)
{
	switch(message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
//			m_dlgProgress = (CGUIDialogProgress*)m_gWindowManager.GetWindow(WINDOW_DIALOG_PROGRESS); // TODO

			// Check for a passed destination path
			CStdString strDestination = message.GetStringParam();
			if(!strDestination.IsEmpty())
			{
				message.SetStringParam("");
				CLog::Log(LOGINFO, "Attempting to quickpath to: %s", strDestination.c_str());

				// Reset directory path, as we have effectively cleared it here
				m_history.ClearPathHistory();
			}

			// Is this the first time accessing this window?
			// A quickpath overrides the a default parameter
			if(m_vecItems.GetPath() == "?" && strDestination.IsEmpty())
			{
				m_vecItems.SetPath(strDestination);// = g_stSettings.m_szDefaultPrograms;
				CLog::Log(LOGINFO, "Attempting to default to: %s", strDestination.c_str());
			}

//			m_database.Open();

			// Try to open the destination path
			if(!strDestination.IsEmpty())
			{
				// Open root
				if(strDestination.Equals("$ROOT"))
				{
					m_vecItems.SetPath("");
					CLog::Log(LOGINFO, "  Success! Opening root listing.");
				}
				else
				{
					// Default parameters if the jump fails
					m_vecItems.SetPath("");

					bool bIsBookmarkName = false;
					SetupShares();

					VECSOURCES shares;
					m_rootDir.GetShares(shares);

					int iIndex = -1;////CUtil::GetMatchingShare(strDestination, shares, bIsBookmarkName);
					
					if(iIndex > -1)
					{
						// Set current directory to matching share
						if(bIsBookmarkName)
							m_vecItems.SetPath(shares[iIndex].strPath);
						else
							m_vecItems.SetPath(strDestination);

						CUtil::RemoveSlashAtEnd(m_vecItems.m_strPath);
						CLog::Log(LOGINFO, "Success! Opened destination path: %s", strDestination.c_str());
					}
					else
					{
						CLog::Log(LOGERROR, "Failed! Destination parameter (%s) does not match a valid share!", strDestination.c_str());
					}
				}
				SetHistoryForPath(m_vecItems.GetPath());
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

bool CGUIWindowPrograms::GetDirectory(const CStdString &strDirectory, CFileItemList &items) // TODO: Progress Bar
{
	if(!CGUIMediaWindow::GetDirectory(strDirectory, items))
		return false;

	if(items.IsVirtualDirectoryRoot())
		return true;

	for (int i = 0; i < items.Size(); i++)
	{
		CFileItem *pItem = items[i];

		if(pItem->IsXEX())
		{
			// Extract the title from the Xex file
			CStdString strXexTitle;
			CXEXUtils::GetXexTitleString(pItem->GetPath(), strXexTitle);

			pItem->SetLabel(strXexTitle);
		}
	}

	return true;
}

bool CGUIWindowPrograms::Update(const CStdString &strDirectory)
{
	if(m_thumbLoader.IsLoading())
		m_thumbLoader.StopThread();

	if(!CGUIMediaWindow::Update(strDirectory))
		return false;

	m_thumbLoader.Load(m_vecItems);

	return true;
}

bool CGUIWindowPrograms::OnPlayMedia(int iItem)
{
	if( iItem < 0 || iItem >= (int)m_vecItems.Size()) return false;
	CFileItem* pItem = m_vecItems[iItem];

	if(pItem->GetPath() == "add" && pItem->GetLabel() == /*g_localizeStrings.Get(1026)*/"Add source") // 'add source button' in empty root
	{
		if(CGUIDialogMediaSource::ShowAndAddMediaSource("programs"))
		{
			Update("");
			return true;
		}
		return false;
	}

//	if(pItem->IsDVD())
//		return MEDIA_DETECT::CAutorun::PlayDisc(); // TODO

	if(pItem->m_bIsFolder) return false;

	// Launch the xex...
	CXEXUtils::RunXex(pItem->GetPath());

	return true;
}