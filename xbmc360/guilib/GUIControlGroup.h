#ifndef H_CGUICONTROLGROUP
#define H_CGUICONTROLGROUP

#include "GUIControl.h"
#include "Geometry.h"
#include <map>

class CGUIControlGroup : public CGUIControl
{
public:
	CGUIControlGroup();
	CGUIControlGroup(int parentID, int controlID, float posX, float posY, float width, float height);
	virtual ~CGUIControlGroup(void);

	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);

	virtual void Render();
	virtual void DoRender(unsigned int currentTime);

	virtual void AddControl(CGUIControl *control, int position = -1);
	virtual void ClearAll();
	virtual bool IsGroup() const { return true; };

protected:
 	// Return the coordinates of the top left of the group, in the group's 
	// parent coordinates return The top left coordinates of the group
	virtual CPoint GetPosition() const { return CPoint(m_posX, m_posY); };


	// Sub controls
	std::vector<CGUIControl *> m_children;
	typedef std::vector<CGUIControl *>::iterator iControls;
	typedef std::vector<CGUIControl *>::const_iterator ciControls;
	typedef std::vector<CGUIControl *>::reverse_iterator rControls;
	typedef std::vector<CGUIControl *>::const_reverse_iterator crControls;

	// Fast lookup by id
	typedef std::multimap<int, CGUIControl *> LookupMap;
	void AddLookup(CGUIControl *control);
	void RemoveLookup(CGUIControl *control);
	const LookupMap &GetLookup() { return m_lookup; };
	LookupMap m_lookup;

	int  m_defaultControl;
	bool m_defaultAlways;
	int m_focusedControl;
	bool m_renderFocusedLast;

	// Render time
	unsigned int m_renderTime;
};

#endif //H_CGUICONTROLGROUP