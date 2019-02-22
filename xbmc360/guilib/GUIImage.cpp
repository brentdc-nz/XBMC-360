#include "GUIImage.h"

CGUIImage::CGUIImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture)
    : CGUIControl(parentID, controlID, posX, posY, width, height)
    , m_texture(posX, posY, width, height, texture)
{

}

CGUIImage::~CGUIImage(void)
{
}

void CGUIImage::AllocResources()
{
	CGUIControl::AllocResources();

//	m_bDynamicResourceAlloc = bOnOff;
	m_texture.AllocResources();//DynamicResourceAlloc(bOnOff); //MARTY FIXME WIP
}

void CGUIImage::FreeResources()
{
	CGUIControl::FreeResources(); 

	m_texture.FreeResources();
}

void CGUIImage::SetInfo(const CGUIInfoLabel &info)
{
	m_info = info;
	// a constant image never needs updating
	if (m_info.IsConstant())
		m_texture.SetFileName(m_info.GetLabel(0));
}

void CGUIImage::Update()
{
	m_texture.SetPosition(m_posX, m_posY);
}

void CGUIImage::Render()
{
	if (!IsVisible()) return;

	if (!m_bAllocated) return;

	m_texture.Render();

	CGUIControl::Render();
}