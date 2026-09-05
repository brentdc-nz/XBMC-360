#include "GUIWindowSettingsCategory.h"
#include "guilib\GUIControlGroupList.h"
#include "guilib\LocalizeStrings.h"
#include "guilib\GUIMessage.h"
#include "guilib\dialogs\GUIDialogKeyboard.h"
#include "utils\Weather.h"
#include "guilib\GUIUserMessages.h"
#include "guilib\GUIWindowManager.h"
#include "xbox\XBVideoConfig.h"
#include "xbox\XBTimeZone.h"
#include "LangInfo.h"
#include "filesystem\Directory.h"
#include "utils\Util.h"

using namespace XFILE;

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

		if (strSetting.Equals("videooutput.aspect"))
		{
			CSettingInt *pSettingInt = (CSettingInt*)pSetting;
			CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(strSetting)->GetID());
			pControl->AddLabel(g_localizeStrings.Get(21375), VIDEO_NORMAL);
			pControl->AddLabel(g_localizeStrings.Get(21376), VIDEO_LETTERBOX);
			pControl->AddLabel(g_localizeStrings.Get(21377), VIDEO_WIDESCREEN);
			pControl->SetValue(pSettingInt->GetData());
		}
		else if (strSetting.Equals("videoplayer.resumeautomatically"))
		{
			CSettingInt *pSettingInt = (CSettingInt*)pSetting;
			CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(strSetting)->GetID());
			pControl->AddLabel(g_localizeStrings.Get(106), RESUME_NO);
			pControl->AddLabel(g_localizeStrings.Get(107), RESUME_YES);
			pControl->AddLabel(g_localizeStrings.Get(12020), RESUME_ASK);
			pControl->SetValue(pSettingInt->GetData());
		}
		else if (strSetting.Equals("locale.timezone"))
		{
			CSettingInt *pSettingInt = (CSettingInt*)pSetting;
			CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(strSetting)->GetID());
			for (int j = 0; j < g_timezone.GetNumberOfTimeZones(); j++)
				pControl->AddLabel(g_timezone.GetTimeZoneString(j), j);
			pControl->SetValue(pSettingInt->GetData());
		}
		else if (strSetting.Equals("videoscreen.resolution"))
		{
			FillInResolutions(pSetting);
		}
		else if (strSetting.Equals("lookandfeel.skin"))
		{
			FillInSkins(pSetting);
		}
		else if (strSetting.Equals("locale.language"))
		{
			FillInLanguages(pSetting);
		}
		else if (strSetting.Equals("locale.country"))
		{
			FillInRegions(pSetting);
		}
		else if (strSetting.Equals("screensaver.mode"))
		{
			FillInScreenSavers(pSetting);
		}
		else if (strSetting.Equals("musicplayer.visualisation"))
		{
			FillInVisualisations(pSetting);
		}
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

	if (strSetting.Equals("lookandfeel.skin"))
	{
		CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(pSettingControl->GetID());
		CStdString strSkin = pControl->GetCurrentLabel();
		if (strSkin != g_guiSettings.GetString("lookandfeel.skin"))
			g_guiSettings.SetString("lookandfeel.skin", strSkin);
	}
	else if (strSetting.Equals("locale.language"))
	{
		CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(pSettingControl->GetID());
		CStdString strLanguage = pControl->GetCurrentLabel();
		if (strLanguage != g_guiSettings.GetString("locale.language"))
			g_guiSettings.SetString("locale.language", strLanguage);
	}
	else if (strSetting.Equals("locale.country"))
	{
		CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(pSettingControl->GetID());
		CStdString strRegion = pControl->GetCurrentLabel();
		if (strRegion != g_guiSettings.GetString("locale.country"))
		{
			g_guiSettings.SetString("locale.country", strRegion);
			g_langInfo.SetCurrentRegion(strRegion);
		}
	}
	else if (strSetting.Equals("screensaver.mode"))
	{
		CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(pSettingControl->GetID());
		CStdString strMode = pControl->GetCurrentLabel();
		g_guiSettings.SetString("screensaver.mode", strMode);
	}
	else if (strSetting.Equals("musicplayer.visualisation"))
	{
		// New visualisation choosen...
		CSettingString *pSettingString = (CSettingString *)pSettingControl->GetSetting();
		CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(pSettingControl->GetID());

		if (pControl->GetValue() == 0)
			pSettingString->SetData("None");
		else
			pSettingString->SetData(pControl->GetCurrentLabel() + ".vis");
	}
	else if (strSetting.Equals("locale.timezone"))
	{
		// Timezone change is applied on next boot via LoadXML
	}

	UpdateSettings();
}

void CGUIWindowSettingsCategory::FillInResolutions(CSetting *pSetting)
{
	CSettingInt *pSettingInt = (CSettingInt*)pSetting;
	CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(pSetting->GetSetting())->GetID());
	pControl->Clear();

	for (int i = HDTV_1080p; i < AUTORES; i++)
	{
		RESOLUTION res = (RESOLUTION)i;
		if (g_videoConfig.IsValidResolution(res))
			pControl->AddLabel(g_settings.m_ResInfo[res].strMode, res);
	}
	pControl->AddLabel(g_localizeStrings.Get(14061), AUTORES); // "Auto"
	pControl->SetValue(pSettingInt->GetData());
}

void CGUIWindowSettingsCategory::FillInSkins(CSetting *pSetting)
{
	CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(pSetting->GetSetting())->GetID());
	pControl->SetType(SPIN_CONTROL_TYPE_TEXT);
	pControl->Clear();

	//find skins...
	CFileItemList items;
	CDirectory::GetDirectory("D:\\skins\\", items);

	int iCurrentSkin = 0;
	int iSkin = 0;
	std::vector<CStdString> vecSkins;

	for (int i = 0; i < items.Size(); ++i)
	{
		CFileItemPtr pItem = items[i];
		if (pItem->m_bIsFolder)
		{
			if (strcmpi(pItem->GetLabel().c_str(), ".svn") == 0) continue;
			if (strcmpi(pItem->GetLabel().c_str(), "fonts") == 0) continue;
			if (strcmpi(pItem->GetLabel().c_str(), "media") == 0) continue;
			vecSkins.push_back(pItem->GetLabel());
		}
	}

	sort(vecSkins.begin(), vecSkins.end(), sortstringbyname());

	for (unsigned int i = 0; i < vecSkins.size(); ++i)
	{
		CStdString strSkin = vecSkins[i];
		if (strcmpi(strSkin.c_str(), g_guiSettings.GetString("lookandfeel.skin").c_str()) == 0)
			iCurrentSkin = iSkin;
		pControl->AddLabel(strSkin, iSkin++);
	}
	pControl->SetValue(iCurrentSkin);
}

void CGUIWindowSettingsCategory::FillInLanguages(CSetting *pSetting)
{
	CSettingString *pSettingString = (CSettingString*)pSetting;
	CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(pSetting->GetSetting())->GetID());
	pControl->Clear();

	//find languages...
	CFileItemList items;
	CDirectory::GetDirectory("D:\\language\\", items);

	int iCurrentLang = 0;
	int iLanguage = 0;
	std::vector<CStdString> vecLanguage;

	for (int i = 0; i < items.Size(); ++i)
	{
		CFileItemPtr pItem = items[i];
		if (pItem->m_bIsFolder)
		{
			if (strcmpi(pItem->GetLabel().c_str(), ".svn") == 0) continue;
			if (strcmpi(pItem->GetLabel().c_str(), "fonts") == 0) continue;
			if (strcmpi(pItem->GetLabel().c_str(), "media") == 0) continue;
			vecLanguage.push_back(pItem->GetLabel());
		}
	}

	sort(vecLanguage.begin(), vecLanguage.end(), sortstringbyname());

	for (unsigned int i = 0; i < vecLanguage.size(); ++i)
	{
		CStdString strLanguage = vecLanguage[i];
		if (strcmpi(strLanguage.c_str(), pSettingString->GetData().c_str()) == 0)
			iCurrentLang = iLanguage;
		pControl->AddLabel(strLanguage, iLanguage++);
	}
	pControl->SetValue(iCurrentLang);
}

void CGUIWindowSettingsCategory::FillInRegions(CSetting *pSetting)
{
	CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(pSetting->GetSetting())->GetID());
	pControl->SetType(SPIN_CONTROL_TYPE_TEXT);
	pControl->Clear();

	int iCurrentRegion = 0;
	CStdStringArray regions;
	g_langInfo.GetRegionNames(regions);

	CStdString strCurrentRegion = g_langInfo.GetCurrentRegion();

	sort(regions.begin(), regions.end(), sortstringbyname());

	for (int i = 0; i < (int)regions.size(); ++i)
	{
		const CStdString& strRegion = regions[i];
		if (strRegion == strCurrentRegion)
			iCurrentRegion = i;
		pControl->AddLabel(strRegion, i);
	}
	pControl->SetValue(iCurrentRegion);
}

void CGUIWindowSettingsCategory::FillInScreenSavers(CSetting *pSetting)
{	// Screensaver mode
	CSettingString *pSettingString = (CSettingString*)pSetting;
	CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(pSetting->GetSetting())->GetID());
	pControl->Clear();

	pControl->AddLabel(g_localizeStrings.Get(351), 0); // Off
	pControl->AddLabel(g_localizeStrings.Get(352), 1); // Dim
	pControl->AddLabel(g_localizeStrings.Get(353), 2); // Black
	pControl->AddLabel("Plasma", 3);

	CStdString strDefaultScr = pSettingString->GetData();
	int iCurrentScr = -1;

	if (strDefaultScr == "Dim")
		iCurrentScr = 1;
	else if (strDefaultScr == "Black")
		iCurrentScr = 2;
	else if (strDefaultScr == "Plasma")
		iCurrentScr = 3;
	else
	{
		iCurrentScr = 0;
		pSettingString->SetData("None");
	}

	pControl->SetValue(iCurrentScr);
}

void CGUIWindowSettingsCategory::FillInVisualisations(CSetting *pSetting)
{
	// Visualisation - From xbmc4xbox, minus the directory scan
	// (we are static linking as we don't have a DLL loader yet, so only
	// MilkDrop2 is available - See CVisualisationFactory::LoadVisualisation)
	CSettingString *pSettingString = (CSettingString*)pSetting;
	if (!pSetting) return;
	int iWinID = g_windowManager.GetActiveWindow();
	CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(pSetting->GetSetting())->GetID());
	pControl->Clear();

	// find visz....
	std::vector<CStdString> vecVis;
	vecVis.push_back("MilkDrop2"); // Statically linked - the only .vis we have

	CStdString strDefaultVis = pSettingString->GetData();
	if (!strDefaultVis.Equals("None"))
		strDefaultVis.Delete(strDefaultVis.size() - 4, 4);

	// add the "disabled" setting first
	int iVis = 0;
	int iCurrentVis = 0;
	pControl->AddLabel(g_localizeStrings.Get(231), iVis++);

	for (int i = 0; i < (int) vecVis.size(); ++i)
	{
		CStdString strVis = vecVis[i];

		if (strcmpi(strVis.c_str(), strDefaultVis.c_str()) == 0)
			iCurrentVis = iVis;

		pControl->AddLabel(strVis, iVis++);
	}
	pControl->SetValue(iCurrentVis);
}