#ifndef H_CGUICONTROL
#define H_CGUICONTROL

#include "GraphicContext.h" // Needed by any rendering operation (all controls)
#include "Key.h"            // Needed by practically all controls (CAction + defines)
#include "GUIMessage.h"     // Needed by practically all controls
#include "VisibleEffect.h"
#include "GUIInfoTypes.h"
#include "GUIAction.h"

enum ORIENTATION { HORIZONTAL = 0, VERTICAL };

class CControlState
{
public:
	CControlState(int id, int data)
	{
		m_id = id;
		m_data = data;
	}
	int m_id;
	int m_data;
};

class CGUIControl
{
public:
	CGUIControl();
	CGUIControl(int parentID, int controlID, float posX, float posY, float width, float height);
	virtual ~CGUIControl(void);
	virtual CGUIControl *Clone() const = 0;

	virtual void DoRender(unsigned int currentTime);
	virtual void Render();
	bool HasRendered() const { return m_hasRendered; };

	// OnAction() is called by our window when we are the focused control.
	// We should process any control-specific actions in the derived classes,
	// and return true if we have taken care of the action.  Returning false
	// indicates that the message may be handed down to the window or application
	// levels.  This base class implementation handles basic movement, and should
	// be called from the derived classes when the action has not been handled.
	// Return true to indicate that the action has been dealt with.
	virtual bool OnAction(const CAction &action);

	// Common actions to make the code easier to read (no ugly switch statements in derived controls)
	virtual void OnUp();
	virtual void OnDown();
	virtual void OnLeft();
	virtual void OnRight();
	virtual bool OnBack();
	virtual void OnNextControl();
	virtual void OnPrevControl();
	virtual void OnFocus() {};
	virtual void OnUnFocus() {};

	// React to a mouse event
	// Mouse events are sent from the window to all controls, and each control can react based on the event
	// and location of the event.
	// point the location in transformed skin coordinates from the upper left corner of the parent control.
	// event the mouse event to perform
	// return true if the control has handled this event, false otherwise
#ifdef _HAS_MOUSE
  virtual bool SendMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif

	// Perform a mouse action
	// Mouse actions are sent from the window to all controls, and each control can react based on the event
	// and location of the actions.

	// point the location in transformed skin coordinates from the upper left corner of the parent control.
	// event the mouse event to perform
	// return true if the control has handled this event, false otherwise
#ifdef _HDS_MOUSE
  virtual bool OnMouseEvent(const CPoint &point, const CMouseEvent &event) { return false; };
#endif

	// Unfocus the control if the given point on screen is not within it's boundary
	// point the location in transformed skin coordinates from the upper left corner of the parent control.
	// CanFocusFromPoin
	virtual void UnfocusFromPoint(const CPoint &point);

	// Used to test whether the point is inside a control.
	// param point location to test
	// return true if the point is inside the bounds of this control
	virtual bool HitTest(const CPoint &point) const;

	virtual bool OnMessage(CGUIMessage& message);
	virtual int GetID(void) const;
	void SetID(int id) { m_controlID = id; };
	virtual bool HasID(int id) const;
	virtual bool HasVisibleID(int id) const;
	int GetParentID() const;
	virtual bool HasFocus() const;
	virtual void PreAllocResources() {}
	virtual void AllocResources();
	virtual void FreeResources(bool immediately = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual bool IsDynamicallyAllocated() { return false; };
	virtual bool CanFocus() const;
	virtual bool IsVisible() const;
	bool IsVisibleFromSkin() const { return m_visibleFromSkinCondition; };
	virtual bool IsDisabled() const;
	virtual void SetPosition(float posX, float posY);
	virtual void SetHitRect(const CRect &rect);
	virtual void SetCamera(const CPoint &camera);
	void SetColorDiffuse(const CGUIInfoColor &color);
	CPoint GetRenderPosition() const;
	virtual float GetXPosition() const;
	virtual float GetYPosition() const;
	virtual float GetWidth() const;
	virtual float GetHeight() const;

	virtual void SetNavigation(int up, int down, int left, int right, int back = 0);
	virtual void SetTabNavigation(int next, int prev);

	// Set actions to perform on navigation
	// Navigations are set if replace is true or if there is no previously set action
	// param up CGUIAction to execute on up
	// param down CGUIAction to execute on down
	// param left CGUIAction to execute on left
	// param right CGUIAction to execute on right
	// param back CGUIAction to execute on back
	// param replace Actions are set only if replace is true or there is no previously set action.  Defaults to true
	// sa SetNavigation
	virtual void SetNavigationActions(const CGUIAction &up, const CGUIAction &down,
										const CGUIAction &left, const CGUIAction &right,
										const CGUIAction &back, bool replace = true);
	int GetControlIdUp() const { return m_actionUp.GetNavigation(); };
	int GetControlIdDown() const { return  m_actionDown.GetNavigation(); };
	int GetControlIdLeft() const { return m_actionLeft.GetNavigation(); };
	int GetControlIdRight() const { return m_actionRight.GetNavigation(); };
	int GetControlIdBack() const { return m_actionBack.GetNavigation(); };
	virtual int GetNextControl(int direction) const;
	virtual void SetFocus(bool focus);
	virtual void SetWidth(float width);
	virtual void SetHeight(float height);
	virtual void SetVisible(bool bVisible);
	void SetVisibleCondition(int visible, const CGUIInfoBool &allowHiddenFocus);
	int GetVisibleCondition() const { return m_visibleCondition; };
	void SetEnableCondition(int condition);
	virtual void UpdateVisibility(const CGUIListItem *item = NULL);
	virtual void SetInitialVisibility();
	virtual void SetEnabled(bool bEnable);
	virtual void SetInvalid() { m_bInvalidated = true; };
	virtual void SetPulseOnSelect(bool pulse) { m_pulseOnSelect = pulse; };
	virtual CStdString GetDescription() const { return ""; };

	void SetAnimations(const std::vector<CAnimation> &animations);
	const std::vector<CAnimation> &GetAnimations() const { return m_animations; };

	virtual void QueueAnimation(ANIMATION_TYPE anim);
	virtual bool IsAnimating(ANIMATION_TYPE anim);
	virtual bool HasAnimation(ANIMATION_TYPE anim);
	CAnimation *GetAnimation(ANIMATION_TYPE type, bool checkConditions = true);
	virtual void ResetAnimation(ANIMATION_TYPE type);
	virtual void ResetAnimations();

	// Push information updates
	virtual void UpdateInfo(const CGUIListItem *item = NULL) {};
	virtual void SetPushUpdates(bool pushUpdates) { m_pushedUpdates = pushUpdates; };

	virtual bool IsGroup() const { return false; };
	virtual bool IsContainer() const { return false; };
	virtual bool GetCondition(int condition, int data) const { return false; };

	void SetParentControl(CGUIControl *control) { m_parentControl = control; };
	virtual void SaveStates(std::vector<CControlState> &states);

	enum GUICONTROLTYPES {
		GUICONTROL_UNKNOWN,
		GUICONTROL_BUTTON,
		GUICONTROL_CHECKMARK,
		GUICONTROL_FADELABEL,
		GUICONTROL_IMAGE,
		GUICONTROL_BORDEREDIMAGE,
		GUICONTROL_LARGE_IMAGE,
		GUICONTROL_LABEL,
		GUICONTROL_LIST,
		GUICONTROL_LISTGROUP,
		GUICONTROL_LISTEX,
		GUICONTROL_PROGRESS,
		GUICONTROL_RADIO,
		GUICONTROL_RSS,
		GUICONTROL_SELECTBUTTON,
		GUICONTROL_SLIDER,
		GUICONTROL_SETTINGS_SLIDER,
		GUICONTROL_SPINBUTTON,
		GUICONTROL_SPIN,
		GUICONTROL_SPINEX,
		GUICONTROL_TEXTBOX,
		GUICONTROL_THUMBNAIL,
		GUICONTROL_TOGGLEBUTTON,
		GUICONTROL_VIDEO,
		GUICONTROL_MOVER,
		GUICONTROL_RESIZE,
		GUICONTROL_BUTTONBAR,
		GUICONTROL_CONSOLE,
		GUICONTROL_EDIT,
		GUICONTROL_VISUALISATION,
		GUICONTROL_MULTI_IMAGE,
		GUICONTROL_GROUP,
		GUICONTROL_GROUPLIST,
		GUICONTROL_SCROLLBAR,
		GUICONTROL_LISTLABEL,
		GUICONTROL_MULTISELECT,
		GUICONTAINER_LIST,
		GUICONTAINER_WRAPLIST,
		GUICONTAINER_FIXEDLIST,
		GUICONTAINER_PANEL
	};

	GUICONTROLTYPES GetControlType() const { return ControlType; }

	enum GUIVISIBLE { HIDDEN = 0, DELAYED, VISIBLE };

#ifdef _DEBUG
	virtual void DumpTextureUse() {};
#endif

protected:
	// Called when the mouse is over the control.
	// Default implementation selects the control.
	// param point location of the mouse in transformed skin coordinates
	// return true if handled, false otherwise.
#ifdef _HAS_MOUSE
	virtual bool OnMouseOver(const CPoint &point);
#endif

	// Test whether we can focus a control from a point on screen
	// param point the location in vanilla skin coordinates from the upper left corner of the parent control.
	// return true if the control can be focused from this location
	// sa UnfocusFromPoint, HitRect
	virtual bool CanFocusFromPoint(const CPoint &point) const;

	virtual void UpdateColors();
	virtual void Animate(unsigned int currentTime);
	virtual bool CheckAnimation(ANIMATION_TYPE animType);
	void UpdateStates(ANIMATION_TYPE type, ANIMATION_PROCESS currentProcess, ANIMATION_STATE currentState);
	bool SendWindowMessage(CGUIMessage &message);

	// Navigation and actions
	CGUIAction m_actionLeft;
	CGUIAction m_actionRight;
	CGUIAction m_actionUp;
	CGUIAction m_actionDown;
	CGUIAction m_actionBack;
	CGUIAction m_actionNext;
	CGUIAction m_actionPrev;

	float m_posX;
	float m_posY;
	float m_height;
	float m_width;
	CRect m_hitRect;
	CGUIInfoColor m_diffuseColor;
	int m_controlID;
	int m_parentID;
	bool m_bHasFocus;
	bool m_bInvalidated;
	bool m_bAllocated;
	bool m_pulseOnSelect;
	GUICONTROLTYPES ControlType;

	CGUIControl *m_parentControl; // Our parent control if we're part of a group

	// Visibility condition/state
	int m_visibleCondition;
	GUIVISIBLE m_visible;
	bool m_visibleFromSkinCondition;
	bool m_forceHidden; // Set from the code when a hidden operation is given - overrides m_visible
	CGUIInfoBool m_allowHiddenFocus;
	bool m_hasRendered;
	// enable/disable state
	int m_enableCondition;
	bool m_enabled;

	bool m_pushedUpdates;

	// Animation effects
	std::vector<CAnimation> m_animations;
	CPoint m_camera;
	bool m_hasCamera;
	TransformMatrix m_transform;
};

#endif //H_CGUICONTROL