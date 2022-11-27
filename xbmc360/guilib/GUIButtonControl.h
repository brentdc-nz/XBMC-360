#ifndef GUILIB_GUIBUTTONCONTROL_H
#define GUILIB_GUIBUTTONCONTROL_H

#include "GUITexture.h"
#include "GUILabel.h"
#include "GUIControl.h"

class CGUIButtonControl : public CGUIControl
{
public:
	CGUIButtonControl(int parentID, int controlID,
					float posX, float posY, float width, float height,
					const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus,
					const CLabelInfo &label);

	virtual ~CGUIButtonControl(void);
	virtual CGUIButtonControl *Clone() const { return new CGUIButtonControl(*this); };

	virtual void Render();
	virtual bool OnAction(const CAction &action) ;
	virtual bool OnMessage(CGUIMessage& message);
	virtual void PreAllocResources();
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual void SetPosition(float posX, float posY);
	virtual void SetLabel(const std::string & aLabel);
	virtual void SetLabel2(const std::string & aLabel2);
	void SetClickActions(const CGUIAction& clickActions) { m_clickActions = clickActions; };
	const CGUIAction &GetClickActions() const { return m_clickActions; };
	void SetFocusActions(const CGUIAction& focusActions) { m_focusActions = focusActions; };
	void SetUnFocusActions(const CGUIAction& unfocusActions) { m_unfocusActions = unfocusActions; };
	const CLabelInfo& GetLabelInfo() const { return m_label.GetLabelInfo(); };
	virtual CStdString GetLabel() const { return GetDescription(); };
	virtual CStdString GetLabel2() const;
	void SetSelected(bool bSelected);
	virtual CStdString GetDescription() const;
	void SetAlpha(unsigned char alpha);

	void PythonSetLabel(const CStdString &strFont, const std::string &strText, color_t textColor, color_t shadowColor, color_t focusedColor);
	void PythonSetDisabledColor(color_t disabledColor);

	void SettingsCategorySetTextAlign(uint32_t align);

	virtual void OnClick();
	bool HasClickActions() { return m_clickActions.HasActionsMeetingCondition(); };

	virtual void UpdateColors();

protected:
	friend class CGUISpinControlEx;
#ifdef _HAS_MOUSE
	virtual bool OnMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif
	void OnFocus();
	void OnUnFocus();
	virtual void RenderText();
	CGUILabel::COLOR GetTextColor() const;

	CGUITexture m_imgFocus;
	CGUITexture m_imgNoFocus;
	unsigned int  m_focusCounter;
	unsigned char m_alpha;

	CGUIInfoLabel  m_info;
	CGUIInfoLabel  m_info2;
	CGUILabel      m_label;
	CGUILabel      m_label2;

	CGUIAction m_clickActions;
	CGUIAction m_focusActions;
	CGUIAction m_unfocusActions;

	bool m_bSelected;
};

#endif //GUILIB_GUIBUTTONCONTROL_H