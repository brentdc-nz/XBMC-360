#include "GUIWindowFullScreen.h"
#include "..\GUIInfoManager.h"
#include "..\..\utils\SingleLock.h"
#include "..\GraphicContext.h"
#include "..\..\Application.h"
#include "..\GUIWindowManager.h"
#include "..\..\cores\VideoRenderers\RenderManager.h"

#define BLUE_BAR                          0
#define LABEL_ROW1                       10
#define LABEL_ROW2                       11
#define LABEL_ROW3                       12

CGUIWindowFullScreen::CGUIWindowFullScreen(void)
    : CGUIWindow(WINDOW_FULLSCREEN_VIDEO, "VideoFullScreen.xml")
{
		m_loadOnDemand = false;
}

CGUIWindowFullScreen::~CGUIWindowFullScreen(void)
{
}

bool CGUIWindowFullScreen::OnAction(const CAction &action)
{
	if (g_application.m_pPlayer != NULL && g_application.m_pPlayer->OnAction(action))
		return true;

	switch (action.GetID())
	{
		case ACTION_SHOW_GUI:
		{
			// switch back to the menu
			OutputDebugString("Switching to GUI\n");
			g_windowManager.PreviousWindow();
			OutputDebugString("Now in GUI\n");
			return true;
		}
		break;
		
		case ACTION_STEP_BACK:
		{
			g_application.m_pPlayer->SeekTime(5000); //TODO
			return true;
		}
		break;

		case ACTION_STEP_FORWARD:
		{
			g_application.m_pPlayer->SeekTime(1000); //TODO
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

			g_graphicsContext.Lock();
			g_graphicsContext.Get3DDevice()->Clear( 0L, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0L );
			g_graphicsContext.Unlock();

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
	if (!g_application.m_pPlayer) return;

	g_infoManager.UpdateFPS();

	if (g_infoManager.GetBool(PLAYER_SHOWCODEC))
	{
		// show audio codec info
		CStdString strAudio, strVideo, strGeneral;
		
		g_application.m_pPlayer->GetAudioInfo(strAudio);
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW1);
			msg.SetLabel(strAudio);
			OnMessage(msg);
		}

		// show video codec info
		g_application.m_pPlayer->GetVideoInfo(strVideo);
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW2);
			msg.SetLabel(strVideo);
			OnMessage(msg);
		}

		// show general info
		g_application.m_pPlayer->GetGeneralInfo(strGeneral);
		{
			CStdString strGeneralFPS;
			float fCpuUsage = 0.0f;//CUtil::CurrentCpuUsage(); //TODO - How to do on PPC?

			strGeneralFPS.Format("fps:%02.2f cpu:%02.2f %s", g_infoManager.GetFPS(), fCpuUsage, strGeneral.c_str() );
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW3);
			msg.SetLabel(strGeneralFPS);
			OnMessage(msg);
		}
	}

	CGUIWindow::Render();
}