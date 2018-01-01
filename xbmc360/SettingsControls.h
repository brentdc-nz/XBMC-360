#ifndef H_CSETTINGSCONTROLS
#define H_CSETTINGSCONTROLS

#include "GUISettings.h"
#include "guilib\GUISpinControlEx.h"

class CBaseSettingControl
{
public:
	CBaseSettingControl(DWORD dwID, CSetting *pSetting);
	~CBaseSettingControl() {};
	virtual void OnClick() {};
	virtual void Update() {};
	DWORD GetID() { return m_dwID; };
	CSetting* GetSetting() { return m_pSetting; };

protected:
	DWORD m_dwID;
	CSetting* m_pSetting;
};

class CSpinExSettingControl : public CBaseSettingControl
{
public:
	CSpinExSettingControl(CGUISpinControlEx* pSpin, DWORD dwID, CSetting *pSetting);
	~CSpinExSettingControl();
	virtual void OnClick();
	virtual void Update();
private:
	CGUISpinControlEx *m_pSpin;
};


#endif //H_CSETTINGSCONTROLS