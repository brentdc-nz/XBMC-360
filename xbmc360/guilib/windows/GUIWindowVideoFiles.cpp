#include "GUIWindowVideoFiles.h"
#include "utils\Util.h"
#include "utils/URIUtils.h"
//#include "pictures/Picture.h"
//#include "utils/IMDB.h"
#include "guilib\GUIInfoManager.h"
//#include "playlists/PlayListFactory.h"
#include "Application.h"
//#include "NfoFile.h"
//#include "PlayListPlayer.h"
//#include "GUIPassword.h"
//#include "dialogs/GUIDialogMediaSource.h"
//#include "settings/GUIDialogContentSettings.h"
//#include "video/dialogs/GUIDialogVideoScan.h"
#include "FileSystem/MultiPathDirectory.h"
//#include "utils/RegExp.h"
#include "guilib\GUIWindowManager.h"
//#include "dialogs/GUIDialogOK.h"
//#include "dialogs/GUIDialogYesNo.h"
#include "FileSystem/File.h"
//#include "playlists/PlayList.h"
#include "utils/log.h"

#include "viewstate.h"

using namespace std;

CGUIWindowVideoFiles::CGUIWindowVideoFiles()
	: CGUIWindowVideoBase(WINDOW_VIDEO_FILES, "MyVideos.xml")
{
}

CGUIWindowVideoFiles::~CGUIWindowVideoFiles()
{
}


bool CGUIWindowVideoFiles::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_INIT:
		{
			// Is this the first time accessing this window?
			if (m_vecItems->GetPath() == "?" && message.GetStringParam().IsEmpty())
				message.SetStringParam(/*g_settings.m_defaultVideoSource*/""); // TODO

			return CGUIWindowVideoBase::OnMessage(message);
		}
		break;

		case GUI_MSG_CLICKED:
		{
			int iControl = message.GetSenderId();

			// TODO
		}
	}
	return CGUIWindowVideoBase::OnMessage(message);
}

void CGUIWindowVideoFiles::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
	CFileItemPtr item;
	
	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);

	if (item)
	{
		// Are we in the playlists location?
		if (m_vecItems->IsVirtualDirectoryRoot())
		{
			// Get the usual shares, and anything for all media windows
			CGUIDialogContextMenu::GetContextButtons("video", item, buttons);
			CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
		}
	}
}

bool CGUIWindowVideoFiles::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
	CFileItemPtr item;
	
	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);

	if ( m_vecItems->IsVirtualDirectoryRoot() && item)
	{
		if (CGUIDialogContextMenu::OnContextButton("video", item, button))
		{
			// TODO: should we search DB for entries from plugins?
/*			if (button == CONTEXT_BUTTON_REMOVE_SOURCE && !item->IsPlugin()
				&& !item->IsLiveTV() &&!item->IsRSS())
			{
				OnUnAssignContent(itemNumber,20375,20340,20341);
			}
*/			Update("");
			return true;
		}
	}
	
	// TODO

	return CGUIWindowVideoBase::OnContextButton(itemNumber, button);
}