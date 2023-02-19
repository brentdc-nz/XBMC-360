#include "GUIWindowSettingsCategory.h"
#include "guilib\GUIControlGroupList.h"
#include "guilib\LocalizeStrings.h"

#define CONTROL_GROUP_BUTTONS           0
#define CONTROL_GROUP_SETTINGS          1
#define CONTROL_SETTINGS_LABEL          2
#define CATEGORY_GROUP_ID               3
#define SETTINGS_GROUP_ID               5
#define CONTROL_DEFAULT_BUTTON          7
#define CONTROL_DEFAULT_RADIOBUTTON     8
#define CONTROL_DEFAULT_SPIN            9
#define CONTROL_DEFAULT_CATEGORY_BUTTON 10
#define CONTROL_DEFAULT_SEPARATOR       11
#define CONTROL_DEFAULT_EDIT            12
#define CONTROL_START_BUTTONS           -40
#define CONTROL_START_CONTROL           -20

CGUIWindowSettingsCategory::CGUIWindowSettingsCategory(void)
    : CGUIWindow(WINDOW_SETTINGS_MYPICTURES, "SettingsCategory.xml")
{
	m_pOriginalSpin = NULL;
	m_pOriginalRadioButton = NULL;
	m_pOriginalButton = NULL;
	m_pOriginalCategoryButton = NULL;
	m_pOriginalImage = NULL;
	m_pOriginalEdit = NULL;
	
	// Set the correct ID range...
	m_idRange = 8;
	m_iScreen = 0;

	m_returningFromSkinLoad = false;
	m_delayedSetting = NULL;
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

bool CGUIWindowSettingsCategory::OnMessage(CGUIMessage &message)
{
	// TODO

	switch (message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
			m_delayedSetting = NULL;
			if (message.GetParam1() != WINDOW_INVALID && !m_returningFromSkinLoad)
			{
				// Coming to this window first time (ie not returning back from some other window)
				// so we reset our section and control states
				m_iSection = 0;
				ResetControlStates();
			}
			m_returningFromSkinLoad = false;
			m_iScreen = (int)message.GetParam2() - (int)CGUIWindow::GetID();
			return CGUIWindow::OnMessage(message);
		}
		break;
	}

	return CGUIWindow::OnMessage(message);
}

void CGUIWindowSettingsCategory::SetupControls()
{
	// Cleanup first, if necessary
	FreeControls();

	m_pOriginalSpin = (CGUISpinControlEx*)GetControl(CONTROL_DEFAULT_SPIN);
	m_pOriginalRadioButton = (CGUIRadioButtonControl*)GetControl(CONTROL_DEFAULT_RADIOBUTTON);
	m_pOriginalCategoryButton = (CGUIButtonControl*)GetControl(CONTROL_DEFAULT_CATEGORY_BUTTON);
	m_pOriginalButton = (CGUIButtonControl*)GetControl(CONTROL_DEFAULT_BUTTON);
	m_pOriginalImage = (CGUIImage*)GetControl(CONTROL_DEFAULT_SEPARATOR);
	
	if (!m_pOriginalCategoryButton || !m_pOriginalSpin || !m_pOriginalRadioButton || !m_pOriginalButton)
		return;
	
	m_pOriginalEdit = (CGUIEditControl*)GetControl(CONTROL_DEFAULT_EDIT);
	
	if (!m_pOriginalEdit || m_pOriginalEdit->GetControlType() != CGUIControl::GUICONTROL_EDIT)
	{
		delete m_pOriginalEdit;
		m_pOriginalEdit = new CGUIEditControl(*m_pOriginalButton);
	}
	
	m_pOriginalSpin->SetVisible(false);
	m_pOriginalRadioButton->SetVisible(false);
	m_pOriginalButton->SetVisible(false);
	m_pOriginalCategoryButton->SetVisible(false);
	m_pOriginalEdit->SetVisible(false);
	
	if (m_pOriginalImage) m_pOriginalImage->SetVisible(false);

	// Setup our control groups...
	CGUIControlGroupList *group = (CGUIControlGroupList *)GetControl(CATEGORY_GROUP_ID);
	if (!group)
		return;
	
	// Get a list of different sections
	CSettingsGroup *pSettingsGroup = g_guiSettings.GetGroup(m_iScreen);
	if (!pSettingsGroup) return;
	
	// Update the screen string
	SET_CONTROL_LABEL(CONTROL_SETTINGS_LABEL, pSettingsGroup->GetLabelID());
	
	// Get the categories we need
	pSettingsGroup->GetCategories(m_vecSections);
	
	// Run through and create our buttons...
	int j = 0;
	for (unsigned int i = 0; i < m_vecSections.size(); i++)
	{
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

		// TODO!

	}

	// Update our settings (turns controls on/off as appropriate)
	UpdateSettings();
}

void CGUIWindowSettingsCategory::UpdateSettings()
{
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

void CGUIWindowSettingsCategory::FreeControls()
{
	// Clear the category group
	CGUIControlGroupList *control = (CGUIControlGroupList *)GetControl(CATEGORY_GROUP_ID);
	
	if (control)
	{
		control->FreeResources();
		control->ClearAll();
	}
	
	m_vecSections.clear();
	FreeSettingsControls();
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

void CGUIWindowSettingsCategory::OnInitWindow()
{
	SetupControls();
	
	CGUIWindow::OnInitWindow();
}