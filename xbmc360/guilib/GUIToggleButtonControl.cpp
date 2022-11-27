#include "include.h"
#include "GUIToggleButtonControl.h"
#include "GUIWindowManager.h"
#include "GUIDialog.h"
//#include "utils\CharsetConverter.h" // TODO
#include "GUIInfoManager.h"

using namespace std;

CGUIToggleButtonControl::CGUIToggleButtonControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus, const CTextureInfo& altTextureFocus, const CTextureInfo& altTextureNoFocus, const CLabelInfo &labelInfo)
	: CGUIButtonControl(parentID, controlID, posX, posY, width, height, textureFocus, textureNoFocus, labelInfo)
	, m_selectButton(parentID, controlID, posX, posY, width, height, altTextureFocus, altTextureNoFocus, labelInfo)
{
	m_toggleSelect = 0;
	ControlType = GUICONTROL_TOGGLEBUTTON;
}

CGUIToggleButtonControl::~CGUIToggleButtonControl(void)
{
}

void CGUIToggleButtonControl::Render() // TODO
{
	// Ask our infoManager whether we are selected or not...
	if (m_toggleSelect)
		m_bSelected = g_infoManager.GetBool(m_toggleSelect, m_parentID); // TODO

	if (m_bSelected)
	{
		// Render our Alternate textures...
		m_selectButton.SetFocus(HasFocus());
		m_selectButton.SetVisible(IsVisible());
		m_selectButton.SetEnabled(!IsDisabled());
		m_selectButton.SetPulseOnSelect(m_pulseOnSelect);
		m_selectButton.Render();
		CGUIControl::Render();
	}
	else
	{
		// Render our Normal textures...
		CGUIButtonControl::Render();
	}
}

bool CGUIToggleButtonControl::OnAction(const CAction &action)
{
	if (action.GetID() == ACTION_SELECT_ITEM)
	{
		m_bSelected = !m_bSelected;
	}

	return CGUIButtonControl::OnAction(action);
}

void CGUIToggleButtonControl::PreAllocResources()
{
	CGUIButtonControl::PreAllocResources();
	m_selectButton.PreAllocResources();
}

void CGUIToggleButtonControl::AllocResources()
{
	CGUIButtonControl::AllocResources();
	m_selectButton.AllocResources();
}

void CGUIToggleButtonControl::FreeResources(bool immediately)
{
	CGUIButtonControl::FreeResources(immediately);
	m_selectButton.FreeResources(immediately);
}

void CGUIToggleButtonControl::DynamicResourceAlloc(bool bOnOff)
{
	CGUIButtonControl::DynamicResourceAlloc(bOnOff);
	m_selectButton.DynamicResourceAlloc(bOnOff);
}

void CGUIToggleButtonControl::SetInvalid()
{
	CGUIButtonControl::SetInvalid();
	m_selectButton.SetInvalid();
}

void CGUIToggleButtonControl::SetPosition(float posX, float posY)
{
	CGUIButtonControl::SetPosition(posX, posY);
	m_selectButton.SetPosition(posX, posY);
}

void CGUIToggleButtonControl::SetWidth(float width)
{
	CGUIButtonControl::SetWidth(width);
	m_selectButton.SetWidth(width);
}

void CGUIToggleButtonControl::SetHeight(float height)
{
	CGUIButtonControl::SetHeight(height);
	m_selectButton.SetHeight(height);
}

void CGUIToggleButtonControl::UpdateColors()
{
	CGUIButtonControl::UpdateColors();
	m_selectButton.UpdateColors();
}

void CGUIToggleButtonControl::SetLabel(const string &strLabel)
{
	CGUIButtonControl::SetLabel(strLabel);
	m_selectButton.SetLabel(strLabel);
}

void CGUIToggleButtonControl::SetAltLabel(const string &label)
{
	if (label.size())
		m_selectButton.SetLabel(label);
}

CStdString CGUIToggleButtonControl::GetLabel() const
{
	if (m_bSelected)
		return m_selectButton.GetLabel();

	return CGUIButtonControl::GetLabel();
}

void CGUIToggleButtonControl::SetAltClickActions(const CGUIAction &clickActions)
{
	m_selectButton.SetClickActions(clickActions);
}

void CGUIToggleButtonControl::OnClick()
{
	// The ! is here as m_bSelected gets updated before this is called
	if (!m_bSelected && m_selectButton.HasClickActions())
		m_selectButton.OnClick();
	else
		CGUIButtonControl::OnClick();
}