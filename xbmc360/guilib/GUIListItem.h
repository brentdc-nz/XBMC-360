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

	virtual void SetLabel(const CStdString& strLabel);
	const CStdString& GetLabel() const;
	void SetThumbnailImage(const CStdString& strThumbnail);
	const CStdString& GetThumbnailImage() const;
	void SetThumbnail(CGUIImage* pImage);
	bool HasThumbnail() const;
	CGUIImage* GetThumbnail();
	void SetIconImage(const CStdString& strIcon);
	const CStdString& GetIconImage() const;
	bool HasIcon() const;
	void SetIcon(CGUIImage* pImage);
	CGUIImage* GetIcon();
	void Select(bool bOnOff);
	bool IsSelected() const;

	bool m_bIsFolder; // Is item a folder or a file

protected:
	CStdString m_strLabel;
	CStdString m_strThumbnailImage; // Filename of thumbnail
	CStdString m_strIcon; // Filename of icon

	CGUIImage* m_pIconImage; // pointer to CImage containing the icon
	CGUIImage* m_pThumbnailImage; // Pointer to CImage containing the thumbnail
	bool m_bSelected; // Item is selected or not
};

#endif //GUILIB_GUILISTITEM_H