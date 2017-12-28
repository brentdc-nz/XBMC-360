/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIVideoControl.h"
#include "GraphicContext.h"
#include "..\Application.h"
#include "..\cores\VideoRenderers\RenderManager.h"

CGUIVideoControl::CGUIVideoControl(int parentID, int controlID, float posX, float posY, float width, float height)
    : CGUIControl(parentID, controlID, posX, posY, width, height)
{
}

CGUIVideoControl::~CGUIVideoControl(void)
{
}

void CGUIVideoControl::Render()
{
	if (!IsVisible()) return;

	// don't render if we aren't playing video, or if the renderer isn't started
	// (otherwise the lock we have from CApplication::Render() may clash with the startup
	// locks in the RenderManager.)
	if (g_application.IsPlayingVideo() && g_renderManager.IsStarted())
	{
//		if (!g_application.m_pPlayer->IsPaused())
//			g_application.ResetScreenSaver();

		RECT rc;
		rc.left = m_posX;
		rc.top = m_posY;
		rc.right = rc.left + m_width;
		rc.bottom = rc.top + m_height;
		g_graphicsContext.SetViewWindow(rc);

		g_renderManager.RenderUpdate(false);
	}

	CGUIControl::Render();
}
