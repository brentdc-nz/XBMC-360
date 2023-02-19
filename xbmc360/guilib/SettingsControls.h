#ifndef CSETTINGSCONTROLS_H
#define CSETTINGSCONTROLS_H

#include "guilib\GUISpinControlEx.h"
#include "guilib\GUIEditControl.h"
#include "guilib\GUIRadioButtonControl.h"

class CGUIImage;
class CSetting;

class CBaseSettingControl
{
public:
	CBaseSettingControl(int id, CSetting *pSetting);
	virtual ~CBaseSettingControl() {}
	virtual bool OnClick() { return false; };
	virtual void Update() {};
	int GetID() { return m_id; };
	CSetting* GetSetting() { return m_pSetting; };
	virtual bool NeedsUpdate() { return false; }; // Returns true if the control needs an update
	virtual void Reset() {}; // Resets the NeedsUpdate() state
  
	/*
	Specifies that this setting should update after a delay
	Useful for settings that have options to navigate through
	and may take a while, or require additional input to update
	once the final setting is chosen.  Settings default to updating
	instantly.
	*/
	void SetDelayed() { m_delayed = true; };
  
	/*
	brief Returns whether this setting should have delayed update
	return true if the setting's update should be delayed
	*/
	bool IsDelayed() const { return m_delayed; };

protected:
	int m_id;
	CSetting* m_pSetting;
	bool m_delayed;
};

class CRadioButtonSettingControl : public CBaseSettingControl
{
public:
	CRadioButtonSettingControl(CGUIRadioButtonControl* pRadioButton, int id, CSetting *pSetting);
	virtual ~CRadioButtonSettingControl();
	virtual bool OnClick();
	virtual void Update();
	void Select(bool bSelect);

private:
	CGUIRadioButtonControl *m_pRadioButton;
};

class CSpinExSettingControl : public CBaseSettingControl
{
public:
	CSpinExSettingControl(CGUISpinControlEx* pSpin, int id, CSetting *pSetting);
	virtual ~CSpinExSettingControl();
	virtual bool OnClick();
	virtual void Update();

private:
	CGUISpinControlEx *m_pSpin;
};

class CButtonSettingControl : public CBaseSettingControl
{
public:
	CButtonSettingControl(CGUIButtonControl* pButton, int id, CSetting *pSetting);
	virtual ~CButtonSettingControl();
	virtual bool OnClick();
	virtual void Update();

private:
	CGUIButtonControl *m_pButton;
};

class CEditSettingControl : public CBaseSettingControl
{
public:
	CEditSettingControl(CGUIEditControl* pButton, int id, CSetting *pSetting);
	virtual ~CEditSettingControl();
	virtual bool OnClick();
	virtual void Update();
	virtual bool NeedsUpdate() { return m_needsUpdate; };
	virtual void Reset() { m_needsUpdate = false; };

private:
	bool IsValidIPAddress(const CStdString &strIP);
	CGUIEditControl *m_pEdit;
	bool m_needsUpdate;
};

class CSeparatorSettingControl : public CBaseSettingControl
{
public:
	CSeparatorSettingControl(CGUIImage* pImage, int id, CSetting *pSetting);
	virtual ~CSeparatorSettingControl();
	virtual bool OnClick() { return false; };
	virtual void Update() {};

private:
	CGUIImage *m_pImage;
};

#endif //CSETTINGSCONTROLS_H