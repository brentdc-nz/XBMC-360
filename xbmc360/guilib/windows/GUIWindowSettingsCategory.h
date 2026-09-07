#ifndef GUILIB_GUIWINDOWSETTINGSCATEGORY_H
#define GUILIB_GUIWINDOWSETTINGSCATEGORY_H

#include "guilib\GUIWindow.h"
#include "SettingsControls.h"
#include "Settings.h"
#include "utils\Stopwatch.h"
#include "GUISettings.h"
#include "guilib\GraphicContext.h"

#include "guilib\GUISpinControlEx.h"
#include "guilib\GUIImage.h"
#include "guilib\GUIEditControl.h"
#include "guilib\GUIToggleButtonControl.h"
#include "guilib\GUIRadioButtonControl.h"

class CGUIWindowSettingsCategory : public CGUIWindow
{
public:
	CGUIWindowSettingsCategory(void);
	virtual ~CGUIWindowSettingsCategory(void);

	virtual bool OnBack(int actionID);
	virtual bool OnMessage(CGUIMessage &message);
	virtual void FrameMove();
	virtual void Render();
	//virtual int GetID() const { return CGUIWindow::GetID() + m_iScreen; };

protected:
	virtual void OnInitWindow();
	virtual void SetupControls();
	virtual void FreeControls();
	void CreateSettings();
	void CheckForUpdates();
	void UpdateSettings();
	void FreeSettingsControls();
	void AddSetting(CSetting *pSetting, float width, int &iControlID);
	CBaseSettingControl* GetSetting(const CStdString &strSetting);
	virtual void OnClick(CBaseSettingControl *pSettingControl);
	virtual void OnSettingChanged(CBaseSettingControl *pSettingControl);

	void FillInResolutions(CSetting *pSetting);
	void FillInSkins(CSetting *pSetting);
	void FillInLanguages(CSetting *pSetting);
	void FillInRegions(CSetting *pSetting);
	void FillInScreenSavers(CSetting *pSetting);
	void FillInVisualisations(CSetting *pSetting);

	CGUISpinControlEx *m_pOriginalSpin;
	CGUIRadioButtonControl *m_pOriginalRadioButton;
	CGUIButtonControl *m_pOriginalCategoryButton;
	CGUIButtonControl *m_pOriginalButton;
	CGUIEditControl *m_pOriginalEdit;
	CGUIImage *m_pOriginalImage;

	int m_iScreen;
	int m_iSection;
	vecSettingsCategory m_vecSections;
	std::vector<CBaseSettingControl*> m_vecSettings;

	// look + feel settings (for delayed loading)
	CStdString m_strNewSkinFontSet;
	CStdString m_strNewSkin;
	CStdString m_strNewLanguage;
	CStdString m_strNewSkinTheme;
	CStdString m_strNewSkinColors;

	RESOLUTION m_NewResolution;
	bool m_returningFromSkinLoad;
	CStdString m_strErrorMessage;
};

#endif //GUILIB_GUIWINDOWSETTINGSCATEGORY_H
