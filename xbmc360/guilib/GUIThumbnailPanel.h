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
	
	void RenderItem(bool bFocus, float fPosX, float fPosY, CGUIListItem* pItem, int iStage);
	void RenderText(float fPosX, float fPosY, DWORD dwTextColor, WCHAR* wszText, bool bScroll);
	virtual void Render();
	virtual bool OnMessage(CGUIMessage& message);

	virtual void AllocResources();
	virtual void FreeResources();
	virtual bool OnAction(const CAction &action);

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

	int m_iItemWidth;
	int m_iItemHeight;

	float m_fThumbTexWidth;
	float m_fThumbTexHeight;

	CLabelInfo m_label;
	std::wstring m_strSuffix;

	int m_iScrollBar;
	int m_iObjectCounterLabel;

	CGUITextureD3D m_imgFocus;
	CGUITextureD3D m_imgNoFocus;

	std::vector<CGUIListItem*> m_vecItems;
};
#endif //GUILIB_THUMBNAILPANEL_H