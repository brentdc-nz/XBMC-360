#ifndef GUILIB_GUILISTCONTROL_H
#define GUILIB_GUILISTCONTROL_H

#include "GUIListItem.h"
#include "GUIButtonControl.h"

class CGUIListControl : public CGUIControl
{
public:
	CGUIListControl(int parentID, int controlID, float fPosX, float fPosY, float fWidth, float fHeight,
				float fSpinWidth, float fSpinHeight,
//				const CStdString& strUp, const CStdString& strDown,
//				const CStdString& strUpFocus, const CStdString& strDownFocus,
//				const CLabelInfo& spinInfo, int iSpinX, int iSpinY,
				const CLabelInfo& labelInfo, //const CLabelInfo& labelInfo2,
				const CTextureInfo& TextureButton, const CTextureInfo& TextureButtonFocus);
	virtual ~CGUIListControl(void);

	virtual void AllocResources();
	virtual void FreeResources();
	virtual bool OnMessage(CGUIMessage& message);
	virtual void Render();
	virtual bool OnAction(const CAction &action);
	virtual void OnDown();
	virtual void OnUp();
	void OnPageUp();
	void OnPageDown();
	void SetSpace(int iHeight);
	void SetScrollbarControl(const int iControl);
	void SetObjectLabelControl(const int iControl);

protected:
	class CListText
	{
		public:
		CListText() { x = y = width = maxwidth = height = 0.0f; selected = highlighted = false; };
		CStdStringW text;
		float x;
		float y;
		float width;
		float maxwidth;
		float height;
		bool selected;
		bool highlighted;
	};
	void RenderText(const CListText &text, const CLabelInfo &label/*, CScrollInfo &scroll*/);

private:
	void SetHeight(float iHeight);
	void UpdateCounterLabel();

	float m_iItemHeight;
	float m_iImageWidth;
	float m_iImageHeight;
	int m_iSpaceBetweenItems;
	int m_iCursorY;
	int m_iItemsPerPage;
	int m_iOffset;
	int m_iScrollBar;
	int m_iObjectCounterLabel;
	int m_iPage;
	CLabelInfo m_label;
	CGUIButtonControl m_imgButton;
	vector<CGUIListItem*> m_vecItems;
	typedef vector<CGUIListItem*> ::iterator ivecItems;
};

#endif //GUILIB_GUILISTCONTROL_H