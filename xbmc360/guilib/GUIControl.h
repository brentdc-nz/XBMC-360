/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#ifndef H_CGUICONTROL
#define H_CGUICONTROL

#include "GraphicContext.h" // needed by any rendering operation (all controls)
#include "GUIMessage.h"     // needed by practically all controls
#include "Key.h"            // needed by practically all controls (CAction + defines)
#include "GUIInfoTypes.h"

enum ORIENTATION 
{ 
	HORIZONTAL = 0, 
	VERTICAL 
};

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
	//virtual CGUIControl *Clone() const=0;

	// OnAction() is called by our window when we are the focused control.
	// We should process any control-specific actions in the derived classes,
	// and return true if we have taken care of the action.  Returning false
	// indicates that the message may be handed down to the window or application
	// levels.  This base class implementation handles basic movement, and should
	// be called from the derived classes when the action has not been handled.
	// Return true to indicate that the action has been dealt with.
	virtual bool OnAction(const CAction &action);

	  /*! \brief Unfocus the control if the given point on screen is not within it's boundary
		  \param point the location in transformed skin coordinates from the upper left corner of the parent control.
		  \sa CanFocusFromPoint
	   */
	virtual void UnfocusFromPoint(const CPoint &point);

   /*! \brief Used to test whether the point is inside a control.
   \param point location to test
   \return true if the point is inside the bounds of this control.
   \sa SetHitRect
   */
    virtual bool HitTest(const CPoint &point) const;

	// Common actions to make the code easier to read (no ugly switch statements in derived controls)
	virtual void OnUp();
	virtual void OnDown();
	virtual void OnLeft();
	virtual void OnRight();
	virtual void OnFocus() {};

	virtual void DoRender(unsigned int currentTime);
	virtual void Render();
	virtual bool OnMessage(CGUIMessage& message);

	virtual void SetEnabled(bool bEnable);
	virtual bool CanFocus() const;
	virtual bool IsDisabled() const;
	virtual bool IsVisible() const;
	virtual void SetVisible(bool bVisible);
	virtual void SetVisibleCondition(int visible/*, const CGUIInfoBool &allowHiddenFocus*/);
	virtual void SetPosition(float posX, float posY);

	virtual int GetID(void) const;
	void SetID(int id) { m_controlID = id; };
	DWORD GetParentID(void) const;
	bool HasFocus(void) const;
	virtual bool HasID(int id) const;
	 virtual bool HasVisibleID(int id) const;
	virtual void SetNavigation(DWORD dwUp, DWORD dwDown, DWORD dwLeft, DWORD dwRight);
	DWORD GetControlIdUp() const { return m_dwControlUp;};
	DWORD GetControlIdDown() const { return m_dwControlDown;};
	DWORD GetControlIdLeft() const { return m_dwControlLeft;};
	DWORD GetControlIdRight() const { return m_dwControlRight;};
	virtual DWORD GetNextControl(int direction) const;
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual bool IsDynamicallyAllocated() { return false; };
	virtual bool IsAllocated() { return m_bAllocated; };
	virtual void PreAllocResources() {};
	virtual void AllocResources();
	virtual void FreeResources();
	virtual void Update() {};

	void SetFocus(bool bOnOff);

	float GetXPosition() const;
	float GetYPosition() const;
	float GetWidth() const;
	float GetHeight() const;
	virtual void SetWidth(float iWidth);
	virtual void SetHeight(float iHeight);
	virtual bool IsGroup() const { return false; };
	void SetParentControl(CGUIControl *control) { m_parentControl = control; };
	virtual void SetPushUpdates(bool pushUpdates) { m_pushedUpdates = pushUpdates; };
	virtual bool IsContainer() const { return false; };
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
		GUICONTAINER_WRAPLIST,
		GUICONTAINER_FIXEDLIST,
		GUICONTAINER_THUMBNAILPANEL
	};
	GUICONTROLTYPES GetControlType() const { return ControlType; }

protected:
	void SendWindowMessage(CGUIMessage &message);

	int m_controlID;
	bool m_bAllocated;

	int m_parentID;
	bool m_bHasFocus;
	bool m_bInvalidated;

	float m_posX;
	float m_posY;
	float m_width;
	float m_height;

	DWORD m_dwControlLeft;
	DWORD m_dwControlRight;
	DWORD m_dwControlUp;
	DWORD m_dwControlDown;

	// visibility condition/state
	int m_visibleCondition;
	bool m_visible;
	bool m_visibleFromSkinCondition;
	bool m_forceHidden;       // set from the code when a hidden operation is given - overrides m_visible
//	CGUIInfoBool m_allowHiddenFocus; //Setup CGUIBool?
	bool m_hasRendered;
	// enable/disable state
	int m_enableCondition;
	bool m_enabled;
	CGUIControl *m_parentControl;
	GUICONTROLTYPES ControlType;
	TransformMatrix m_transform;
	bool m_pushedUpdates;
	CRect m_hitRect;


};

#endif //H_CGUICONTROL