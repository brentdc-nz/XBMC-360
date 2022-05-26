#include "GUIProgressControl.h"
#include "GUIInfoManager.h"

CGUIProgressControl::CGUIProgressControl(int parentID, int controlID, 
                                         float posX, float posY, float width, 
                                         float height, const CTextureInfo& backGroundTexture, 
                                         const CTextureInfo& midTexture, 
                                         float min, float max)
    : CGUIControl(parentID, controlID, posX, posY, width, height)
    , m_guiBackground(posX, posY, width, height, backGroundTexture)
    , m_guiMid(posX, posY, width, height, midTexture)
{
	ControlType = GUICONTROL_PROGRESS;
	m_RangeMin = min;
	m_RangeMax = max;
	m_fPercent = 0;
	m_iInfoCode = 0;
}

CGUIProgressControl::~CGUIProgressControl(void)
{
}

void CGUIProgressControl::AllocResources()
{
	CGUIControl::AllocResources();

	m_guiBackground.AllocResources();
	m_guiMid.AllocResources();
}

void CGUIProgressControl::FreeResources()
{
	CGUIControl::FreeResources();

	m_guiBackground.FreeResources();
	m_guiMid.FreeResources();
}

void CGUIProgressControl::Render()
{
	if(!IsDisabled())
	{
		if(m_iInfoCode)
		{
			m_fPercent = (float)g_infoManager.GetInt(m_iInfoCode);
			
			if((m_RangeMax - m_RangeMin)> 0 && (m_RangeMax != 100 && m_RangeMin != 0))
			{
				if(m_fPercent > m_RangeMax)
					m_fPercent = m_RangeMax;
				
				if(m_fPercent < m_RangeMin) 
					m_fPercent = m_RangeMin;
				
				m_fPercent = ((100*(m_fPercent - m_RangeMin)) / (m_RangeMax - m_RangeMin));
			}
		}

		if(m_fPercent < 0.0f)
			m_fPercent = 0.0f;

		if(m_fPercent > 100.0f)
			m_fPercent = 100.0f;

		if(m_width == 0)
			m_width = m_guiBackground.GetTextureWidth();

		if(m_height == 0)
			m_height = m_guiBackground.GetTextureHeight();

		m_guiBackground.SetHeight(m_height);
		m_guiBackground.SetWidth(m_width);

		m_guiBackground.Render();

		float width = m_fPercent * m_width * 0.01f;

		if(m_fPercent && width > 1)
		{
			m_guiMid.SetWidth(width);
			m_guiMid.Render();
		}
	}
	CGUIControl::Render();
}

void CGUIProgressControl::SetInfo(int iInfo)
{
	m_iInfoCode = iInfo;
}

bool CGUIProgressControl::CanFocus() const
{
	return false;
}