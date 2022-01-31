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

#include "GUIScrollBarControl.h"
#include "GUIWindowManager.h"

#define MIN_NIB_SIZE 4.0f

CGUIScrollBar::CGUIScrollBar(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& backGroundTexture, const CTextureInfo& barTexture,
							 const CTextureInfo& barTextureFocus)
    : CGUIControl(parentID, controlID, posX, posY, width, height)
    , m_guiBackground(posX, posY, width, height, backGroundTexture)
	, m_guiBarNoFocus(posX, posY, width, 100, barTexture) //FIXME
	, m_guiBarFocus(posX, posY, width, 100, barTextureFocus) //FIXME
{
	m_numItems = 100;
	m_offset = 0;
	m_pageSize = 10;
	m_binvalidated = true;
}

CGUIScrollBar::~CGUIScrollBar(void)
{
}

void CGUIScrollBar::AllocResources()
{
	m_guiBackground.AllocResources();
	m_guiBarNoFocus.AllocResources();
	m_guiBarFocus.AllocResources();

	CGUIControl::AllocResources();
}

void CGUIScrollBar::FreeResources()
{
	m_guiBackground.FreeResources();
	m_guiBarNoFocus.FreeResources();
	m_guiBarFocus.FreeResources();

	CGUIControl::FreeResources();
}

void CGUIScrollBar::Render()
{
	if (!IsVisible())
		return;

	m_guiBackground.Render();

	if(m_binvalidated)
		UpdateBarSize();

	if(HasFocus())
		m_guiBarFocus.Render();
	else
		m_guiBarNoFocus.Render();

	CGUIControl::Render();
}

bool CGUIScrollBar::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_ITEM_SELECT:
			SetValue(message.GetParam1());
			return true;

		case GUI_MSG_LABEL_RESET:
			SetRange(message.GetParam1(), message.GetParam2());
			return true;

		case GUI_MSG_PAGE_UP:
			OnUp();
			return true;

		case GUI_MSG_PAGE_DOWN:
			OnDown();
			return true;
	}

	return CGUIControl::OnMessage(message);
}

void CGUIScrollBar::UpdateBarSize()
{
	m_binvalidated = false;

    // Calculate the height to display the nib at

    float percent = (float)m_pageSize / m_numItems;
    float nibSize = GetHeight() * percent;

    if(nibSize < MIN_NIB_SIZE) 
		nibSize = MIN_NIB_SIZE;

    if(nibSize > GetHeight()) 
		nibSize = (float)GetHeight();

    m_guiBarNoFocus.SetHeight(nibSize);
    m_guiBarFocus.SetHeight(nibSize);

    // Also calculate the position
    percent = (float)m_offset / m_numItems;

    float nibPos = GetHeight() * percent;
  
	if(nibPos < 0)
		nibPos = 0;

    if(nibPos > GetHeight() - nibSize)
		nibPos = GetHeight() - nibSize;

    m_guiBarNoFocus.SetPosition((float)GetXPosition(), (float)GetYPosition() + nibPos);
    m_guiBarFocus.SetPosition((float)GetXPosition(), (float)GetYPosition() + nibPos);
}

void CGUIScrollBar::SetRange(int pageSize, int numItems)
{
	m_pageSize = pageSize;
	m_numItems = numItems;
//	m_offset = 0; //TODO
	m_binvalidated = true;
}

void CGUIScrollBar::SetValue(int value)
{
	m_offset = value;
	m_binvalidated = true;
}

void CGUIScrollBar::OnUp()
{
	m_offset -= /*numSteps*/1 * m_pageSize;

	if(m_offset > m_numItems - m_pageSize)
		m_offset = m_numItems - m_pageSize;
	
	if(m_offset < 0)
		m_offset = 0;

	CGUIMessage message(GUI_MSG_NOTIFY_ALL, GetParentID(), GetID(), GUI_MSG_SCROLL_CHANGE, GUI_MSG_SCROLL_UP);
	g_windowManager.SendMessage(message);

	m_binvalidated = true;
}

void CGUIScrollBar::OnDown()
{
	m_offset += /*numSteps*/1 * m_pageSize;

	if(m_offset > m_numItems - m_pageSize)
		m_offset = m_numItems - m_pageSize;
	
	if(m_offset < 0)
		m_offset = 0;

	CGUIMessage message(GUI_MSG_NOTIFY_ALL, GetParentID(), GetID(), GUI_MSG_SCROLL_CHANGE, GUI_MSG_SCROLL_DOWN);
	g_windowManager.SendMessage(message);

	m_binvalidated = true;
}