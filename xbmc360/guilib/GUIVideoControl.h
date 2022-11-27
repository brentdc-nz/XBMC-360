#ifndef GUILIB_GUIVIDEOCONTROL_H
#define GUILIB_GUIVIDEOCONTROL_H

#include "GUIControl.h"

class CGUIVideoControl :
	public CGUIControl
{
public:
	CGUIVideoControl(int parentID, int controlID, float posX, float posY, float width, float height);
	virtual ~CGUIVideoControl(void);
	virtual CGUIVideoControl *Clone() const { return new CGUIVideoControl(*this); };

	virtual void Render();
#ifdef _HAS_MOUSE
	virtual bool OnMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif
	virtual bool CanFocus() const;
	virtual bool CanFocusFromPoint(const CPoint &point) const;
};

#endif //GUILIB_GUIVIDEOCONTROL_H