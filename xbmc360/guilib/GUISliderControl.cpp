#include "include.h"
#include "GUISliderControl.h"
#include "GUIInfoManager.h"
#include "utils\MathUtils.h"

CGUISliderControl::CGUISliderControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& backGroundTexture, const CTextureInfo& nibTexture, const CTextureInfo& nibTextureFocus, int iType)
	: CGUIControl(parentID, controlID, posX, posY, width, height)
	, m_guiBackground(posX, posY, width, height, backGroundTexture)
	, m_guiMid(posX, posY, width, height, nibTexture)
	, m_guiMidFocus(posX, posY, width, height, nibTextureFocus)
{
	m_iType = iType;
	m_iPercent = 0;
	m_iStart = 0;
	m_iEnd = 100;
	m_fStart = 0.0f;
	m_fEnd = 1.0f;
	m_fInterval = 0.1f;
	m_iValue = 0;
	m_fValue = 0.0;
	ControlType = GUICONTROL_SLIDER;
	m_iInfoCode = 0;
}

CGUISliderControl::~CGUISliderControl(void)
{
}

void CGUISliderControl::Render()
{
	m_guiBackground.SetPosition( m_posX, m_posY );
	if (!IsDisabled())
	{
		if (m_iInfoCode)
			SetIntValue(g_infoManager.GetInt(m_iInfoCode));

		float fScaleX = m_width == 0 ? 1.0f : m_width / m_guiBackground.GetTextureWidth();
		float fScaleY = m_height == 0 ? 1.0f : m_height / m_guiBackground.GetTextureHeight();

		m_guiBackground.SetHeight(m_height);
		m_guiBackground.SetWidth(m_width);
		m_guiBackground.Render();

		float fWidth = (m_guiBackground.GetTextureWidth() - m_guiMid.GetTextureWidth())*fScaleX;

		float fPos = m_guiBackground.GetXPosition() + GetProportion() * fWidth;

		if ((int)fWidth > 1)
		{
			if (m_bHasFocus)
			{
				m_guiMidFocus.SetPosition(fPos, m_guiBackground.GetYPosition() );
				m_guiMidFocus.SetWidth(m_guiMidFocus.GetTextureWidth() * fScaleX);
				m_guiMidFocus.SetHeight(m_guiMidFocus.GetTextureHeight() * fScaleY);
				m_guiMidFocus.Render();
			}
			else
			{
				m_guiMid.SetPosition(fPos, m_guiBackground.GetYPosition() );
				m_guiMid.SetWidth(m_guiMid.GetTextureWidth()*fScaleX);
				m_guiMid.SetHeight(m_guiMid.GetTextureHeight()*fScaleY);
				m_guiMid.Render();
			}
		}
	}
	CGUIControl::Render();
}

bool CGUISliderControl::OnMessage(CGUIMessage& message)
{
	if (message.GetControlId() == GetID() )
	{
		switch (message.GetMessage())
		{
			case GUI_MSG_ITEM_SELECT:
				SetPercentage( message.GetParam1() );
				return true;
			break;

			case GUI_MSG_LABEL_RESET:
			{
				SetPercentage(0);
				return true;
			}
			break;
		}
	}
	return CGUIControl::OnMessage(message);
}

bool CGUISliderControl::OnAction(const CAction &action)
{
	switch ( action.GetID() )
	{
		case ACTION_MOVE_LEFT:
			Move( -1);
			return true;
		break;

		case ACTION_MOVE_RIGHT:
			Move(1);
			return true;
		break;
		default:
			return CGUIControl::OnAction(action);
	}
}

void CGUISliderControl::Move(int iNumSteps)
{
	switch (m_iType)
	{
		case SPIN_CONTROL_TYPE_FLOAT:
			m_fValue += m_fInterval * iNumSteps;
			if (m_fValue < m_fStart) m_fValue = m_fStart;
			if (m_fValue > m_fEnd) m_fValue = m_fEnd;
		break;

		case SPIN_CONTROL_TYPE_INT:
			m_iValue += iNumSteps;
			if (m_iValue < m_iStart) m_iValue = m_iStart;
			if (m_iValue > m_iEnd) m_iValue = m_iEnd;
		break;

		default:
			m_iPercent += iNumSteps;
			if (m_iPercent < 0) m_iPercent = 0;
			if (m_iPercent > 100) m_iPercent = 100;
		break;
	}
	SEND_CLICK_MESSAGE(GetID(), GetParentID(), MathUtils::round_int(100*GetProportion()));
}

void CGUISliderControl::SetPercentage(int iPercent)
{
	if (iPercent > 100) iPercent = 100;
	if (iPercent < 0) iPercent = 0;
	m_iPercent = iPercent;
}

int CGUISliderControl::GetPercentage() const
{
	return m_iPercent;
}

void CGUISliderControl::SetIntValue(int iValue)
{
	if (m_iType == SPIN_CONTROL_TYPE_FLOAT)
		m_fValue = (float)iValue;
	else if (m_iType == SPIN_CONTROL_TYPE_INT)
		m_iValue = iValue;
	else
		SetPercentage(iValue);
}

int CGUISliderControl::GetIntValue() const
{
	if (m_iType == SPIN_CONTROL_TYPE_FLOAT)
		return (int)m_fValue;
	else if (m_iType == SPIN_CONTROL_TYPE_INT)
		return m_iValue;
	else
		return m_iPercent;
}

void CGUISliderControl::SetFloatValue(float fValue)
{
	if (m_iType == SPIN_CONTROL_TYPE_FLOAT)
		m_fValue = fValue;
	else if (m_iType == SPIN_CONTROL_TYPE_INT)
		m_iValue = (int)fValue;
	else
		SetPercentage((int)fValue);
}

float CGUISliderControl::GetFloatValue() const
{
	if (m_iType == SPIN_CONTROL_TYPE_FLOAT)
		return m_fValue;
	else if (m_iType == SPIN_CONTROL_TYPE_INT)
		return (float)m_iValue;
	else
		return (float)m_iPercent;
}

void CGUISliderControl::SetFloatInterval(float fInterval)
{
	m_fInterval = fInterval;
}

void CGUISliderControl::SetRange(int iStart, int iEnd)
{
	if (m_iType == SPIN_CONTROL_TYPE_FLOAT)
		SetFloatRange((float)iStart,(float)iEnd);
	else
	{
		m_iStart = iStart;
		m_iEnd = iEnd;
	}
}

void CGUISliderControl::SetFloatRange(float fStart, float fEnd)
{
	if (m_iType == SPIN_CONTROL_TYPE_INT)
		SetRange((int)fStart, (int)fEnd);
	else
	{
		m_fStart = fStart;
		m_fEnd = fEnd;
	}
}

void CGUISliderControl::FreeResources(bool immediately)
{
	CGUIControl::FreeResources(immediately);

	m_guiBackground.FreeResources(immediately);
	m_guiMid.FreeResources(immediately);
	m_guiMidFocus.FreeResources(immediately);
}

void CGUISliderControl::DynamicResourceAlloc(bool bOnOff)
{
	CGUIControl::DynamicResourceAlloc(bOnOff);

	m_guiBackground.DynamicResourceAlloc(bOnOff);
	m_guiMid.DynamicResourceAlloc(bOnOff);
	m_guiMidFocus.DynamicResourceAlloc(bOnOff);
}

void CGUISliderControl::PreAllocResources()
{
	CGUIControl::PreAllocResources();

	m_guiBackground.PreAllocResources();
	m_guiMid.PreAllocResources();
	m_guiMidFocus.PreAllocResources();
}

void CGUISliderControl::AllocResources()
{
	CGUIControl::AllocResources();

	m_guiBackground.AllocResources();
	m_guiMid.AllocResources();
	m_guiMidFocus.AllocResources();
}

bool CGUISliderControl::HitTest(const CPoint &point) const
{
	if (m_guiBackground.HitTest(point)) return true;
	if (m_guiMid.HitTest(point)) return true;
	return false;
}

void CGUISliderControl::SetFromPosition(const CPoint &point)
{
	float fPercent = (point.x - m_guiBackground.GetXPosition()) / m_guiBackground.GetWidth();
	if (fPercent < 0) fPercent = 0;
	if (fPercent > 1) fPercent = 1;
	
	switch (m_iType)
	{
		case SPIN_CONTROL_TYPE_FLOAT:
			m_fValue = m_fStart + (m_fEnd - m_fStart) * fPercent;
		break;

		case SPIN_CONTROL_TYPE_INT:
			m_iValue = (int)(m_iStart + (float)(m_iEnd - m_iStart) * fPercent + 0.49f);
		break;

		default:
			m_iPercent = (int)(fPercent * 100 + 0.49f);
		break;
	}
	SEND_CLICK_MESSAGE(GetID(), GetParentID(), MathUtils::round_int(fPercent));
}

#ifdef _HAS_MOUSE
bool CGUISliderControl::OnMouseEvent(const CPoint &point, const CMouseEvent &event)
{
	if (event.m_id == ACTION_MOUSE_DRAG)
	{
		if (event.m_state == 1)
		{
			// Grab exclusive access
			CGUIMessage msg(GUI_MSG_EXCLUSIVE_MOUSE, GetID(), GetParentID());
			SendWindowMessage(msg);
		}
		else if (event.m_state == 3)
		{
			// Release exclusive access
			CGUIMessage msg(GUI_MSG_EXCLUSIVE_MOUSE, 0, GetParentID());
			SendWindowMessage(msg);
		}
		SetFromPosition(point);
		return true;
	}
	else if (event.m_id == ACTION_MOUSE_LEFT_CLICK && m_guiBackground.HitTest(point))
	{
		SetFromPosition(point);
		return true;
	}
	else if (event.m_id == ACTION_MOUSE_WHEEL_UP)
	{
		Move(10);
		return true;
	}
	else if (event.m_id == ACTION_MOUSE_WHEEL_DOWN)
	{
		Move(-10);
		return true;
	}
	return false;
}
#endif

void CGUISliderControl::SetInfo(int iInfo)
{
	m_iInfoCode = iInfo;
}

CStdString CGUISliderControl::GetDescription() const
{
	if (!m_textValue.IsEmpty())
		return m_textValue;
	
	CStdString description;
	
	if (m_iType == SPIN_CONTROL_TYPE_FLOAT)
		description.Format("%2.2f", m_fValue);
	else if (m_iType == SPIN_CONTROL_TYPE_INT)
		description.Format("%i", m_iValue);
	else
		description.Format("%i%%", m_iPercent);
	
	return description;
}

void CGUISliderControl::UpdateColors()
{
	CGUIControl::UpdateColors();

	m_guiBackground.SetDiffuseColor(m_diffuseColor);
	m_guiMid.SetDiffuseColor(m_diffuseColor);
	m_guiMidFocus.SetDiffuseColor(m_diffuseColor);
}

float CGUISliderControl::GetProportion() const
{
	if (m_iType == SPIN_CONTROL_TYPE_FLOAT)
		return (m_fValue - m_fStart) / (m_fEnd - m_fStart);
	else if (m_iType == SPIN_CONTROL_TYPE_INT)
		return (float)(m_iValue - m_iStart) / (float)(m_iEnd - m_iStart);
	
	return 0.01f * m_iPercent;
}