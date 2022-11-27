#include "GUIWindowMusicFiles.h"
#include "utils\Util.h"
#include "utils\URIUtils.h"
#include "guilib\GUIInfoManager.h"
#include "Application.h"
#include "FileSystem\MultiPathDirectory.h"
#include "guilib\GUIWindowManager.h"
#include "FileSystem\File.h"
#include "utils\Log.h"

#include "viewstate.h"

using namespace std;

CGUIWindowMusicFiles::CGUIWindowMusicFiles()
	: CGUIMediaWindow(WINDOW_MUSIC, "MyMusic.xml")
{
}

CGUIWindowMusicFiles::~CGUIWindowMusicFiles()
{
}


bool CGUIWindowMusicFiles::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_INIT:
		{
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

bool CGUIWindowMusicFiles::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
	CFileItemPtr item;

	if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
		item = m_vecItems->Get(itemNumber);

	if ( m_vecItems->IsVirtualDirectoryRoot() && item)
	{
		if (CGUIDialogContextMenu::OnContextButton("music", item, button))
		{
			// TODO: should we search DB for entries from plugins?
			if (button == CONTEXT_BUTTON_REMOVE_SOURCE/* && !item->IsPlugin()
				&& !item->IsLiveTV() &&!item->IsRSS()*/)
			{
				int iTest = 0;
//				OnUnAssignContent(itemNumber,20375,20340,20341); // TODO
			}
			Update("");
			return true;
		}
	}

	return CGUIMediaWindow::OnContextButton(itemNumber, button);
}