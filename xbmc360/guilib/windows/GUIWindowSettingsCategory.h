#ifndef GUILIB_GUIWINDOWSETTINGSCATEGORY_H
#define GUILIB_GUIWINDOWSETTINGSCATEGORY_H

#include "..\GUIWindow.h"
#include "..\..\GUISettings.h"
#include "..\GUIButtonControl.h"
#include "..\GUISpinControlEx.h"
#include "..\..\SettingsControls.h"

class CGUIWindowSettingsCategory : public CGUIWindow
{
public:
	CGUIWindowSettingsCategory(void);
	virtual ~CGUIWindowSettingsCategory(void);

	virtual bool OnMessage(CGUIMessage& message);

protected:
	void OnClick(CBaseSettingControl *pSettingControl);
	void SetupControls();
	void CreateSettings();
	void AddSetting(CSetting *pSetting, int iPosX, int &iPosY, int iGap, int iWidth, int &iControlID);
	virtual void OnInitWindow();
	void FreeControls();
	void FreeSettingsControls();

	void FillInSkins(CSetting *pSetting);

	CBaseSettingControl* GetSetting(const CStdString &strSetting);

	CGUISpinControlEx *m_pOriginalSpin;
	CGUIButtonControl *m_pOriginalSettingsButton;

	std::vector<CBaseSettingControl *> m_vecSettings;
	int m_iScreen;

	vecSettingsCategory m_vecSections;
	int m_iSection;

	// look + feel settings (for delayed loading)
	CStdString m_strNewSkin;
};

#endif //GUILIB_GUIWINDOWSETTINGSCATEGORY_H
