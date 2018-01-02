/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIWindowSettingsCategory.h"
#include "..\..\GUISettings.h"
#include "..\LocalizeStrings.h"
#include "..\..\Application.h"
#include "..\GUIWindowManager.h"
#include "..\..\Settings.h"

#define CONTROL_GROUP_BUTTONS           0
#define CONTROL_GROUP_SETTINGS          1
#define CONTROL_SETTINGS_LABEL          2
#define CONTROL_BUTTON_AREA             3
#define CONTROL_BUTTON_GAP              4
#define CONTROL_AREA                    5
#define CONTROL_GAP                     6
#define CONTROL_DEFAULT_BUTTON          7
#define CONTROL_DEFAULT_RADIOBUTTON     8
#define CONTROL_DEFAULT_SPIN            9
#define CONTROL_DEFAULT_SETTINGS_BUTTON 10
#define CONTROL_DEFAULT_SEPARATOR       11
#define CONTROL_START_BUTTONS           30
#define CONTROL_START_CONTROL           50

CGUIWindowSettingsCategory::CGUIWindowSettingsCategory(void) : CGUIWindow(WINDOW_SETTINGS_MYPICTURES, "SettingsCategory.xml")
{
	m_loadOnDemand = false;
	
	m_pOriginalSpin = NULL;
	m_pOriginalSettingsButton = NULL;

	// set the correct ID range...
	m_dwIDRange = 8;
	m_iScreen = 0;
}

CGUIWindowSettingsCategory::~CGUIWindowSettingsCategory(void)
{
}

bool CGUIWindowSettingsCategory::OnMessage(CGUIMessage &message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_CLICKED:
		{
			unsigned int iControl = message.GetSenderId();

			for (unsigned int i = 0; i < m_vecSettings.size(); i++)
			{
				if (m_vecSettings[i]->GetID() == iControl)
				OnClick(m_vecSettings[i]);
			}
		}
		break;

		case GUI_MSG_LOAD_SKIN: //TODO
		{
			// Do we need to reload the language file
/*			if (!m_strNewLanguage.IsEmpty())
			{
				g_guiSettings.SetString("LookAndFeel.Language", m_strNewLanguage);
*/				g_settings.Save();

/*				CStdString strLangInfoPath;
				strLangInfoPath.Format("Q:\\language\\%s\\langinfo.xml", m_strNewLanguage.c_str());
				g_langInfo.Load(strLangInfoPath);

				if (g_langInfo.ForceUnicodeFont() && !g_fontManager.IsFontSetUnicode())
				{
					CLog::Log(LOGINFO, "Language needs a ttf font, loading first ttf font available");
					CStdString strFontSet;
					if (g_fontManager.GetFirstFontSetUnicode(strFontSet))
					{
						m_strNewSkinFontSet=strFontSet;
					}
					else
					CLog::Log(LOGERROR, "No ttf font found but needed.", strFontSet.c_str());
				}

				g_charsetConverter.reset();

				CStdString strLanguagePath;
				strLanguagePath.Format("Q:\\language\\%s\\strings.xml", m_strNewLanguage.c_str());
				g_localizeStrings.Load(strLanguagePath);
			}

			// Do we need to reload the skin font set
			if (!m_strNewSkinFontSet.IsEmpty())
			{
				g_guiSettings.SetString("LookAndFeel.Font", m_strNewSkinFontSet);
				g_settings.Save();
			}
*/
			// Reload another skin
			if (!m_strNewSkin.IsEmpty())
			{
				g_guiSettings.SetString("LookAndFeel.Skin", m_strNewSkin);
				g_settings.Save();
			}

/*			// Reload a skin theme
			if (!m_strNewSkinTheme.IsEmpty())
			{
				g_guiSettings.SetString("LookAndFeel.SkinTheme", m_strNewSkinTheme);
				g_settings.Save();
			}
*/
			// Reload the skin.  Save the current focused control, and refocus it
			// when done.
			unsigned iCtrlID = GetFocusedControl();
//			CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), iCtrlID, 0, 0, NULL);
//			g_graphicsContext.SendMessage(msg);
			g_application.LoadSkin(g_guiSettings.GetString("LookAndFeel.Skin"));
			g_windowManager.ActivateWindow(/*GetID()*/WINDOW_SETTINGS_APPEARANCE);
//			SET_CONTROL_FOCUS(iCtrlID, 0);
//			CGUIMessage msgSelect(GUI_MSG_ITEM_SELECT, GetID(), iCtrlID, msg.GetParam1(), msg.GetParam2());
//			OnMessage(msgSelect);
		}
		break;

		case GUI_MSG_WINDOW_INIT:
		{
			if (message.GetParam1() != WINDOW_INVALID)
			{	
				// coming to this window first time (ie not returning back from some other window)
				// so we reset our section and control states
				m_iSection = 0;
			}
			m_iScreen = (int)message.GetParam2() - (int)iWindowID;
			return CGUIWindow::OnMessage(message);
		}
		break;
  
		case GUI_MSG_WINDOW_DEINIT:
		{
			CGUIWindow::OnMessage(message);
			FreeControls();
			return true;
		}
		break;	
	}

	return CGUIWindow::OnMessage(message);
}

void CGUIWindowSettingsCategory::OnClick(CBaseSettingControl *pSettingControl)
{
	CStdString strSetting = pSettingControl->GetSetting()->GetSetting();

	if (strSetting == "LookAndFeel.Skin")
	{ 
		// new skin choosen...
		CSettingString *pSettingString = (CSettingString *)pSettingControl->GetSetting();
		CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(pSettingControl->GetID());
		CStdString strSkin = pControl->GetCurrentLabel();
		CStdString strSkinPath = "D:\\skins\\" + strSkin;
		
/*		if (g_SkinInfo.Check(strSkinPath)) //TODO
		{
			m_strErrorMessage.Empty();
			pControl->SettingsCategorySetSpinTextColor(pControl->GetButtonLabelInfo().textColor);
*/			if (strSkin != "CVS" && strSkin != g_guiSettings.GetString("LookAndFeel.Skin"))
			{
				m_strNewSkin = strSkin;
				g_application.DelayLoadSkin();
			}
			else
			{
				// Do not reload the skin we are already using
				m_strNewSkin.Empty();
				g_application.CancelDelayLoadSkin();
			}
/*		}
		else 
		{
			m_strErrorMessage.Format("Incompatible skin. We require skins of version %0.2f or higher", g_SkinInfo.GetMinVersion());
			m_strNewSkin.Empty();
			g_application.CancelDelayLoadSkin();
			pControl->SettingsCategorySetSpinTextColor(pControl->GetButtonLabelInfo().disabledColor);
		}*/
	}
}

void CGUIWindowSettingsCategory::SetupControls()
{
	// get the area to use...
	const CGUIControl *pButtonArea = GetControl(CONTROL_BUTTON_AREA);
	const CGUIControl *pControlGap = GetControl(CONTROL_BUTTON_GAP);
	m_pOriginalSpin = (CGUISpinControlEx*)GetControl(CONTROL_DEFAULT_SPIN);
	m_pOriginalSettingsButton = (CGUIButtonControl *)GetControl(CONTROL_DEFAULT_SETTINGS_BUTTON);

	if (!m_pOriginalSpin || !m_pOriginalSettingsButton || !pButtonArea || !pControlGap )
		return;

	m_pOriginalSpin->SetVisible(false);
	m_pOriginalSettingsButton->SetVisible(false);

	// get a list of different sections
	CSettingsGroup *pSettingsGroup = g_guiSettings.GetGroup(m_iScreen);

	if (!pSettingsGroup) return ;

	// update the screen string
//	SET_CONTROL_LABEL(CONTROL_SETTINGS_LABEL, pSettingsGroup->GetLabelID()); //TODO
	
	// get the categories we need
	pSettingsGroup->GetCategories(m_vecSections);
	
	// run through and create our buttons...
	for (unsigned int i = 0; i < m_vecSections.size(); i++)
	{
		CGUIButtonControl *pButton = new CGUIButtonControl(*m_pOriginalSettingsButton);
		pButton->SetLabel(g_localizeStrings.Get(m_vecSections[i]->m_dwLabelID));
		pButton->SetID(CONTROL_START_BUTTONS + i);
//		pButton->SetGroup(CONTROL_GROUP_BUTTONS); //TODO
		pButton->SetPosition((float)pButtonArea->GetXPosition(), (float)pButtonArea->GetYPosition() + i*(float)pControlGap->GetHeight());
		pButton->SetNavigation(CONTROL_START_BUTTONS + (int)i - 1, CONTROL_START_BUTTONS + i + 1, CONTROL_START_CONTROL, CONTROL_START_CONTROL);
		pButton->SetVisible(true);
		pButton->AllocResources();

		InsertControl(pButton, m_pOriginalSettingsButton);
	}

	// update the first and last buttons...
	CGUIControl *pControl = (CGUIControl *)GetControl(CONTROL_START_BUTTONS);
	pControl->SetNavigation(CONTROL_START_BUTTONS + (int)m_vecSections.size() - 1, pControl->GetControlIdDown(),
                          pControl->GetControlIdLeft(), pControl->GetControlIdRight());

	pControl = (CGUIControl *)GetControl(CONTROL_START_BUTTONS + (int)m_vecSections.size() - 1);
	pControl->SetNavigation(pControl->GetControlIdUp(), CONTROL_START_BUTTONS,
                          pControl->GetControlIdLeft(), pControl->GetControlIdRight());
	
	if (m_iSection < 0 || m_iSection >= (int)m_vecSections.size())
		m_iSection = 0;

	CreateSettings();

	// set focus correctly
	m_dwDefaultFocusControlID = CONTROL_START_BUTTONS;
}

void CGUIWindowSettingsCategory::CreateSettings()
{
//	FreeSettingsControls();
//	m_vecGroups.push_back(CControlGroup(1)); // add the control group

	const CGUIControl *pControlArea = GetControl(CONTROL_AREA);
	const CGUIControl *pControlGap = GetControl(CONTROL_GAP);

	if (!pControlArea || !pControlGap)
		return ;

	int iPosX = pControlArea->GetXPosition();
	int iWidth = pControlArea->GetWidth();
	int iPosY = pControlArea->GetYPosition();
	int iGapY = pControlGap->GetHeight();

	vecSettings settings;
	g_guiSettings.GetSettingsGroup(m_vecSections[m_iSection]->m_strCategory, settings);
	int iControlID = CONTROL_START_CONTROL;

	for (unsigned int i = 0; i < settings.size(); i++)
	{
		CSetting *pSetting = settings[i];
		AddSetting(pSetting, iPosX, iPosY, iGapY, iWidth, iControlID);

		CStdString strSetting = pSetting->GetSetting();

		if (strSetting == "LookAndFeel.Skin")
		{
			FillInSkins(pSetting);
		}
	}
}

void CGUIWindowSettingsCategory::AddSetting(CSetting *pSetting, int iPosX, int &iPosY, int iGap, int iWidth, int &iControlID)
{
	CBaseSettingControl *pSettingControl = NULL;
	CGUIControl *pControl = NULL;
	CGUIControl *baseControl = NULL;

	if ( pSetting->GetControlType() == SPIN_CONTROL_TEXT )
	{
		baseControl = m_pOriginalSpin;
		pControl = new CGUISpinControlEx(*m_pOriginalSpin);
		if (!pControl) return ;
		pControl->SetPosition((float)iPosX, (float)iPosY);
//		pControl->SetWidth(iWidth);
		((CGUISpinControlEx *)pControl)->SetText(g_localizeStrings.Get(pSetting->GetLabel()));
//		pControl->SetWidth(iWidth);
		pSettingControl = new CSpinExSettingControl((CGUISpinControlEx *)pControl, iControlID, pSetting);
		iPosY += iGap;
	}

	if (!pControl) return;
	
	pControl->SetNavigation(iControlID - 1,
                          iControlID + 1,
                          CONTROL_START_BUTTONS,
                          CONTROL_START_BUTTONS);

	pControl->SetID(iControlID++);
//	pControl->SetGroup(CONTROL_GROUP_SETTINGS);
	pControl->SetVisible(true);
	InsertControl(pControl, baseControl);
	pControl->AllocResources();
	m_vecSettings.push_back(pSettingControl);
}

void CGUIWindowSettingsCategory::OnInitWindow()
{
	SetupControls();

	CGUIWindow::OnInitWindow();
}

void CGUIWindowSettingsCategory::FreeControls()
{
	// free any created controls
	for (unsigned int i = 0; i < m_vecSections.size(); i++)
	{
		CGUIControl *pControl = (CGUIControl *)GetControl(CONTROL_START_BUTTONS + i);
		RemoveControl(CONTROL_START_BUTTONS + i);
		if (pControl)
		{
			pControl->FreeResources();
			delete pControl;
		}
	}
	m_vecSections.clear();
	FreeSettingsControls();
}

void CGUIWindowSettingsCategory::FreeSettingsControls()
{
	// remove the settings group  //TODO
//	if (m_vecGroups.size() > 1)
//		m_vecGroups.erase(m_vecGroups.begin() + 1);
 
	for (unsigned int i = 0; i < m_vecSettings.size(); i++)
	{
		CGUIControl *pControl = (CGUIControl *)GetControl(CONTROL_START_CONTROL + i);
		RemoveControl(CONTROL_START_CONTROL + i);
		
		if (pControl)
		{
			pControl->FreeResources();
			delete pControl;
		}
		delete m_vecSettings[i];
	}
	m_vecSettings.clear();
}

void CGUIWindowSettingsCategory::FillInSkins(CSetting *pSetting)
{
	CSettingString *pSettingString = (CSettingString*)pSetting;
	CGUISpinControlEx *pControl = (CGUISpinControlEx *)GetControl(GetSetting(pSetting->GetSetting())->GetID());
	pControl->SetType(SPIN_CONTROL_TYPE_TEXT);
	pControl->Clear();
//	pControl->SetShowRange(true); //TODO

	m_strNewSkin.Empty();

	//find skins...
	CStdString strPath = "D:\\skin\\";
	vector<CStdString> vecSkins;

	//TODO - Search for skins on the HDD

	//TEST 
	      vecSkins.push_back("Project Mayhem III");
	      vecSkins.push_back("PM3");
	//TEST

	int iCurrentSkin = 0;
	int iSkin = 0;

	for (int i = 0; i < (int) vecSkins.size(); ++i)
	{
		CStdString strSkin = vecSkins[i];
		if (strcmpi(strSkin.c_str(), g_guiSettings.GetString("LookAndFeel.Skin").c_str()) == 0)
		{
			iCurrentSkin = iSkin;
		}
		pControl->AddLabel(strSkin, iSkin++);
	}

	pControl->SetValue(iCurrentSkin);
	return ;
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