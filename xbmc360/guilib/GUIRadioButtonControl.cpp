#include "include.h"
#include "GUIRadioButtonControl.h"
#include "GUIInfoManager.h"
#include "GUIFontManager.h"

CGUIRadioButtonControl::CGUIRadioButtonControl(int parentID, int controlID, float posX, float posY, float width, float height,
	const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus,
	const CLabelInfo& labelInfo,
	const CTextureInfo& radioOn, const CTextureInfo& radioOff)
	: CGUIButtonControl(parentID, controlID, posX, posY, width, height, textureFocus, textureNoFocus, labelInfo)
	, m_imgRadioOn(posX, posY, 16, 16, radioOn)
	, m_imgRadioOff(posX, posY, 16, 16, radioOff)
{
	m_radioPosX = 0;
	m_radioPosY = 0;
	m_toggleSelect = 0;
	m_imgRadioOn.SetAspectRatio(CAspectRatio::AR_KEEP);\
	m_imgRadioOff.SetAspectRatio(CAspectRatio::AR_KEEP);
	ControlType = GUICONTROL_RADIO;
}

CGUIRadioButtonControl::~CGUIRadioButtonControl(void)
{
}

void CGUIRadioButtonControl::Render()
{
	CGUIButtonControl::Render();

	// Ask our infoManager whether we are selected or not...
	if (m_toggleSelect)
		m_bSelected = g_infoManager.GetBool(m_toggleSelect, m_parentID);

	if ( IsSelected() && !IsDisabled() )
		m_imgRadioOn.Render();
	else
		m_imgRadioOff.Render();
}

bool CGUIRadioButtonControl::OnAction(const CAction &action)
{
	if (action.GetID() == ACTION_SELECT_ITEM)
	{
		m_bSelected = !m_bSelected;
	}
	return CGUIButtonControl::OnAction(action);
}

bool CGUIRadioButtonControl::OnMessage(CGUIMessage& message)
{
	return CGUIButtonControl::OnMessage(message);
}

void CGUIRadioButtonControl::PreAllocResources()
{
	CGUIButtonControl::PreAllocResources();
	m_imgRadioOn.PreAllocResources();
	m_imgRadioOff.PreAllocResources();
}

void CGUIRadioButtonControl::AllocResources()
{
	CGUIButtonControl::AllocResources();
	m_imgRadioOn.AllocResources();
	m_imgRadioOff.AllocResources();

	SetPosition(m_posX, m_posY);
}

void CGUIRadioButtonControl::FreeResources(bool immediately)
{
	CGUIButtonControl::FreeResources(immediately);
	m_imgRadioOn.FreeResources(immediately);
	m_imgRadioOff.FreeResources(immediately);
}

void CGUIRadioButtonControl::DynamicResourceAlloc(bool bOnOff)
{
	CGUIControl::DynamicResourceAlloc(bOnOff);
	m_imgRadioOn.DynamicResourceAlloc(bOnOff);
	m_imgRadioOff.DynamicResourceAlloc(bOnOff);
}

void CGUIRadioButtonControl::SetPosition(float posX, float posY)
{
	CGUIButtonControl::SetPosition(posX, posY);
	float radioPosX = m_radioPosX ? m_posX + m_radioPosX : (m_posX + m_width - 8) - m_imgRadioOn.GetWidth();
	float radioPosY = m_radioPosY ? m_posY + m_radioPosY : m_posY + (m_height - m_imgRadioOn.GetHeight()) / 2;
	m_imgRadioOn.SetPosition(radioPosX, radioPosY);
	m_imgRadioOff.SetPosition(radioPosX, radioPosY);
}

void CGUIRadioButtonControl::SetRadioDimensions(float posX, float posY, float width, float height)
{
	m_radioPosX = posX;
	m_radioPosY = posY;
	
	if (width)
	{
		m_imgRadioOn.SetWidth(width);
		m_imgRadioOff.SetWidth(width);
	}
	
	if (height)
	{
		m_imgRadioOn.SetHeight(height);
		m_imgRadioOff.SetHeight(height);
	}

	SetPosition(GetXPosition(), GetYPosition());
}

void CGUIRadioButtonControl::SetWidth(float width)
{
	CGUIButtonControl::SetWidth(width);
	SetPosition(GetXPosition(), GetYPosition());
}

void CGUIRadioButtonControl::SetHeight(float height)
{
	CGUIButtonControl::SetHeight(height);
	SetPosition(GetXPosition(), GetYPosition());
}

CStdString CGUIRadioButtonControl::GetDescription() const
{
	CStdString strLabel = CGUIButtonControl::GetDescription();
	
	if (m_bSelected)
		strLabel += " (*)";
	else
		strLabel += " ( )";
	
	return strLabel;
}

void CGUIRadioButtonControl::UpdateColors()
{
	CGUIButtonControl::UpdateColors();
	m_imgRadioOn.SetDiffuseColor(m_diffuseColor);
	m_imgRadioOff.SetDiffuseColor(m_diffuseColor);
}