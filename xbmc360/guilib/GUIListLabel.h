#ifndef CGUILISTLABEL_H
#define CGUILISTLABEL_H

#include "GUIControl.h"
#include "GUILabel.h"

class CGUIListLabel :
	public CGUIControl
{
public:
	CGUIListLabel(int parentID, int controlID, float posX, float posY, float width, float height, const CLabelInfo& labelInfo, const CGUIInfoLabel &label, bool alwaysScroll);
	virtual ~CGUIListLabel(void);
	virtual CGUIListLabel *Clone() const { return new CGUIListLabel(*this); };

	virtual void Render();
	virtual bool CanFocus() const { return false; };
	virtual void UpdateInfo(const CGUIListItem *item = NULL);
	virtual void SetFocus(bool focus);
	virtual void SetInvalid();

	void SetLabel(const CStdString &label);
	void SetSelected(bool selected);
	void SetScrolling(bool scrolling);

	static void CheckAndCorrectOverlap(CGUIListLabel &label1, CGUIListLabel &label2)
	{
		CGUILabel::CheckAndCorrectOverlap(label1.m_label, label2.m_label);
	}

protected:
	virtual void UpdateColors();

	CGUILabel     m_label;
	CGUIInfoLabel m_info;
	bool          m_alwaysScroll;
};

#endif //CGUILISTLABEL_H