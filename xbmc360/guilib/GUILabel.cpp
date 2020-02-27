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

	m_iPosX = posX;
	m_iPosY = posY;
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
	m_iPosX = fPosX;
	m_iPosY = fPosY;
}

void CGUILabel::Render()
{
	if(m_strText.IsEmpty()) // Not used for some buttons
		return;

	if(!m_label.font)
	{
		CLog::Log(LOGWARNING, "Font not found!");
		return;
	}

	m_label.font->DrawText(m_iPosX + m_label.offsetX, m_iPosY + m_label.offsetY, m_label.dwTextColor, m_strText, m_label.dwAlign );
}