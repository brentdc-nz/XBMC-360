#include "utils\Log.h"
#include "guilib\dialogs\GUIDialogFileBrowser.h"
#include "utils\Util.h"
#include "utils\URIUtils.h"
//#include "storage\DetectDVDType.h" // TODO
#include "guilib\dialogs\GUIDialogNetworkSetup.h"
#include "guilib\dialogs\GUIDialogMediaSource.h"
#include "guilib\dialogs\GUIDialogContextMenu.h"
#include "MediaManager.h"
//#include "AutoSwitch.h" // TODO
#include "Application.h"
//#include "GUIPassword.h" // TODO
#include "guilib\GUIWindowManager.h"
//#include "dialogs\GUIDialogOK.h" // TODO
//#include "dialogs\GUIDialogYesNo.h" // TODO
//#include "GUIDialogKeyboard.h" // TODO
//#include "GUIUserMessages.h" // TODO
#include "FileSystem\Directory.h"
#include "FileSystem\File.h"
#include "FileItem.h"
//#include "settings\AdvancedSettings.h" // TODO
#include "guilib\LocalizeStrings.h"
#include "viewstate.h"

using namespace XFILE;

#define CONTROL_LIST          450
#define CONTROL_THUMBS        451
#define CONTROL_HEADING_LABEL 411
#define CONTROL_LABEL_PATH    412
#define CONTROL_OK            413
#define CONTROL_CANCEL        414
#define CONTROL_NEWFOLDER     415
#define CONTROL_FLIP          416

CGUIDialogFileBrowser::CGUIDialogFileBrowser()
	: CGUIDialog(WINDOW_DIALOG_FILE_BROWSER, "FileBrowser.xml")
{
	m_Directory = new CFileItem;
	m_vecItems = new CFileItemList;
	m_bConfirmed = false;
	m_Directory->m_bIsFolder = true;
	m_browsingForFolders = 0;
	m_browsingForImages = false;
	m_useFileDirectories = false;
	m_addNetworkShareEnabled = false;
	m_singleList = false;
//	m_thumbLoader.SetObserver(this); // TODO
	m_flipEnabled = false;
}

CGUIDialogFileBrowser::~CGUIDialogFileBrowser()
{
	delete m_Directory;
	delete m_vecItems;
}

bool CGUIDialogFileBrowser::OnAction(const CAction &action)
{
	if (action.GetID() == ACTION_PARENT_DIR)
	{
		GoParentFolder();
		return true;
	}

#ifdef _HAS_MOUSE
	if ((action.GetID() == ACTION_CONTEXT_MENU || action.GetID() == ACTION_MOUSE_RIGHT_CLICK) && m_Directory->GetPath().IsEmpty())
#else
	if ((action.GetID() == ACTION_CONTEXT_MENU) && m_Directory->GetPath().IsEmpty())
#endif
	{
		int iItem = m_viewControl.GetSelectedItem();
		
		if ((!m_addSourceType.IsEmpty() && iItem != m_vecItems->Size()-1))
			return OnPopupMenu(iItem);
		
		if (m_addNetworkShareEnabled && g_mediaManager.HasLocation(m_selectedPath))
		{
			// Need to make sure this source is not an auto added location
			// as users locations might have the same paths
			CFileItemPtr pItem = (*m_vecItems)[iItem];
			for (unsigned int i=0;i<m_shares.size();++i)
			{
				if (m_shares[i].strName.Equals(pItem->GetLabel()) && m_shares[i].m_ignore)
					return false;
			}
			return OnPopupMenu(iItem);
		}
		return false;
	}
	return CGUIDialog::OnAction(action);
}

bool CGUIDialogFileBrowser::OnBack(int actionID)
{
	if (actionID == ACTION_NAV_BACK && !m_vecItems->IsVirtualDirectoryRoot())
	{
		GoParentFolder();
		return true;
	}
	return CGUIDialog::OnBack(actionID);
}

bool CGUIDialogFileBrowser::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_DEINIT:
		{
//			if (m_thumbLoader.IsLoading()) // TODO
//				m_thumbLoader.StopThread(); // TODO
			
			CGUIDialog::OnMessage(message);
			ClearFileItems();
			m_addNetworkShareEnabled = false;
			return true;
		}
		break;

		case GUI_MSG_WINDOW_INIT:
		{
			m_bConfirmed = false;
			m_bFlip = false;
			bool bIsDir = false;

			// This code allows two different selection modes for directories
			// end the path with a slash to start inside the directory
			if (URIUtils::HasSlashAtEnd(m_selectedPath))
			{
				bIsDir = true;
				bool bFool;
				int iSource = CUtil::GetMatchingSource(m_selectedPath, m_shares, bFool);
				bFool = true;

				if (iSource > -1 && iSource < (int)m_shares.size())
				{
					if (m_shares[iSource].strPath.Equals(m_selectedPath))
						bFool = false;
				}
      
				if (bFool && !CDirectory::Exists(m_selectedPath))
					m_selectedPath.Empty();
			}
			else
			{
				if (!CFile::Exists(m_selectedPath) && !CDirectory::Exists(m_selectedPath))
					m_selectedPath.Empty();
			}

			// Find the parent folder if we are a file browser (don't do this for folders)
			m_Directory->SetPath(m_selectedPath);
			
			if (!m_browsingForFolders && !bIsDir)
				m_Directory->SetPath(URIUtils::GetParentPath(m_selectedPath));
			
			Update(m_Directory->GetPath());
			m_viewControl.SetSelectedItem(m_selectedPath);
			
			return CGUIDialog::OnMessage(message);
		}
		break;

		case GUI_MSG_CLICKED:
		{
			if (m_viewControl.HasControl(message.GetSenderId())) // List control
			{
				int iItem = m_viewControl.GetSelectedItem();
				int iAction = message.GetParam1();
				if (iItem < 0) break;
#ifdef _HAS_MOUSE
				if (iAction == ACTION_SELECT_ITEM || iAction == ACTION_MOUSE_LEFT_CLICK)
#else
				if (iAction == ACTION_SELECT_ITEM)
#endif
				{
					OnClick(iItem);
					return true;
				}
			}
			else if (message.GetSenderId() == CONTROL_OK)
			{
				if (m_browsingForFolders == 2)
				{
					CStdString strTest;
					int iItem = m_viewControl.GetSelectedItem();

					CStdString strPath;
					
					if (iItem == 0)
						strPath = m_selectedPath;
					else
						strPath = (*m_vecItems)[iItem]->GetPath();

					URIUtils::AddFileToFolder(strPath,"1",strTest);
					CFile file;
					
					if (file.OpenForWrite(strTest,true))
					{
						file.Close();
//						CFile::Delete(strTest); // TODO
						m_bConfirmed = true;
						Close();
					}
//					else // TODO
//						CGUIDialogOK::ShowAndGetInput(257,20072,0,0); // TODO
				}
				else
				{
					m_bConfirmed = true;
					Close();
				}
				return true;
			}
			else if (message.GetSenderId() == CONTROL_CANCEL)
			{
				Close();
				return true;
			}
			else if (message.GetSenderId() == CONTROL_NEWFOLDER)
			{
				CStdString strInput;
//				if (CGUIDialogKeyboard::ShowAndGetInput(strInput,g_localizeStrings.Get(119),false)) // TODO
				{
					CStdString strPath;
					URIUtils::AddFileToFolder(m_vecItems->GetPath(),strInput,strPath);

					if (CDirectory::Create(strPath))
						Update(m_vecItems->GetPath());
//					else // TODO
//						CGUIDialogOK::ShowAndGetInput(20069,20072,20073,0); // TODO
				}
			}
			else if (message.GetSenderId() == CONTROL_FLIP)
				m_bFlip = !m_bFlip;
		}
		break;
		case GUI_MSG_SETFOCUS:
		{
			if (m_viewControl.HasControl(message.GetControlId()) && m_viewControl.GetCurrentControl() != message.GetControlId())
			{
				m_viewControl.SetFocused();
				return true;
			}
		}
		break;
		case GUI_MSG_NOTIFY_ALL:
		{
			// Message is received only if this window is active
			if (message.GetParam1() == GUI_MSG_REMOVED_MEDIA)
			{
				if (m_Directory->IsVirtualDirectoryRoot() && IsActive())
				{
					int iItem = m_viewControl.GetSelectedItem();
					Update(m_Directory->GetPath());
					m_viewControl.SetSelectedItem(iItem);
				}
				else if (m_Directory->IsRemovable())
				{
					// Check that we have this removable share still
					if (!m_rootDir.IsInSource(m_Directory->GetPath()))
					{
						// Don't have this share any more
						if (IsActive())
							Update("");
						else
						{
							m_history.ClearPathHistory();
							m_Directory->SetPath("");
						}
					}
				}
				return true;
			}
			else if (message.GetParam1()==GUI_MSG_UPDATE_SOURCES)
			{
				// State of the sources changed, so update our view
				if (m_Directory->IsVirtualDirectoryRoot() && IsActive())
				{
					int iItem = m_viewControl.GetSelectedItem();
					Update(m_Directory->GetPath());
					m_viewControl.SetSelectedItem(iItem);
				}
				return true;
			}
		}
		break;
	}
	return CGUIDialog::OnMessage(message);
}

void CGUIDialogFileBrowser::ClearFileItems()
{
	m_viewControl.Clear();
	m_vecItems->Clear(); // Will clean up everything
}

void CGUIDialogFileBrowser::OnSort()
{
	if (!m_singleList)
		m_vecItems->Sort(SORT_METHOD_LABEL, SORT_ORDER_ASC);
}

void CGUIDialogFileBrowser::Update(const CStdString &strDirectory)
{
//	if (m_browsingForImages && m_thumbLoader.IsLoading())*/ // TODO
//		m_thumbLoader.StopThread();*/ // TODO
	
	// Get selected item
	int iItem = m_viewControl.GetSelectedItem();
	CStdString strSelectedItem = "";
	
	if (iItem >= 0 && iItem < m_vecItems->Size())
	{
		CFileItemPtr pItem = (*m_vecItems)[iItem];
		if (!pItem->IsParentFolder())
		{
			strSelectedItem = pItem->GetPath();
			URIUtils::RemoveSlashAtEnd(strSelectedItem);
			m_history.SetSelectedItem(strSelectedItem, m_Directory->GetPath()==""?"empty":m_Directory->GetPath());
		}
	}

	if (!m_singleList)
	{
		CFileItemList items;
		CStdString strParentPath;
		bool bParentExists = URIUtils::GetParentPath(strDirectory, strParentPath);

		if ( !m_rootDir.IsSource(strDirectory))
		{
			// No, do we got a parent dir?
			if (bParentExists)
			{
				// Yes
				CFileItemPtr pItem(new CFileItem(".."));
				pItem->SetPath(strParentPath);
				pItem->m_bIsFolder = true;
				pItem->m_bIsShareOrDrive = false;
				items.Add(pItem);
			}
		}
		else
		{
			// Yes, this is the root of a share
			// add parent path to the virtual directory
			CFileItemPtr pItem(new CFileItem(".."));
			pItem->SetPath("");
			pItem->m_bIsShareOrDrive = false;
			pItem->m_bIsFolder = true;
			items.Add(pItem);
			strParentPath = "";
		}

		if (!m_rootDir.GetDirectory(strDirectory, items, m_useFileDirectories))
		{
			CLog::Log(LOGERROR,"CGUIDialogFileBrowser::GetDirectory(%s) failed", strDirectory.c_str());

			// We assume, we can get the parent
			// directory again
			CStdString strParentPath = m_history.GetParentPath();
			m_history.RemoveParentPath();
			Update(strParentPath);  
			return;  
		}

		ClearFileItems();
		*m_vecItems = items;
		m_Directory->SetPath(strDirectory);
		m_strParentPath = strParentPath;
	}
  
	// If we're getting the root source listing
	// make sure the path history is clean
	if (strDirectory.IsEmpty())
		m_history.ClearPathHistory();
    
	// Some evil stuff don't work with the '/' mask, e.g. shoutcast directory - Make sure no files are in there
	if (m_browsingForFolders)
	{
		for (int i = 0; i < m_vecItems->Size(); ++i)
		if (!(*m_vecItems)[i]->m_bIsFolder)
		{
			m_vecItems->Remove(i);
			i--;
		}
	}

	// No need to set thumbs

	m_vecItems->FillInDefaultIcons();

	OnSort();

	if (m_Directory->GetPath().IsEmpty() && m_addNetworkShareEnabled/* &&
		(g_settings.GetMasterProfile().getLockMode() == LOCK_MODE_EVERYONE || // TODO
		g_settings.IsMasterUser() || g_passwordManager.bMasterUser)*/)
	{
		// We are in the virtual directory - Add the "Add Network Location" item
		CFileItemPtr pItem(new CFileItem(g_localizeStrings.Get(1032)));
		pItem->SetPath("net://");
		pItem->m_bIsFolder = true;
		m_vecItems->Add(pItem);
	}

	if (m_Directory->GetPath().IsEmpty() && !m_addSourceType.IsEmpty())
	{
		CFileItemPtr pItem(new CFileItem(g_localizeStrings.Get(21359)));
		pItem->SetPath("source://");
		pItem->m_bIsFolder = true;
		m_vecItems->Add(pItem);
	}

	m_viewControl.SetItems(*m_vecItems);
	m_viewControl.SetCurrentView(/*(m_browsingForImages && CAutoSwitch::ByFileCount(*m_vecItems)) ? DEFAULT_VIEW_ICONS :*/ DEFAULT_VIEW_LIST); // TODO

	CStdString strPath2 = m_Directory->GetPath();
	URIUtils::RemoveSlashAtEnd(strPath2);
	strSelectedItem = m_history.GetSelectedItem(strPath2==""?"empty":strPath2);

	bool bSelectedFound = false;

	for (int i = 0; i < (int)m_vecItems->Size(); ++i)
	{
		CFileItemPtr pItem = (*m_vecItems)[i];
		strPath2 = pItem->GetPath();
		URIUtils::RemoveSlashAtEnd(strPath2);
		
		if (strPath2 == strSelectedItem)
		{
			m_viewControl.SetSelectedItem(i);
			break;
		}
	}
//	if (m_browsingForImages) // TODO
//		m_thumbLoader.Load(*m_vecItems); // TODO
}

void CGUIDialogFileBrowser::FrameMove()
{
	int item = m_viewControl.GetSelectedItem();
	if (item >= 0)
	{
		// If we are browsing for folders, and not in the root directory, then we use the parent path,
		// else we use the current file's path
		if (m_browsingForFolders && !m_Directory->IsVirtualDirectoryRoot())
			m_selectedPath = m_Directory->GetPath();
		else
			m_selectedPath = (*m_vecItems)[item]->GetPath();
		
		if (m_selectedPath == "net://")
		{
			SET_CONTROL_LABEL(CONTROL_LABEL_PATH, g_localizeStrings.Get(1032)); // "Add Network Location..."
		}
		else
		{
			// Update the current path label
			CURL url(m_selectedPath);
			CStdString safePath = url.GetWithoutUserDetails();
			SET_CONTROL_LABEL(CONTROL_LABEL_PATH, safePath);
		}
		
		if ((!m_browsingForFolders && (*m_vecItems)[item]->m_bIsFolder) || ((*m_vecItems)[item]->GetPath() == "image://Browse"))
		{
			CONTROL_DISABLE(CONTROL_OK);
		}
		else
		{
			CONTROL_ENABLE(CONTROL_OK);
		}
		
		if (m_browsingForFolders == 2)
		{
			CONTROL_ENABLE(CONTROL_NEWFOLDER);
		}
		else
		{
			CONTROL_DISABLE(CONTROL_NEWFOLDER);
		}
		
		if (m_flipEnabled)
		{
			CONTROL_ENABLE(CONTROL_FLIP);
		}
		else
		{
			CONTROL_DISABLE(CONTROL_FLIP);
		}
	}
	CGUIDialog::FrameMove();
}

void CGUIDialogFileBrowser::OnClick(int iItem)
{
	if ( iItem < 0 || iItem >= (int)m_vecItems->Size() ) return ;
	CFileItemPtr pItem = (*m_vecItems)[iItem];
	CStdString strPath = pItem->GetPath();

	if (pItem->m_bIsFolder)
	{
		if (pItem->GetPath() == "net://")
		{
			// Special "Add Network Location" item
			OnAddNetworkLocation();
			return;
		}
		
		if (pItem->GetPath() == "source://")
		{
			// Special "Add Source" item
			OnAddMediaSource();
			return;
		}
		
		if (!m_addSourceType.IsEmpty())
		{
			OnEditMediaSource(pItem.get());
			return;
		}
		
		if ( pItem->m_bIsShareOrDrive )
		{
			if ( !HaveDiscOrConnection( pItem->m_iDriveType ) )
				return ;
		}
		
		Update(strPath);
	}
	else if (!m_browsingForFolders)
	{
		m_selectedPath = pItem->GetPath();
		m_bConfirmed = true;
		Close();
	}
}

bool CGUIDialogFileBrowser::HaveDiscOrConnection( int iDriveType )
{
	if ( iDriveType == CMediaSource::SOURCE_TYPE_DVD )
	{
//		MEDIA_DETECT::CDetectDVDMedia::WaitMediaReady(); // TODO
		
//		if ( !MEDIA_DETECT::CDetectDVDMedia::IsDiscInDrive() ) // TODO
		{
//			CGUIDialogOK::ShowAndGetInput(218, 219, 0, 0); // TODO
			return false;
		}
	}
	else if ( iDriveType == CMediaSource::SOURCE_TYPE_REMOTE )
	{
		// TODO: Handle not connected to a remote share
		if ( !g_application.getNetwork().IsEthernetConnected() )
		{
//			CGUIDialogOK::ShowAndGetInput(220, 221, 0, 0); // TODO
			return false;
		}
	}

	return true;
}

void CGUIDialogFileBrowser::GoParentFolder()
{
	CStdString strPath(m_strParentPath), strOldPath(m_Directory->GetPath());
	
	if (strPath.size() == 2)
		if (strPath[1] == ':')
			URIUtils::AddSlashAtEnd(strPath);
	
	Update(strPath);
}

void CGUIDialogFileBrowser::OnWindowLoaded()
{
	CGUIDialog::OnWindowLoaded();
	m_viewControl.Reset();
	m_viewControl.SetParentWindow(GetID());
	m_viewControl.AddView(GetControl(CONTROL_LIST));
	m_viewControl.AddView(GetControl(CONTROL_THUMBS));
}

void CGUIDialogFileBrowser::OnWindowUnload()
{
	CGUIDialog::OnWindowUnload();
	m_viewControl.Reset();
}

bool CGUIDialogFileBrowser::ShowAndGetImage(const CFileItemList &items, const VECSOURCES &shares, const CStdString &heading, CStdString &result, bool* flip, int label)
{
	CStdString mask = ".png|.jpg|.bmp|.gif";
	CGUIDialogFileBrowser *browser = new CGUIDialogFileBrowser();
	
	if (!browser)
		return false;

	g_windowManager.AddUniqueInstance(browser);

	browser->m_browsingForImages = true;
	browser->m_singleList = true;
	browser->m_vecItems->Clear();
	browser->m_vecItems->Append(items);
	
	if (true)
	{
		CFileItemPtr item(new CFileItem("image://Browse", false));
		item->SetLabel(g_localizeStrings.Get(20153));
		item->SetIconImage("DefaultFolder.png");
		browser->m_vecItems->Add(item);
	}
	
	browser->SetHeading(heading);
	browser->m_flipEnabled = flip?true:false;
	browser->DoModalThreadSafe();

	bool confirmed(browser->IsConfirmed());
	if (confirmed)
	{
		result = browser->m_selectedPath;
		if (result == "image://Browse")
		{
			// "Browse for thumb"
			g_windowManager.Remove(browser->GetID());
			delete browser;
			return ShowAndGetImage(shares, g_localizeStrings.Get(label), result);
		}
	}

	if (flip)
		*flip = browser->m_bFlip != 0;

	g_windowManager.Remove(browser->GetID());
	delete browser;

	return confirmed;
}

bool CGUIDialogFileBrowser::ShowAndGetImage(const VECSOURCES &shares, const CStdString &heading, CStdString &path)
{
	return ShowAndGetFile(shares, ".png|.jpg|.bmp|.gif|.tbn", heading, path, true); // True for use thumbs
}

bool CGUIDialogFileBrowser::ShowAndGetDirectory(const VECSOURCES &shares, const CStdString &heading, CStdString &path, bool bWriteOnly)
{
	// An extension mask of "/" ensures that no files are shown
	if (bWriteOnly)
	{
		VECSOURCES shareWritable;
		for (unsigned int i = 0; i < shares.size(); ++i)
		{
			if (shares[i].IsWritable())
				shareWritable.push_back(shares[i]);
		}

		return ShowAndGetFile(shareWritable, "/w", heading, path);
	}

	return ShowAndGetFile(shares, "/", heading, path);
}

bool CGUIDialogFileBrowser::ShowAndGetFile(const VECSOURCES &shares, const CStdString &mask, const CStdString &heading, CStdString &path, bool useThumbs /* = false */, bool useFileDirectories /* = false */)
{
	CGUIDialogFileBrowser *browser = new CGUIDialogFileBrowser();

	if (!browser)
		return false;
	
	g_windowManager.AddUniqueInstance(browser);

	browser->m_useFileDirectories = useFileDirectories;

	browser->m_browsingForImages = useThumbs;
	browser->SetHeading(heading);
	browser->SetSources(shares);
	
	CStdString strMask = mask;
	if (mask == "/")
		browser->m_browsingForFolders = 1;
	else
	{
		if (mask == "/w")
		{
			browser->m_browsingForFolders = 2;
			strMask = "/";
		}
		else
			browser->m_browsingForFolders = 0;
	}

	browser->m_rootDir.SetMask(strMask);
	browser->m_selectedPath = path;
	browser->m_addNetworkShareEnabled = false;
	browser->DoModal();//	browser->DoModalThreadSafe(); // FIXME
	
	bool confirmed(browser->IsConfirmed());

	if (confirmed)
		path = browser->m_selectedPath;
	
	g_windowManager.Remove(browser->GetID());
	delete browser;
	return confirmed;
}

// Same as above, starting in a single directory
bool CGUIDialogFileBrowser::ShowAndGetFile(const CStdString &directory, const CStdString &mask, const CStdString &heading, CStdString &path, bool useThumbs /* = false */, bool useFileDirectories /* = false */)
{
	CGUIDialogFileBrowser *browser = new CGUIDialogFileBrowser();
	
	if (!browser)
		return false;
	
	g_windowManager.AddUniqueInstance(browser);

	browser->m_useFileDirectories = useFileDirectories;
	
	// Add a single share for this directory
	VECSOURCES shares;
	CMediaSource share;
	share.strPath = directory;
	URIUtils::RemoveSlashAtEnd(share.strPath); // This is needed for the dodgy code in WINDOW_INIT
	shares.push_back(share);
	browser->m_browsingForImages = useThumbs;
	browser->SetHeading(heading);
	browser->SetSources(shares);
	CStdString strMask = mask;

	if (mask == "/")
		browser->m_browsingForFolders=1;
	else
	{
		if (mask == "/w")
		{
			browser->m_browsingForFolders=2;
			strMask = "/";
		}
		else
			browser->m_browsingForFolders = 0;
	}

	browser->m_rootDir.SetMask(strMask);
	browser->m_selectedPath = directory;
	browser->m_addNetworkShareEnabled = false;
	browser->DoModalThreadSafe();
	
	bool confirmed(browser->IsConfirmed());
	
	if (confirmed)
		path = browser->m_selectedPath;
	
	g_windowManager.Remove(browser->GetID());
	delete browser;
	return confirmed;
}

void CGUIDialogFileBrowser::SetHeading(const CStdString &heading)
{
	 Initialize();
	SET_CONTROL_LABEL(CONTROL_HEADING_LABEL, heading);
}

bool CGUIDialogFileBrowser::ShowAndGetSource(CStdString &path, bool allowNetworkShares, VECSOURCES* additionalShare /* = NULL */, const CStdString& strType /* = "" */)
{
	// Technique is
	// 1.  Show Filebrowser with currently defined local, and optionally the network locations.
	// 2.  Have the "Add Network Location" option in addition.
	// 3a. If the "Add Network Location" is pressed, then:
	//     a) Fire up the network location dialog to grab the new location
	//     b) Check the location by doing a GetDirectory() - if it fails, prompt the user
	//        to allow them to add currently disconnected network shares.
	//     c) Save this location to our xml file (network.xml)
	//     d) Return to 1.
	// 3b. If the "Add Source" is pressed, then:
	//     a) Fire up the media source dialog to add the new location
	// 4.  Optionally allow user to browse the local and network locations for their share.
	// 5.  On OK, return.

	// Create a new filebrowser window
	CGUIDialogFileBrowser *browser = new CGUIDialogFileBrowser();
	if (!browser) return false;

	// Add it to our window manager
	g_windowManager.AddUniqueInstance(browser);

	VECSOURCES shares;
	if (!strType.IsEmpty())
	{
		if (additionalShare)
			shares = *additionalShare;
		
		browser->m_addSourceType = strType;
	}
	else
	{
		browser->SetHeading(g_localizeStrings.Get(1023));

		g_mediaManager.GetLocalDrives(shares);

		// Now the additional share if appropriate
		if (additionalShare)
		{
			shares.insert(shares.end(),additionalShare->begin(),additionalShare->end());
		}

		// Now add the network shares...
		if (allowNetworkShares)
		{
			g_mediaManager.GetNetworkLocations(shares);
		}
	}

	browser->SetSources(shares);
	browser->m_rootDir.SetMask("/");
	browser->m_rootDir.AllowNonLocalSources(false); // Don't allow plug n play shares
	browser->m_browsingForFolders = 1;
	browser->m_addNetworkShareEnabled = allowNetworkShares;
	browser->m_selectedPath = "";
	browser->DoModal(); //browser->DoModalThreadSafe(); // TODO // FIXME - Important to fix asap!
	
	bool confirmed = browser->IsConfirmed();
	if (confirmed)
		path = browser->m_selectedPath;

	g_windowManager.Remove(browser->GetID());
	delete browser;
	return confirmed;
}

void CGUIDialogFileBrowser::SetSources(const VECSOURCES &shares)
{
	m_shares = shares;
	
	if (!m_shares.size() && m_addSourceType.IsEmpty())
		g_mediaManager.GetLocalDrives(m_shares);
	
	m_rootDir.SetSources(m_shares);
}

void CGUIDialogFileBrowser::OnAddNetworkLocation()
{
	// Ok, fire up the network location dialog
	CStdString path;
	
	if (CGUIDialogNetworkSetup::ShowAndGetNetworkAddress(path))
	{
		// Verify the path by doing a GetDirectory.
		CFileItemList items;
		
//		if (CDirectory::GetDirectory(path, items, "", false, true) || CGUIDialogYesNo::ShowAndGetInput(1001,1002,1003,1004)) // TODO
		{
			// Add the network location to the shares list
			CMediaSource share;
			share.strPath = path;
			CURL url(path);
			share.strName = url.GetWithoutUserDetails();
			URIUtils::RemoveSlashAtEnd(share.strName);
			m_shares.push_back(share);
			
			// Add to our location manager...
			g_mediaManager.AddNetworkLocation(path);
		}
	}
	m_rootDir.SetSources(m_shares);
	Update(m_vecItems->GetPath());
}

void CGUIDialogFileBrowser::OnAddMediaSource()
{
	if (CGUIDialogMediaSource::ShowAndAddMediaSource(m_addSourceType))
	{
		SetSources(*g_settings.GetSourcesFromType(m_addSourceType));
		Update("");
	}
}

void CGUIDialogFileBrowser::OnEditMediaSource(CFileItem* pItem)
{
	if (CGUIDialogMediaSource::ShowAndEditMediaSource(m_addSourceType,pItem->GetLabel()))
	{
		SetSources(*g_settings.GetSourcesFromType(m_addSourceType));
		Update("");
	}
}

bool CGUIDialogFileBrowser::OnPopupMenu(int iItem)
{
	CContextButtons choices;
	choices.Add(1, m_addSourceType.IsEmpty() ? 20133 : 21364);
	choices.Add(2, m_addSourceType.IsEmpty() ? 20134 : 21365);

	int btnid =0;// CGUIDialogContextMenu::ShowAndGetChoice(choices); // TODO
	if (btnid == 1)
	{
		if (m_addNetworkShareEnabled)
		{
			CStdString strOldPath=m_selectedPath,newPath=m_selectedPath;
			VECSOURCES shares=m_shares;
			
//			if (CGUIDialogNetworkSetup::ShowAndGetNetworkAddress(newPath)) // TODO
			{
				g_mediaManager.SetLocationPath(strOldPath,newPath);
				
				for (unsigned int i=0;i<shares.size();++i)
				{
					if (shares[i].strPath.Equals(strOldPath))
					{
						shares[i].strName = newPath;
						shares[i].strPath = newPath;
						break;
					}
				}
				
				// Re-open our dialog
				SetSources(shares);
				m_rootDir.SetMask("/");
				m_browsingForFolders = 1;
				m_addNetworkShareEnabled = true;
				m_selectedPath = newPath;
				DoModal();
			}
		}
		else
		{
			CFileItemPtr item = m_vecItems->Get(iItem);
			OnEditMediaSource(item.get());
		}
	}
	
	if (btnid == 2)
	{
		if (m_addNetworkShareEnabled)
		{
			g_mediaManager.RemoveLocation(m_selectedPath);

			for (unsigned int i=0;i<m_shares.size();++i)
			{
				if (m_shares[i].strPath.Equals(m_selectedPath) && !m_shares[i].m_ignore)
				{
					m_shares.erase(m_shares.begin()+i);
					break;
				}
			}

			m_rootDir.SetSources(m_shares);
			m_rootDir.SetMask("/");

			m_browsingForFolders = 1;
			m_addNetworkShareEnabled = true;
			m_selectedPath = "";

			Update(m_Directory->GetPath());
		}
		else
		{
			g_settings.DeleteSource(m_addSourceType,(*m_vecItems)[iItem]->GetLabel(),(*m_vecItems)[iItem]->GetPath());
			SetSources(*g_settings.GetSourcesFromType(m_addSourceType));
			Update("");
		}
	}
	return true;
}

CFileItemPtr CGUIDialogFileBrowser::GetCurrentListItem(int offset)
{
	int item = m_viewControl.GetSelectedItem();

	if (item < 0 || !m_vecItems->Size())
		return CFileItemPtr();

	item = (item + offset) % m_vecItems->Size();
	
	if (item < 0)
		item += m_vecItems->Size();

	return (*m_vecItems)[item];
}

CGUIControl *CGUIDialogFileBrowser::GetFirstFocusableControl(int id)
{
	if (m_viewControl.HasControl(id))
		id = m_viewControl.GetCurrentControl();

	return CGUIWindow::GetFirstFocusableControl(id);
}