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

#include "GUILabel.h"
#include "..\utils\Log.h"

CGUILabel::CGUILabel(float posX, float posY, float width, float height, const CLabelInfo& labelInfo/*, CGUILabel::OVER_FLOW overflow*/)
{
	m_label = labelInfo;
	m_strText = "";

	m_fWidth = width;
	m_fHeight = height;
	m_fPosX = posX;
	m_fPosY = posY;

	m_bDisabled = false;
}

CGUILabel::~CGUILabel(void)
{
}

void CGUILabel::SetText(CStdString strText)
{
	m_strText = strText;
}

CStdString CGUILabel::GetText()
{
	return m_strText;
}

void CGUILabel::SetPosition(float fPosX, float fPosY)
{
	m_fPosX = fPosX;
	m_fPosY = fPosY;
}

void CGUILabel::SetDisabledColor(bool bDisabled)
{
	m_bDisabled = bDisabled;
}

void CGUILabel::Render()
{
	// Not used for some buttons
	if(m_strText.IsEmpty())
		return;

	if(!m_label.font)
	{
		CLog::Log(LOGWARNING, "Font not found!");
		return;
	}

	DWORD dwColor = 0;

	if(m_bDisabled)
		dwColor = m_label.dwDisabledTextColor;
	else
		dwColor = m_label.dwTextColor;

	m_label.font->DrawText(m_fPosX + m_label.offsetX, m_fPosY + m_label.offsetY, dwColor, m_strText, m_fWidth, m_fHeight, m_label.dwAlign);
}