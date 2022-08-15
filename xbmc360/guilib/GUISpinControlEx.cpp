#include "GUISpinControlEx.h"
#include "utils\Log.h"
CGUISpinControlEx::CGUISpinControlEx(int parentID, int controlID, float posX, float posY, float width, float height, float spinWidth, float spinHeight, const CLabelInfo& spinInfo, const CTextureInfo &textureFocus, const CTextureInfo &textureNoFocus, const CTextureInfo& textureUp, const CTextureInfo& textureDown, const CTextureInfo& textureUpFocus, const CTextureInfo& textureDownFocus, const CLabelInfo& labelInfo, int iType)
: CGUISpinControl(parentID, controlID, posX, posY, spinWidth, spinHeight, textureUp, textureDown, textureUpFocus, textureDownFocus, spinInfo, iType)
,m_buttonControl(parentID, controlID, posX, posY, width, height, textureNoFocus, textureNoFocus, labelInfo) // TODO: Remove null texture
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

	// Make sure the button has focus if it should have...
	m_buttonControl.SetFocus(HasFocus());
	m_buttonControl.Render();

	if(HasFocus())
		m_buttonControl.Render();

	CGUISpinControl::Render();
}

void CGUISpinControlEx::SetPosition(float posX, float posY)
{
	int iFocusHeight = 10; 
	int iFocusWidth = 10;  

	m_buttonControl.SetPosition(posX - iFocusWidth, posY - iFocusHeight);
	
	//TODO + NOTES: Use CLog to calculate focus width and height with porcentage(WIP).
	//For now use the integer number for results
	CLog::Log(LOGNOTICE, "Width Position: %d", posX - iFocusWidth, 
	"Height Position: %d", posY - iFocusHeight);

	float spinPosX = posX + m_buttonControl.GetWidth() - GetSpinWidth() * 2 - 0/*(m_spinPosX ? m_spinPosX : m_buttonControl.GetLabelInfo().offsetX)*/;
	float spinPosY = posY; //+ (m_buttonControl.GetHeight() - GetSpinHeight()) * 0.5f;
	CGUISpinControl::SetPosition(spinPosX, spinPosY);
}

void CGUISpinControlEx::AllocResources()
{
	CGUISpinControl::AllocResources();
	m_buttonControl.AllocResources();

	m_buttonControl.SetVisible(true);
	m_buttonControl.AllocResources();

	CLog::Log(LOGNOTICE, "Alloc resources");
	
	SetPosition((float)GetXPosition(), (float)GetYPosition());
}

void CGUISpinControlEx::FreeResources()
{
	CGUISpinControl::FreeResources();
	m_buttonControl.FreeResources();
	CLog::Log(LOGNOTICE, "Freeing resources");
}

const CStdString CGUISpinControlEx::GetCurrentLabel() const
{
	return CGUISpinControl::GetLabel();
}
