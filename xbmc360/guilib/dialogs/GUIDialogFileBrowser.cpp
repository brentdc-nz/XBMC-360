#include "GUIDialogFileBrowser.h"
#include "..\GUIWindowManager.h"
#include "..\Key.h"
#include "..\GUIMessage.h"
#include "..\LocalizeStrings.h"
#include "..\GUIListControl.h"
#include "..\..\URL.h"
#include "..\..\MediaManager.h"
#include "..\..\utils\Util.h"
#include "GUIDialogNetworkSetup.h"
#include  "..\..\filesystem\Directory.h"

#define CONTROL_LIST          450
#define CONTROL_THUMBS        451
#define CONTROL_HEADING_LABEL 411
#define CONTROL_LABEL_PATH    412
#define CONTROL_OK            413
#define CONTROL_CANCEL        414
#define CONTROL_NEWFOLDER     415

CGUIDialogFileBrowser::CGUIDialogFileBrowser()
	: CGUIDialog(WINDOW_DIALOG_FILE_BROWSER, "FileBrowser.xml")
{
	m_bConfirmed = false;
	m_Directory.m_bIsFolder = true;
	m_browsingForFolders = 0;
	m_singleList = false;
}

CGUIDialogFileBrowser::~CGUIDialogFileBrowser()
{
}

void CGUIDialogFileBrowser::Render()
{
	int item = m_viewControl.GetSelectedItem();
	if(item >= 0)
	{
		// As we don't have a "." item, assume that if the user
		// is highlighting ".." then they wish to use that as the path
		if(m_vecItems[item]->IsParentFolder())
			m_selectedPath = m_Directory.GetPath();
		else
			m_selectedPath = m_vecItems[item]->GetPath();

		if(m_selectedPath == "net://")
		{
			SET_CONTROL_LABEL(CONTROL_LABEL_PATH, g_localizeStrings.Get(1032)); // "Add Network Location..."
		}
		else
		{
			// Update the current path label
			CURL url(m_selectedPath);
			CStdString safePath;
			url.GetURLWithoutUserDetails(safePath);
			SET_CONTROL_LABEL(CONTROL_LABEL_PATH, safePath);
		}
	}
	CGUIDialog::Render();
}

bool CGUIDialogFileBrowser::OnMessage(CGUIMessage& message)
{
	switch(message.GetMessage())
	{
		case GUI_MSG_WINDOW_DEINIT:
		{
//			if(m_thumbLoader.IsLoading())
//				m_thumbLoader.StopThread();

			CGUIDialog::OnMessage(message);
			ClearFileItems();
//			m_addNetworkShareEnabled = false;
			return true;
		}
		break;

		case GUI_MSG_WINDOW_INIT:
		{
			m_bConfirmed = false;
			bool bIsDir = false;
/*			
			// This code allows two different selection modes for directories
			// end the path with a slash to start inside the directory
			if(m_selectedPath[m_selectedPath.size()-1] == '\\' || m_selectedPath[m_selectedPath.size()-1] == '/' ) // assume directory
			{
				bIsDir = true;

//				if(!CDirectory::Exists(m_selectedPath))
//					m_selectedPath.Empty();
			}
			else
			{
//				if(!CFile::Exists(m_selectedPath) && !CDirectory::Exists(m_selectedPath))
//				m_selectedPath.Empty();
			}
*/
			// Find the parent folder if we are a file browser (don't do this for folders)
			m_Directory.SetPath(m_selectedPath);

//			if(!m_browsingForFolders && !bIsDir)
//				CUtil::GetParentPath(m_selectedPath, m_Directory.m_strPath);

			Update(m_Directory.GetPath());
			m_viewControl.SetSelectedItem(m_selectedPath);

			return CGUIDialog::OnMessage(message);
		}
		break;

		case GUI_MSG_CLICKED:
		{
			if(m_viewControl.HasControl(message.GetSenderId())) // List control
			{
				int iItem = m_viewControl.GetSelectedItem();
				int iAction = message.GetParam1();

				if(iItem < 0) break;
				
				if(iAction == ACTION_SELECT_ITEM)
				{
					OnClick(iItem);
					return true;
				}
			}
			else if(message.GetSenderId() == CONTROL_OK)
			{
/*				if(m_browsingForFolders == 2)
				{
					CStdString strTest;
					int iItem = m_viewControl.GetSelectedItem();

					CStdString strPath;

					if(iItem == 0)
						strPath = m_selectedPath;
					else
						strPath = m_vecItems[iItem]->m_strPath;

					CUtil::AddFileToFolder(strPath,"1",strTest);
					
					CFile file;
					if(file.OpenForWrite(strTest,true,true))
					{
						file.Close();
						CFile::Delete(strTest);
						m_bConfirmed = true;
						Close();
					}
					else
						CGUIDialogOK::ShowAndGetInput(257,20072,0,0);
				}
				else
*/				{
					m_bConfirmed = true;
					Close();
				}
				return true;
			}
			else if(message.GetSenderId() == CONTROL_CANCEL)
			{
				Close();
				return true;
			}
/*			else if(message.GetSenderId() == CONTROL_NEWFOLDER)
			{
				CStdString strInput;
				if(CGUIDialogKeyboard::ShowAndGetInput(strInput,g_localizeStrings.Get(119),false))
				{
					CStdString strPath;
					CUtil::AddFileToFolder(m_vecItems.m_strPath,strInput,strPath);
					
					if(CDirectory::Create(strPath))
						Update(m_vecItems.m_strPath);
					else
						CGUIDialogOK::ShowAndGetInput(20069,20072,20073,0);
				}
			}
*/		}
		break;

//		case GUI_MSG_SETFOCUS:
//		{
//			if(m_viewControl.HasControl(message.GetControlId()) && m_viewControl.GetCurrentControl() != message.GetControlId())
//			{
//				m_viewControl.SetFocused();
//				return true;
//			}
//		}
//		break;

	}
	return CGUIDialog::OnMessage(message);
}

void CGUIDialogFileBrowser::OnClick(int iItem)
{
	if(iItem < 0 || iItem >= (int)m_vecItems.Size())
		return;

	CFileItem* pItem = m_vecItems[iItem];
	CStdString strPath = pItem->GetPath();

	if(pItem->m_bIsFolder)
	{
		if(pItem->GetPath() == "net://")
		{
			// Special "Add Network Location" item
			OnAddNetworkLocation();
			return;
		}
//		if(pItem->m_bIsShareOrDrive)
//		{
//			if(!HaveDiscOrConnection(pItem->m_strPath, pItem->m_iDriveType))
//				return;
//		}
		Update(strPath);
	}
	else if(!m_browsingForFolders)
	{
		m_selectedPath = pItem->GetPath();
		m_bConfirmed = true;
		Close();
	}
}

void CGUIDialogFileBrowser::ClearFileItems()
{
	m_viewControl.Clear();
	m_vecItems.Clear(); // Will clean up everything
}

void CGUIDialogFileBrowser::OnWindowLoaded()
{
	CGUIDialog::OnWindowLoaded();
	m_viewControl.Reset();
	m_viewControl.SetParentWindow(GetID());
	m_viewControl.AddView(VIEW_METHOD_LIST, GetControl(CONTROL_LIST));
//	m_viewControl.AddView(VIEW_METHOD_ICONS, GetControl(CONTROL_THUMBS));
	m_viewControl.SetCurrentView(VIEW_METHOD_LIST);
	
	// Set the page spin controls to hidden
/*	CGUIListControl *pList = (CGUIListControl *)GetControl(CONTROL_LIST);

//	if(pList)
//		pList->SetPageControlVisible(false);

//	CGUIThumbnailPanel *pThumbs = (CGUIThumbnailPanel *)GetControl(CONTROL_THUMBS);

//	if(pThumbs)
//		pThumbs->SetPageControlVisible(false);*/
}

void CGUIDialogFileBrowser::OnWindowUnload()
{
	CGUIDialog::OnWindowUnload();
	m_viewControl.Reset();
}

bool CGUIDialogFileBrowser::ShowAndGetShare(CStdString &path, bool allowNetworkShares/*, VECSHARES* additionalShare */)
{
	// Technique is:
	// 1.  Show Filebrowser with currently defined local, and optionally the network locations.
	// 2.  Have the "Add Network Location" option in addition.
	// 3.  If the "Add Network Location" is pressed, then:
	//     a) Fire up the network location dialog to grab the new location
	//     b) Check the location by doing a GetDirectory() - if it fails, prompt the user
	//        to allow them to add currently disconnected network shares.
	//     c) Save this location to our xml file (network.xml)
	//     d) Return to 1.
	// 4.  Allow user to browse the local and network locations for their share.
	// 5.  On OK, return to the Add share dialog.

	// Create a new filebrowser window
	CGUIDialogFileBrowser *browser = new CGUIDialogFileBrowser();
	if(!browser) return false;

	// Add it to our window manager
	g_windowManager.AddUniqueInstance(browser);

//	browser->SetHeading(g_localizeStrings.Get(1023));

	VECSOURCES shares;
	g_mediaManager.GetLocalDrives(shares);

	// Now the additional share if appropriate
//	if(additionalShare)
	{
//		for(unsigned int i=0;i<additionalShare->size();++i)
//			shares.push_back((*additionalShare)[i]);
	}

	// Now add the network shares...
//	if(allowNetworkShares)
//		g_mediaManager.GetNetworkLocations(shares);
	
	browser->SetShares(shares);
//	browser->m_rootDir.SetMask("/");
//	browser->m_rootDir.AllowNonLocalShares(false); // Don't allow plug n play shares
	browser->m_browsingForFolders = true;
//	browser->m_addNetworkShareEnabled = allowNetworkShares;
	browser->m_selectedPath = "";
	browser->DoModal();

	bool confirmed = browser->IsConfirmed();

	if(confirmed)
		path = browser->m_selectedPath;

	g_windowManager.Remove(browser->GetID());
	delete browser;

	return confirmed;
}

void CGUIDialogFileBrowser::SetShares(VECSOURCES &shares)
{
	m_shares = shares;
	m_rootDir.SetShares(shares);
}

void CGUIDialogFileBrowser::OnAddNetworkLocation()
{
	// Ok, fire up the network location dialog
	CStdString path;
	if(CGUIDialogNetworkSetup::ShowAndGetNetworkAddress(path))
	{
		// Verify the path by doing a GetDirectory.
		CFileItemList items;
		if(DIRECTORY::CDirectory::GetDirectory(path, items, "", false, true)/* || CGUIDialogYesNo::ShowAndGetInput(1001,1002,1003,1004)*/)
		{
			// Add the network location to the shares list
			CMediaSource share;
			share.strPath = path;
			CURL url(path);
			url.GetURLWithoutUserDetails(share.strName);
			m_shares.push_back(share);
			// Add to our location manager...
//			g_mediaManager.AddNetworkLocation(path);
		}
	}
	m_rootDir.SetShares(m_shares);
	Update(m_vecItems.m_strPath);
}

void CGUIDialogFileBrowser::Update(const CStdString &strDirectory)
{
//	if(m_browsingForImages && m_thumbLoader.IsLoading())
//		m_thumbLoader.StopThread();
	
	// Get selected item
	int iItem = m_viewControl.GetSelectedItem();
	CStdString strSelectedItem = "";
	if(iItem >= 0 && iItem < m_vecItems.Size())
	{
		CFileItem* pItem = m_vecItems[iItem];
		if(!pItem->IsParentFolder())
		{
			strSelectedItem = pItem->GetPath();
			CUtil::RemoveSlashAtEnd(strSelectedItem);
			m_history.SetSelectedItem(strSelectedItem, m_Directory.GetPath() == ""?"empty":m_Directory.GetPath());
		}
	}

	if(!m_singleList)
	{
		ClearFileItems();

		CStdString strParentPath;
		bool bParentExists = CUtil::GetParentPath(strDirectory, strParentPath);

		// Check if current directory is a root share
		if(!m_rootDir.IsShare(strDirectory))
		{
			// No, do we got a parent dir?
			if(bParentExists)
			{
				// Yes
				CFileItem *pItem = new CFileItem("..");
				pItem->SetPath(strParentPath);
				pItem->m_bIsFolder = true;
				pItem->m_bIsShareOrDrive = false;
				m_vecItems.Add(pItem);
				m_strParentPath = strParentPath;
			}
		}
		else
		{
			// Yes, this is the root of a share
			// add parent path to the virtual directory
			CFileItem *pItem = new CFileItem("..");
			pItem->SetPath("");
			pItem->m_bIsShareOrDrive = false;
			pItem->m_bIsFolder = true;
			m_vecItems.Add(pItem);
			m_strParentPath = "";
		}
		m_Directory.SetPath(strDirectory);
		m_rootDir.GetDirectory(strDirectory, m_vecItems, m_useFileDirectories);
	}

	// Some evil stuff don't work with the '/' mask, e.g. shoutcast directory - make sure no files are in there
	if(m_browsingForFolders)
	{
		for(int i = 0; i < m_vecItems.Size(); ++i)
		if(!m_vecItems[i]->m_bIsFolder)
		{
			m_vecItems.Remove(i);
			i--;
		}
	}

	// Now need to set thumbs
	m_vecItems.FillInDefaultIcons();

//	OnSort();

	if(m_Directory.GetPath().IsEmpty()/* && m_addNetworkShareEnabled && (g_settings.m_vecProfiles[0].getLockMode() == LOCK_MODE_EVERYONE || (g_settings.m_iLastLoadedProfileIndex == 0) || g_passwordManager.bMasterUser)*/)
	{
		// We are in the virtual directory - add the "Add Network Location" item
		CFileItem *pItem = new CFileItem(g_localizeStrings.Get(1032));
		pItem->SetPath("net://");
		pItem->m_bIsFolder = true;
		m_vecItems.Add(pItem);
	}

	m_viewControl.SetItems(m_vecItems);

//	if(m_browsingForImages)
//		m_viewControl.SetCurrentView(/*CAutoSwitch::ByFileCount(m_vecItems) ? VIEW_METHOD_ICONS : */VIEW_METHOD_LIST);

	CStdString strPath2 = m_Directory.GetPath();
	CUtil::RemoveSlashAtEnd(strPath2);
	strSelectedItem = m_history.GetSelectedItem(strPath2==""?"empty":strPath2);

	for(int i = 0; i < (int)m_vecItems.Size(); ++i)
	{
		CFileItem* pItem = m_vecItems[i];
		strPath2 = pItem->GetPath();
		CUtil::RemoveSlashAtEnd(strPath2);

		if(strPath2 == strSelectedItem)
		{
			m_viewControl.SetSelectedItem(i);
			break;
		}
	}
//	if(m_browsingForImages)
//		m_thumbLoader.Load(m_vecItems);
}