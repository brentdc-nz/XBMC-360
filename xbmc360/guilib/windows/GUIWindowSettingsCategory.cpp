#include "GUIWindowSettingsCategory.h"
#include "guilib\GUIControlGroupList.h"
#include "guilib\LocalizeStrings.h"
#include "guilib\GUIMessage.h"
#include "guilib\dialogs\GUIDialogKeyboard.h"
#include "SettingsControls.h" // TODO: Move to GUILIB?
#include "utils\Weather.h"
#include "guilib\GUIUserMessages.h"

#define CONTROL_SETTINGS_LABEL              2
#define CATEGORY_GROUP_ID                   3
#define SETTINGS_GROUP_ID                   5
#define CONTROL_DEFAULT_BUTTON              7
#define CONTROL_DEFAULT_RADIOBUTTON         8
#define CONTROL_DEFAULT_SPIN                9
#define CONTROL_DEFAULT_CATEGORY_BUTTON     10
#define CONTROL_DEFAULT_SEPARATOR           11
#define CONTROL_DEFAULT_EDIT                12
#define CONTROL_START_BUTTONS               -40
#define CONTROL_START_CONTROL               -20

CGUIWindowSettingsCategory::CGUIWindowSettingsCategory(void)
    : CGUIWindow(WINDOW_SETTINGS_MYPICTURES, "SettingsCategory.xml")
{
	m_pOriginalSpin = NULL;
	m_pOriginalRadioButton = NULL;
	m_pOriginalButton = NULL;
	m_pOriginalCategoryButton = NULL;
	m_pOriginalImage = NULL;
	m_pOriginalEdit = NULL;
	m_pOriginalImage = NULL;
	m_pOriginalEdit = NULL;

	m_iScreen = 0;
	SetIDRange(8); // Needed to cover all groups
}

CGUIWindowSettingsCategory::~CGUIWindowSettingsCategory(void)
{
	FreeControls();
	delete m_pOriginalEdit;
}

bool CGUIWindowSettingsCategory::OnBack(int actionID)
{
	g_settings.Save();
	m_lastControlID = 0; // Don't save the control as we go to a different window each time
	return CGUIWindow::OnBack(actionID);
}

void CGUIWindowSettingsCategory::OnInitWindow()
{
	SetupControls();
	CGUIWindow::OnInitWindow();
}

bool CGUIWindowSettingsCategory::OnMessage(CGUIMessage &message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_CLICKED:
		{
			unsigned int iControl = message.GetSenderId();

/*			if (iControl >= CONTROL_START_BUTTONS && iControl < CONTROL_START_BUTTONS + m_vecSections.size())
			{
				// Change the setting...
				m_iSection = iControl-CONTROL_START_BUTTONS;
				CheckNetworkSettings();
				CreateSettings();
				return true;
			}
*/
			for (unsigned int i = 0; i < m_vecSettings.size(); i++)
			{
				if (m_vecSettings[i]->GetID() == iControl)
					OnClick(m_vecSettings[i]);
			}
		}
		break;
		case GUI_MSG_FOCUSED:
		{
			CGUIWindow::OnMessage(message);
			int focusedControl = GetFocusedControlID();

			if (focusedControl >= CONTROL_START_BUTTONS && focusedControl < (int)(CONTROL_START_BUTTONS + m_vecSections.size()) &&
				focusedControl - CONTROL_START_BUTTONS != m_iSection)
			{
				// Changing section, check for updates and cancel any delayed changes
//				m_delayedSetting = NULL;
				CheckForUpdates();

				if (m_vecSections[focusedControl-CONTROL_START_BUTTONS]->m_strCategory == "masterlock")
				{
/*					if (!g_passwordManager.IsMasterLockUnlocked(true)) // TODO: No locking atm!
					{
						// Unable to go to this category - focus the previous one
						SET_CONTROL_FOCUS(CONTROL_START_BUTTONS + m_iSection, 0);
						return false;
					}
*/				}
				m_iSection = focusedControl - CONTROL_START_BUTTONS;
//				CheckNetworkSettings();
				CreateSettings();
			}
			return true;
		}
		case GUI_MSG_WINDOW_INIT:
		{
//			m_delayedSetting = NULL;
			if (message.GetParam1() != WINDOW_INVALID/* && !m_returningFromSkinLoad*/)
			{
				// Coming to this window first time (ie not returning back from some other window)
				// so we reset our section and control states
				m_iSection = 0;
				ResetControlStates();
			}
//			m_returningFromSkinLoad = false;
			m_iScreen = (int)message.GetParam2() - (int)CGUIWindow::GetID();
			return CGUIWindow::OnMessage(message);
		}
		break;
		case GUI_MSG_UPDATE_ITEM: // TODO
		{
/*			if (m_delayedSetting)
			{
				OnSettingChanged(m_delayedSetting);
				m_delayedSetting = NULL;
				return true;
			}
*/		}
		break;
		case GUI_MSG_WINDOW_DEINIT:
		{
//			m_delayedSetting = NULL; // TODO
			CheckForUpdates();
			CGUIWindow::OnMessage(message);
			FreeControls();
			return true;
		}
		break;
	}

	return CGUIWindow::OnMessage(message);
}

void CGUIWindowSettingsCategory::FrameMove()
{
	// Update realtime changeable stuff
//	UpdateRealTimeSettings(); //TODO

	CGUIWindow::FrameMove();
}

void CGUIWindowSettingsCategory::Render()
{
	// Update alpha status of current button
	bool bAlphaFaded = false;
	
	CGUIControl *control = GetFirstFocusableControl(CONTROL_START_BUTTONS + m_iSection);
	if (control && !control->HasFocus())
	{
		if (control->GetControlType() == CGUIControl::GUICONTROL_BUTTON)
		{
			control->SetFocus(true);
			((CGUIButtonControl *)control)->SetAlpha(0x80);
			bAlphaFaded = true;
		}
		else if (control->GetControlType() == CGUIControl::GUICONTROL_TOGGLEBUTTON)
		{
			control->SetFocus(true);
			((CGUIButtonControl *)control)->SetSelected(true);
			bAlphaFaded = true;
		}
	}

	CGUIWindow::Render();

	if (bAlphaFaded)
	{
		control->SetFocus(false);
		
		if (control->GetControlType() == CGUIControl::GUICONTROL_BUTTON)
			((CGUIButtonControl *)control)->SetAlpha(0xFF);
		else
			((CGUIButtonControl *)control)->SetSelected(false);
	}

	// Render the error message if necessary
/*
	if (m_strErrorMessage.size()) // TODO
	{
		CGUIFont *pFont = g_fontManager.GetFont("font13");
		float fPosY = g_graphicsContext.GetHeight() * 0.8f;
		float fPosX = g_graphicsContext.GetWidth() * 0.5f;
		CGUITextLayout::DrawText(pFont, fPosX, fPosY, 0xffffffff, 0, m_strErrorMessage, XBFONT_CENTER_X);
	}
*/
}

void CGUIWindowSettingsCategory::SetupControls()
{
	// Cleanup first, if necessary
	FreeControls();

	m_pOriginalSpin = (CGUISpinControlEx*)GetControl(CONTROL_DEFAULT_SPIN);
	m_pOriginalRadioButton = (CGUIRadioButtonControl *)GetControl(CONTROL_DEFAULT_RADIOBUTTON);
	m_pOriginalCategoryButton = (CGUIButtonControl *)GetControl(CONTROL_DEFAULT_CATEGORY_BUTTON);
	m_pOriginalButton = (CGUIButtonControl *)GetControl(CONTROL_DEFAULT_BUTTON);
	m_pOriginalImage = (CGUIImage *)GetControl(CONTROL_DEFAULT_SEPARATOR);

	if (!m_pOriginalCategoryButton || !m_pOriginalSpin || !m_pOriginalRadioButton || !m_pOriginalButton)
		return ;

	m_pOriginalEdit = (CGUIEditControl *)GetControl(CONTROL_DEFAULT_EDIT);
	
	if (!m_pOriginalEdit || m_pOriginalEdit->GetControlType() != CGUIControl::GUICONTROL_EDIT)
	{
		delete m_pOriginalEdit;
		m_pOriginalEdit = new CGUIEditControl(*m_pOriginalButton);
	}

	// Now hide them
	m_pOriginalSpin->SetVisible(false);
	m_pOriginalRadioButton->SetVisible(false);
	m_pOriginalButton->SetVisible(false);
	m_pOriginalCategoryButton->SetVisible(false);
	m_pOriginalEdit->SetVisible(false);
	if (m_pOriginalImage) m_pOriginalImage->SetVisible(false);

	CGUIControlGroupList *group = (CGUIControlGroupList *)GetControl(CATEGORY_GROUP_ID);
	if (!group)
		return;

	// Get a list of different sections
	CSettingsGroup *pSettingsGroup = g_guiSettings.GetGroup(m_iScreen);
	if (!pSettingsGroup)
		return;

	// Update the screen string
	SET_CONTROL_LABEL(CONTROL_SETTINGS_LABEL, pSettingsGroup->GetLabelID());

	// Get the categories we need
	pSettingsGroup->GetCategories(m_vecSections);
	
	// Run through and create our buttons...
	for (unsigned int j = 0, i = 0; i < m_vecSections.size(); i++)
	{
//		if (m_vecSections[i]->m_labelID == 12360 && !g_settings.IsMasterUser()) // TODO : No locking atm!
//			continue;

		CGUIButtonControl *pButton = NULL;

		if (m_pOriginalCategoryButton->GetControlType() == CGUIControl::GUICONTROL_TOGGLEBUTTON)
			pButton = new CGUIToggleButtonControl(*(CGUIToggleButtonControl *)m_pOriginalCategoryButton);
		else
			pButton = new CGUIButtonControl(*m_pOriginalCategoryButton);

		pButton->SetLabel(g_localizeStrings.Get(m_vecSections[i]->m_labelID));
		pButton->SetID(CONTROL_START_BUTTONS + j);
		pButton->SetVisible(true);
		pButton->AllocResources();
		group->AddControl(pButton);
		j++;
	}

	if (m_iSection < 0 || m_iSection >= (int)m_vecSections.size())
		m_iSection = 0;

	CreateSettings();
	
	// Set focus correctly
	m_defaultControl = CONTROL_START_BUTTONS;
}

void CGUIWindowSettingsCategory::FreeControls()
{
	// Clear the category group
	CGUIControlGroupList *control = (CGUIControlGroupList *)GetControl(CATEGORY_GROUP_ID);

	if (control)
	{
		control->FreeResources();
		control->ClearAll();
	}
}

void CGUIWindowSettingsCategory::CreateSettings()
{
	FreeSettingsControls();

	CGUIControlGroupList *group = (CGUIControlGroupList *)GetControl(SETTINGS_GROUP_ID);
	if (!group)
		return;

	vecSettings settings;

	g_guiSettings.GetSettingsGroup(m_vecSections[m_iSection]->m_strCategory, settings);
	int iControlID = CONTROL_START_CONTROL;

	for (unsigned int i = 0; i < settings.size(); i++)
	{
		CSetting *pSetting = settings[i];
		AddSetting(pSetting, group->GetWidth(), iControlID);
		CStdString strSetting = pSetting->GetSetting();
    }

	// Update our settings (turns controls on/off as appropriate)
	UpdateSettings();
}

void CGUIWindowSettingsCategory::CheckForUpdates()
{
	for (unsigned int i = 0; i < m_vecSettings.size(); i++)
	{
		CBaseSettingControl *pSettingControl = m_vecSettings[i];
		
		if (pSettingControl->NeedsUpdate())
		{
			OnSettingChanged(pSettingControl);
			pSettingControl->Reset();
		}
	}
}

void CGUIWindowSettingsCategory::UpdateSettings()
{
	for (unsigned int i = 0; i < m_vecSettings.size(); i++)
	{
		CBaseSettingControl *pSettingControl = m_vecSettings[i];
		pSettingControl->Update();
		CStdString strSetting = pSettingControl->GetSetting()->GetSetting();

		if (strSetting.Left(16).Equals("weather.location"))
		{
			CSettingString *pSetting = (CSettingString *)GetSetting(strSetting)->GetSetting();
			CGUIButtonControl *pControl = (CGUIButtonControl *)GetControl(GetSetting(strSetting)->GetID());
			pControl->SetLabel2(g_weatherManager.GetLocationName(pSetting->GetData()));
		}
	}
}

void CGUIWindowSettingsCategory::FreeSettingsControls()
{
	// Clear the settings group
	CGUIControlGroupList *control = (CGUIControlGroupList *)GetControl(SETTINGS_GROUP_ID);
	
	if (control)
	{
		control->FreeResources();
		control->ClearAll();
	}

	for(int i = 0; (size_t)i < m_vecSettings.size(); i++)
	{
		delete m_vecSettings[i];
	}
	
	m_vecSettings.clear();
}

void CGUIWindowSettingsCategory::AddSetting(CSetting *pSetting, float width, int &iControlID)
{
	CBaseSettingControl *pSettingControl = NULL;
	CGUIControl *pControl = NULL;

	if (pSetting->GetControlType() == CHECKMARK_CONTROL)
	{
		pControl = new CGUIRadioButtonControl(*m_pOriginalRadioButton);
		if (!pControl) return ;
		((CGUIRadioButtonControl *)pControl)->SetLabel(g_localizeStrings.Get(pSetting->GetLabel()));
		pControl->SetWidth(width);
		pSettingControl = new CRadioButtonSettingControl((CGUIRadioButtonControl *)pControl, iControlID, pSetting);
	}
	else if (pSetting->GetControlType() == SPIN_CONTROL_FLOAT || pSetting->GetControlType() == SPIN_CONTROL_INT_PLUS || pSetting->GetControlType() == SPIN_CONTROL_TEXT || pSetting->GetControlType() == SPIN_CONTROL_INT)
	{
		pControl = new CGUISpinControlEx(*m_pOriginalSpin);
		if (!pControl) return ;
		pControl->SetWidth(width);
		((CGUISpinControlEx *)pControl)->SetText(g_localizeStrings.Get(pSetting->GetLabel()));
		pSettingControl = new CSpinExSettingControl((CGUISpinControlEx *)pControl, iControlID, pSetting);
	}
	else if (pSetting->GetControlType() == SEPARATOR_CONTROL && m_pOriginalImage)
	{
		pControl = new CGUIImage(*m_pOriginalImage);
		if (!pControl) return;
		pControl->SetWidth(width);
		pSettingControl = new CSeparatorSettingControl((CGUIImage *)pControl, iControlID, pSetting);
	}
	else if (pSetting->GetControlType() == EDIT_CONTROL_INPUT ||
			pSetting->GetControlType() == EDIT_CONTROL_HIDDEN_INPUT ||
			pSetting->GetControlType() == EDIT_CONTROL_MD5_INPUT ||
			pSetting->GetControlType() == EDIT_CONTROL_NUMBER_INPUT ||
			pSetting->GetControlType() == EDIT_CONTROL_IP_INPUT)
	{
				pControl = new CGUIEditControl(*m_pOriginalEdit);
				if (!pControl) return ;
				((CGUIEditControl *)pControl)->SettingsCategorySetTextAlign(XBFONT_CENTER_Y);
				((CGUIEditControl *)pControl)->SetLabel(g_localizeStrings.Get(pSetting->GetLabel()));
				pControl->SetWidth(width);
				pSettingControl = new CEditSettingControl((CGUIEditControl *)pControl, iControlID, pSetting);
	}
	else if (pSetting->GetControlType() != SEPARATOR_CONTROL) // Button control
	{
		pControl = new CGUIButtonControl(*m_pOriginalButton);
		if (!pControl) return ;
		((CGUIButtonControl *)pControl)->SettingsCategorySetTextAlign(XBFONT_CENTER_Y);
		((CGUIButtonControl *)pControl)->SetLabel(g_localizeStrings.Get(pSetting->GetLabel()));
		pControl->SetWidth(width);
		pSettingControl = new CButtonSettingControl((CGUIButtonControl *)pControl, iControlID, pSetting);
	}

	if (!pControl) return;

	pControl->SetID(iControlID++);
	pControl->SetVisible(true);
	CGUIControlGroupList *group = (CGUIControlGroupList *)GetControl(SETTINGS_GROUP_ID);
	
	if (group)
	{
		pControl->AllocResources();
		group->AddControl(pControl);
		m_vecSettings.push_back(pSettingControl);
	}
}

CBaseSettingControl *CGUIWindowSettingsCategory::GetSetting(const CStdString &strSetting)
{
	for (unsigned int i = 0; i < m_vecSettings.size(); i++)
	{
		if (m_vecSettings[i]->GetSetting()->GetSetting() == strSetting)
			return m_vecSettings[i];
	}
	return NULL;
}

void CGUIWindowSettingsCategory::OnClick(CBaseSettingControl *pSettingControl)
{
	CStdString strSetting = pSettingControl->GetSetting()->GetSetting();
	
	if (strSetting.Left(16).Equals("weather.location"))
	{
		CStdString strSearch;

		if (CGUIDialogKeyboard::ShowAndGetInput(strSearch, g_localizeStrings.Get(14024), false))
		{
			CStdString strResult = ((CSettingString*)pSettingControl->GetSetting())->GetData();

			// Trim the coordinates (is it required to put the old value into the search?)
			int iSep = strResult.Find("~");
			if (iSep >= 0)
				strResult = strResult.Mid(iSep + 5).Trim();

			if (g_weatherManager.GetSearchResults(strSearch, strResult))
				((CSettingString *)pSettingControl->GetSetting())->SetData(strResult);

			g_weatherManager.Refresh();
		}
	}

	/* WIP HERE */

	// If OnClick() returns false, the setting hasn't changed or doesn't
	// require immediate update
	if (!pSettingControl->OnClick())
	{
		UpdateSettings();

		if (!pSettingControl->IsDelayed())
			return;
	}

	if (pSettingControl->IsDelayed()) // TODO
	{
		// Delayed setting
//		m_delayedSetting = pSettingControl;
//		m_delayedTimer.StartZero();
	}
	else
		OnSettingChanged(pSettingControl);
}

void CGUIWindowSettingsCategory::OnSettingChanged(CBaseSettingControl *pSettingControl)
{
	CStdString strSetting = pSettingControl->GetSetting()->GetSetting();

	// Ok, now check the various special things we need to do

	/* WIP HERE */

	UpdateSettings();
}