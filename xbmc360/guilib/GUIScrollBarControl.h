#ifndef GUILIB_GUISCROLLBAR_H
#define GUILIB_GUISCROLLBAR_H

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

#include "GUID3DTexture.h"
#include "GUIControl.h"

class CGUIScrollBar : public CGUIControl
{
public:
	CGUIScrollBar(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& backGroundTexture, const CTextureInfo& barTexture,
				  const CTextureInfo& barTextureFocus);
	virtual ~CGUIScrollBar(void);

	virtual void AllocResources();
	virtual void FreeResources();
	virtual void Render();
	virtual bool OnMessage(CGUIMessage& message);

	void UpdateBarSize();

	virtual void OnUp();
	virtual void OnDown();

	void SetRange(int pageSize, int numItems);
	void SetValue(int value);

private:
	int m_numItems;
	int m_offset;
	int m_pageSize;
	bool m_binvalidated;

	CGUID3DTexture m_guiBackground;
	CGUID3DTexture m_guiBarNoFocus;
	CGUID3DTexture m_guiBarFocus;
};

#endif //GUILIB_GUISCROLLBAR_H