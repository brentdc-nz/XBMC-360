#ifndef GUILIB_CGUIRADIOBUTTON_H
#define GUILIB_CGUIRADIOBUTTON_H

#include "GUIButtonControl.h"

class CGUIRadioButtonControl :	public CGUIButtonControl
{
public:
  CGUIRadioButtonControl(int parentID, int controlID,
						float posX, float posY, float width, float height,
						const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus,
						const CLabelInfo& labelInfo,
						const CTextureInfo& radioOn, const CTextureInfo& radioOff);

	virtual ~CGUIRadioButtonControl(void);
	virtual CGUIRadioButtonControl *Clone() const { return new CGUIRadioButtonControl(*this); };

	virtual void Render();
	virtual bool OnAction(const CAction &action) ;
	virtual bool OnMessage(CGUIMessage& message);
	virtual void PreAllocResources();
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual void SetPosition(float posX, float posY);
	virtual void SetWidth(float width);
	virtual void SetHeight(float height);
	virtual CStdString GetDescription() const;
	void SetRadioDimensions(float posX, float posY, float width, float height);
	void SetToggleSelect(int toggleSelect) { m_toggleSelect = toggleSelect; };
	bool IsSelected() const { return m_bSelected; };

protected:
	virtual void UpdateColors();
	CGUITexture m_imgRadioOn;
	CGUITexture m_imgRadioOff;
	float m_radioPosX;
	float m_radioPosY;
	int m_toggleSelect;
};

#endif //GUILIB_CGUIRADIOBUTTON_H