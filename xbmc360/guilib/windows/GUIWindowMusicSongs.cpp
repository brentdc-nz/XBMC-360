#include "GUIWindowMusicSongs.h"
#include "utils\Util.h"
#include "utils\URIUtils.h"
#include "guilib\GUIInfoManager.h"
#include "guilib\dialogs\GUIDialogProgress.h"
#include "GUISettings.h"
#include "Application.h"
#include "FileSystem\MultiPathDirectory.h"
#include "guilib\GUIWindowManager.h"
#include "FileSystem\File.h"
#include "utils\Log.h"
#include "URL.h"

#include "viewstate.h"

using namespace std;

CGUIWindowMusicSongs::CGUIWindowMusicSongs()
	: CGUIMediaWindow(WINDOW_MUSIC_FILES, "MyMusic.xml")
{
}

CGUIWindowMusicSongs::~CGUIWindowMusicSongs()
{
}


bool CGUIWindowMusicSongs::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_INIT:
		{
			m_dlgProgress = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);

			// Is this the first time accessing this window?
			if (m_vecItems->GetPath() == "?" && message.GetStringParam().IsEmpty())
				message.SetStringParam(/*g_settings.m_defaultMusicSource*/""); // TODO

			return CGUIMediaWindow::OnMessage(message);
		}
		break;

		case GUI_MSG_CLICKED:
		{
			int iControl = message.GetSenderId();

			// TODO
		}
	}
	return CGUIMediaWindow::OnMessage(message);
}

void CGUIWindowMusicSongs::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
	CFileItemPtr item;

	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);

	if (item)
	{
		if (m_vecItems->IsVirtualDirectoryRoot())
		{
			// Get the usual music shares, and anything for all media windows
			CGUIDialogContextMenu::GetContextButtons("music", item, buttons);

#if 0 // TODO: OG Xbox stuff to be modded to suit 360
			// Enable Rip CD an audio disc
			if (CDetectDVDMedia::IsDiscInDrive() && item->IsCDDA())
			{
				// Those cds can also include Audio Tracks: CDExtra and MixedMode!
				CCdInfo *pCdInfo = CDetectDVDMedia::GetCdInfo();

				if (pCdInfo->IsAudio(1) || pCdInfo->IsCDExtra(1) || pCdInfo->IsMixedMode(1))
					buttons.Add(CONTEXT_BUTTON_RIP_CD, 600);
			}
#endif
			CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
		}
		else
		{
			// To be addded
		}

#if 0 // TODO: OG Xbox stuff to be modded to suit 360
		if (!m_vecItems->IsVirtualDirectoryRoot())
			buttons.Add(CONTEXT_BUTTON_SWITCH_MEDIA, 523);
#endif
	}

	CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
}

bool CGUIWindowMusicSongs::OnContextButton(int itemNumber, CONTEXT_BUTTON button)	
{
	CFileItemPtr item;
	
	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);
	
	if ( m_vecItems->IsVirtualDirectoryRoot() && item)
	{
		if (CGUIDialogContextMenu::OnContextButton("music", item, button))
		{
#if 0 // TODO: OG Xbox stuff to be modded to suit 360
			if (button == CONTEXT_BUTTON_REMOVE_SOURCE)
				OnRemoveSource(itemNumber);
#endif
			Update("");
			return true;
		}
	}

#if 0 // TODO: OG Xbox stuff to be modded to suit 360
	switch (button)
	{
		case CONTEXT_BUTTON_SCAN:
			OnScan(itemNumber);
			return true;

		case CONTEXT_BUTTON_RIP_TRACK:
			OnRipTrack(itemNumber);
			return true;

		case CONTEXT_BUTTON_RIP_CD:
			OnRipCD();
			return true;

		case CONTEXT_BUTTON_CDDB:
			if (m_musicdatabase.LookupCDDBInfo(true))
			Update(m_vecItems->GetPath());
			return true;

		case CONTEXT_BUTTON_DELETE:
			OnDeleteItem(itemNumber);
			return true;

		case CONTEXT_BUTTON_RENAME:
			OnRenameItem(itemNumber);
			return true;

		case CONTEXT_BUTTON_SWITCH_MEDIA:
			CGUIDialogContextMenu::SwitchMedia("music", m_vecItems->GetPath());
			return true;
		default:
			break;
	}
#endif

	return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

bool CGUIWindowMusicSongs::Update(const CStdString &strDirectory)
{
	if (m_thumbLoader.IsLoading())
		m_thumbLoader.StopThread();

	if (!CGUIMediaWindow::Update(strDirectory))
		return false;

	m_thumbLoader.Load(*m_vecItems);

	return true;
}

void CGUIWindowMusicSongs::OnPrepareFileItems(CFileItemList &items)
{
	RetrieveMusicInfo();

	items.SetCachedMusicThumbs();
}

void CGUIWindowMusicSongs::RetrieveMusicInfo()
{
	DWORD dwStartTick = GetTickCount();

	OnRetrieveMusicInfo(*m_vecItems);

	CLog::Log(LOGDEBUG, "RetrieveMusicInfo() took %u msec",
		GetTickCount() - dwStartTick);
}

void CGUIWindowMusicSongs::OnRetrieveMusicInfo(CFileItemList& items)
{
	if (items.GetFolderCount() == items.Size() ||
		!g_guiSettings.GetBool("musicfiles.usetags"))
	{
		return;
	}

	// Start the music info loader thread
	m_musicInfoLoader.SetProgressCallback(m_dlgProgress);
	m_musicInfoLoader.Load(items);

	bool bShowProgress = !g_windowManager.HasModalDialog();
	bool bProgressVisible = false;

	DWORD dwTick = GetTickCount();

	while (m_musicInfoLoader.IsLoading())
	{
		if (bShowProgress)
		{ // Do we have to init a progress dialog?
			DWORD dwElapsed = GetTickCount() - dwTick;

			if (!bProgressVisible && dwElapsed > 1500 && m_dlgProgress)
			{ // tag loading takes more then 1.5 secs, show a progress dialog
				CURL url(items.GetPath());
				CStdString strStrippedPath = url.GetWithoutUserDetails();
				m_dlgProgress->SetHeading(189);
				m_dlgProgress->SetLine(0, 505);
				m_dlgProgress->SetLine(1, "");
				m_dlgProgress->SetLine(2, strStrippedPath);
				m_dlgProgress->StartModal();
				m_dlgProgress->ShowProgressBar(true);
				bProgressVisible = true;
			}

			if (bProgressVisible && m_dlgProgress && !m_dlgProgress->IsCanceled())
			{ // keep GUI alive
				m_dlgProgress->Progress();
			}
		} // if (bShowProgress)
		Sleep(1);
	} // while (m_musicInfoLoader.IsLoading())

	if (bProgressVisible && m_dlgProgress)
		m_dlgProgress->Close();
}
