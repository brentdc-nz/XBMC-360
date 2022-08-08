#ifndef GUILIB_GUIWINDOW_H
#define GUILIB_GUIWINDOW_H

#include "..\utils\StdString.h"
#include "tinyXML\tinyxml.h"
#include "GUIControl.h"
#include "GUIControlGroup.h"
#include "GUIMessage.h"
#include "key.h"

class CGUIWindow
{
public:
	CGUIWindow(int id, const CStdString &xmlFile);
	virtual ~CGUIWindow(void);

	virtual bool IsDialog() const { return false; };
	bool Initialize();  // loads the window
	virtual void DynamicResourceAlloc(bool bOnOff);	
	
	bool Load(const CStdString& strFileName, bool bContainsPath = false);

	virtual bool OnMessage(CGUIMessage& message);
	void AddControl(CGUIControl* pControl);
	void InsertControl(CGUIControl *control, const CGUIControl *insertPoint);
	void RemoveControl(DWORD dwId);

	virtual void OnInitWindow();
	virtual void OnDeinitWindow();
	virtual void OnWindowUnload() {};

	virtual void AllocResources(bool forceLoad = false );
	virtual void FreeResources(bool forceUnload = false);

	void ClearAll();
	const CGUIControl* GetControl(int iControl) const;
	virtual CGUIControl *GetFirstFocusableControl(int id);
	int GetFocusedControlID() const;
	CGUIControl *GetFocusedControl() const;
	bool OnMove(int fromControl, int moveAction);
	bool GetLoadOnDemand() { return m_loadOnDemand; }

	virtual void Render();
	// OnAction() is called by our window manager.  We should process any messages
	// that should be handled at the window level in the derived classes, and any
	// unhandled messages should be dropped through to here where we send the message
	// on to the currently focused control.  Returns true if the action has been handled
	// and does not need to be passed further down the line (to our global action handlers)
	virtual bool OnAction(const CAction &action);
	virtual bool OnBack(int actionID);
	virtual bool IsModalDialog() const { return false; };

	/*
	Clear the background (if necessary) prior to rendering the window
	*/
	virtual void ClearBackground();
	int GetID() { return iWindowID; };
	DWORD GetIDRange() const { return m_dwIDRange; };
	void SetID(int id) { iWindowID = id; };
	virtual bool HasID(int id) { return (id >= m_controlID && id < m_controlID + m_idRange); };



protected:
	// Control state saving on window close
	virtual void SaveControlStates();
	virtual void RestoreControlStates();

	CStdString m_xmlFile;

	virtual bool LoadXML(const CStdString& strPath, const CStdString &strLowerPath);  ///< Loads from the given file
	bool Load(TiXmlDocument &xmlDoc);   	
	void LoadControl(TiXmlElement* pControl);

	virtual void OnWindowLoaded();

	bool m_loadOnDemand;  // True if the window should be loaded only as needed
	bool m_windowLoaded;  // True if the window's xml file has been loaded
	int iWindowID;
	DWORD m_dwIDRange;

	bool m_bAllocated;

	DWORD m_clearBackground; // Colour to clear the window

	int m_visibleCondition;

	vector<CGUIControl*> m_vecControls;
	typedef std::vector<CGUIControl*>::iterator ivecControls;
	
	DWORD m_dwDefaultFocusControlID;

	bool m_WindowAllocated;
	bool m_dynamicResourceAlloc;
	int m_idRange;
	int m_controlID;
	// Control states
	bool m_saveLastControl;
	int m_lastControlID;
};

#endif //GUILIB_GUIWINDOW_H