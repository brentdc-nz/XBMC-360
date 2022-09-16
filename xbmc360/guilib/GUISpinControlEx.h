#ifndef GUILIB_SPINCONTROLEX_H
#define GUILIB_SPINCONTROLEX_H

#include "GUISpinControl.h"
#include "GUIButtonControl.h"
#include "utils\log.h"

class CGUISpinControlEx : public CGUISpinControl
{
public:
	CGUISpinControlEx(int parentID, int controlID, float posX, float posY, float width, float height, float spinWidth, float spinHeight, const CLabelInfo& spinInfo, const CTextureInfo &textureFocus, const CTextureInfo &textureNoFocus, const CTextureInfo& textureUp, const CTextureInfo& textureDown, const CTextureInfo& textureUpFocus, const CTextureInfo& textureDownFocus, const CLabelInfo& labelInfo, int iType);
	virtual ~CGUISpinControlEx(void);

	virtual void Render();

	virtual void SetPosition(float posX, float posY);

	virtual void AllocResources();
	virtual void FreeResources();

	virtual float GetXPosition() const { return m_buttonControl.GetXPosition();};
	virtual float GetYPosition() const { return m_buttonControl.GetYPosition();};

	void SetText(const string & aLabel) { m_buttonControl.SetLabel(aLabel); };
	const CStdString GetCurrentLabel() const;
protected:
	CGUIButtonControl m_buttonControl;
	int m_spinPosX;

};

#endif //GUILIB_SPINCONTROLEX_H