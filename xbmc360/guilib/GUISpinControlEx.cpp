#include "GUISpinControlEx.h"

CGUISpinControlEx::CGUISpinControlEx(int parentID, int controlID, float posX, float posY, float width, float height, float spinWidth, float spinHeight, const CLabelInfo& spinInfo, const CTextureInfo &textureFocus, const CTextureInfo &textureNoFocus, const CTextureInfo& textureUp, const CTextureInfo& textureDown, const CTextureInfo& textureUpFocus, const CTextureInfo& textureDownFocus, const CLabelInfo& labelInfo, int iType)
    : CGUISpinControl(parentID, controlID, posX, posY, spinWidth, spinHeight, textureUp, textureDown, textureUpFocus, textureDownFocus, spinInfo, iType)
    , m_buttonControl(parentID, controlID, posX, posY, width, height, textureFocus, textureNoFocus, labelInfo),
	m_focus(posX, posY, width+35, height, textureFocus) //HACK - FIXME
{
	ControlType = GUICONTROL_SPINEX;
	m_spinPosX = 0;
}

CGUISpinControlEx::~CGUISpinControlEx(void)
{
}

void CGUISpinControlEx::Render()
{
	if (!IsVisible()) return;

	// make sure the button has focus if it should have...
	m_buttonControl.SetFocus(HasFocus());
	m_buttonControl.Render();

	if(HasFocus())
		m_focus.Render(); //HACK - Remove me and use button class

	CGUISpinControl::Render();
}

void CGUISpinControlEx::SetPosition(float posX, float posY)
{
	m_buttonControl.SetPosition(posX, posY);

	int iFocusHeight = 10; //TODO Calculate percent of button hight!
	int iFocusPosX = 15; //TODO Calc with percent
	m_focus.Update((int)posX - iFocusPosX, (int)posY - iFocusHeight);

	float spinPosX = posX + m_buttonControl.GetWidth() - GetSpinWidth() * 2 - 0/*(m_spinPosX ? m_spinPosX : m_buttonControl.GetLabelInfo().offsetX)*/;
	float spinPosY = posY; //+ (m_buttonControl.GetHeight() - GetSpinHeight()) * 0.5f;
	
	CGUISpinControl::SetPosition(spinPosX, spinPosY);
}

void CGUISpinControlEx::AllocResources()
{
	CGUISpinControl::AllocResources();
	m_buttonControl.AllocResources();

	m_focus.SetVisible(true); //HACK - Remove me and use button class
	m_focus.AllocResources();

	SetPosition((float)GetXPosition(), (float)GetYPosition());
}

void CGUISpinControlEx::FreeResources()
{
	CGUISpinControl::FreeResources();
	m_buttonControl.FreeResources();

//	m_focus.FreeResources();
}

const CStdString CGUISpinControlEx::GetCurrentLabel() const
{
	return CGUISpinControl::GetLabel();
}