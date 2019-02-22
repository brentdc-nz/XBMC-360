#ifndef GUILIB_GUILISTITEM_H
#define GUILIB_GUILISTITEM_H

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

#include "..\utils\StdString.h"
#include "GUIImage.h"

class CGUIListItem
{
public:
	CGUIListItem(void);
	~CGUIListItem(void);

	CStdString GetLabel() { return m_strLabel; };

	void SetThumbnail(CGUIImage* pImage);
	CGUIImage* GetThumbnail();

	bool m_bIsFolder; // Is item a folder or a file

protected:
	CStdString m_strLabel;
	CGUIImage* m_pThumbnailImage;  // Pointer to CImage containing the thumbnail
};

#endif //GUILIB_GUILISTITEM_H