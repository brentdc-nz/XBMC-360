#ifndef GUILIB_GUIPROGRESSCONTROL_H
#define GUILIB_GUIPROGRESSCONTROL_H

#include "GUITexture.h"
#include "GUIControl.h"

class CGUIProgressControl :
	public CGUIControl
{
public:
	CGUIProgressControl(int parentID, int controlID, float posX, float posY,
						float width, float height, const CTextureInfo& backGroundTexture,
						const CTextureInfo& leftTexture, const CTextureInfo& midTexture,
						const CTextureInfo& rightTexture, const CTextureInfo& overlayTexture,
						bool reveal=false);
	virtual ~CGUIProgressControl(void);
	virtual CGUIProgressControl *Clone() const { return new CGUIProgressControl(*this); };

	virtual void Render();
	virtual bool CanFocus() const;
	virtual void PreAllocResources();
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual bool OnMessage(CGUIMessage& message);
	virtual void SetPosition(float posX, float posY);
	void SetPercentage(float fPercent);
	void SetInfo(int iInfo);
	int GetInfo() const {return m_iInfoCode;};

	float GetPercentage() const;
	CStdString GetDescription() const;

protected:
	virtual void UpdateColors();
	CGUITexture m_guiBackground;
	CGUITexture m_guiLeft;
	CGUITexture m_guiMid;
	CGUITexture m_guiRight;
	CGUITexture m_guiOverlay;
	int m_iInfoCode;
	float m_fPercent;
	bool m_bReveal;
};

#endif //GUILIB_GUIPROGRESSCONTROL_H