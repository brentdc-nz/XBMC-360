#include "include.h"
#include "GUIBorderedImage.h"

CGUIBorderedImage::CGUIBorderedImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture, const CTextureInfo& borderTexture, const FRECT &borderSize)
	: CGUIImage(parentID, controlID, posX + borderSize.left, posY + borderSize.top, width - borderSize.left - borderSize.right, height - borderSize.top - borderSize.bottom, texture),
	m_borderImage(posX, posY, width, height, borderTexture)
{
	memcpy(&m_borderSize, &borderSize, sizeof(FRECT));
	ControlType = GUICONTROL_BORDEREDIMAGE;
}

CGUIBorderedImage::CGUIBorderedImage(const CGUIBorderedImage &right)
: CGUIImage(right), m_borderImage(right.m_borderImage)
{
	memcpy(&m_borderSize, &right.m_borderSize, sizeof(FRECT));
	ControlType = GUICONTROL_BORDEREDIMAGE;
}

CGUIBorderedImage::~CGUIBorderedImage(void)
{
}

void CGUIBorderedImage::Render()
{
	if (!m_borderImage.GetFileName().IsEmpty() && m_texture.ReadyToRender())
	{
		CRect rect = CRect(m_texture.GetXPosition(), m_texture.GetYPosition(), m_texture.GetXPosition() + m_texture.GetWidth(), m_texture.GetYPosition() + m_texture.GetHeight());
		rect.Intersect(m_texture.GetRenderRect());
		m_borderImage.SetPosition(rect.x1 - m_borderSize.left, rect.y1 - m_borderSize.top);
		m_borderImage.SetWidth(rect.Width() + m_borderSize.left + m_borderSize.right);
		m_borderImage.SetHeight(rect.Height() + m_borderSize.top + m_borderSize.bottom);
		m_borderImage.Render();
	}
	CGUIImage::Render();
}

void CGUIBorderedImage::PreAllocResources()
{
	m_borderImage.PreAllocResources();
	CGUIImage::PreAllocResources();
}

void CGUIBorderedImage::AllocResources()
{
	m_borderImage.AllocResources();
	CGUIImage::AllocResources();
}

void CGUIBorderedImage::FreeResources(bool immediately)
{
	m_borderImage.FreeResources(immediately);
	CGUIImage::FreeResources(immediately);
}

void CGUIBorderedImage::DynamicResourceAlloc(bool bOnOff)
{
	m_borderImage.DynamicResourceAlloc(bOnOff);
	CGUIImage::DynamicResourceAlloc(bOnOff);
}