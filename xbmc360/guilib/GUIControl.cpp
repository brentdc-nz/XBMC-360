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

#include "GUIControl.h"
#include "GUIWindowManager.h"
#include "GUIInfoManager.h"

using namespace std;

CGUIControl::CGUIControl()
{
	m_hasRendered = false;
	m_bHasFocus = false;
	m_controlID = 0;
	m_parentID = 0;
	m_visible = true;

	m_posX = 0;
	m_posY = 0;
	m_width = 0;
	m_height = 0;

	m_dwControlLeft = 0;
	m_dwControlRight = 0;
	m_dwControlUp = 0;
	m_dwControlDown = 0;
}

CGUIControl::CGUIControl(int parentID, int controlID, float posX, float posY, float width, float height)
{
	m_controlID = controlID;
	m_visible = true;
	m_parentID = parentID;
	m_forceHidden = false;
	m_hasRendered = false;

	m_posX = (int)posX;
	m_posY = (int)posY;
	m_width = (int)width;
	m_height = (int)height;

	m_bHasFocus = false;

	m_dwControlLeft = 0;
	m_dwControlRight = 0;
	m_dwControlUp = 0;
	m_dwControlDown = 0;
}

CGUIControl::~CGUIControl(void)
{
}

int CGUIControl::GetID(void) const
{
	return m_controlID;
}

DWORD CGUIControl::GetParentID(void) const
{
	return (DWORD)m_parentID;
}

bool CGUIControl::HasFocus(void) const
{
	return m_bHasFocus;
}

void CGUIControl::SetFocus(bool bOnOff)
{
	m_bHasFocus = bOnOff;
}

bool CGUIControl::HasID(int id) const
{
	return GetID() == id;
}

void CGUIControl::SetNavigation(DWORD dwUp, DWORD dwDown, DWORD dwLeft, DWORD dwRight)
{
	m_dwControlUp = dwUp;
	m_dwControlDown = dwDown;
	m_dwControlLeft = dwLeft;
	m_dwControlRight = dwRight;
}

void CGUIControl::DynamicResourceAlloc(bool bOnOff)
{
}

void CGUIControl::AllocResources()
{
	m_hasRendered = false;
	m_bInvalidated = true;
	m_bAllocated = true;
}

void CGUIControl::FreeResources()
{
	if (m_bAllocated)
	{
		// Reset our animation states
//		for (unsigned int i = 0; i < m_animations.size(); i++)
//		m_animations[i].ResetAnimation();
		m_bAllocated=false;
	}
	m_hasRendered = false;
}

// the main render routine.
// 1. animate and set the animation transform
// 2. if visible, paint
// 3. reset the animation transform
void CGUIControl::DoRender()
{
/*	Animate(currentTime);
	if (m_hasCamera)
		g_graphicsContext.SetCameraPosition(m_camera);*/

	m_visible = g_infoManager.GetBool(m_visibleCondition);

	if (IsVisible())
		Render();
/*	if (m_hasCamera)
		g_graphicsContext.RestoreCameraPosition();
	g_graphicsContext.RemoveTransform();*/
}

void CGUIControl::Render()
{
	m_bInvalidated = false;
	m_hasRendered = true;
}

bool CGUIControl::OnAction(const CAction &action)
{
	switch (action.wID)
	{
		case ACTION_MOVE_DOWN:
		OnDown();
		return true;
		break;

		case ACTION_MOVE_UP:
		OnUp();
		return true;
		break;

		case ACTION_MOVE_LEFT:
		OnLeft();
		return true;
		break;
		
		case ACTION_MOVE_RIGHT:
		OnRight();
		return true;
		break;
	}
	return false;
}

// Movement controls (derived classes can override)
void CGUIControl::OnUp()
{
	if (HasFocus() && m_controlID != m_dwControlUp)
	{
		SetFocus(false);
		CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), m_dwControlUp, ACTION_MOVE_UP);
		SendWindowMessage(msg);
	}
}

void CGUIControl::OnDown()
{
	if (HasFocus() &&m_controlID != m_dwControlDown)
	{
		SetFocus(false);
		CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), m_dwControlDown, ACTION_MOVE_DOWN);
		SendWindowMessage(msg);
	}
}

void CGUIControl::OnLeft()
{
	if (HasFocus() && m_controlID != m_dwControlLeft)
	{
		SetFocus(false);
		CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), m_dwControlLeft, ACTION_MOVE_LEFT);
		SendWindowMessage(msg);
	}
}

void CGUIControl::OnRight()
{
	if (HasFocus() && m_controlID != m_dwControlRight)
	{
		SetFocus(false);
		CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), m_dwControlRight, ACTION_MOVE_RIGHT);
		SendWindowMessage(msg);
	}
}

void CGUIControl::SendWindowMessage(CGUIMessage &message)
{
	CGUIWindow *pWindow = g_windowManager.GetWindow(GetParentID());
	if (pWindow)
		pWindow->OnMessage(message);
//	else
//		g_graphicsContext.SendMessage(message);
}

bool CGUIControl::OnMessage(CGUIMessage& message)
{
	if ( message.GetControlId() == GetID() )
	{
		switch (message.GetMessage() )
		{
			case GUI_MSG_SETFOCUS:
				SetFocus(true);
				return true;
				break;

			case GUI_MSG_LOSTFOCUS:
				SetFocus(false);
				return true;
				break;
		}
	}
	return false;
}

bool CGUIControl::IsVisible() const
{
	if (m_forceHidden) return false;
	return m_visible;
}

void CGUIControl::SetVisibleCondition(int visible/*, const CGUIInfoBool &allowHiddenFocus*/)
{
	m_visibleCondition = visible;
}