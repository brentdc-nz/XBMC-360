#ifndef GUILIB_GUIIMAGECONTROL_H
#define GUILIB_GUIIMAGECONTROL_H

#include "GUIControl.h"
#include "GUIInfoTypes.h"

#include "GUITextureD3D.h"

class CGUIImage : public CGUIControl
{
public:
	CGUIImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture);
	virtual ~CGUIImage(void);

	virtual void AllocResources();
	virtual void FreeResources();
	virtual void SetInfo(const CGUIInfoLabel &info);
	void SetFileName(const CStdString strFileName);
	virtual void Update();
	virtual void Render();

protected:
	CGUIInfoLabel m_info;
	CTextureInfo m_image;
	CGUITextureD3D m_texture;
};

#endif //GUILIB_GUIIMAGECONTROL_H