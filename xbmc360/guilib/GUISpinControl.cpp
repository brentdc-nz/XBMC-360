#include "GUISpinControl.h"
#include "GUIWindowManager.h"

CGUISpinControl::CGUISpinControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& textureUp, const CTextureInfo& textureDown, const CTextureInfo& textureUpFocus, const CTextureInfo& textureDownFocus, const CLabelInfo &labelInfo, int iType)
    : CGUIControl(parentID, controlID, posX, posY, width, height)
    , m_imgspinUp(parentID, controlID, posX, posY, width, height, textureUp)
    , m_imgspinDown(parentID, controlID, posX, posY, width, height, textureDown)
    , m_imgspinUpFocus(parentID, controlID, posX, posY, width, height, textureUpFocus)
    , m_imgspinDownFocus(parentID, controlID, posX, posY, width, height, textureDownFocus)
{
	m_label = labelInfo;
	ControlType = GUICONTROL_SPIN;
	m_iSelect = SPIN_BUTTON_DOWN;
	m_iType = iType;

	m_iValue = 0;
}

CGUISpinControl::~CGUISpinControl(void)
{
}

void CGUISpinControl::Render()
{
	if (!IsVisible()) return;

	CStdString text;

	if (m_iValue >= 0 && m_iValue < (int)m_vecLabels.size() )
	{
		if (/*m_bShowRange*/1) //TODO
		{
			text.Format("(%i/%i) %s", m_iValue + 1, (int)m_vecLabels.size(), CStdString(m_vecLabels[m_iValue]).c_str() );
		}
		else
		{
			text.Format("%s", CStdString(m_vecLabels[m_iValue]).c_str() );
		}
	}
    else text.Format("?%i?", m_iValue);

	if ( HasFocus() )
	{
		if ( m_iSelect == SPIN_BUTTON_UP )
			m_imgspinUpFocus.Render();
		else
			m_imgspinUp.Render();

		if ( m_iSelect == SPIN_BUTTON_DOWN )
			m_imgspinDownFocus.Render();
		else
			m_imgspinDown.Render();
	}
	else
	{
		m_imgspinUp.Render();
		m_imgspinDown.Render();
	}

	if (m_label.font)
	{
		float fPosY;
		if (/*m_label.align & XBFONT_CENTER_Y*/0)
		{
			fPosY = (float)m_posY + m_height * 0.5f;
		}
		else
		{
			fPosY = (float)(m_posY + m_label.offsetY);
		}

		float fPosX = (float)(m_posX + m_label.offsetX) - 3;
		if ( /*!IsDisabled()*/1 /*HasFocus()*/ ) //TODO
		{
			m_label.font->DrawText(fPosX, fPosY, m_label.dwTextColor, text.c_str(), m_label.dwAlign);
		}
		else
		{
			m_label.font->DrawText(fPosX, fPosY, m_label.dwTextColor, text.c_str(), m_label.dwAlign);
		}
	}

	CGUIControl::Render();
}

void CGUISpinControl::AllocResources()
{
	CGUIControl::AllocResources();

	// Correct alignment - We always align the spincontrol on the right.
	m_label.dwAlign = XUI_FONT_STYLE_RIGHT_ALIGN;

	m_imgspinUp.AllocResources();
	m_imgspinUpFocus.AllocResources();
	m_imgspinDown.AllocResources();
	m_imgspinDownFocus.AllocResources();

	m_imgspinDownFocus.SetPosition(m_posX, m_posY);
	m_imgspinDown.SetPosition(m_posX, m_posY);
	m_imgspinUp.SetPosition(m_posX + m_imgspinDown.GetWidth(), m_posY);
	m_imgspinUpFocus.SetPosition(m_posX + m_imgspinDownFocus.GetWidth(), m_posY);
}

void CGUISpinControl::FreeResources()
{
	CGUIControl::FreeResources();

//	m_imgspinUp.FreeResources(); //FIXME 
//	m_imgspinUpFocus.FreeResources();
//	m_imgspinDown.FreeResources();
//	m_imgspinDownFocus.FreeResources();
}

void CGUISpinControl::SetPosition(float posX, float posY)
{
	m_imgspinDownFocus.SetPosition(posX, posY);
	m_imgspinDown.SetPosition(posX, posY);

	m_imgspinUp.SetPosition(m_posX + m_imgspinDown.GetWidth(), m_posY);
	m_imgspinUpFocus.SetPosition(m_posX + m_imgspinDownFocus.GetWidth(), m_posY);\

	CGUIControl::SetPosition(posX, posY);
}

bool CGUISpinControl::OnAction(const CAction &action)
{
	switch (action.wID)
	{
		case ACTION_SELECT_ITEM:
		if (m_iSelect == SPIN_BUTTON_UP)
		{
			MoveUp();
			return true;
		}
		if (m_iSelect == SPIN_BUTTON_DOWN)
		{
			MoveDown();
			return true;
		}
		break;
	}

	return CGUIControl::OnAction(action);
}

void CGUISpinControl::OnLeft()
{
	if (m_iSelect == SPIN_BUTTON_UP)
	{
		// select the down button
		m_iSelect = SPIN_BUTTON_DOWN;
	}
	else
	{ 
		// base class
		CGUIControl::OnLeft();
	}
}

void CGUISpinControl::OnRight()
{
	if (m_iSelect == SPIN_BUTTON_DOWN)
	{
		// select the up button
		m_iSelect = SPIN_BUTTON_UP;
	}
	else
	{
		// base class
		CGUIControl::OnRight();
	}
}

void CGUISpinControl::AddLabel(const string& strLabel, int iValue)
{
	m_vecLabels.push_back(strLabel);
	m_vecValues.push_back(iValue);
}

const string CGUISpinControl::GetLabel() const
{
	if (m_iValue >= 0 && m_iValue < (int)m_vecLabels.size())
	{
		return m_vecLabels[ m_iValue];
	}
	return "";
}

void CGUISpinControl::Clear()
{
	m_vecLabels.erase(m_vecLabels.begin(), m_vecLabels.end());
	m_vecValues.erase(m_vecValues.begin(), m_vecValues.end());
	SetValue(0);
}

void CGUISpinControl::SetValue(int iValue)
{  
	if (m_iType == SPIN_CONTROL_TYPE_TEXT)
	{
		m_iValue = -1;
		for (unsigned int i = 0; i < m_vecValues.size(); i++)
		if (iValue == m_vecValues[i])
			m_iValue = i;
	}
	else
		m_iValue = iValue;
}

void CGUISpinControl::SetType(int iType)
{
	m_iType = iType;
}

void CGUISpinControl::MoveUp(bool bTestReverse)
{
/*
	if (bTestReverse && m_bReverse)  //TODO
	{
		// actually should move down.
		MoveDown(false);
		return ;
	}
*/
	switch (m_iType)
	{
		case SPIN_CONTROL_TYPE_TEXT:
		{
			if (m_iValue - 1 >= 0)
				m_iValue--;
			else if (m_iValue == 0)
				m_iValue = (int)m_vecLabels.size() - 1;
		
			CGUIMessage msg(GUI_MSG_CLICKED, GetID(), GetParentID());
			g_windowManager.SendMessage(msg);
			return ;
		}
		break;
	}
}

void CGUISpinControl::MoveDown(bool bTestReverse)
{
/*
	if (bTestReverse && m_bReverse)  //TODO
	{ 
		// actually should move up.
		MoveUp(false);
		return ;
	}
*/
	switch (m_iType)
	{
		case SPIN_CONTROL_TYPE_TEXT:
		{
			if (m_iValue + 1 < (int)m_vecLabels.size() )
				m_iValue++;
			else if (m_iValue == (int)m_vecLabels.size() - 1)
			m_iValue = 0;
			CGUIMessage msg(GUI_MSG_CLICKED, GetID(), GetParentID());
			g_windowManager.SendMessage(msg);
			return ;
		}
		break;
	}
}