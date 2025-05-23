#include "SettingsControls.h"
#include "GUISettings.h"
#include "guilib\GUIImage.h"
#include "guilib\LocalizeStrings.h"
#include "utils\Util.h"
#include "guilib\dialogs\GUIDialogOK.h"

CBaseSettingControl::CBaseSettingControl(int id, CSetting *pSetting)
{
	m_id = id;
	m_pSetting = pSetting;
	m_delayed = false;
}

CRadioButtonSettingControl::CRadioButtonSettingControl(CGUIRadioButtonControl *pRadioButton, int id, CSetting *pSetting)
	: CBaseSettingControl(id, pSetting)
{
	m_pRadioButton = pRadioButton;
	m_pRadioButton->SetID(id);
	Update();
}

CRadioButtonSettingControl::~CRadioButtonSettingControl()
{
}

bool CRadioButtonSettingControl::OnClick()
{
	((CSettingBool *)m_pSetting)->SetData(!((CSettingBool *)m_pSetting)->GetData());
	return true;
}

void CRadioButtonSettingControl::Update()
{
	m_pRadioButton->SetSelected(((CSettingBool *)m_pSetting)->GetData());
}

CSpinExSettingControl::CSpinExSettingControl(CGUISpinControlEx *pSpin, int id, CSetting *pSetting)
	: CBaseSettingControl(id, pSetting)
{
	m_pSpin = pSpin;
	m_pSpin->SetID(id);
	
	if (pSetting->GetControlType() == SPIN_CONTROL_FLOAT)
	{
		CSettingFloat *pSettingFloat = (CSettingFloat *)pSetting;
		m_pSpin->SetType(SPIN_CONTROL_TYPE_FLOAT);
		m_pSpin->SetFloatRange(pSettingFloat->m_fMin, pSettingFloat->m_fMax);
		m_pSpin->SetFloatInterval(pSettingFloat->m_fStep);
	}
	else if (pSetting->GetControlType() == SPIN_CONTROL_INT_PLUS || pSetting->GetControlType() == SPIN_CONTROL_INT)
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

bool CSpinExSettingControl::OnClick()
{
	// TODO: Should really check for a change here (as end of spincontrols may
	//       cause no change)
	if (m_pSetting->GetControlType() == SPIN_CONTROL_FLOAT)
		((CSettingFloat *)m_pSetting)->SetData(m_pSpin->GetFloatValue());
	else
	{
		if (m_pSetting->GetType() != SETTINGS_TYPE_STRING)
		{
			CSettingInt *pSettingInt = (CSettingInt *)m_pSetting;
			pSettingInt->SetData(m_pSpin->GetValue());
		}
	}
	return true;
}

void CSpinExSettingControl::Update()
{
	if (m_pSetting->GetControlType() == SPIN_CONTROL_FLOAT)
	{
		CSettingFloat *pSettingFloat = (CSettingFloat *)m_pSetting;
		m_pSpin->SetFloatValue(pSettingFloat->GetData());
	}
	else if (m_pSetting->GetControlType() == SPIN_CONTROL_INT_PLUS || m_pSetting->GetControlType() == SPIN_CONTROL_INT)
	{
		CSettingInt *pSettingInt = (CSettingInt *)m_pSetting;
		m_pSpin->SetValue(pSettingInt->GetData());
	}
}

CButtonSettingControl::CButtonSettingControl(CGUIButtonControl *pButton, int id, CSetting *pSetting)
	: CBaseSettingControl(id, pSetting)
{
	m_pButton = pButton;
	m_pButton->SetID(id);
	Update();
}

CButtonSettingControl::~CButtonSettingControl()
{
}

bool CButtonSettingControl::OnClick()
{
	// This is pretty much a no-op as all click action is done in the calling class
	Update();
	return true;
}

void CButtonSettingControl::Update()
{
	 if (m_pSetting->GetControlType() == BUTTON_CONTROL_STANDARD)
		return ;

	CStdString strText = ((CSettingString *)m_pSetting)->GetData();

	if (m_pSetting->GetControlType() == BUTTON_CONTROL_PATH_INPUT)
	{
		CStdString shortPath;

		if (CUtil::MakeShortenPath(strText, shortPath, 30 ))
			strText = shortPath;
	}
	m_pButton->SetLabel2(strText);
}

CEditSettingControl::CEditSettingControl(CGUIEditControl *pEdit, int id, CSetting *pSetting)
	: CBaseSettingControl(id, pSetting)
{
	m_needsUpdate = false;
	m_pEdit = pEdit;
	m_pEdit->SetID(id);
	int heading = ((CSettingString *)m_pSetting)->m_iHeadingString;
	
	if (heading < 0) heading = 0;
	
	if (pSetting->GetControlType() == EDIT_CONTROL_HIDDEN_INPUT)
		m_pEdit->SetInputType(CGUIEditControl::INPUT_TYPE_PASSWORD, heading);
	else if (pSetting->GetControlType() == EDIT_CONTROL_MD5_INPUT)
		m_pEdit->SetInputType(CGUIEditControl::INPUT_TYPE_PASSWORD_MD5, heading);    
	else if (pSetting->GetControlType() == EDIT_CONTROL_IP_INPUT)
		m_pEdit->SetInputType(CGUIEditControl::INPUT_TYPE_IPADDRESS, heading);
	else if (pSetting->GetControlType() == EDIT_CONTROL_NUMBER_INPUT)
		m_pEdit->SetInputType(CGUIEditControl::INPUT_TYPE_NUMBER, heading);
	else
		m_pEdit->SetInputType(CGUIEditControl::INPUT_TYPE_TEXT, heading);
	
	Update();
}

CEditSettingControl::~CEditSettingControl()
{
}

bool CEditSettingControl::OnClick()
{
	// Update our string
	((CSettingString *)m_pSetting)->SetData(m_pEdit->GetLabel2());
	
	// We update on exit only
	m_needsUpdate = true;
	return false;
}

void CEditSettingControl::Update()
{
	if (!m_needsUpdate)
		m_pEdit->SetLabel2(((CSettingString *)m_pSetting)->GetData());
}

bool CEditSettingControl::IsValidIPAddress(const CStdString &strIP)
{
	const char* s = strIP.c_str();
	bool legalFormat = true;
	bool numSet = false;
	int num = 0;
	int dots = 0;

	while (*s != '\0')
	{
		if (*s == '.')
		{
			++dots;

			// There must be a number before a .
			if (!numSet)
			{
				legalFormat = false;
				break;
			}

			if (num > 255)
			{
				legalFormat = false;
				break;
			}

			num = 0;
			numSet = false;
		}
		else if (*s >= '0' && *s <= '9')
		{
			num = (num * 10) + (*s - '0');
			numSet = true;
		}
		else
		{
			legalFormat = false;
			break;
		}

		++s;
	}

	if (legalFormat)
	{
		if (!numSet)
		{
			legalFormat = false;
		}

		if (num > 255 || dots != 3)
		{
			legalFormat = false;
		}
	}

	if (!legalFormat)
		CGUIDialogOK::ShowAndGetInput(257, 724, 725, 0); // TODO

	return legalFormat;
}

CSeparatorSettingControl::CSeparatorSettingControl(CGUIImage *pImage, int id, CSetting *pSetting)
	: CBaseSettingControl(id, pSetting)
{
	m_pImage = pImage;
	m_pImage->SetID(id);
}

CSeparatorSettingControl::~CSeparatorSettingControl()
{
}