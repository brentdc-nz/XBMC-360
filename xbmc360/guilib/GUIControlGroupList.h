#ifndef GUILIB_CGUICONTROLLISTGROUP_H
#define GUILIB_CGUICONTROLLISTGROUP_H

#include "GUIControlGroup.h"

class CGUIControlGroupList : public CGUIControlGroup
{
public:
	CGUIControlGroupList(int parentID, int controlID, float posX, float posY, float width, float height, float itemGap, int pageControl, ORIENTATION orientation, bool useControlPositions, uint32_t alignment, unsigned int scrollTime);
	virtual ~CGUIControlGroupList(void);
	virtual CGUIControlGroupList *Clone() const { return new CGUIControlGroupList(*this); };

	virtual void Render();
	virtual bool OnMessage(CGUIMessage& message);

#ifdef _HAS_MOUSE
	virtual bool SendMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif
	virtual void UnfocusFromPoint(const CPoint &point);

	virtual void AddControl(CGUIControl *control, int position = -1);
	virtual void ClearAll();

	virtual bool GetCondition(int condition, int data) const;

protected:
#ifdef _HAS_MOUSE
	virtual bool OnMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif
	bool IsFirstFocusableControl(const CGUIControl *control) const;
	bool IsLastFocusableControl(const CGUIControl *control) const;
	void ValidateOffset();
	inline float Size(const CGUIControl *control) const;
	inline float Size() const;
	void ScrollTo(float offset);
	float GetAlignOffset() const;

	float m_itemGap;
	int m_pageControl;

	float m_offset; // Measurement in pixels of our origin
	float m_totalSize;

	float m_scrollSpeed;
	float m_scrollOffset;
	unsigned int m_scrollLastTime;
	unsigned int m_scrollTime;

	bool m_useControlPositions;
	ORIENTATION m_orientation;
	uint32_t m_alignment;
};

#endif //GUILIB_CGUICONTROLLISTGROUP_H