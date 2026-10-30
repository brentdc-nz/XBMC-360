#include "utils\Log.h"
#include "guilib\windows\GUIWindowOSD.h"
#include "Application.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\GUIUserMessages.h"

CGUIWindowOSD::CGUIWindowOSD(void)
	: CGUIDialog(WINDOW_OSD, "VideoOSD.xml")
{
}

CGUIWindowOSD::~CGUIWindowOSD(void)
{
}

void CGUIWindowOSD::OnWindowLoaded()
{
	CGUIDialog::OnWindowLoaded();
	m_bRelativeCoords = true;
}

void CGUIWindowOSD::FrameMove()
{
	if (m_autoClosing)
	{
		// Check for a submenu open
		if (g_windowManager.IsWindowActive(WINDOW_DIALOG_AUDIO_OSD_SETTINGS)
							|| g_windowManager.IsWindowActive(WINDOW_DIALOG_VIDEO_OSD_SETTINGS)
							|| g_windowManager.IsWindowActive(WINDOW_DIALOG_VIDEO_BOOKMARKS))
							
			SetAutoClose(100); // Enough for 10fps
	}
	CGUIDialog::FrameMove();
}

bool CGUIWindowOSD::OnAction(const CAction &action)
{
	if (action.GetID() == ACTION_NEXT_ITEM || action.GetID() == ACTION_PREV_ITEM)
	{
		// These could indicate next chapter if video supports it
		if (g_application.m_pPlayer != NULL && g_application.m_pPlayer->OnAction(action))
			return true;
	}

	return CGUIDialog::OnAction(action);
}


bool CGUIWindowOSD::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
	case GUI_MSG_VIDEO_MENU_STARTED:
		{
			// We have gone to the DVD menu, so close the OSD.
			Close();
		}
	case GUI_MSG_WINDOW_DEINIT:  // fired when OSD is hidden
		{
			// Remove our subdialogs if visible
			CGUIDialog *pDialog = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_DIALOG_VIDEO_OSD_SETTINGS);
		
			if (pDialog && pDialog->IsDialogRunning())
				pDialog->Close(true);
			
			pDialog = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_DIALOG_AUDIO_OSD_SETTINGS);
			if (pDialog && pDialog->IsDialogRunning()) pDialog->Close(true);
			pDialog = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_DIALOG_VIDEO_BOOKMARKS);
			if (pDialog && pDialog->IsDialogRunning()) pDialog->Close(true);
		}
		break;
	}
	return CGUIDialog::OnMessage(message);
}
