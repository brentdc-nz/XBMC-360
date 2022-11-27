#ifndef GUILIB_GUITOGGLEBUTTONCONTROL_H
#define GUILIB_GUITOGGLEBUTTONCONTROL_H

#include "GUIButtonControl.h"

class CGUIToggleButtonControl : public CGUIButtonControl
{
public:
	CGUIToggleButtonControl(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus, const CTextureInfo& altTextureFocus, const CTextureInfo& altTextureNoFocus, const CLabelInfo &labelInfo);
	virtual ~CGUIToggleButtonControl(void);

	virtual CGUIToggleButtonControl *Clone() const { return new CGUIToggleButtonControl(*this); };

	virtual void Render();
	virtual bool OnAction(const CAction &action);
	virtual void PreAllocResources();
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual void SetPosition(float posX, float posY);
	virtual void SetWidth(float width);
	virtual void SetHeight(float height);
	void SetLabel(const std::string& strLabel);
	void SetAltLabel(const std::string& label);
	virtual CStdString GetLabel() const;
	void SetToggleSelect(int toggleSelect) { m_toggleSelect = toggleSelect; };
	void SetAltClickActions(const CGUIAction &clickActions);

protected:
	virtual void UpdateColors();
	virtual void OnClick();
	virtual void SetInvalid();
	CGUIButtonControl m_selectButton;
	int m_toggleSelect;
};

#endif //GUILIB_GUITOGGLEBUTTONCONTROL_H