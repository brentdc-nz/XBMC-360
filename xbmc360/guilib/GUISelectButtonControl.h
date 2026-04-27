/*!
\file GUISelectButtonControl.h
\brief
*/

#ifndef GUILIB_GUIWINDOWSELECTCONTROL_H
#define GUILIB_GUIWINDOWSELECTCONTROL_H

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

/*!
 \ingroup controls
 \brief Button with multi selection choice.
 */
class CGUISelectButtonControl : public CGUIButtonControl
{
public:
	CGUISelectButtonControl(int parentID, int controlID,
	                        float posX, float posY,
	                        float width, float height,
	                        const CTextureInfo& buttonFocus, const CTextureInfo& button,
	                        const CLabelInfo& labelInfo,
	                        const CTextureInfo& selectBackground,
	                        const CTextureInfo& selectArrowLeft, const CTextureInfo& selectArrowLeftFocus,
	                        const CTextureInfo& selectArrowRight, const CTextureInfo& selectArrowRightFocus);
	virtual ~CGUISelectButtonControl(void);
	virtual CGUISelectButtonControl *Clone() const { return new CGUISelectButtonControl(*this); };

	virtual void Render();
	virtual bool OnAction(const CAction &action) ;
	virtual void OnLeft();
	virtual void OnRight();
	virtual bool OnMessage(CGUIMessage& message);
#ifdef _HAS_MOUSE
	virtual bool OnMouseOver(const CPoint &point);
#endif

	virtual void PreAllocResources();
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual void SetPosition(float posX, float posY);

protected:
#ifdef _HAS_MOUSE
	virtual bool OnMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif
	virtual void UpdateColors();
	bool m_bShowSelect;
	CGUITexture m_imgBackground;
	CGUITexture m_imgLeft;
	CGUITexture m_imgLeftFocus;
	CGUITexture m_imgRight;
	CGUITexture m_imgRightFocus;
	std::vector<std::string> m_vecItems;
	int m_iCurrentItem;
	int m_iDefaultItem;
	int m_iStartFrame;
	bool m_bLeftSelected;
	bool m_bRightSelected;
	bool m_bMovedLeft;
	bool m_bMovedRight;
	unsigned int m_ticks;
};
#endif
