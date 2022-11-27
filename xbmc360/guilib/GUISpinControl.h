#ifndef GUILIB_SPINCONTROL_H
#define GUILIB_SPINCONTROL_H

#include "guilib\GUIControl.h"
#include "guilib\GUITexture.h"
#include "guilib\GUILabel.h"

#define SPIN_CONTROL_TYPE_INT    1
#define SPIN_CONTROL_TYPE_FLOAT  2
#define SPIN_CONTROL_TYPE_TEXT   3
#define SPIN_CONTROL_TYPE_PAGE   4

class CGUISpinControl : public CGUIControl
{
public:
	CGUISpinControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& textureUp, const CTextureInfo& textureDown, const CTextureInfo& textureUpFocus, const CTextureInfo& textureDownFocus, const CLabelInfo& labelInfo, int iType);
	virtual ~CGUISpinControl(void);
	virtual CGUISpinControl *Clone() const { return new CGUISpinControl(*this); };

	virtual void Render();
	virtual bool OnAction(const CAction &action);
	virtual void OnLeft();
	virtual void OnRight();
	virtual bool HitTest(const CPoint &point) const;
#ifdef _HAS_MOUSE
	virtual bool OnMouseOver(const CPoint &point);
#endif
	virtual bool OnMessage(CGUIMessage& message);
	virtual void PreAllocResources();
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual void SetPosition(float posX, float posY);
	virtual float GetWidth() const;
	void SetRange(int iStart, int iEnd);
	void SetFloatRange(float fStart, float fEnd);
	void SetValue(int iValue);
	void SetValueFromLabel(const CStdString &label);
	void SetFloatValue(float fValue);
	int GetValue() const;
	float GetFloatValue() const;
	void AddLabel(const std::string& strLabel, int iValue);
	const std::string GetLabel() const;
	void SetReverse(bool bOnOff);
	int GetMaximum() const;
	int GetMinimum() const;
	void SetSpinAlign(uint32_t align, float offsetX) { m_label.GetLabelInfo().align = align; m_label.GetLabelInfo().offsetX = offsetX; };
	void SetType(int iType) { m_iType = iType; };
	float GetSpinWidth() const { return m_imgspinUp.GetWidth(); };
	float GetSpinHeight() const { return m_imgspinUp.GetHeight(); };
	void SetFloatInterval(float fInterval);
	void SetShowRange(bool bOnoff) ;
	void SetShowOnePage(bool showOnePage) { m_showOnePage = showOnePage; };
	void Clear();
	virtual CStdString GetDescription() const;
	bool IsFocusedOnUp() const;

	virtual bool IsVisible() const;

protected:
#ifdef _HAS_MOUSE
	virtual bool OnMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif
	virtual void UpdateColors();

	// Render the spinner text
	// param posX position of the left edge of the text
	// param width width of the text
	virtual void RenderText(float posX, float width);
	CGUILabel::COLOR GetTextColor() const;
	void PageUp();
	void PageDown();
	bool CanMoveDown(bool bTestReverse = true);
	bool CanMoveUp(bool bTestReverse = true);
	void MoveUp(bool bTestReverse = true);
	void MoveDown(bool bTestReverse = true);
	void ChangePage(int amount);
	int m_iStart;
	int m_iEnd;
	float m_fStart;
	float m_fEnd;
	int m_iValue;
	float m_fValue;
	int m_iType;
	int m_iSelect;
	bool m_bReverse;
	float m_fInterval;
	std::vector<std::string> m_vecLabels;
	std::vector<int> m_vecValues;
	CGUITexture m_imgspinUp;
	CGUITexture m_imgspinDown;
	CGUITexture m_imgspinUpFocus;
	CGUITexture m_imgspinDownFocus;
	CGUILabel   m_label;
	bool m_bShowRange;
	char m_szTyped[10];
	int m_iTypedPos;

	int m_currentItem;
	int m_itemsPerPage;
	int m_numItems;
	bool m_showOnePage;
};

#endif //GUILIB_SPINCONTROL_H