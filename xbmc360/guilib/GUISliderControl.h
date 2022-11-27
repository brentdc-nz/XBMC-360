#ifndef GUILIB_GUISLIDERCONTROL_H
#define GUILIB_GUISLIDERCONTROL_H

#include "guilib\GUIControl.h"
#include "guilib\GUITexture.h"

#define SPIN_CONTROL_TYPE_INT       1
#define SPIN_CONTROL_TYPE_FLOAT     2
#define SPIN_CONTROL_TYPE_TEXT      3

class CGUISliderControl :
	public CGUIControl
{
public:
	CGUISliderControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& backGroundTexture, const CTextureInfo& mibTexture, const CTextureInfo& nibTextureFocus, int iType);
	virtual ~CGUISliderControl(void);
	virtual CGUISliderControl *Clone() const { return new CGUISliderControl(*this); };

	virtual void Render();
	virtual bool OnAction(const CAction &action);
	virtual void PreAllocResources();
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual void SetRange(int iStart, int iEnd);
	virtual void SetFloatRange(float fStart, float fEnd);
	virtual bool OnMessage(CGUIMessage& message);
	void SetInfo(int iInfo);
	void SetPercentage(int iPercent);
	int GetPercentage() const;
	void SetIntValue(int iValue);
	int GetIntValue() const;
	void SetFloatValue(float fValue);
	float GetFloatValue() const;
	void SetFloatInterval(float fInterval);
	void SetType(int iType) { m_iType = iType; };
	virtual CStdString GetDescription() const;
	void SetTextValue(const CStdString &textValue) { m_textValue = textValue; };
	// Get the current position of the slider as a proportion
	// return slider position in the range [0,1]
	float GetProportion() const;

protected:
	virtual bool HitTest(const CPoint &point) const;
#ifdef _HAS_MOUSE
	virtual bool OnMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif
	virtual void UpdateColors();
	virtual void Move(int iNumSteps);
	virtual void SetFromPosition(const CPoint &point);

	CGUITexture m_guiBackground;
	CGUITexture m_guiMid;
	CGUITexture m_guiMidFocus;
	int m_iType;

	int m_iPercent;

	int m_iValue;
	int m_iStart;
	int m_iEnd;

	float m_fValue;
	float m_fStart;
	float m_fInterval;
	float m_fEnd;

	int m_iInfoCode;
	CStdString m_textValue; // Allows overriding of the text value to be displayed (parent must update when the slider updates)
};

#endif //GUILIB_GUISLIDERCONTROL_H