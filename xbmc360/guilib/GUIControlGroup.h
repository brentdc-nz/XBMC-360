#ifndef H_CGUICONTROLGROUP
#define H_CGUICONTROLGROUP

#include "GUIControl.h"
#include <map>

// Group of controls, useful for remembering last control + animating/hiding together
class CGUIControlGroup : public CGUIControl
{
public:
	CGUIControlGroup();
	CGUIControlGroup(int parentID, int controlID, float posX, float posY, float width, float height);
	CGUIControlGroup(const CGUIControlGroup &from);
	virtual ~CGUIControlGroup(void);
	virtual CGUIControlGroup *Clone() const { return new CGUIControlGroup(*this); };

	virtual void Render();
	virtual bool OnAction(const CAction &action);
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool SendControlMessage(CGUIMessage& message);
	virtual bool HasFocus() const;
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual bool CanFocus() const;

#ifdef _HAS_MOUSE
	virtual bool SendMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif
	virtual void UnfocusFromPoint(const CPoint &point);

	virtual void SetInitialVisibility();

	virtual void DoRender(unsigned int currentTime);
	virtual bool IsAnimating(ANIMATION_TYPE anim);
	virtual bool HasAnimation(ANIMATION_TYPE anim);
	virtual void QueueAnimation(ANIMATION_TYPE anim);
	virtual void ResetAnimation(ANIMATION_TYPE anim);
	virtual void ResetAnimations();

	virtual bool HasID(int id) const;
	virtual bool HasVisibleID(int id) const;

	int GetFocusedControlID() const;
	CGUIControl *GetFocusedControl() const;
	const CGUIControl *GetControl(int id) const;
	virtual CGUIControl *GetFirstFocusableControl(int id);
	void GetContainers(std::vector<CGUIControl *> &containers) const;

	virtual void AddControl(CGUIControl *control, int position = -1);
	bool InsertControl(CGUIControl *control, const CGUIControl *insertPoint);
	virtual bool RemoveControl(const CGUIControl *control);
	virtual void ClearAll();
	void SetDefaultControl(int id, bool always) { m_defaultControl = id; m_defaultAlways = always; };
	void SetRenderFocusedLast(bool renderLast) { m_renderFocusedLast = renderLast; };

	virtual void SaveStates(std::vector<CControlState> &states);

	virtual bool IsGroup() const { return true; };

#ifdef _DEBUG
	virtual void DumpTextureUse();
#endif

protected:
 	// Return the coordinates of the top left of the group, in the group's parent coordinates
	// return The top left coordinates of the group
	virtual CPoint GetPosition() const { return CPoint(m_posX, m_posY); };
  
	// Check whether a given control is valid
	// Runs through controls and returns whether this control is valid.  Only functional
	// for controls with non-zero id.
	// param control to check
	// return true if the control is valid, false otherwise.
	bool IsValidControl(const CGUIControl *control) const;

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