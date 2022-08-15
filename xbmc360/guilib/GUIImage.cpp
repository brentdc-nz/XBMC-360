#include "GUIImage.h"

CGUIImage::CGUIImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture)
    : CGUIControl(parentID, controlID, posX, posY, width, height)
    , m_texture(posX, posY, width, height, texture)
{
	ControlType = GUICONTROL_IMAGE;
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
	// A constant image never needs updating
	if(m_info.IsConstant())
		m_texture.SetFileName(m_info.GetLabel(0));
}

void CGUIImage::SetFileName(const CStdString strFileName)
{
	CStdString strTexture = m_texture.GetFileName();

	if(strFileName == strTexture)
		return;

	// Don't completely free resources here - we may be just changing
	// Filenames mid-animation
	m_texture.FreeResources();
	m_texture.SetFileName(strFileName);
	// Don't allocate resources here as this is done at render time
}

void CGUIImage::Update()
{
	m_texture.SetPosition(m_posX, m_posY);
	m_texture.SetHeight(m_height);
	m_texture.SetWidth(m_width);
}

void CGUIImage::Render()
{
	if(!IsVisible()) return;

	if(!m_bAllocated)
		m_texture.AllocResources();

	m_texture.Render();

	CGUIControl::Render();
}