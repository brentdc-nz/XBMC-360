#include "GUIWindowFullScreen.h"
#include "guilib\GUIInfoManager.h"
#include "utils\SingleLock.h"
#include "guilib\GraphicContext.h"
#include "Application.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\AudioContext.h"
#include "cores\VideoRenderers\RenderManager.h"
#include "utils\Util.h"

#define BLUE_BAR		100
#define LABEL_ROW1		10
#define LABEL_ROW2		11
#define LABEL_ROW3		12

CGUIWindowFullScreen::CGUIWindowFullScreen(void)
    : CGUIWindow(WINDOW_FULLSCREEN_VIDEO, "VideoFullScreen.xml")
{
		m_loadOnDemand = false;
}

CGUIWindowFullScreen::~CGUIWindowFullScreen(void)
{
}

void CGUIWindowFullScreen::PreloadDialog(unsigned int windowID)
{
	CGUIWindow *pWindow = g_windowManager.GetWindow(windowID);

	if (pWindow) 
	{
		pWindow->Initialize();
		pWindow->DynamicResourceAlloc(false);
		pWindow->AllocResources(false);
	}
}

void CGUIWindowFullScreen::UnloadDialog(unsigned int windowID)
{
	CGUIWindow *pWindow = g_windowManager.GetWindow(windowID);
	if (pWindow) 
	{
		pWindow->FreeResources(pWindow->GetLoadOnDemand());
	}
}

void CGUIWindowFullScreen::AllocResources(bool forceLoad)
{
	CGUIWindow::AllocResources();

	PreloadDialog(WINDOW_DIALOG_SEEK_BAR);
}

void CGUIWindowFullScreen::FreeResources(bool forceUnload)
{
	DynamicResourceAlloc(true);

	UnloadDialog(WINDOW_DIALOG_SEEK_BAR);

	CGUIWindow::FreeResources();
}


bool CGUIWindowFullScreen::OnAction(const CAction &action)
{
	if (g_application.m_pPlayer != NULL && g_application.m_pPlayer->OnAction(action))
		return true;

	switch (action.GetID())
	{
		case ACTION_SHOW_GUI:
		{
			// Switch back to the menu
			OutputDebugString("Switching to GUI\n");
			g_windowManager.PreviousWindow();
			OutputDebugString("Now in GUI\n");
			return true;
		}
		break;
		
		case ACTION_STEP_BACK:
		{	
			Seek(false, false);
			return true;
		}
		break;

		case ACTION_STEP_FORWARD:
		{
			Seek(true, false);
			return true;
		}
		break;

		case ACTION_BIG_STEP_BACK:
		{
			Seek(false, true);
			return true;
		}
		break;

		case ACTION_BIG_STEP_FORWARD:
		{
			Seek(true, true);
			return true;
		}
		break;
	}

	return CGUIWindow::OnAction(action);
}

bool CGUIWindowFullScreen::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
			// check whether we've come back here from a window during which time we've actually
			// stopped playing videos
			if (message.GetParam1() == WINDOW_INVALID && !g_application.IsPlayingVideo())
			{ 
				// why are we here if nothing is playing???
				g_windowManager.PreviousWindow();
				return true;
			}

			g_infoManager.SetShowCodec(false);

			CGUIWindow::OnMessage(message);

			g_graphicsContext.TLock();
			g_graphicsContext.Get3DDevice()->Clear( 0L, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0L );
			g_graphicsContext.TUnlock();

			// switch resolution
			CSingleLock lock (g_graphicsContext);
			g_graphicsContext.SetFullScreenVideo(true);
//			RESOLUTION res = g_renderManager.GetResolution();
//			g_graphicsContext.SetVideoResolution(res, false, false);
			lock.Leave();

			return true;
		}
		case GUI_MSG_WINDOW_DEINIT:
		{
			CGUIWindow::OnMessage(message);

			FreeResources(true);

			CSingleLock lock (g_graphicsContext);

			g_graphicsContext.SetFullScreenVideo(false);
			lock.Leave();

			// make sure renderer is uptospeed
//			g_renderManager.Update(false);

			Sleep(10);

			return true;
		}
		case GUI_MSG_SETFOCUS:
		case GUI_MSG_LOSTFOCUS:
			if (message.GetSenderId() != WINDOW_FULLSCREEN_VIDEO) return true;
		break;
	}

	return CGUIWindow::OnMessage(message);
}

// Dummy override of Render() - RenderFullScreen() is where the action takes place
// this is called via DVDPlayer when the video window is flipped (indicating a frame
// change) so that we get smooth video playback
void CGUIWindowFullScreen::Render()
{
	return;
}

bool CGUIWindowFullScreen::NeedRenderFullScreen()
{
	CSingleLock lock (g_graphicsContext);

	if (g_application.m_pPlayer)
	{
		if (g_application.m_pPlayer->IsPaused() ) return true;
//		if (g_application.m_pPlayer->IsCaching() ) return true; //TODO
		if (!g_application.m_pPlayer->IsPlaying() ) return true;
	}

	if (g_infoManager.GetBool(PLAYER_SHOWCODEC)) return true;

	return false;
}

void CGUIWindowFullScreen::RenderFullScreen()
{
	if(!g_application.m_pPlayer) return;

	g_infoManager.UpdateFPS();

	if (g_infoManager.GetBool(PLAYER_SHOWCODEC))
	{
		// Show audio codec info
		CStdString strAudio, strVideo, strGeneral;
		
		g_application.m_pPlayer->GetAudioInfo(strAudio);
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW1);
			msg.SetLabel(strAudio);
			OnMessage(msg);
		}

		// Show video codec info
		g_application.m_pPlayer->GetVideoInfo(strVideo);
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW2);
			msg.SetLabel(strVideo);
			OnMessage(msg);
		}

		// Show general info
		g_application.m_pPlayer->GetGeneralInfo(strGeneral);
		{
			CStdString strGeneralFPS;
			float fCpuUsage = CUtil::CurrentCpuUsage();

			strGeneralFPS.Format("fps:%02.2f cpu:%02.2f %s", g_infoManager.GetFPS(), fCpuUsage, strGeneral.c_str());
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW3);
			msg.SetLabel(strGeneralFPS);
			OnMessage(msg);
		}
	}

	int iSpeed = g_application.GetPlaySpeed();

	if (g_application.m_pPlayer->IsPaused() || iSpeed != 1)
	{
		SET_CONTROL_HIDDEN(LABEL_ROW1);
		SET_CONTROL_HIDDEN(LABEL_ROW2);
		SET_CONTROL_HIDDEN(LABEL_ROW3);
		SET_CONTROL_HIDDEN(BLUE_BAR);
	}
	else if (g_infoManager.GetBool(PLAYER_SHOWCODEC))
	{
		SET_CONTROL_VISIBLE(LABEL_ROW1);
		SET_CONTROL_VISIBLE(LABEL_ROW2);
		SET_CONTROL_VISIBLE(LABEL_ROW3);
		SET_CONTROL_VISIBLE(BLUE_BAR);
	}
	else
	{
		SET_CONTROL_HIDDEN(LABEL_ROW1);
		SET_CONTROL_HIDDEN(LABEL_ROW2);
		SET_CONTROL_HIDDEN(LABEL_ROW3);
		SET_CONTROL_HIDDEN(BLUE_BAR);
	}

	CGUIWindow::Render();

	g_windowManager.RenderDialogs();
}

void CGUIWindowFullScreen::Seek(bool bPlus, bool bLargeStep)
{
	CGUIDialogSeekBar* pDialogSeekBar = (CGUIDialogSeekBar*)g_windowManager.GetWindow(WINDOW_DIALOG_SEEK_BAR);

	pDialogSeekBar->Show();
	pDialogSeekBar->ResetTimer();

	// Temporary solution

	g_application.m_pPlayer->Seek(bPlus, bLargeStep);

	// Make sure gui items are visible
//	g_infoManager.SetDisplayAfterSeek(); //TODO
}