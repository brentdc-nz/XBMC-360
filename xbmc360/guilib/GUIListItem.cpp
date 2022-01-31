#include "GUIListItem.h"

CGUIListItem::CGUIListItem()
{
	m_bIsFolder = false;
	m_bSelected = false;
	m_strLabel = "";
	m_pThumbnailImage = NULL;
	m_pIconImage = NULL;
	m_strIcon = "";
	m_strThumbnailImage = "";
}

CGUIListItem::~CGUIListItem()
{
}

void CGUIListItem::SetLabel(const CStdString& strLabel)
{
	m_strLabel = strLabel;
}

const CStdString& CGUIListItem::GetLabel() const
{
	return m_strLabel;
}

CGUIImage* CGUIListItem::GetThumbnail()
{
	return m_pThumbnailImage;
}

void CGUIListItem::SetThumbnailImage(const CStdString& strThumbnail)
{
	m_strThumbnailImage = strThumbnail;
}

void CGUIListItem::SetIconImage(const CStdString& strIcon)
{
	m_strIcon = strIcon;
}

const CStdString& CGUIListItem::GetIconImage() const
{
	return m_strIcon;
}

bool CGUIListItem::HasIcon() const
{
	return (m_strIcon.size() != 0);
}

void CGUIListItem::SetIcon(CGUIImage* pImage)
{
	if (m_pIconImage)
	{
		m_pIconImage->FreeResources();
		delete m_pIconImage;
	}
	m_pIconImage = pImage;
}

CGUIImage* CGUIListItem::GetIcon()
{
	return m_pIconImage;
}

void CGUIListItem::SetThumbnail(CGUIImage* pImage)
{
	if(m_pThumbnailImage)
	{
		m_pThumbnailImage->FreeResources();
		delete m_pThumbnailImage;
	}
	m_pThumbnailImage = pImage;
}

const CStdString& CGUIListItem::GetThumbnailImage() const
{
	return m_strThumbnailImage;
}

bool CGUIListItem::HasThumbnail() const
{
	return (m_strThumbnailImage.size() != 0);
}

void CGUIListItem::Select(bool bOnOff)
{
	m_bSelected = bOnOff;
}

bool CGUIListItem::IsSelected() const
{
	return m_bSelected;
}