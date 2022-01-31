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

#ifndef GUILIB_GUIBUTTONCONTROL_H
#define GUILIB_GUIBUTTONCONTROL_H

#include "GUITexture.h"
#include "GUID3DTexture.h"
#include "GUILabel.h"
#include "GUIControl.h"

class CGUIButtonControl : public CGUIControl
{
public:
	CGUIButtonControl(int parentID, int controlID,
                    float posX, float posY, float width, float height,
                    const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus,
                    const CLabelInfo &label);

	virtual ~CGUIButtonControl(void);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual void AllocResources();
	virtual void Update();
	virtual void Render();
	virtual bool OnAction(const CAction &action);
	virtual bool OnMessage(CGUIMessage& message);
	const CStdString GetLabel();
	void SetLabel(const string & aLabel);
	void SetClickActions(const vector<CStdString>& clickActions) { m_clickActions = clickActions; };

protected:
	virtual void RenderText();

	void OnClick();

	CGUID3DTexture m_imgFocus;
	CGUID3DTexture m_imgNoFocus;

	CGUILabel      m_label;
	CGUILabel      m_label2;
	
	vector<CStdString> m_clickActions;

	bool m_bSelected;
};

#endif //GUILIB_GUIBUTTONCONTROL_H