#ifndef CGUIBOARDEREDIMAGE_H
#define CGUIBOARDEREDIMAGE_H

#include "GUIControl.h"
#include "TextureManager.h"
#include "GUIImage.h"

class CGUIBorderedImage : public CGUIImage
{
public:
	CGUIBorderedImage(int parentID, int controlID, float posX, float posY, float width, float height, const CTextureInfo& texture, const CTextureInfo& borderTexture, const FRECT &borderSize);
	CGUIBorderedImage(const CGUIBorderedImage &right);
	virtual ~CGUIBorderedImage(void);
	virtual CGUIBorderedImage *Clone() const { return new CGUIBorderedImage(*this); };

	virtual void Render();
	virtual void PreAllocResources();
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);

protected:
	CGUITexture m_borderImage;
	FRECT m_borderSize;
};

#endif //CGUIBOARDEREDIMAGE_H
