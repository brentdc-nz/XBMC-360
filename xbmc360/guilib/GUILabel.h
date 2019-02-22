#ifndef GUILIB_GUILABEL_H
#define GUILIB_GUILABEL_H

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

#include "GUIInfoTypes.h"
#include "GUIFont.h"

class CLabelInfo
{
public:
	CLabelInfo()
	{
		font = NULL;
		offsetX = 0;
		offsetY = 0;
		dwTextColor = D3DCOLOR_ARGB(255, 255, 255, 255);
		dwSelectedTextColor = D3DCOLOR_ARGB(255, 255, 0, 0);
		dwAlign = NULL;
		scrollSuffix = " | ";
	};

	CGUIFont *font;
	float offsetX;
	float offsetY;
	DWORD dwTextColor;
	DWORD dwSelectedTextColor;
	DWORD dwAlign;
	CStdString scrollSuffix;
};

class CGUILabel
{
public:
	CGUILabel(float posX, float posY, float width, float height, const CLabelInfo& labelInfo/*, CGUILabel::OVER_FLOW overflow*/);
	virtual ~CGUILabel(void);

	void SetText(CStdString strText);
	CStdString GetText();
	void SetPosition(float fPosX, float fPosY);
	void Render();

private:
	CLabelInfo m_label;

	CStdString m_strText;
	float m_iPosX;
	float m_iPosY;
};

#endif //GUILIB_GUILABEL_H
