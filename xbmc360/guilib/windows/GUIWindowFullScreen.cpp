#include "GUIWindowFullScreen.h"
#include "guilib\GUIInfoManager.h"
#include "utils\SingleLock.h"
#include "guilib\GraphicContext.h"
#include "Application.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\AudioContext.h"
#include "cores\VideoRenderers\RenderManager.h"
#include "utils\Util.h"
#include "guilib\GUILabelControl.h"
#include "Settings.h"
#include "AdvancedSettings.h"
#include "guilib\LocalizeStrings.h"

#define BLUE_BAR		100
#define LABEL_ROW1		10
#define LABEL_ROW2		11
#define LABEL_ROW3		12

// Displays current position, visible after seek or when forced
// Alt, use conditional visibility Player.DisplayAfterSeek
#define LABEL_CURRENT_TIME               22

CGUIWindowFullScreen::CGUIWindowFullScreen(void)
    : CGUIWindow(WINDOW_FULLSCREEN_VIDEO, "VideoFullScreen.xml")
{
		m_loadOnDemand = false;
		m_bShowCurrentTime = false;
		m_bShowViewModeInfo = false;
		m_dwShowViewModeTimeout = 0;
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
	CGUIWindow::AllocResources(forceLoad);
	DynamicResourceAlloc(false);
}

void CGUIWindowFullScreen::FreeResources(bool forceUnload)
{
	g_settings.Save();
	DynamicResourceAlloc(true);

	CGUIWindow::FreeResources(forceUnload);
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
			g_application.m_pPlayer->Seek(false, false);
			return true;
		}
		break;

		case ACTION_STEP_FORWARD:
		{
			g_application.m_pPlayer->Seek(true, false);
			return true;
		}
		break;

		case ACTION_BIG_STEP_BACK:
		{
			g_application.m_pPlayer->Seek(false, true);
			return true;
		}
		break;

		case ACTION_BIG_STEP_FORWARD:
		{
			g_application.m_pPlayer->Seek(true, true);
			return true;
		}
		break;

		case ACTION_SMALL_STEP_BACK:
		{
			int iOrgPos = (int)g_application.GetTime();
			int iJumpSize = g_advancedSettings.m_videoSmallStepBackSeconds; // Secs
			int iSetPos = (iOrgPos > iJumpSize) ? iOrgPos - iJumpSize : 0;
			g_application.SeekTime((double)iSetPos);
			return true;
		}
		break;

		case ACTION_SHOW_OSD:
		{
			// TODO: Toggle OSD when implemented
			return true;
		}
		break;

		case ACTION_ASPECT_RATIO:
		{
			// toggle the aspect ratio mode (only if the info is onscreen)
			if (m_bShowViewModeInfo)
			{
				g_renderManager.SetViewMode(++g_settings.m_currentVideoSettings.m_ViewMode);
			}
			m_bShowViewModeInfo = true;
			m_dwShowViewModeTimeout = GetTickCount();
		}
		return true;

		case ACTION_ZOOM_IN:
		{
			g_settings.m_currentVideoSettings.m_CustomZoomAmount += 0.01f;
			if (g_settings.m_currentVideoSettings.m_CustomZoomAmount > 2.0f)
				g_settings.m_currentVideoSettings.m_CustomZoomAmount = 2.0f;
			g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
			g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
		}
		return true;

		case ACTION_ZOOM_OUT:
		{
			g_settings.m_currentVideoSettings.m_CustomZoomAmount -= 0.01f;
			if (g_settings.m_currentVideoSettings.m_CustomZoomAmount < 0.5f)
				g_settings.m_currentVideoSettings.m_CustomZoomAmount = 0.5f;
			g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
			g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
		}
		return true;

		case ACTION_INCREASE_PAR:
		{
			g_settings.m_currentVideoSettings.m_CustomPixelRatio += 0.01f;
			if (g_settings.m_currentVideoSettings.m_CustomPixelRatio > 2.0f)
				g_settings.m_currentVideoSettings.m_CustomPixelRatio = 2.0f;
			g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
			g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
		}
		return true;

		case ACTION_DECREASE_PAR:
		{
			g_settings.m_currentVideoSettings.m_CustomPixelRatio -= 0.01f;
			if (g_settings.m_currentVideoSettings.m_CustomPixelRatio < 0.5f)
				g_settings.m_currentVideoSettings.m_CustomPixelRatio = 0.5f;
			g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
			g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
		}
		return true;

		default:
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
			// Check whether we've come back here from a window during which time we've actually
			// stopped playing videos
			if (message.GetParam1() == WINDOW_INVALID && !g_application.IsPlayingVideo())
			{ 
				// why are we here if nothing is playing???
				g_windowManager.PreviousWindow();
				return true;
			}

			m_bLastRender = false;
			g_infoManager.SetShowCodec(false);
			m_bShowCurrentTime = false;
			m_bShowViewModeInfo = false;
			g_infoManager.SetDisplayAfterSeek(0); // Make sure display after seek is off

			// switch resolution
			CSingleLock lock (g_graphicsContext);
			g_graphicsContext.SetFullScreenVideo(true);
//			RESOLUTION res = g_renderManager.GetResolution();
//			g_graphicsContext.SetVideoResolution(res, false, false);
			lock.Leave();

			// Make sure renderer is uptospeed
			g_renderManager.Update(false);

			// Now call the base class to load our windows
			CGUIWindow::OnMessage(message);

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

void CGUIWindowFullScreen::OnWindowLoaded()
{
	CGUIWindow::OnWindowLoaded();

	// Override the clear colour - We must never clear fullscreen
	m_clearBackground = 0;

	CGUILabelControl* pLabel = (CGUILabelControl*)GetControl(LABEL_CURRENT_TIME);
	if(pLabel && pLabel->GetVisibleCondition() == 0)
	{
		pLabel->SetVisibleCondition(PLAYER_DISPLAY_AFTER_SEEK, false);
		pLabel->SetVisible(true);
		pLabel->SetLabel("$INFO(VIDEOPLAYER.TIME) / $INFO(VIDEOPLAYER.DURATION)");
	}
}

// Dummy override of Render() - RenderFullScreen() is where the action takes place
// this is called via DVDPlayer when the video window is flipped (indicating a frame
// change) so that we get smooth video playback
void CGUIWindowFullScreen::Render()
{
	g_renderManager.RenderUpdate(true);
	return;
}

bool CGUIWindowFullScreen::NeedRenderFullScreen()
{
	CSingleLock lock (g_graphicsContext);

	if(g_application.m_pPlayer)
	{
		if (g_application.GetPlaySpeed() != 1) return true;
		if(g_application.m_pPlayer->IsPaused()) return true;
//		if(g_application.m_pPlayer->IsCaching()) return true; //TODO
		if(!g_application.m_pPlayer->IsPlaying()) return true;
	}

	if(g_infoManager.GetBool(PLAYER_SHOWCODEC)) return true;
	if(m_bShowCurrentTime) return true;
	if(m_bShowViewModeInfo) return true;
	if (IsAnimating(ANIM_TYPE_HIDDEN)) return true; // for the above info conditions
	if (g_infoManager.GetDisplayAfterSeek()) return true;
	if (g_infoManager.GetBool(PLAYER_SEEKBAR, GetID())) return true;

	if(m_bLastRender)
	{
		m_bLastRender = false;
	}

	return false;
}

void CGUIWindowFullScreen::RenderFullScreen()
{
	if (g_application.GetPlaySpeed() != 1)
		g_infoManager.SetDisplayAfterSeek();
	if (m_bShowCurrentTime)
		g_infoManager.SetDisplayAfterSeek();

	m_bLastRender = true;
	if(!g_application.m_pPlayer) return;

	if(g_application.m_pPlayer->IsCaching()) 
	{
		g_infoManager.SetDisplayAfterSeek(0); // Make sure these stuff aren't visible now
	}

	//------------------------

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

	//----------------------
	// ViewMode Information
	//----------------------
	if (m_bShowViewModeInfo && GetTickCount() - m_dwShowViewModeTimeout > 2500)
	{
		m_bShowViewModeInfo = false;
	}
	if (m_bShowViewModeInfo)
	{
		{
			// get the "View Mode" string
			CStdString strTitle = g_localizeStrings.Get(629);
			CStdString strMode = g_localizeStrings.Get(630 + g_settings.m_currentVideoSettings.m_ViewMode);
			CStdString strInfo;
			strInfo.Format("%s : %s", strTitle.c_str(), strMode.c_str());
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW1);
			msg.SetLabel(strInfo);
			OnMessage(msg);
		}
		// show sizing information
		{
			CStdString strSizing;
			strSizing.Format("Zoom x%2.2f Pixel Ratio: %2.2f:1", g_settings.m_fZoomAmount, g_settings.m_fPixelRatio);
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW2);
			msg.SetLabel(strSizing);
			OnMessage(msg);
		}
		// show resolution information
		{
			RESOLUTION iResolution = g_graphicsContext.GetVideoResolution();
			CStdString strStatus;
			strStatus.Format("%ix%i %s", g_settings.m_ResInfo[iResolution].iWidth, g_settings.m_ResInfo[iResolution].iHeight, g_settings.m_ResInfo[iResolution].strMode);
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW3);
			msg.SetLabel(strStatus);
			OnMessage(msg);
		}
	}

	if (g_infoManager.GetBool(PLAYER_SHOWCODEC) || m_bShowViewModeInfo)
	{
		SET_CONTROL_VISIBLE(LABEL_ROW1);
		SET_CONTROL_VISIBLE(LABEL_ROW2);
		SET_CONTROL_VISIBLE(LABEL_ROW3);
		SET_CONTROL_VISIBLE(BLUE_BAR);
	}
	else if (/*m_timeCodeShow*/0) // TODO
	{
		SET_CONTROL_VISIBLE(LABEL_ROW1);
		SET_CONTROL_HIDDEN(LABEL_ROW2);
		SET_CONTROL_HIDDEN(LABEL_ROW3);
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
}