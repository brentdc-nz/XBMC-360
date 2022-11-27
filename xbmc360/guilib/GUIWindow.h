#ifndef GUILIB_GUIWINDOW_H
#define GUILIB_GUIWINDOW_H

#include "GUIControlGroup.h"

#ifdef _HAS_BOOST
class CFileItem; typedef boost::shared_ptr<CFileItem> CFileItemPtr;
#else
class CFileItem; typedef std::shared_ptr<CFileItem> CFileItemPtr;
#endif

#include "GUICallback.h" // for GUIEvent

#include <map>
#include <vector>

#define ON_CLICK_MESSAGE(i,c,m) \
{ \
 GUIEventHandler<c, CGUIMessage&> clickHandler(this, &m); \
 m_mapClickEvents[i] = clickHandler; \
} \

#define ON_SELECTED_MESSAGE(i,c,m) \
{ \
 GUIEventHandler<c, CGUIMessage&> selectedHandler(this, &m); \
 m_mapSelectedEvents[i] = selectedHandler; \
} \

// Forward
class TiXmlNode;
class TiXmlElement;
class TiXmlDocument;

class COrigin
{
public:
	COrigin()
	{
		x = y = 0;
		condition = 0;
	};

	float x;
	float y;
	int condition;
};

class CGUIWindow	 : public CGUIControlGroup
{
public:
	enum WINDOW_TYPE { WINDOW = 0, MODAL_DIALOG, MODELESS_DIALOG, BUTTON_MENU, SUB_MENU };

	CGUIWindow(int id, const CStdString &xmlFile);
	virtual ~CGUIWindow(void);

	bool Initialize(); // Loads the window
	bool Load(const CStdString& strFileName, bool bContainsPath = false);

	void CenterWindow();

	// Main render function, called every frame.
	// Window classes should override this only if they need to alter how something is rendered.
	// General updating on a per-frame basis should be handled in FrameMove instead, as Render
	// is not necessarily re-entrant.
	virtual void Render();

	// Main update function, called every frame prior to rendering
	// Any window that requires updating on a frame by frame basis (such as to maintain
	// timers and the like) should override this function.
	virtual void FrameMove() {};

	// Close should never be called on this base class (only on derivatives) - its here so that window-manager can use a general close
	virtual void Close(bool forceClose = false);

	// OnAction() is called by our window manager.  We should process any messages
	// that should be handled at the window level in the derived classes, and any
	// unhandled messages should be dropped through to here where we send the message
	// on to the currently focused control.  Returns true if the action has been handled
	// and does not need to be passed further down the line (to our global action handlers)
	virtual bool OnAction(const CAction &action);
  
	virtual bool OnBack(int actionID);

	// Clear the background (if necessary) prior to rendering the window
	virtual void ClearBackground();

	bool OnMove(int fromControl, int moveAction);
	virtual bool OnMessage(CGUIMessage& message);

	bool ControlGroupHasFocus(int groupID, int controlID);
	virtual bool HasID(int id) { return (id >= m_controlID && id < m_controlID + m_idRange); };
	void SetIDRange(int range) { m_idRange = range; };
	int GetIDRange() const { return m_idRange; };
	int GetPreviousWindow() { return m_previousWindow; };
	FRECT GetScaledBounds() const;
	virtual void ClearAll();
	virtual void AllocResources(bool forceLoad = false);
	virtual void FreeResources(bool forceUnLoad = false);
	virtual void DynamicResourceAlloc(bool bOnOff);
	virtual bool IsDialog() const { return false; };
	virtual bool IsDialogRunning() const { return false; };
	virtual bool IsModalDialog() const { return false; };
	virtual bool IsMediaWindow() const { return false; };
	virtual bool HasListItems() const { return false; };
	virtual CFileItemPtr GetCurrentListItem(int offset = 0) { return CFileItemPtr(); };
	virtual int GetViewContainerID() const { return 0; };
	virtual bool IsActive() const;
	void SetCoordsRes(RESOLUTION res) { m_coordsRes = res; };
	RESOLUTION GetCoordsRes() const { return m_coordsRes; };
	void LoadOnDemand(bool loadOnDemand) { m_loadOnDemand = loadOnDemand; };
	bool GetLoadOnDemand() { return m_loadOnDemand; }
	int GetRenderOrder() { return m_renderOrder; };
	virtual void SetInitialVisibility();

	enum OVERLAY_STATE { OVERLAY_STATE_PARENT_WINDOW=0, OVERLAY_STATE_SHOWN, OVERLAY_STATE_HIDDEN };

	OVERLAY_STATE GetOverlayState() const { return m_overlayState; };

	virtual bool IsAnimating(ANIMATION_TYPE animType);
	void DisableAnimations();

	virtual void ResetControlStates();

	void SetRunActionsManually();
	void RunLoadActions();
	void RunUnloadActions();

	// Set a property
	// Sets the value of a property referenced by a key.
	// param key name of the property to set
	// param value value to set, may be a string, integer, boolean or double.
	// GetProperty
	void SetProperty(const CStdString &key, const CStdString &value);
	void SetProperty(const CStdString &key, const char *value);
	void SetProperty(const CStdString &key, int value);
	void SetProperty(const CStdString &key, bool value);
	void SetProperty(const CStdString &key, double value);

	// Retreive a property
	// param key name of the property to retrieve
	// return value of the property, empty if it doesn't exist
	// SetProperty, GetPropertyInt, GetPropertyBool, GetPropertyDouble
	CStdString GetProperty(const CStdString &key) const;

	// Retreive an integer property
	// key name of the property to retrieve
	// return value of the property, 0 if it doesn't exist
	int GetPropertyInt(const CStdString &key) const;

	// Retreive a boolean property
	// key name of the property to retrieve
	// return value of the property, false if it doesn't exist
	bool GetPropertyBool(const CStdString &key) const;

	// Retreive a double precision property
	// param key name of the property to retrieve
	// return value of the property, 0 if it doesn't exist
	double GetPropertyDouble(const CStdString &key) const;

	// Clear a all the window's properties
	// SetProperty, HasProperty, GetProperty
	void ClearProperties();

#ifdef _DEBUG
	void DumpTextureUse();
#endif

	bool HasSaveLastControl() const { return m_saveLastControl; };

protected:
#ifdef _HAS_MOUSE
	virtual bool OnMouseEvent(const CPoint &point, const CMouseEvent &event);
#endif
	virtual bool LoadXML(const CStdString& strPath, const CStdString &strLowerPath); //Loads from the given file
	bool Load(TiXmlDocument &xmlDoc); // Loads from the given XML document
	virtual void LoadAdditionalTags(TiXmlElement *root) {}; // Load additional information from the XML document

	virtual void SetDefaults();
	virtual void OnWindowUnload() {}
	virtual void OnWindowLoaded();
	virtual void OnInitWindow();
	virtual void OnDeinitWindow(int nextWindowID);
#ifdef _HAS_MOUSE
	bool OnMouseAction(const CAction &action);
#endif
	virtual bool RenderAnimation(unsigned int time);
	virtual bool CheckAnimation(ANIMATION_TYPE animType);

	CAnimation *GetAnimation(ANIMATION_TYPE animType, bool checkConditions = true);

	// Control state saving on window close
	virtual void SaveControlStates();
	virtual void RestoreControlStates();

	// Methods for updating controls and sending messages
	void OnEditChanged(int id, CStdString &text);
	bool SendMessage(int message, int id, int param1 = 0, int param2 = 0);

	typedef GUIEvent<CGUIMessage&> CLICK_EVENT;
	typedef std::map<int, CLICK_EVENT> MAPCONTROLCLICKEVENTS;
	MAPCONTROLCLICKEVENTS m_mapClickEvents;

	typedef GUIEvent<CGUIMessage&> SELECTED_EVENT;
	typedef std::map<int, SELECTED_EVENT> MAPCONTROLSELECTEDEVENTS;
	MAPCONTROLSELECTEDEVENTS m_mapSelectedEvents;

	void LoadControl(TiXmlElement* pControl, CGUIControlGroup *pGroup);

//#ifdef PRE_SKIN_VERSION_9_10_COMPATIBILITY
	void ChangeButtonToEdit(int id, bool singleLabel = false);
//#endif

	int m_idRange;
	bool m_bRelativeCoords;
	OVERLAY_STATE m_overlayState;
	RESOLUTION m_coordsRes; // Resolution that the window coordinates are in.
	bool m_needsScaling;
	bool m_windowLoaded;  // True if the window's xml file has been loaded
	bool m_loadOnDemand;  // True if the window should be loaded only as needed
	bool m_isDialog;      // True if we have a dialog, false otherwise.
	bool m_dynamicResourceAlloc;
	CGUIInfoColor m_clearBackground; // Colour to clear the window

	int m_renderOrder; // for render order of dialogs

	// Grabs the window's top,left position in skin coordinates
	// The window origin may change based on <origin> tag conditions in the skin.
	// return the window's origin in skin coordinates
	virtual CPoint GetPosition() const;
	std::vector<COrigin> m_origins; // Positions of dialogs depending on base window

	// Control states
	bool m_saveLastControl;
	int m_lastControlID;
	std::vector<CControlState> m_controlStates;
	int m_previousWindow;

	bool m_animationsEnabled;
	struct icompare
	{
		bool operator()(const CStdString &s1, const CStdString &s2) const
		{
			return s1.CompareNoCase(s2) < 0;
		}
	};

	std::map<CStdString, CStdString, icompare> m_mapProperties;

	CGUIAction m_loadActions;
	CGUIAction m_unloadActions;
  
	bool m_manualRunActions;

	int m_exclusiveMouseControl; // id of child control that wishes to receive all mouse events \sa GUI_MSG_EXCLUSIVE_MOUSE
};

#endif //GUILIB_GUIWINDOW_H