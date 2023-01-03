#include "GUIWindowPrograms.h"
#include "utils\Log.h"
#include "utils\Util.h"
#include "utils\XEXUtils.h"
#include "guilib\dialogs\GUIDialogMediaSource.h"
#include "guilib\GUIWindowManager.h"
#include "utils\URIUtils.h"
#include "filesystem\File.h"

using namespace XFILE;

CGUIWindowPrograms::CGUIWindowPrograms(void)
	: CGUIMediaWindow(WINDOW_PROGRAMS, "MyPrograms.xml")
{
	m_dlgProgress = NULL;
	m_rootDir.AllowNonLocalSources(false); // No nonlocal shares for this window please
}

CGUIWindowPrograms::~CGUIWindowPrograms(void)
{
}

bool CGUIWindowPrograms::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_DEINIT: // TODO
		{
/*			if (m_thumbLoader.IsLoading())
				m_thumbLoader.StopThread();
			
			m_database.Close();
*/		}
		break;

		case GUI_MSG_WINDOW_INIT: // TODO
		{
//			m_iRegionSet = 0;
			m_dlgProgress = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);

			// Is this the first time accessing this window?
			if (m_vecItems->GetPath() == "?" && message.GetStringParam().IsEmpty())
				message.SetStringParam(/*g_settings.m_defaultProgramSource*/""); // TODO

//			m_database.Open();

			return CGUIMediaWindow::OnMessage(message);
		}
		break;

		case GUI_MSG_CLICKED:
		{
/*			if (message.GetSenderId() == CONTROL_BTNSORTBY)
			{
				// Need to update shortcuts manually
				if (CGUIMediaWindow::OnMessage(message))
				{
					LABEL_MASKS labelMasks;
					m_guiState->GetSortMethodLabelMasks(labelMasks);
					CLabelFormatter formatter("", labelMasks.m_strLabel2File);
					
					for (int i = 0; i < m_vecItems->Size(); ++i)
					{
						CFileItemPtr item = m_vecItems->Get(i);
						if (item->IsShortCut())
							formatter.FormatLabel2(item.get());
					}
					return true;
				}
				else
					return false;
			}
*/			if (m_viewControl.HasControl(message.GetSenderId()))  // list/thumb control
			{
/*				if (message.GetParam1() == ACTION_PLAYER_PLAY) //TODO
				{
					OnPlayMedia(m_viewControl.GetSelectedItem());
					return true;
				}
*/			}
		}
		break;
	}
	return CGUIMediaWindow::OnMessage(message);
}

bool CGUIWindowPrograms::GetDirectory(const CStdString &strDirectory, CFileItemList &items)
{
	bool bFlattened = false;
	
/*	if (URIUtils::IsDVD(strDirectory)) // TODO
	{
		CStdString strPath;
		URIUtils::AddFileToFolder(strDirectory,"default.xbe",strPath);
		
		if (CFile::Exists(strPath)) // Flatten dvd
		{
			CFileItemPtr item(new CFileItem("default.xex"));
			item->SetPath(strPath);
			items.Add(item);
			items.SetPath(strDirectory);
			bFlattened = true;
		}
	}
*/	
	if (!bFlattened)
		if (!CGUIMediaWindow::GetDirectory(strDirectory, items))
			return false;

	if (items.IsVirtualDirectoryRoot())
	{
		items.SetLabel("");
		return true;
	}
/*
	if (strDirectory.Equals("plugin://programs/")) // TODO
	{
		items.SetContent("plugins");
		items.SetLabel(g_localizeStrings.Get(24001));
	}
*/
	// Flatten any folders
//	m_database.BeginTransaction();
	DWORD dwTick = CTimeUtils::timeGetTime();
	bool bProgressVisible = false;
	
	for (int i = 0; i < items.Size(); i++)
	{
		CStdString shortcutPath;
		CFileItemPtr item = items[i];
		
		if (!bProgressVisible && CTimeUtils::timeGetTime()-dwTick>1500 && m_dlgProgress)
		{
			// Tag loading takes more then 1.5 secs, show a progress dialog
			m_dlgProgress->SetHeading(189);
			m_dlgProgress->SetLine(0, 20120);
			m_dlgProgress->SetLine(1,"");
			m_dlgProgress->SetLine(2, item->GetLabel());
			m_dlgProgress->StartModal();
			bProgressVisible = true;
		}
	
		if (bProgressVisible)
		{
			m_dlgProgress->SetLine(2,item->GetLabel());
			m_dlgProgress->Progress();
		}

		if (item->m_bIsFolder && !item->IsParentFolder()/* && !item->IsPlugin()*/)
		{
			// Folder item - let's check for a default.xbe file, and flatten if we have one
			CStdString defaultXBE;
			URIUtils::AddFileToFolder(item->GetPath(), "default.xbe", defaultXBE);
			
			if (CFile::Exists(defaultXBE))
			{
				// Yes, format the item up
				item->SetPath(defaultXBE);
				item->m_bIsFolder = false;
			}
		}
/*		else if (item->IsShortCut())
		{
			// Resolve the shortcut to set it's description etc.
			// and save the old shortcut path (so we can reassign it later)
			CShortcut cut;
			if (cut.Create(item->GetPath()))
			{
				shortcutPath = item->GetPath();
				item->SetPath(cut.m_strPath);
				item->SetThumbnailImage(cut.m_strThumb);

				LABEL_MASKS labelMasks;
				m_guiState->GetSortMethodLabelMasks(labelMasks);
				CLabelFormatter formatter("", labelMasks.m_strLabel2File);
			
				if (!cut.m_strLabel.IsEmpty())
				{
					item->SetLabel(cut.m_strLabel);
					__stat64 stat;
				
					if (CFile::Stat(item->GetPath(),&stat) == 0)
						item->m_dwSize = stat.st_size;

					formatter.FormatLabel2(item.get());
					item->SetLabelPreformated(true);
				}
			}
		}
		if (item->IsXBE())
		{
			if (URIUtils::GetFileName(item->GetPath()).Equals("default_ffp.xbe"))
			{
				m_vecItems->Remove(i--);
				continue;
			}
	
			// Add to database if not already there
			DWORD dwTitleID = item->IsOnDVD() ? 0 : m_database.GetProgramInfo(item.get());
			if (!dwTitleID)
			{
				CStdString description;
			
				if (CUtil::GetXBEDescription(item->GetPath(), description) && (!item->IsLabelPreformated() && !item->GetLabel().IsEmpty()))
					item->SetLabel(description);

				dwTitleID = CUtil::GetXbeID(item->GetPath());
				if (!item->IsOnDVD())
					m_database.AddProgramInfo(item.get(), dwTitleID);
			}

			// SetOverlayIcons()
			if (m_database.ItemHasTrainer(dwTitleID))
			{
				if (m_database.GetActiveTrainer(dwTitleID) != "")
					item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_TRAINED);
				else
					item->SetOverlayImage(CGUIListItem::ICON_OVERLAY_HAS_TRAINER);
			}
		}

		if (!shortcutPath.IsEmpty())
			item->SetPath(shortcutPath);
*/	}

/*	m_database.CommitTransaction();
	
	// Set the cached thumbs
	items.SetThumbnailImage("");
	items.SetCachedProgramThumbs();
	items.SetCachedProgramThumb();
	
	if (!items.HasThumbnail())
		items.SetUserProgramThumb();
*/
	if (bProgressVisible)
		m_dlgProgress->Close();

	return true;
}

void CGUIWindowPrograms::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
	if (itemNumber < 0 || itemNumber >= m_vecItems->Size())
		return;
	
	CFileItemPtr item = m_vecItems->Get(itemNumber);

	if (item && !item->GetPropertyBOOL("pluginreplacecontextitems"))
	{
		if ( m_vecItems->IsVirtualDirectoryRoot() )
		{
			CGUIDialogContextMenu::GetContextButtons("programs", item, buttons);
		}
		else
		{
#if 0 // TODO: OG Xbox stuff to be modded to suit 360
			if (item->IsXBE() || item->IsShortCut())
			{
				CStdString strLaunch = g_localizeStrings.Get(518); // Launch
				if (g_guiSettings.GetBool("myprograms.gameautoregion"))
				{
					int iRegion = GetRegion(itemNumber);
					
					if (iRegion == VIDEO_NTSCM)
						strLaunch += " (NTSC-M)";
					if (iRegion == VIDEO_NTSCJ)
						strLaunch += " (NTSC-J)";
					if (iRegion == VIDEO_PAL50)
						strLaunch += " (PAL)";
					if (iRegion == VIDEO_PAL60)
						strLaunch += " (PAL-60)";
				}
				
				buttons.Add(CONTEXT_BUTTON_LAUNCH, strLaunch);
  
				DWORD dwTitleId = CUtil::GetXbeID(item->GetPath());
				CStdString strTitleID;
				CStdString strGameSavepath;
				strTitleID.Format("%08X",dwTitleId);
				URIUtils::AddFileToFolder("E:\\udata\\",strTitleID,strGameSavepath);
  
				if (CDirectory::Exists(strGameSavepath))
						buttons.Add(CONTEXT_BUTTON_GAMESAVES, 20322); // Goto GameSaves
  
				if (g_guiSettings.GetBool("myprograms.gameautoregion"))
					buttons.Add(CONTEXT_BUTTON_LAUNCH_IN, 519); // Launch in video mode
  
				if (g_passwordManager.IsMasterLockUnlocked(false) || g_settings.GetCurrentProfile().canWriteDatabases())
				{
					if (item->IsShortCut())
						buttons.Add(CONTEXT_BUTTON_RENAME, 16105); // Rename
					else
						buttons.Add(CONTEXT_BUTTON_RENAME, 520); // Edit xbe title
				}
  
				if (m_database.ItemHasTrainer(dwTitleId))
					buttons.Add(CONTEXT_BUTTON_TRAINER_OPTIONS, 12015); // Trainer options
			}
			buttons.Add(CONTEXT_BUTTON_SCAN_TRAINERS, 12012); // Scan trainers
			buttons.Add(CONTEXT_BUTTON_GOTO_ROOT, 20128); // Go to Root
#endif		
		}  
	}

	CGUIMediaWindow::GetContextButtons(itemNumber, buttons);

#if 0 // TODO: OG Xbox stuff to be modded to suit 360	
	if (item && !item->GetPropertyBOOL("pluginreplacecontextitems")) 
		buttons.Add(CONTEXT_BUTTON_SETTINGS, 5); // Settings 
#endif
}

bool CGUIWindowPrograms::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
	CFileItemPtr item = (itemNumber >= 0 && itemNumber < m_vecItems->Size()) ? m_vecItems->Get(itemNumber) : CFileItemPtr();

	if (item && m_vecItems->IsVirtualDirectoryRoot())
	{
		if (CGUIDialogContextMenu::OnContextButton("programs", item, button))
		{
			Update("");
			return true;
		}
	}
	
	switch (button)
	{
/*		case CONTEXT_BUTTON_RENAME: // TODO
		{
			CStdString strDescription;
			CShortcut cut;
			
			if (item->IsShortCut())
			{
				cut.Create(item->GetPath());
				strDescription = cut.m_strLabel;
			}
			else
				strDescription = item->GetLabel();

			if (CGUIDialogKeyboard::ShowAndGetInput(strDescription, g_localizeStrings.Get(16008), false))
			{
				if (item->IsShortCut())
				{
					cut.m_strLabel = strDescription;
					cut.Save(item->GetPath());
				}
				else
				{
					// SetXBEDescription will truncate to 40 characters.
					CUtil::SetXBEDescription(item->GetPath(),strDescription);
					m_database.SetDescription(item->GetPath(),strDescription);
				}
				Update(m_vecItems->GetPath());
			}
			return true;
		}
*/
/*		case CONTEXT_BUTTON_TRAINER_OPTIONS: // TODO
		{
			DWORD dwTitleId = CUtil::GetXbeID(item->GetPath());
			
			if (CGUIDialogTrainerSettings::ShowForTitle(dwTitleId,&m_database))
				Update(m_vecItems->GetPath());
			
			return true;
		}
*/
/*		case CONTEXT_BUTTON_SCAN_TRAINERS: // TODO
		{
			PopulateTrainersList();
			Update(m_vecItems->GetPath());
			return true;
		}
*/
		case CONTEXT_BUTTON_SETTINGS:
//		g_windowManager.ActivateWindow(WINDOW_SETTINGS_MYPROGRAMS); // TODO
		return true;

		case CONTEXT_BUTTON_GOTO_ROOT:
		Update("");
		return true;

		case CONTEXT_BUTTON_LAUNCH:
		OnClick(itemNumber);
		return true;

/*		case CONTEXT_BUTTON_GAMESAVES: // TODO
		{
			CStdString strTitleID;
			CStdString strGameSavepath;
			strTitleID.Format("%08X",CUtil::GetXbeID(item->GetPath()));
			URIUtils::AddFileToFolder("E:\\udata\\",strTitleID,strGameSavepath);
			g_windowManager.ActivateWindow(WINDOW_GAMESAVES,strGameSavepath);
			return true;
		}
*/
		case CONTEXT_BUTTON_LAUNCH_IN:
//		OnChooseVideoModeAndLaunch(itemNumber); // TODO
		return true;
		
		default:
			break;
	}
	return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

bool CGUIWindowPrograms::OnPlayMedia(int iItem) // TODO - Get more info from the XEX, etc
{
	if ( iItem < 0 || iItem >= (int)m_vecItems->Size() ) return false;
	CFileItemPtr pItem = m_vecItems->Get(iItem);

//	if (pItem->IsDVD())
//		return MEDIA_DETECT::CAutorun::PlayDisc(); // TODO

	if (pItem->m_bIsFolder) return false;

	// Launch xex...
	char szPath[1024];
//	char szParameters[1024];// TODO

	strcpy(szPath, pItem->GetPath().c_str());

	CXEXUtils::RunXex(szPath);

	return true;
}