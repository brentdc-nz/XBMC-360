#include "include.h"
#include "GUIVideoControl.h"
#include "GUIWindowManager.h"
#include "Application.h"
#include "cores/VideoRenderers/RenderManager.h"

CGUIVideoControl::CGUIVideoControl(int parentID, int controlID, float posX, float posY, float width, float height)
	: CGUIControl(parentID, controlID, posX, posY, width, height)
{
	ControlType = GUICONTROL_VIDEO;
}

CGUIVideoControl::~CGUIVideoControl(void)
{
}

void CGUIVideoControl::Render()
{
	// Don't render if we aren't playing video, or if the renderer isn't started
	// (otherwise the lock we have from CApplication::Render() may clash with the startup
	// locks in the RenderManager)
	if (g_application.IsPlayingVideo() && g_renderManager.IsStarted())
	{
		if (!g_application.m_pPlayer->IsPaused())
			g_application.ResetScreenSaver();

		g_graphicsContext.SetViewWindow(m_posX, m_posY, m_posX + m_width, m_posY + m_height);
		g_graphicsContext.SetViewPort(m_posX, m_posY, m_width, m_height);

		color_t alpha = g_graphicsContext.MergeAlpha(0xFF000000) >> 24;
		g_renderManager.RenderUpdate(false, 0, alpha);

		g_graphicsContext.RestoreViewPort();
	}
	CGUIControl::Render();
}

#ifdef _HAS_MOUSE
bool CGUIVideoControl::OnMouseEvent(const CPoint &point, const CMouseEvent &event)
{
	if (!g_application.IsPlayingVideo()) return false;

	if (event.m_id == ACTION_MOUSE_LEFT_CLICK)
	{
		// Switch to fullscreen
		CGUIMessage message(GUI_MSG_FULLSCREEN, GetID(), GetParentID());
		g_windowManager.SendMessage(message);
		return true;
	}
	else if (event.m_id == ACTION_MOUSE_RIGHT_CLICK)
	{
		// Toggle the playlist window
		if (g_windowManager.GetActiveWindow() == WINDOW_VIDEO_PLAYLIST)
			g_windowManager.PreviousWindow();
		else
			g_windowManager.ActivateWindow(WINDOW_VIDEO_PLAYLIST);
		return true;
	}
	return false;
}
#endif

bool CGUIVideoControl::CanFocus() const
{
	// Unfocusable
	return false;
}

bool CGUIVideoControl::CanFocusFromPoint(const CPoint &point) const
{
	// Mouse is allowed to focus this control, but it doesn't actually receive focus
	return IsVisible() && HitTest(point);
}
