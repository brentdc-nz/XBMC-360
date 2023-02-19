#ifndef GUILIB_GUIWINDOWSETTINGSCATEGORY_H
#define GUILIB_GUIWINDOWSETTINGSCATEGORY_H

#include "guilib\GUIWindow.h"
#include "guilib\SettingsControls.h"
#include "Settings.h"
#include "utils\Stopwatch.h"
#include "GUISettings.h"

#include "guilib\GUISpinControlEx.h"
#include "guilib\GUIImage.h"
#include "guilib\GUIEditControl.h"
#include "guilib\GUIToggleButtonControl.h"

class CGUIWindowSettingsCategory : public CGUIWindow
{
public:
	CGUIWindowSettingsCategory(void);
	virtual ~CGUIWindowSettingsCategory(void);

	virtual bool OnBack(int actionID);
	virtual bool OnMessage(CGUIMessage &message);

protected:
	virtual void OnInitWindow();

	void CreateSettings();
	virtual void SetupControls();
	void UpdateSettings();
	void FreeSettingsControls();
	virtual void FreeControls();
	void AddSetting(CSetting *pSetting, float width, int &iControlID);
	CBaseSettingControl* GetSetting(const CStdString &strSetting);

	std::vector<CBaseSettingControl*> m_vecSettings;
	int m_iSection;
	int m_iScreen;
	vecSettingsCategory m_vecSections;

	CGUISpinControlEx *m_pOriginalSpin;
	CGUIRadioButtonControl *m_pOriginalRadioButton;
	CGUIButtonControl *m_pOriginalCategoryButton;
	CGUIButtonControl *m_pOriginalButton;
	CGUIEditControl *m_pOriginalEdit;
	CGUIImage *m_pOriginalImage;

	bool m_returningFromSkinLoad; // True if we are returning from loading the skin

	CBaseSettingControl *m_delayedSetting; // Current delayed setting \sa CBaseSettingControl::SetDelayed()
};

#endif //GUILIB_GUIWINDOWSETTINGSCATEGORY_H
