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
#include "..\utils\log.h"

using namespace std;

CGUIControl::CGUIControl()
{
	m_hasRendered = false;
	m_bHasFocus = false;
	m_enabled = true;
	m_controlID = 0;
	m_parentID = 0;
	m_visible = true;
	m_forceHidden = false;

	m_posX = 0;
	m_posY = 0;
	m_width = 0;
	m_height = 0;

	m_dwControlLeft = 0;
	m_dwControlRight = 0;
	m_dwControlUp = 0;
	m_dwControlDown = 0;

	ControlType = GUICONTROL_UNKNOWN;
	m_parentControl = NULL;
}

CGUIControl::CGUIControl(int parentID, int controlID, float posX, float posY, float width, float height)
{
	m_controlID = controlID;
	m_visible = true;
	m_forceHidden = false;
	m_parentID = parentID;
	m_forceHidden = false;
	m_hasRendered = false;
	m_bAllocated = false;

	m_posX = posX;
	m_posY = posY;
	m_width = width;
	m_height = height;

	m_bHasFocus = false;
	m_enabled = true;

	m_dwControlLeft = 0;
	m_dwControlRight = 0;
	m_dwControlUp = 0;
	m_dwControlDown = 0;

	ControlType = GUICONTROL_UNKNOWN;
	m_parentControl = NULL;
}

CGUIControl::~CGUIControl(void)
{
}

int CGUIControl::GetID(void) const
{
	return m_controlID;
}

bool CGUIControl::HasVisibleID(int id) const
{
  return GetID() == id && IsVisible();
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

void CGUIControl::SaveStates(vector<CControlState> &states)
{
  // empty for now - do nothing with the majority of controls
}

void CGUIControl::SetNavigation(DWORD dwUp, DWORD dwDown, DWORD dwLeft, DWORD dwRight)
{
	m_dwControlUp = dwUp;
	m_dwControlDown = dwDown;
	m_dwControlLeft = dwLeft;
	m_dwControlRight = dwRight;
}

DWORD CGUIControl::GetNextControl(int direction) const
{
	switch(direction)
	{
		case ACTION_MOVE_UP:
			return m_dwControlUp;
		case ACTION_MOVE_DOWN:
			return m_dwControlDown;
		case ACTION_MOVE_LEFT:
			return m_dwControlLeft;
		case ACTION_MOVE_RIGHT:
			return m_dwControlRight;
		default:
			return -1;
	}
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
void CGUIControl::DoRender(unsigned int currentTime)
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
	switch (action.GetID())
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
		// Send a message to the window with the sender set as the window
		CGUIMessage msg(GUI_MSG_MOVE, GetParentID(), GetID(), ACTION_MOVE_UP);
		SendWindowMessage(msg);
	}
}

void CGUIControl::OnDown()
{
	if (HasFocus() && m_controlID != m_dwControlDown)
	{
		// Send a message to the window with the sender set as the window
		CGUIMessage msg(GUI_MSG_MOVE, GetParentID(), GetID(), ACTION_MOVE_DOWN);
		SendWindowMessage(msg);
	}
}

void CGUIControl::OnLeft()
{
	if (HasFocus() && m_controlID != m_dwControlLeft)
	{
		// Send a message to the window with the sender set as the window
		CGUIMessage msg(GUI_MSG_MOVE, GetParentID(), GetID(), ACTION_MOVE_LEFT);
		SendWindowMessage(msg);
	}
}

void CGUIControl::OnRight()
{
	if (HasFocus() && m_controlID != m_dwControlRight)
	{
		// Send a message to the window with the sender set as the window
		CGUIMessage msg(GUI_MSG_MOVE, GetParentID(), GetID(), ACTION_MOVE_RIGHT);
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
	if(message.GetControlId() == GetID())
	{
		switch(message.GetMessage())
		{
			case GUI_MSG_SETFOCUS:
			// If control is disabled then move 2 the next control
			if(!CanFocus())
			{
				CLog::Log(LOGERROR, "Control %u in window %u has been asked to focus, "
						  "but it can't",
						  GetID(), GetParentID());
				return false;
			}
			SetFocus(true);
			{
				// Inform our parent window that this has happened
				CGUIMessage message(GUI_MSG_FOCUSED, GetParentID(), GetID());
				if(m_parentControl)
					m_parentControl->OnMessage(message);
			}
			return true;
			break;

			case GUI_MSG_LOSTFOCUS:
			{
				SetFocus(false);
				// And tell our parent so it can unfocus
				if(m_parentControl)
					m_parentControl->OnMessage(message);
				
				return true;
			}
			break;

			case GUI_MSG_VISIBLE:
				m_visible = m_visibleCondition ? g_infoManager.GetBool(m_visibleCondition) : true;
				m_forceHidden = false;
				return true;
				break;

			case GUI_MSG_HIDDEN:
				m_forceHidden = true;
				return true;
				break;

			// Note that the skin <enable> tag will override these messages
			case GUI_MSG_ENABLED:
				SetEnabled(true);
				return true;

			case GUI_MSG_DISABLED:
				SetEnabled(false);
				return true;
		}
	}
	return false;
}

void CGUIControl::SetEnabled(bool bEnable)
{
	m_enabled = bEnable;
}

bool CGUIControl::CanFocus() const
{
	if (!IsVisible()) return false;
	if (IsDisabled()) return false;

	return true;
}

bool CGUIControl::IsDisabled() const
{
	return !m_enabled;
}

bool CGUIControl::IsVisible() const
{
	if(m_forceHidden) return false;
	return m_visible;
}

void CGUIControl::SetVisible(bool bVisible)
{
	// just force to hidden if necessary
	m_forceHidden = !bVisible;
}

void CGUIControl::SetVisibleCondition(int visible/*, const CGUIInfoBool &allowHiddenFocus*/)
{
	m_visibleCondition = visible;
}

void CGUIControl::SetPosition(float posX, float posY)
{
	if ((m_posX != posX) || (m_posY != posY))
	{
		m_posX = posX;
		m_posY = posY;
		Update();
	}
}

float CGUIControl::GetXPosition() const
{
	return m_posX;
}

float CGUIControl::GetYPosition() const
{
	return m_posY;
}

float CGUIControl::GetWidth() const
{
	return m_width;
}

float CGUIControl::GetHeight() const
{
	return m_height;
}

void CGUIControl::SetWidth(float iWidth)
{
	if(m_width != iWidth)
	{
		m_width = iWidth;
		Update();
	}
}

void CGUIControl::SetHeight(float iHeight)
{
	if (m_height != iHeight)
	{
		m_height = iHeight;
		Update();
	}
}