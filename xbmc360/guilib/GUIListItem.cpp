#include "GUIListItem.h"

CGUIListItem::CGUIListItem()
{
	m_bIsFolder = false;
	m_strLabel = "";
	m_pThumbnailImage = NULL;
}

CGUIListItem::~CGUIListItem()
{
}

CGUIImage* CGUIListItem::GetThumbnail()
{
	return m_pThumbnailImage;
}

void CGUIListItem::SetThumbnail(CGUIImage* pImage)
{
	if (m_pThumbnailImage)
	{
		m_pThumbnailImage->FreeResources();
		delete m_pThumbnailImage;
	}
	m_pThumbnailImage = pImage;
}