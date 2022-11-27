#ifndef GUILIB_CGUILISTITEMLAYOUT_H
#define GUILIB_CGUILISTITEMLAYOUT_H

#include "GUIListGroup.h"
#include "GUITexture.h"

class CGUIListItem;
class CFileItem;
class CLabelInfo;

class CGUIListItemLayout
{
public:
	CGUIListItemLayout();
	CGUIListItemLayout(const CGUIListItemLayout &from);
	virtual ~CGUIListItemLayout();
	void LoadLayout(TiXmlElement *layout, bool focused);
	void Render(CGUIListItem *item, int parentID, unsigned int time = 0);
	float Size(ORIENTATION orientation) const;
	unsigned int GetFocusedItem() const;
	void SetFocusedItem(unsigned int focus);
	bool IsAnimating(ANIMATION_TYPE animType);
	void ResetAnimation(ANIMATION_TYPE animType);
	void SetInvalid() { m_invalidated = true; };
	void FreeResources(bool immediately = false);

//#ifdef PRE_SKIN_VERSION_9_10_COMPATIBILITY
	void CreateListControlLayouts(float width, float height, bool focused, const CLabelInfo &labelInfo, const CLabelInfo &labelInfo2, const CTextureInfo &texture, const CTextureInfo &textureFocus, float texHeight, float iconWidth, float iconHeight, int nofocusCondition, int focusCondition);
//#endif

	void SelectItemFromPoint(const CPoint &point);
	bool MoveLeft();
	bool MoveRight();

	int GetCondition() const { return m_condition; };
#ifdef _DEBUG
	virtual void DumpTextureUse();
#endif

protected:
	void LoadControl(TiXmlElement *child, CGUIControlGroup *group);
	void Update(CFileItem *item);

	CGUIListGroup m_group;

	float m_width;
	float m_height;
	bool m_focused;
	bool m_invalidated;

	int m_condition;
	bool m_isPlaying;
};

#endif //GUILIB_CGUILISTITEMLAYOUT_H