#include "GUIMediaWindow.h"
#include "FileItem.h"
#include "GUIThumbnailPanel.h"
#include "GUIListControl.h"
#include "Application.h"
#include "LocalizeStrings.h"
#include "utils\log.h"
#include "utils\Util.h"
#include "dialogs\GUIDialogContextMenu.h"
#include "GUIWindowManager.h"

#define CONTROL_BTNVIEWASICONS        2
#define CONTROL_LIST                 50
#define CONTROL_THUMBS               51
#define CONTROL_SCROLLBAR            52
#define CONTROL_ITEM_COUNTER_LABEL   53

CGUIMediaWindow::CGUIMediaWindow(int id, const CStdString &xmlFile)
    : CGUIWindow(id, xmlFile)
{
	m_vecItems.m_strPath = "?";

	m_guiState.reset(CGUIViewState::GetViewState(GetID(), m_vecItems));
}

CGUIMediaWindow::~CGUIMediaWindow(void)
{
}

void CGUIMediaWindow::OnWindowLoaded() // TODO: The other view types
{
	CGUIWindow::OnWindowLoaded();
	m_viewControl.Reset();

	m_viewControl.SetParentWindow(GetID());
	m_viewControl.AddView(VIEW_METHOD_LIST, GetControl(CONTROL_LIST));
	m_viewControl.AddView(VIEW_METHOD_THUMBS, GetControl(CONTROL_THUMBS));
/*	m_viewControl.AddView(VIEW_METHOD_LARGE_ICONS, GetControl(CONTROL_THUMBS));
	m_viewControl.AddView(VIEW_METHOD_LARGE_LIST, GetControl(CONTROL_BIG_LIST));*/
	m_viewControl.SetViewControlID(CONTROL_BTNVIEWASICONS);
	SetupShares();

	CGUIThumbnailPanel* pCtrlThumbnails = (CGUIThumbnailPanel*)GetControl(CONTROL_THUMBS);
	if(pCtrlThumbnails)
	{
		// Set the scrollbar for the thumbnail control
		pCtrlThumbnails->SetScrollbarControl(CONTROL_SCROLLBAR);

		// Set the object / page counter label control for the thubs control
		pCtrlThumbnails->SetObjectLabelControl(CONTROL_ITEM_COUNTER_LABEL);
	}

	CGUIListControl* pCtrlList = (CGUIListControl*)GetControl(CONTROL_LIST);
	if(pCtrlList)
	{
		// Set the scrollbar for the thumbnail control
		pCtrlList->SetScrollbarControl(CONTROL_SCROLLBAR);

		// Set the object / page counter label control for the thubs control
		pCtrlList->SetObjectLabelControl(CONTROL_ITEM_COUNTER_LABEL);
	}
}

void CGUIMediaWindow::OnWindowUnload()
{
	CGUIWindow::OnWindowUnload();
	m_viewControl.Reset();
}

void CGUIMediaWindow::OnInitWindow()
{
	Update(m_vecItems.GetPath());

//	if(m_iSelectedItem > -1)
//		m_viewControl.SetSelectedItem(m_iSelectedItem);

	CGUIWindow::OnInitWindow();
}

bool CGUIMediaWindow::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_WINDOW_DEINIT:
		{
//			m_iSelectedItem = m_viewControl.GetSelectedItem();
//			m_iLastControl = GetFocusedControl();

			CGUIWindow::OnMessage(message);

			// Call ClearFileItems() after our window has 
			// finished doing any WindowClose animations
			ClearFileItems(); // Cleanup all items

			return true;
		}
		break;

		case GUI_MSG_CLICKED:
		{
			int iControl = message.GetSenderId();

			if(iControl == CONTROL_BTNVIEWASICONS)
			{
				if(m_guiState.get())
				while(!m_viewControl.HasViewMode(m_guiState->SetNextViewAsControl()));

				UpdateButtons();
				return true;
			}
/*			else if (iControl == CONTROL_BTNSORTASC) // sort asc
			{
				if (m_guiState.get())
				m_guiState->SetNextSortOrder();
				UpdateFileList();
				return true;
			}
			else if (iControl == CONTROL_BTNSORTBY) // sort by
			{
				if (m_guiState.get())
				m_guiState->SetNextSortMethod();
				UpdateFileList();
				return true;
			}*/ 
			else if(m_viewControl.HasControl(iControl)) // list/thumb control
			{
				int iItem = m_viewControl.GetSelectedItem();
				int iAction = message.GetParam1();
				
				if(iItem < 0) break;

				if (ACTION_CONTEXT_MENU == iAction)
				{
					int iItem = m_viewControl.GetSelectedItem();
					// iItem is checked inside OnPopupMenu
					OnPopupMenu(iItem);
				}

				if(iAction == ACTION_SELECT_ITEM)
					OnClick(iItem);
			}
		}
		break;

		case GUI_MSG_SETFOCUS:
		{
			if(m_viewControl.HasControl(message.GetControlId()) && m_viewControl.GetCurrentControl() != message.GetControlId())
			{
				m_viewControl.SetFocused();
				return true;
			}
		}
		break;
	}

	return CGUIWindow::OnMessage(message);
}

bool CGUIMediaWindow::OnBack(int actionID)
{
	if((actionID == ACTION_NAV_BACK || actionID == ACTION_PREVIOUS_MENU) && !m_vecItems.IsVirtualDirectoryRoot())
	{
		GoParentFolder();
		return true;
	}

	return CGUIWindow::OnBack(actionID);
}

// Updates the states (enable, disable, visible...) 
// of the controls defined by this window
// Override this function in a derived class to add new controls
void CGUIMediaWindow::UpdateButtons()
{
    // Update list/thumb control
    m_viewControl.SetCurrentView(m_guiState->GetViewAsControl());

	int iItems = m_vecItems.Size();
	if(iItems)
	{
		CFileItem* pItem = m_vecItems[0];
		if(pItem->IsParentFolder()) iItems--;
	}
}

// With this function you can react on a users click in the list/thumb panel.
// It returns true, if the click is handled.
// This function calls OnPlayMedia()
bool CGUIMediaWindow::OnClick(int iItem)
{
	if(iItem < 0 || iItem >= (int)m_vecItems.Size())
		return true;

	CFileItem* pItem = m_vecItems[iItem];

	if(pItem->IsParentFolder())
	{
		GoParentFolder();
		return true;
	}
	else if(pItem->m_bIsFolder)
	{
		if(pItem->m_bIsShareOrDrive)
		{
//			if(!HaveDiscOrConnection(pItem->m_strPath, pItem->m_iDriveType))
			//	return true;
		}
		CFileItem directory(*pItem);
		
		if(!Update(directory.m_strPath))
		{
//			ShowShareErrorMessage(&directory);
		}

		return true;
	}
	else
	{
//		m_iSelectedItem = m_viewControl.GetSelectedItem();

		return OnPlayMedia(iItem);

	}
	return false;
}

// When the user clicks on a file.
// This function is called by OnClick()
bool CGUIMediaWindow::OnPlayMedia(int iItem)
{
	CFileItem* pItem = m_vecItems[iItem];

	bool bResult = g_application.PlayFile(*pItem);
  
	return bResult;
}

bool CGUIMediaWindow::OnPopupMenu(int iItem)
{
	// Calculate our position
	float posX = 200;
	float posY = 100;

	const CGUIControl *pList = GetControl(CONTROL_LIST);
	if(pList)
	{
		posX = pList->GetXPosition() + pList->GetWidth() / 2;
		posY = pList->GetYPosition() + pList->GetHeight() / 2;
	}

	if(m_vecItems.IsVirtualDirectoryRoot())
	{
		if(iItem < 0)
		{
			// TODO: Add option to add shares in this case
			return false;
		}

		// Mark the item
		m_vecItems[iItem]->Select(true);

		// And do the popup menu
		if(CGUIDialogContextMenu::SourcesMenu(GetWindowType(), m_vecItems[iItem], posX, posY))
		{
	//		m_rootDir.SetShares(g_settings.m_vecMyProgramsShares); // TODO: Check what Window we are in
			Update("");
			return true;
		}

		m_vecItems[iItem]->Select(false);
		return false;
	}

	return false;
}

CStdString CGUIMediaWindow::GetWindowType()
{
	switch(g_windowManager.GetActiveWindow())
	{
		case WINDOW_PROGRAMS:
		return "programs";

		case WINDOW_VIDEOS:
		return "videos";

		case WINDOW_MUSIC:
		return "music";

		case WINDOW_PICTURES:
		return "pictures";
	}
	return "";
}

// Prepares and adds the fileitems list/thumb panel
void CGUIMediaWindow::OnSort()
{
	// TODO
//	FormatItemLabels();
//	SortItems(m_vecItems);
	m_viewControl.SetItems(m_vecItems);
}

// Overwrite to fill fileitems from a source
// param strDirectory Path to read
// param items Fill with items specified in \e strDirectory
bool CGUIMediaWindow::GetDirectory(const CStdString &strDirectory, CFileItemList &items)
{
	// Cleanup items
	if(items.Size())
		items.Clear();

	CStdString strParentPath = m_history.GetParentPath();

	CLog::Log(LOGDEBUG,"CGUIMediaWindow::GetDirectory (%s)", strDirectory.c_str());
	CLog::Log(LOGDEBUG,"  ParentPath = [%s]", strParentPath.c_str());

	if(m_guiState.get()/* && !m_guiState->HideParentDirItems()*/) // TODO: Do we want option to hide parent items?
	{
		CFileItem *pItem = new CFileItem("..");
		pItem->SetPath(strParentPath);
		pItem->m_bIsFolder = true;
		pItem->m_bIsShareOrDrive = false;
		items.Add(pItem);
	}

	if(!m_rootDir.GetDirectory(strDirectory, items))
		return false;

	return true;
}

// Override the function to change the default behavior on how
// a selected item history should look like
void CGUIMediaWindow::GetDirectoryHistoryString(const CFileItem* pItem, CStdString& strHistoryString)
{
	if(pItem->m_bIsShareOrDrive)
	{
		// We are in the virual directory

		// History string of the DVD drive
		// must be handel separately
/*		if(pItem->m_iDriveType == SHARE_TYPE_DVD)
		{
			// Remove disc label from item label
			// and use as history string, m_strPath
			// can change for new discs
			CStdString strLabel = pItem->GetLabel();
			int nPosOpen = strLabel.Find('(');
			int nPosClose = strLabel.ReverseFind(')');
			
			if(nPosOpen > -1 && nPosClose > -1 && nPosClose > nPosOpen)
			{
				strLabel.Delete(nPosOpen + 1, (nPosClose) - (nPosOpen + 1));
				strHistoryString = strLabel;
			}
			else
				strHistoryString = strLabel;
		}
		else
*/		{
			// Other items in virual directory
			CStdString strPath = pItem->GetPath();

			while(CUtil::HasSlashAtEnd(strPath))
				strPath.Delete(strPath.size() - 1);

			strHistoryString = pItem->GetLabel() + strPath;
		}
	}
/*	else if(pItem->m_lEndOffset>pItem->m_lStartOffset && pItem->m_lStartOffset != -1)
	{
		// Could be a cue item, all items of a cue share the same filename
		// so add the offsets to build the history string
		strHistoryString.Format("%ld%ld", pItem->m_lStartOffset, pItem->m_lEndOffset);
		strHistoryString += pItem->m_strPath;

		if(CUtil::HasSlashAtEnd(strHistoryString))
			strHistoryString.Delete(strHistoryString.size() - 1);
	}
	else
*/	{
		// Normal directory items
		strHistoryString = pItem->GetPath();

		if(CUtil::HasSlashAtEnd(strHistoryString))
			strHistoryString.Delete(strHistoryString.size() - 1);
	}
	strHistoryString.ToLower();
}

// Call this function to create a directory history
// for the path given by strDirectory
void CGUIMediaWindow::SetHistoryForPath(const CStdString& strDirectory)
{
	// Make sure our shares are configured
	SetupShares();
	if(!strDirectory.IsEmpty())
	{
		// Build the directory history for default path
		CStdString strPath, strParentPath;
		strPath = strDirectory;

		while(CUtil::HasSlashAtEnd(strPath))
			strPath.Delete(strPath.size() - 1);

		CFileItemList items;
		m_rootDir.GetDirectory("", items);

		m_history.ClearPathHistory();

		while(CUtil::GetParentPath(strPath, strParentPath))
		{
			bool bSet = false;
			for(int i = 0; i < (int)items.Size(); ++i)
			{
				CFileItem* pItem = items[i];

				while(CUtil::HasSlashAtEnd(pItem->m_strPath))
					pItem->m_strPath.Delete(pItem->m_strPath.size() - 1);
    
				if(pItem->GetPath() == strPath)
				{
					CStdString strHistory;
					GetDirectoryHistoryString(pItem, strHistory);
					m_history.SetSelectedItem(strHistory, "");
					m_history.AddPathFront(strPath);
					m_history.AddPathFront("");

					//m_history.DumpPathHistory();
					return;
				}
			}

			m_history.AddPathFront(strPath);
			m_history.SetSelectedItem(strPath, strParentPath);
			strPath = strParentPath;
			
			while(CUtil::HasSlashAtEnd(strPath))
				strPath.Delete(strPath.size() - 1);
			}
		}
		else
			m_history.ClearPathHistory();
		
	m_history.DumpPathHistory();
}

// Set window to a specific directory
// param strDirectory The directory to be displayed in list/thumb control
// This function calls OnPrepareFileItems() and OnFinalizeFileItems()
bool CGUIMediaWindow::Update(const CStdString &strDirectory)
{
	// Get selected item
	int iItem = m_viewControl.GetSelectedItem();
	CStdString strSelectedItem = "";

	if(iItem >= 0 && iItem < m_vecItems.Size())
	{
		CFileItem* pItem = m_vecItems[iItem];
		if(!pItem->IsParentFolder())
		{
			GetDirectoryHistoryString(pItem, strSelectedItem);
		}
	}

	CStdString strOldDirectory = m_vecItems.GetPath();

	m_history.SetSelectedItem(strSelectedItem, strOldDirectory);

	ClearFileItems();

	if(!GetDirectory(strDirectory, m_vecItems))
	{
		CLog::Log(LOGERROR,"CGUIMediaWindow::GetDirectory(%s) failed", strDirectory.c_str());

		// If the directory is the same as the old directory, then we'll return
		// false. Else, we assume we can get the previous directory
		if (strDirectory.Equals(strOldDirectory))
			return false;

		// We assume, we can get the parent 
		// directory again, but we have to 
		// return false to be able to eg. show 
		// an error message.    
		CStdString strParentPath = m_history.GetParentPath();
		m_history.RemoveParentPath();
		Update(strParentPath);

		return false;
	} 

	// If we're getting the root bookmark listing
	// make sure the path history is clean
	if(strDirectory.IsEmpty())
		m_history.ClearPathHistory();

	int iWindow = GetID();
	bool bOkay = (iWindow == WINDOW_MUSIC || iWindow == WINDOW_VIDEOS || iWindow == WINDOW_PICTURES || iWindow == WINDOW_PROGRAMS);
	
	bool bTest = m_vecItems.IsEmpty();

	if(strDirectory.IsEmpty()/* && m_vecItems.IsEmpty()*/ && bOkay) // Add 'add source button'
	{
		CStdString strLabel = "Add source";//g_localizeStrings.Get(1026); //FIXME
		CFileItem *pItem = new CFileItem(strLabel);
		pItem->SetPath("add");
		pItem->SetThumbnailImage("DefaultAddSource.png");
		pItem->SetLabel(strLabel);
//		pItem->SetLabelPreformated(true);
		m_vecItems.Add(pItem);
	}

//	m_iLastControl = GetFocusedControl();
/*
	//  Ask the derived class if it wants to load additional info
	//  for the fileitems like media info or additional 
	//  filtering on the items, setting thumbs.
	OnPrepareFileItems(m_vecItems);
*/
	m_vecItems.FillInDefaultIcons();

	m_guiState.reset(CGUIViewState::GetViewState(GetID(), m_vecItems));
	OnSort();
	UpdateButtons();

/*	// Ask the devived class if it wants to do custom list operations,
	// eg. changing the label
	OnFinalizeFileItems(m_vecItems);

	strSelectedItem = m_history.GetSelectedItem(m_vecItems.m_strPath);

	const CFileItem &currentItem = g_application.CurrentFileItem();

	bool isPlayingWindow=(m_guiState.get() && m_guiState->IsCurrentPlaylistDirectory(m_vecItems.m_strPath));
*/
	bool bSelectedFound = false;
	
	for (int i = 0; i < m_vecItems.Size(); ++i)
	{
		CFileItem* pItem = m_vecItems[i];

		// Update selected item
		if(!bSelectedFound)
		{
			CStdString strHistory;
			GetDirectoryHistoryString(pItem, strHistory);
			
			if(strHistory == strSelectedItem)
			{
				m_viewControl.SetSelectedItem(i);
				bSelectedFound = true;
			}
		}
	}

	m_history.AddPath(strDirectory);

	return true;
}

void CGUIMediaWindow::ClearFileItems()
{
	m_viewControl.Clear();
	m_vecItems.Clear(); // Will clean up everything
}

// The functon goes up one level in the directory tree
void CGUIMediaWindow::GoParentFolder()
{
	// Remove current directory if its on the stack
	if(m_history.GetParentPath() == m_vecItems.m_strPath)
		m_history.RemoveParentPath();

	// If vector is not empty, pop parent
	// If vector is empty, parent is bookmark listing
	CStdString strParent = m_history.RemoveParentPath();

	CStdString strOldPath(m_vecItems.m_strPath);
	Update(strParent);

//	if(!g_guiSettings.GetBool("filelists.fulldirectoryhistory"))
//		m_history.RemoveSelectedItem(strOldPath); //Delete current path
}

void CGUIMediaWindow::SetupShares()
{
	// Setup shares and filemasks for this window
	CFileItemList items;
	CGUIViewState* viewState = CGUIViewState::GetViewState(GetID(), items);

	if(viewState)
	{
		m_rootDir.SetMask(viewState->GetExtensions());
		m_rootDir.SetShares(viewState->GetShares());
		delete viewState;
	}
}