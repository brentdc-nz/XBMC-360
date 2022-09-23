#include "SettingsControls.h"
#include "guilib\LocalizeStrings.h"

CBaseSettingControl::CBaseSettingControl(DWORD dwID, CSetting *pSetting)
{
	m_dwID = dwID;
	m_pSetting = pSetting;
}

CSpinExSettingControl::CSpinExSettingControl(CGUISpinControlEx *pSpin, DWORD dwID, CSetting *pSetting)
    : CBaseSettingControl(dwID, pSetting)
{
	m_pSpin = pSpin;
	m_pSpin->SetID(dwID);
/*	if (pSetting->GetControlType() == SPIN_CONTROL_FLOAT)
	{
		CSettingFloat *pSettingFloat = (CSettingFloat *)pSetting;
		m_pSpin->SetType(SPIN_CONTROL_TYPE_FLOAT);
		m_pSpin->SetFloatRange(pSettingFloat->m_fMin, pSettingFloat->m_fMax);
		m_pSpin->SetFloatInterval(pSettingFloat->m_fStep);
	}
	else
*/ if (pSetting->GetControlType() == SPIN_CONTROL_INT_PLUS /*|| pSetting->GetControlType() == SPIN_CONTROL_INT*/)
	{
		CSettingInt *pSettingInt = (CSettingInt *)pSetting;
		m_pSpin->SetType(SPIN_CONTROL_TYPE_TEXT);
		m_pSpin->Clear();
		CStdString strLabel;
		int i = pSettingInt->m_iMin;
		if (pSettingInt->m_iLabelMin>-1)
		{
			strLabel=g_localizeStrings.Get(pSettingInt->m_iLabelMin);
			m_pSpin->AddLabel(strLabel, pSettingInt->m_iMin);
			i += pSettingInt->m_iStep;
		}
		for (; i <= pSettingInt->m_iMax; i += pSettingInt->m_iStep)
		{
			if (pSettingInt->m_iFormat > -1)
			{
				CStdString strFormat = g_localizeStrings.Get(pSettingInt->m_iFormat);
				strLabel.Format(strFormat, i);
			}
			else
				strLabel.Format(pSettingInt->m_strFormat, i);
			m_pSpin->AddLabel(strLabel, i);
		}
	}
	else // if (pSetting->GetControlType() == SPIN_CONTROL_TEXT)
	{
		m_pSpin->SetType(SPIN_CONTROL_TYPE_TEXT);
		m_pSpin->Clear();
	}
	Update();
}

CSpinExSettingControl::~CSpinExSettingControl()
{
}

void CSpinExSettingControl::OnClick()
{
/*	if (m_pSetting->GetControlType() == SPIN_CONTROL_FLOAT) //TODO
		((CSettingFloat *)m_pSetting)->SetData(m_pSpin->GetFloatValue());
	else
*/	{
//		if (m_pSetting->GetType() != SETTINGS_TYPE_STRING) //TODO
		{
			CSettingInt *pSettingInt = (CSettingInt *)m_pSetting;
			pSettingInt->SetData(m_pSpin->GetValue());
		}
	}
}

void CSpinExSettingControl::Update()
{
/*	if (m_pSetting->GetControlType() == SPIN_CONTROL_FLOAT)
	{
		CSettingFloat *pSettingFloat = (CSettingFloat *)m_pSetting;
		m_pSpin->SetFloatValue(pSettingFloat->GetData());
	}
	else*/ if (m_pSetting->GetControlType() == SPIN_CONTROL_INT_PLUS/* || m_pSetting->GetControlType() == SPIN_CONTROL_INT*/)
	{
		CSettingInt *pSettingInt = (CSettingInt *)m_pSetting;
		m_pSpin->SetValue(pSettingInt->GetData());
	}
}