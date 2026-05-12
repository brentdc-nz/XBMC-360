#include "GUIWindowVideoBase.h"
#include "video\VideoDatabase.h"
#include "guilib\dialogs\GUIDialogContextMenu.h"
#include "guilib\LocalizeStrings.h"
#include "guilib\GUIWindowManager.h"
#include "GUISettings.h"
//#include "GUIPassword.h" // TODO: GUIPassword not ported
#include "PlayListPlayer.h"
#include "playlists\PlayListFactory.h"
#include "utils\URIUtils.h"
#include "utils\StringUtils.h"
#include "utils\log.h"
#include "FileItem.h"
#include "video\VideoInfoTag.h"

using namespace std;
using namespace PLAYLIST;

CGUIWindowVideoBase::CGUIWindowVideoBase(int id, const CStdString &xmlFile)
	: CGUIMediaWindow(id, xmlFile)
{
}

CGUIWindowVideoBase::~CGUIWindowVideoBase()
{
}

void CGUIWindowVideoBase::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
	CFileItemPtr item;
	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);

	// Contextual buttons
	if (item && !item->GetPropertyBOOL("pluginreplacecontextitems"))
	{
		if (!item->IsParentFolder())
		{
			CStdString path(item->GetPath());
			if (item->IsVideoDb() && item->HasVideoInfoTag())
				path = item->GetVideoInfoTag()->m_strFileNameAndPath;

			if (URIUtils::IsStack(path))
			{
				vector<int> times;
				if (m_database.GetStackTimes(path, times))
					buttons.Add(CONTEXT_BUTTON_PLAY_PART, 20324);
			}

			if (GetID() != WINDOW_VIDEO_NAV || (!m_vecItems->GetPath().IsEmpty() &&
				!item->GetPath().Left(19).Equals("newsmartplaylist://")))
			{
				buttons.Add(CONTEXT_BUTTON_QUEUE_ITEM, 13347); // Add to Playlist
			}

			// allow a folder to be ad-hoc queued and played by the default player
			if (item->m_bIsFolder || (item->IsPlayList() /*&&
				!g_advancedSettings.m_playlistAsFolders*/)) // TODO
			{
				buttons.Add(CONTEXT_BUTTON_PLAY_ITEM, 208);
			}
/*			else
			{ // get players - TODO: GetPlayers/SelectPlayerDialog not ported
				VECPLAYERCORES vecCores;
				if (item->IsVideoDb())
				{
					CFileItem item2;
					item2.SetPath(item->GetVideoInfoTag()->m_strFileNameAndPath);
					CPlayerCoreFactory::GetPlayers(item2, vecCores);
				}
				else
					CPlayerCoreFactory::GetPlayers(*item, vecCores);
				if (vecCores.size() > 1)
					buttons.Add(CONTEXT_BUTTON_PLAY_WITH, 15213);
			}
*/
//			if (item->IsSmartPlayList()) // TODO - SmartPlayList not ported
//			{
//				buttons.Add(CONTEXT_BUTTON_PLAY_PARTYMODE, 15216); // Play in Partymode
//			}

			// if autoresume is enabled then add restart video button
			// check to see if the Resume Video button is applicable
			if (GetResumeItemOffset(item.get()) > 0)
			{
				if (g_guiSettings.GetInt("videoplayer.resumeautomatically") == RESUME_YES)
					buttons.Add(CONTEXT_BUTTON_RESTART_ITEM, 20132);    // Restart Video
				if (g_guiSettings.GetInt("videoplayer.resumeautomatically") == RESUME_NO)
					buttons.Add(CONTEXT_BUTTON_RESUME_ITEM, 13381);     // Resume Video
			}

//			if (item->IsSmartPlayList() || m_vecItems->IsSmartPlayList()) // TODO - SmartPlayList not ported
//				buttons.Add(CONTEXT_BUTTON_EDIT_SMART_PLAYLIST, 586);
		}
	}

	CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
}

bool CGUIWindowVideoBase::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
	CFileItemPtr item;
	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);

	switch (button)
	{
	case CONTEXT_BUTTON_PLAY_PART:
		{
			CFileItemList items;
			CStdString path(item->GetPath());
			if (item->IsVideoDb())
				path = item->GetVideoInfoTag()->m_strFileNameAndPath;

			// TODO: CGUIDialogFileStacking not ported
			// For now, just play from the beginning
			item->m_lStartOffset = 0;
			CGUIMediaWindow::OnClick(itemNumber);
			return true;
		}
	case CONTEXT_BUTTON_QUEUE_ITEM:
		OnQueueItem(itemNumber);
		return true;

	case CONTEXT_BUTTON_PLAY_ITEM:
		PlayItem(itemNumber);
		return true;

/*	case CONTEXT_BUTTON_PLAY_WITH: // TODO: GetPlayers/SelectPlayerDialog not ported
		{
			VECPLAYERCORES vecCores;
			if (item->IsVideoDb())
			{
				CFileItem item2(*item->GetVideoInfoTag());
				CPlayerCoreFactory::GetPlayers(item2, vecCores);
			}
			else
				CPlayerCoreFactory::GetPlayers(*item, vecCores);
			g_application.m_eForcedNextPlayer = CPlayerCoreFactory::SelectPlayerDialog(vecCores);
			if (g_application.m_eForcedNextPlayer != EPC_NONE)
				OnClick(itemNumber);
			return true;
		}

	case CONTEXT_BUTTON_PLAY_PARTYMODE: // TODO: PartyModeManager not ported
		g_partyModeManager.Enable(PARTYMODECONTEXT_VIDEO, m_vecItems->Get(itemNumber)->GetPath());
		return true;
*/
	case CONTEXT_BUTTON_RESTART_ITEM:
		OnRestartItem(itemNumber);
		return true;

	case CONTEXT_BUTTON_RESUME_ITEM:
		OnResumeItem(itemNumber);
		return true;

	case CONTEXT_BUTTON_GOTO_ROOT:
		Update("");
		return true;

	case CONTEXT_BUTTON_NOW_PLAYING:
		g_windowManager.ActivateWindow(WINDOW_VIDEO_PLAYLIST);
		return true;

	case CONTEXT_BUTTON_SETTINGS:
		g_windowManager.ActivateWindow(WINDOW_SETTINGS_MYVIDEOS);
		return true;

	default:
		break;
	}

	return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

int CGUIWindowVideoBase::GetResumeItemOffset(const CFileItem *item)
{
	// Do not resume livetv
	if (item->IsLiveTV())
		return 0;

	m_database.Open();
	long startoffset = 0;

	if (item->IsStack()/* && (!g_guiSettings.GetBool("myvideos.treatstackasfile") ||
		CFileItem(CStackDirectory::GetFirstStackedFile(item->GetPath()),false).IsDVDImage())*/) // TODO
	{
		// TODO: Stack resume not fully ported
	}
	else if (/*!item->IsNFO() &&*/ !item->IsPlayList()) // TODO: IsNFO not ported
	{
//		if (item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_resumePoint.timeInSeconds > 0.0) // TODO: m_resumePoint not ported
//			startoffset = (long)(item->GetVideoInfoTag()->m_resumePoint.timeInSeconds*75);
//		else
		{
			CBookmark bookmark;
			CStdString strPath = item->GetPath();
			if ((item->IsVideoDb() /*|| item->IsDVD()*/) && item->HasVideoInfoTag()) // TODO: IsDVD not ported
				strPath = item->GetVideoInfoTag()->m_strFileNameAndPath;

			if (m_database.GetResumeBookMark(strPath, bookmark))
				startoffset = (long)(bookmark.timeInSeconds*75);
		}
	}
	m_database.Close();

	return startoffset;
}

void CGUIWindowVideoBase::OnQueueItem(int iItem)
{
	if (iItem < 0 || iItem >= m_vecItems->Size()) return;

	// We take a copy so that we can alter the queue state
	CFileItemPtr item(new CFileItem(*m_vecItems->Get(iItem)));

	//  Allow queuing of unqueueable items
	//  when we try to queue them directly
//	if (!item->CanQueue()) // TODO: CanQueue not ported
//		item->SetCanQueue(true);

	CFileItemList queuedItems;
	AddItemToPlayList(item, queuedItems);

//	if (g_partyModeManager.IsEnabled(PARTYMODECONTEXT_VIDEO)) // TODO: PartyModeManager not ported
//	{
//		g_partyModeManager.AddUserSongs(queuedItems, false);
//		return;
//	}

	g_playlistPlayer.Add(PLAYLIST_VIDEO, queuedItems);
	// Video does not auto play on queue like music
	m_viewControl.SetSelectedItem(iItem + 1);
}

void CGUIWindowVideoBase::AddItemToPlayList(const CFileItemPtr &pItem, CFileItemList &queuedItems)
{
//	if (!pItem->CanQueue() || pItem->IsRAR() || pItem->IsZIP() || pItem->IsParentFolder()) // TODO: CanQueue/IsRAR/IsZIP not ported
	if (pItem->IsParentFolder())
		return;

	if (pItem->m_bIsFolder)
	{
		if (pItem->IsParentFolder())
			return;

		// Check if we add a locked share
		if (pItem->m_bIsShareOrDrive)
		{
//			CFileItem item = *pItem; // TODO: GUIPassword not ported
//			if (!g_passwordManager.IsItemUnlocked(&item, "video"))
//				return;
		}

		// recursive
		CFileItemList items;
		GetDirectory(pItem->GetPath(), items);
		FormatAndSort(items);

		for (int i = 0; i < items.Size(); ++i)
		{
			if (items[i]->m_bIsFolder)
			{
				CStdString strPath = items[i]->GetPath();
				URIUtils::RemoveSlashAtEnd(strPath);
//				strPath.ToLower();
//				if (strPath.size() == 3 && strPath[1] == ':') // TODO: video path check
//				{
//					items[i]->SetPath(strPath);
//					items[i]->m_bIsFolder = false;
//				}
			}
			AddItemToPlayList(items[i], queuedItems);
		}
	}
	else
	{
		// Just an item
		if (pItem->IsPlayList())
		{
			auto_ptr<CPlayList> pPlayList(CPlayListFactory::Create(*pItem));
			if (pPlayList.get())
			{
				// Load it
				if (!pPlayList->Load(pItem->GetPath()))
				{
					// Productional log
					return; // Can't load, abort
				}

				CPlayList playlist = *pPlayList;
				for (int i = 0; i < (int)playlist.size(); ++i)
				{
					AddItemToPlayList(playlist[i], queuedItems);
				}
			}
		}
		else
		{
			CFileItemPtr item(new CFileItem(*pItem));
			queuedItems.Add(item);
		}
	}
}

void CGUIWindowVideoBase::PlayItem(int iItem)
{
	// Restrictions should be placed in the appropriate window code
	// only call the base code if the item passes since this clears
	// the currently playing temp playlist

	const CFileItemPtr pItem = m_vecItems->Get(iItem);
	// If its a folder, build a temp playlist
	if (pItem->m_bIsFolder && !pItem->IsPlugin())
	{
		// take a copy so we can alter the queue state
		CFileItemPtr item(new CFileItem(*m_vecItems->Get(iItem)));

		//  Allow queuing of unqueueable items
		//  when we try to queue them directly
//		if (!item->CanQueue()) // TODO: CanQueue not ported
//			item->SetCanQueue(true);

		// skip ".."
		if (item->IsParentFolder())
			return;

		// Recursively add items to list
		CFileItemList queuedItems;
		AddItemToPlayList(item, queuedItems);

		g_playlistPlayer.ClearPlaylist(PLAYLIST_VIDEO);
		g_playlistPlayer.Reset();
		g_playlistPlayer.Add(PLAYLIST_VIDEO, queuedItems);
		g_playlistPlayer.SetCurrentPlaylist(PLAYLIST_VIDEO);
		g_playlistPlayer.Play();
	}
	else if (pItem->IsPlayList())
	{
		// Load the playlist the old way
		LoadPlayList(pItem->GetPath());
	}
	else
	{
		// Single item, play it
		OnClick(iItem);
	}
}

bool CGUIWindowVideoBase::Update(const CStdString &strDirectory)
{
	if (m_thumbLoader.IsLoading())
		m_thumbLoader.StopThread();

	if (!CGUIMediaWindow::Update(strDirectory))
		return false;

	m_thumbLoader.Load(*m_vecItems);

	return true;
}

bool CGUIWindowVideoBase::OnClick(int iItem)
{
	if (g_guiSettings.GetInt("videoplayer.resumeautomatically") != RESUME_NO)
		OnResumeItem(iItem);
	else
		return CGUIMediaWindow::OnClick(iItem);

	return true;
}

void CGUIWindowVideoBase::OnRestartItem(int iItem)
{
	CGUIMediaWindow::OnClick(iItem);
}

bool CGUIWindowVideoBase::OnResumeShowMenu(CFileItem &item)
{
	// We always resume the movie if the user doesn't want us to ask
	bool resumeItem = g_guiSettings.GetInt("videoplayer.resumeautomatically") != RESUME_ASK;

	if (!item.m_bIsFolder && !item.IsLiveTV() && !resumeItem)
	{
		// Check to see whether we have a resume offset available
		CVideoDatabase db;
		if (db.Open())
		{
			CBookmark bookmark;
			CStdString itemPath(item.GetPath());
			if (db.GetResumeBookMark(itemPath, bookmark))
			{
				// Prompt user whether they wish to resume
				CContextButtons choices;
				CStdString resumeString;
				resumeString.Format(g_localizeStrings.Get(12022).c_str(), CStringUtils::SecondsToTimeString((long)bookmark.timeInSeconds).c_str());
				choices.Add(1, resumeString);
				choices.Add(2, 12021); // Start from the beginning
				int retVal = CGUIDialogContextMenu::ShowAndGetChoice(choices);
				if (retVal < 0)
					return false; // Don't do anything
				resumeItem = (retVal == 1);
			}
			db.Close();
		}
	}
	if (resumeItem)
		item.m_lStartOffset = STARTOFFSET_RESUME;

	return true;
}

void CGUIWindowVideoBase::OnResumeItem(int iItem)
{
	if (iItem < 0 || iItem >= m_vecItems->Size()) return;
	CFileItemPtr item = m_vecItems->Get(iItem);

	// Show menu asking the user
	if (OnResumeShowMenu(*item))
		CGUIMediaWindow::OnClick(iItem);
}