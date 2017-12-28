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

#include "GUIButtonControl.h"
#include "GUIWindowManager.h" //FIXME ME - Remove for grahicscontext.h

#include "GUIFontManager.h" //REMOVE ME FIXME ME

using namespace std;

CGUIButtonControl::CGUIButtonControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus, const CLabelInfo& labelInfo)
    : CGUIControl(parentID, controlID, posX, posY, width, height)
    , m_imgFocus(posX, posY, width, height, textureFocus)
    , m_imgNoFocus(posX, posY, width, height, textureNoFocus)
    , m_label(posX, posY, width, height, labelInfo)
    , m_label2(posX, posY, width, height, labelInfo)
{
	m_bSelected = false;
//	m_alpha = 255;
//	m_focusCounter = 0;
//	ControlType = GUICONTROL_BUTTON;
}

CGUIButtonControl::~CGUIButtonControl(void)
{
}

void CGUIButtonControl::DynamicResourceAlloc(bool bOnOff)
{
	CGUIControl::DynamicResourceAlloc(bOnOff);
//	m_imgFocus.DynamicResourceAlloc(bOnOff);
//	m_imgNoFocus.DynamicResourceAlloc(bOnOff);
	m_imgFocus.AllocResources();
	m_imgNoFocus.AllocResources();
}

void CGUIButtonControl::Render()
{
	if (!IsVisible()) return;

/*	if (m_bInvalidated)
  {
    m_imgFocus.SetWidth(m_width);
    m_imgFocus.SetHeight(m_height);

    m_imgNoFocus.SetWidth(m_width);
    m_imgNoFocus.SetHeight(m_height);
  }

  if (HasFocus())
  {
    if (m_pulseOnSelect)
    {
      unsigned int alphaCounter = m_focusCounter + 2;
      unsigned int alphaChannel;
      if ((alphaCounter % 128) >= 64)
        alphaChannel = alphaCounter % 64;
      else
        alphaChannel = 63 - (alphaCounter % 64);

      alphaChannel += 192;
      alphaChannel = (unsigned int)((float)m_alpha * (float)alphaChannel / 255.0f);
      m_imgFocus.SetAlpha((unsigned char)alphaChannel);
    }
    m_imgFocus.SetVisible(true);
    m_imgNoFocus.SetVisible(false);
    m_focusCounter++;
  }
  else
  {
    m_imgFocus.SetVisible(false);
    m_imgNoFocus.SetVisible(true);
  }*/
	// render both so the visibility settings cause the frame counter to resetcorrectly
	if(HasFocus())
		m_imgFocus.Render();
	else
		m_imgNoFocus.Render();

	RenderText();
	CGUIControl::Render();
}

void CGUIButtonControl::RenderText()
{
//	m_label.m_label.font = g_fontManager.GetFont("font10"); //FIXME
	
	m_label.Render();

	//m_label.m_label.font->DrawText(80, 80, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "M" );
}

bool CGUIButtonControl::OnAction(const CAction &action)
{
	if (action.wID == ACTION_SELECT_ITEM)
	{
		OnClick();
		return true;
	}
	return CGUIControl::OnAction(action);
}

void CGUIButtonControl::OnClick()
{
	// Save values, SEND_CLICK_MESSAGE may deactivate the window
//	long lHyperLinkWindowID = m_lHyperLinkWindowID;
	vector<CStdString> clickActions = m_clickActions;
	DWORD dwControlID = GetID();
	DWORD dwParentID = GetParentID();

	// button selected, send a message
	CGUIMessage msg(GUI_MSG_CLICKED, m_parentID, m_controlID, 0);
	g_windowManager.SendMessage(msg); 

	if (clickActions.size())
	{
		for (unsigned int i = 0; i < clickActions.size(); i++)
		{
			CGUIMessage message(GUI_MSG_EXECUTE, dwControlID, dwParentID);
			message.SetStringParam(clickActions[i]);
			g_windowManager.SendMessage(message);
		}
		return;
	}

//  if (lHyperLinkWindowID != WINDOW_INVALID)
//  {
 //		g_windowManager.ActivateWindow(lHyperLinkWindowID);
 // }
}

void CGUIButtonControl::SetLabel(const string &label)
{	
	m_label.SetText(label);
}