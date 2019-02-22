#ifndef GUILIB_THUMBNAILPANEL_H
#define GUILIB_THUMBNAILPANEL_H

#include "GUIControl.h"
#include "GUIListItem.h"
#include "GUILabel.h"
#include "GUIImage.h"

class CGUIThumbnailPanel : public CGUIControl
{
public:
	CGUIThumbnailPanel(int parentID, int controlID, float posX, float posY, float width, float height, float thumbWidth, float thumbHeight, float thumbTexWidth, float thumbTexHeight, const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus, const CLabelInfo& labelInfo);
	virtual ~CGUIThumbnailPanel(void);
	
	void RenderItem(bool bFocus, float iPosX, float iPosY, CGUIListItem* pItem);
	void RenderText(float fPosX, float fPosY, DWORD dwTextColor, WCHAR* wszText, bool bScroll);
	virtual void Render();
	virtual bool OnMessage(CGUIMessage& message);

	virtual void AllocResources();
	virtual void FreeResources();
	virtual bool CGUIThumbnailPanel::OnAction(const CAction &action);

	virtual void OnUp();
	virtual void OnDown();
	virtual void OnLeft();
	virtual void OnRight();

	void SetScrollbarControl(const int iControl);
	void SetObjectLabelControl(const int iControl);

private:
	bool ValidItem(int iX, int iY);
	void UpdateCounterLabel();
	void OnPageUp();
	void OnPageDown();

	int m_iColumns;
	int m_iRows;

	int m_iOffset;
	int m_iCursorX;
	int m_iCursorY;

	int m_iPage;
	int m_iTotalPages;

	bool m_bScrollUp;
	bool m_bScrollDown;

	int m_iScrollCounter;

	RECT m_RCScissorRect;

	CGUID3DTexture m_imgFocus;
	CGUID3DTexture m_imgNoFocus;

	int m_iItemWidth;
	int m_iItemHeight;

	int m_iThumbTexWidth;
	int m_iThumbTexHeight;

	CLabelInfo m_label;
	std::wstring m_strSuffix;

	int m_iScrollBar;
	int m_iObjectCounterLabel;

	std::vector<CGUIListItem*> m_vecItems;
};
#endif //GUILIB_THUMBNAILPANEL_H