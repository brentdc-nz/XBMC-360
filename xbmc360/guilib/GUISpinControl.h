#ifndef GUILIB_SPINCONTROL_H
#define GUILIB_SPINCONTROL_H

#include "GUIControl.h"
#include "GUILabel.h"
#include "GUIImage.h"
#include "..\utils\Stdafx.h"
#include "..\utils\StdString.h"

#define SPIN_CONTROL_TYPE_INT    1
#define SPIN_CONTROL_TYPE_FLOAT  2
#define SPIN_CONTROL_TYPE_TEXT   3

#define SPIN_BUTTON_DOWN 1
#define SPIN_BUTTON_UP   2

class CGUISpinControl : public CGUIControl
{
public:
	CGUISpinControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& textureUp, const CTextureInfo& textureDown, const CTextureInfo& textureUpFocus, const CTextureInfo& textureDownFocus, const CLabelInfo& labelInfo, int iType);
	virtual ~CGUISpinControl(void);
	
	virtual void OnLeft();
	virtual void OnRight();

	virtual void Render();
	virtual void AllocResources();
	virtual void FreeResources();

	virtual void SetPosition(float posX, float posY);
	virtual bool OnAction(const CAction &action);

	void SetValue(int iValue);
	int GetValue() const;

	DWORD GetSpinWidth() const { return m_imgspinUp.GetWidth(); };
	DWORD GetSpinHeight() const { return m_imgspinUp.GetHeight(); };

	void AddLabel(const string& strLabel, int iValue);
	const string GetLabel() const;
	void Clear();
	void SetType(int iType);

protected:
	void MoveUp(bool bTestReverse = false);
	void MoveDown(bool bTestReverse = false);
	
	vector<string> m_vecLabels;
	vector<int> m_vecValues;

	int m_iType;

	int m_iValue;
	int m_iSelect;

	CGUIImage m_imgspinUp;
	CGUIImage m_imgspinDown;
	CGUIImage m_imgspinUpFocus;
	CGUIImage m_imgspinDownFocus;
	CLabelInfo m_label;
};

#endif //GUILIB_SPINCONTROL_H