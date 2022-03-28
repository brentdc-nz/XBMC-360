#include "GUIControl.h"
#include "GUITexture.h"
#include "GUID3DTexture.h"

#ifndef GUILIB_GUIPROGRESS_H
#define GUILIB_GUIPROGRESS_H

class CGUIProgressControl : public CGUIControl
{
public:
	CGUIProgressControl(int parentID, int controlID, float posX, float posY, 
						float width, float height, const CTextureInfo& backGroundTexture, 
						const CTextureInfo& midTexture, float min, float max);
	virtual ~CGUIProgressControl(void);

	virtual void AllocResources();
	virtual void FreeResources();
	virtual void Render();
	void SetInfo(int iInfo);
	virtual bool CanFocus() const;

protected:
	float m_RangeMin;
	float m_RangeMax;
	int m_iInfoCode;
	float m_fPercent;
	CGUID3DTexture m_guiBackground;
	CGUID3DTexture m_guiMid;
};

#endif //GUILIB_GUIPROGRESS_H