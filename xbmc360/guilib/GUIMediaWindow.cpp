#include "GUIMediaWindow.h"
#include "..\FileItem.h"
#include "..\Application.h"
#include "GUIThumbnailPanel.h"

#include "..\filesystem\HDDirectory.h" // TESTING

#define CONTROL_THUMBS				 51
#define CONTROL_THUMBS_SCROLLBAR	 52
#define CONTROL_THUMBS_COUNTER_LABEL 53

CGUIMediaWindow::CGUIMediaWindow(int id, const CStdString &xmlFile)
    : CGUIWindow(id, xmlFile)
{
}

CGUIMediaWindow::~CGUIMediaWindow(void)
{
}

void CGUIMediaWindow::OnWindowLoaded() // TODO: The other view types
{
	CGUIWindow::OnWindowLoaded();
	m_viewControl.Reset();

	m_viewControl.SetParentWindow(GetID());
/*	m_viewControl.AddView(VIEW_METHOD_LIST, GetControl(CONTROL_LIST));
*/	m_viewControl.AddView(VIEW_METHOD_THUMBS, GetControl(CONTROL_THUMBS));
/*	m_viewControl.AddView(VIEW_METHOD_LARGE_ICONS, GetControl(CONTROL_THUMBS));
	m_viewControl.AddView(VIEW_METHOD_LARGE_LIST, GetControl(CONTROL_BIG_LIST));
	m_viewControl.SetViewControlID(CONTROL_BTNVIEWASICONS);
	SetupShares();*/

	CGUIThumbnailPanel* pCtrlThumbnails = (CGUIThumbnailPanel*)GetControl(CONTROL_THUMBS);
	if(pCtrlThumbnails)
	{
		// Set the scrollbar for the thumbnail control
		pCtrlThumbnails->SetScrollbarControl(CONTROL_THUMBS_SCROLLBAR);

		// Set the object / page counter label control for the thubs control
		pCtrlThumbnails->SetObjectLabelControl(CONTROL_THUMBS_COUNTER_LABEL);
	}
}

void CGUIMediaWindow::OnWindowUnload()
{
	CGUIWindow::OnWindowUnload();
	m_viewControl.Reset();
}

void CGUIMediaWindow::OnInitWindow()
{
//	Update(m_vecItems.m_strPath); // TODO

//	if (m_iSelectedItem > -1)
//		m_viewControl.SetSelectedItem(m_iSelectedItem);  // TODO

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

/*			if (iControl == CONTROL_BTNVIEWASICONS)   // TODO - Other view types
			{
				if (m_guiState.get())
				while (!m_viewControl.HasViewMode(m_guiState->SetNextViewAsControl()));

				UpdateButtons();
				return true;
			}
			else if (iControl == CONTROL_BTNSORTASC) // sort asc
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
			}
			else */ if (m_viewControl.HasControl(iControl))  // Thumb control
			{
				int iItem = m_viewControl.GetSelectedItem();
				int iAction = message.GetParam1();
				
				if (iItem < 0) break;
				
				if (iAction == ACTION_SELECT_ITEM)
				{
					OnClick(iItem);
				}
			}
		}
		break;
	}

	return CGUIWindow::OnMessage(message);
}

// With this function you can react on a users click in the list/thumb panel.
// It returns true, if the click is handled.
// This function calls OnPlayMedia()
bool CGUIMediaWindow::OnClick(int iItem)
{
	if (iItem < 0 || iItem >= (int)m_vecItems.Size()) return true;
	CFileItem* pItem = m_vecItems[iItem];

	// TODO - Handle parent folders and folders

/*	if (pItem->IsParentFolder())
	{
		// TODO

		return true;
	}
	else if (pItem->m_bIsFolder)
	{
 		// TODO
	}
    else
*/	{
      return OnPlayMedia(iItem);
    }

	return false;
}

// Override if you want to change the default behavior,
// what is done when the user clicks on a file.
// This function is called by OnClick()
bool CGUIMediaWindow::OnPlayMedia(int iItem)
{
	CFileItem* pItem=m_vecItems[iItem];

	return g_application.PlayFile("D:\\testvideos\\" + pItem->GetLabel()); // FIXME: Don't hardcode tempoary test folder
}																		   //		 will be removed once settings souces works 

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
	//TEST
	XFILE::CHDDirectory directory;

	// Find all video in test video folder!
	return directory.GetDirectory(strDirectory, items);

	return false;
	//TEST END
}

// Set window to a specific directory
// param strDirectory The directory to be displayed in list/thumb control
// This function calls OnPrepareFileItems() and OnFinalizeFileItems()
bool CGUIMediaWindow::Update(const CStdString &strDirectory)			//TODO - Reimplement commented sections
{
	// Get selected item
/*	int iItem = m_viewControl.GetSelectedItem();
	
	CStdString strSelectedItem = "";
	if (iItem >= 0 && iItem < m_vecItems.Size())
	{
		CFileItem* pItem = m_vecItems[iItem];
		if (!pItem->IsParentFolder())
		{
			GetDirectoryHistoryString(pItem, strSelectedItem);
		}
	}

	CStdString strOldDirectory = m_vecItems.m_strPath;

	m_history.SetSelectedItem(strSelectedItem, strOldDirectory);
*/
	ClearFileItems();

	if (!GetDirectory(strDirectory, m_vecItems))
	{
/*		CLog::Log(LOGERROR,"CGUIMediaWindow::GetDirectory(%s) failed", strDirectory.c_str());
		// if the directory is the same as the old directory, then we'll return
		// false.  Else, we assume we can get the previous directory
		if (strDirectory.Equals(strOldDirectory))
			return false;

		// We assume, we can get the parent 
		// directory again, but we have to 
		// return false to be able to eg. show 
		// an error message.    
		CStdString strParentPath = m_history.GetParentPath();
		m_history.RemoveParentPath();
		Update(strParentPath);
*/		return false;
	} 

	// if we're getting the root bookmark listing
	// make sure the path history is clean
/*	if (strDirectory.IsEmpty())
		m_history.ClearPathHistory();

	int iWindow = GetID();
	bool bOkay = (iWindow == WINDOW_MUSIC_FILES || iWindow == WINDOW_VIDEO_FILES || iWindow == WINDOW_FILES || iWindow == WINDOW_PICTURES || iWindow == WINDOW_PROGRAMS);
	
	if (strDirectory.IsEmpty() && m_vecItems.IsEmpty() && bOkay) // add 'add source button'
	{
		CStdString strLabel = g_localizeStrings.Get(1026);
		CFileItem *pItem = new CFileItem(strLabel);
		pItem->m_strPath = "add";
		pItem->SetThumbnailImage("settings-network-focus.png");
		pItem->SetLabel(strLabel);
		pItem->SetLabelPreformated(true);
		m_vecItems.Add(pItem);
	}
	m_iLastControl = GetFocusedControl();

	//  Ask the derived class if it wants to load additional info
	//  for the fileitems like media info or additional 
	//  filtering on the items, setting thumbs.
	OnPrepareFileItems(m_vecItems);

	m_vecItems.FillInDefaultIcons();

	m_guiState.reset(CGUIViewState::GetViewState(GetID(), m_vecItems));
*/	OnSort();
/*	UpdateButtons();

	// Ask the devived class if it wants to do custom list operations,
	// eg. changing the label
	OnFinalizeFileItems(m_vecItems);

	strSelectedItem = m_history.GetSelectedItem(m_vecItems.m_strPath);

	const CFileItem &currentItem = g_application.CurrentFileItem();

	bool isPlayingWindow=(m_guiState.get() && m_guiState->IsCurrentPlaylistDirectory(m_vecItems.m_strPath));

	bool bSelectedFound = false, bCurrentSongFound = false;
	//int iSongInDirectory = -1;
	
	for (int i = 0; i < m_vecItems.Size(); ++i)
	{
		CFileItem* pItem = m_vecItems[i];

		// unselect all items
		if (pItem)
			pItem->Select(false);

		// Update selected item
		if (!bSelectedFound)
		{
			CStdString strHistory;
			GetDirectoryHistoryString(pItem, strHistory);
			
			if (strHistory == strSelectedItem)
			{
				m_viewControl.SetSelectedItem(i);
				bSelectedFound = true;
			}
		}

		// synchronize playlist with current directory
		if (isPlayingWindow && !bCurrentSongFound && !currentItem.m_strPath.IsEmpty())
		{
			if (pItem->m_strPath == currentItem.m_strPath &&
				pItem->m_lStartOffset == currentItem.m_lStartOffset)
			{
				pItem->Select(true);
				bCurrentSongFound = true;
			}
		}
	}

	m_history.AddPath(strDirectory);

	//m_history.DumpPathHistory();
*/
	return true;
}

void CGUIMediaWindow::ClearFileItems()
{
	m_viewControl.Clear();
	m_vecItems.Clear(); // Will clean up everything
}