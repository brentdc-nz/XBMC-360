#include "GUIWindow.h"
#include "utils\Log.h"
#include "Application.h"
#include "GUIWindowManager.h"
#include "GUIControlFactory.h"
#include "GUITextureD3D.h"
#include "SkinInfo.h"
#include "Settings.h"
#include "GUISettings.h"

CGUIWindow::CGUIWindow(int id, const CStdString &xmlFile)	
{
	SetID(id);
	SetProperty("xmlfile", xmlFile);
	m_dwIDRange = 1;
	m_xmlFile = xmlFile;
	m_dwDefaultFocusControlID = 0;
	m_saveLastControl = false;
	m_lastControlID = 0;
	m_visibleCondition = 0;
	m_WindowAllocated = false;
	m_coordsRes = g_guiSettings.m_LookAndFeelResolution;
	m_needsScaling = true;
	m_loadOnDemand = true;
	m_dynamicResourceAlloc = true;
	m_windowLoaded = false;
	m_bAllocated = false;
	m_clearBackground = 0xff000000; // opaque black -> always clear
}

CGUIWindow::~CGUIWindow(void)
{
}

bool CGUIWindow::Initialize()
{
	if (!g_windowManager.Initialized())
		return false; // Can't load if we have no skin yet

	return Load(m_xmlFile);
}

void CGUIWindow::DynamicResourceAlloc(bool bOnOff)
{
	m_dynamicResourceAlloc = bOnOff;
	for (ivecControls i = m_vecControls.begin();i != m_vecControls.end(); ++i)
	{
		CGUIControl* pControl = *i;
		pControl->DynamicResourceAlloc(bOnOff);
	}
}

bool CGUIWindow::Load(const CStdString& strFileName, bool bContainsPath)
{
	if(m_windowLoaded)
		return true; // No point loading if it's already there

#ifdef _DEBUG
	int64_t start;
	start = CurrentHostCounter();
#endif

	RESOLUTION resToUse = INVALID;
	CLog::Log(LOGINFO, "Loading skin file: %s", strFileName.c_str());
	TiXmlDocument xmlDoc;
	
	// Find appropriate skin folder + resolution to load from
	CStdString strPath;
	CStdString strLowerPath;

	if(bContainsPath)
		strPath = strFileName;
	else
		strPath = g_SkinInfo.GetSkinPath(strFileName, &resToUse); //TODO

	if(!bContainsPath)
		m_coordsRes = HDTV_480p_16x9;

	bool ret = LoadXML(strPath.c_str(), strLowerPath.c_str());

#ifdef _DEBUG
	int64_t end, freq;
	end = CurrentHostCounter();
	freq = CurrentHostFrequency();
	CLog::Log(LOGDEBUG,"Load %s: %.2fms", GetProperty("xmlfile").c_str(), 1000.f * (end - start) / freq);
#endif

	return ret;
}

bool CGUIWindow::OnMessage(CGUIMessage& message)
{
	switch(message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
			CLog::Log(LOGDEBUG, "------ Window Init () ------", m_xmlFile.c_str());
			if (m_dynamicResourceAlloc || !m_WindowAllocated) AllocResources();
			OnInitWindow();
			return true;
		}
		break;

		case GUI_MSG_WINDOW_DEINIT:
		{
			CLog::Log(LOGDEBUG, "------ Window Deinit () ------", m_xmlFile.c_str());
			OnDeinitWindow(/*message.GetParam1()*/);
			if (m_dynamicResourceAlloc) FreeResources();
			return true;
		}
		break;

		case GUI_MSG_MOVE:
		{
//			if(HasID(message.GetSenderId())) // TODO
				return OnMove(message.GetControlId(), message.GetParam1());
			break;
		}

		case GUI_MSG_SETFOCUS:
		{
			// First unfocus the current control
			CGUIControl *control = GetFocusedControl();
			if(control)
			{
				CGUIMessage msgLostFocus(GUI_MSG_LOSTFOCUS, GetID(), control->GetID(), message.GetControlId());
				control->OnMessage(msgLostFocus);
			}

			// Get the control to focus
			CGUIControl* pFocusedControl = GetFirstFocusableControl(message.GetControlId());
			if(!pFocusedControl) pFocusedControl = (CGUIControl *)GetControl(message.GetControlId());

			// And focus it
			if(pFocusedControl)
				return pFocusedControl->OnMessage(message);

			break;
		}

		case GUI_MSG_NOTIFY_ALL:
		{
			// Only process those notifications that come from this window, or those intended for every window
			if((GetID() == message.GetSenderId()) || !message.GetSenderId())
			{
				if(message.GetParam1() == GUI_MSG_SCROLL_CHANGE)//||
//				    message.GetParam1() == GUI_MSG_REFRESH_THUMBS || // TODO
//					message.GetParam1() == GUI_MSG_REFRESH_LIST) // TODO
				{
					ivecControls it;
					for (it = m_vecControls.begin(); it != m_vecControls.end(); ++it)
					{
						CGUIControl *control = *it;
						CGUIMessage msg(message.GetParam1(), message.GetControlId(), control->GetID(), message.GetParam2());
						control->OnMessage(msg);
					}
				}
			}
		}
		break;
	}

	ivecControls i;
	// Send to the visible matching control first
	for (i = m_vecControls.begin();i != m_vecControls.end(); ++i)
	{
		CGUIControl* pControl = *i;
		if (pControl && message.GetControlId() == pControl->GetID() && pControl->IsVisible())
		{
			if (pControl->OnMessage(message))
				return true;
		}
	}

	// Unhandled - send to all matching invisible controls as well
	bool handled(false);
	for (i = m_vecControls.begin();i != m_vecControls.end(); ++i)
	{
		CGUIControl* pControl = *i;
		if (pControl && message.GetControlId() == pControl->GetID() && !pControl->IsVisible())
		{
			if (pControl->OnMessage(message))
				handled = true;
		}
	}

	return handled;
}

void CGUIWindow::InsertControl(CGUIControl *control, const CGUIControl *insertPoint)
{
	// get the insertion point
	ivecControls i = m_vecControls.begin();
	while (i != m_vecControls.end())
	{
		if (*i == insertPoint)
		break;
		i++;
	}
	m_vecControls.insert(i, control);
}

void CGUIWindow::RemoveControl(DWORD dwId)
{
	ivecControls i = m_vecControls.begin();
	while (i != m_vecControls.end())
	{
		CGUIControl* pControl = *i;
		if (pControl->GetID() == dwId)
		{
			m_vecControls.erase(i);
			return ;
		}
		++i;
	}
}

/// \brief Called on window open.
///  * Restores the control state(s)
///  * Sets initial visibility of controls
///  * Queue WindowOpen animation
///  * Set overlay state
/// Override this function and do any window-specific initialisation such
/// as filling control contents and setting control focus before
/// calling the base method.
void CGUIWindow::OnInitWindow()
{
	m_bAllocated = true;
	
	RestoreControlStates();
}

// Called on window close.
//  * Executes the window close animation(s)
//  * Saves control state(s)
// Override this function and call the base class before doing any dynamic memory freeing
void CGUIWindow::OnDeinitWindow()
{
	m_bAllocated = false;

	SaveControlStates();
}

void CGUIWindow::Render()
{
	// If we're rendering from a different thread, then we should wait for the main
	// app thread to finish AllocResources(), as dynamic resources (images in particular)
	// will try and be allocated from 2 different threads, which causes nasty things to occur
	if(!m_WindowAllocated) return;

	g_graphicsContext.SetRenderingResolution(m_coordsRes, m_needsScaling);

	// Render our window animation - returns false if it needs to stop rendering
	if(!RenderAnimation(m_renderTime))
		return;

	CGUIControlGroup::Render();
}

bool CGUIWindow::RenderAnimation(unsigned int time)
{
	g_graphicsContext.ResetWindowTransform();

	//TODO

	return true;
}

bool CGUIWindow::OnAction(const CAction &action)
{
	CGUIControl *focusedControl = GetFocusedControl();
	if(focusedControl)
	{
		if(focusedControl->OnAction(action))
			return true;
	}
	else
	{
		// No control has focus?
		// set focus to the default control then
		CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), m_dwDefaultFocusControlID);
		OnMessage(msg);
	}

	// Default implementations
	if(action.GetID() == ACTION_NAV_BACK || action.GetID() == ACTION_PREVIOUS_MENU)
		return OnBack(action.GetID());

	return false;
}

bool CGUIWindow::OnBack(int actionID)
{
	g_windowManager.PreviousWindow();
	return true;
}

void CGUIWindow::ClearBackground()
{
	DWORD color = m_clearBackground;
	if (color)
		g_graphicsContext.Clear(color);
}

void CGUIWindow::OnWindowLoaded()
{
	DynamicResourceAlloc(true);
}

void CGUIWindow::AllocResources(bool forceLoad /*= FALSE */)
{
	// load skin xml file
	if (m_xmlFile.size() && (forceLoad || m_loadOnDemand || !m_windowLoaded)) Load(m_xmlFile);

	// and now allocate resources
	ivecControls i;
	for (i = m_vecControls.begin();i != m_vecControls.end(); ++i)
	{
		CGUIControl* pControl = *i;
		if (!pControl->IsDynamicallyAllocated()) 
			pControl->PreAllocResources();
	}
 
	for (i = m_vecControls.begin();i != m_vecControls.end(); ++i)
	{
		CGUIControl* pControl = *i;
		if (!pControl->IsDynamicallyAllocated()) 
			pControl->AllocResources();
	}

	CGUIControlGroup::AllocResources();

	m_WindowAllocated = true;
}

void CGUIWindow::FreeResources(bool forceUnload /*= FALSE */)
{
	m_WindowAllocated = false;
	CGUIControlGroup::FreeResources();

	ivecControls i;
	for (i = m_vecControls.begin();i != m_vecControls.end(); ++i)
	{
		CGUIControl* pControl = *i;
		pControl->FreeResources();
	}
	//g_TextureManager.Dump();
	// unload the skin
	if (m_loadOnDemand || forceUnload) ClearAll();
}

void CGUIWindow::ClearAll()
{
	OnWindowUnload();

	for (int i = 0; i < (int)m_vecControls.size(); ++i)
	{
		CGUIControl* pControl = m_vecControls[i];
		delete pControl;
	}

	m_vecControls.erase(m_vecControls.begin(), m_vecControls.end());
	m_windowLoaded = false;
	m_dynamicResourceAlloc = true;
}

// Find the first focusable control with this id.
// if no focusable control exists with this id, return NULL
CGUIControl *CGUIWindow::GetFirstFocusableControl(int id)
{
	for(ivecControls i = m_vecControls.begin();i != m_vecControls.end(); ++i)
	{
		CGUIControl* pControl = *i;
/*		if(pControl->IsGroup())
		{
			CGUIControlGroup *group = (CGUIControlGroup *)pControl;
			CGUIControl *control = group->GetFirstFocusableControl(id);
			if(control) return control;
		}
*/		if(pControl->GetID() == id && pControl->CanFocus())
			return pControl;
	}
	return NULL;
}

int CGUIWindow::GetFocusedControlID() const
{
	for (int i = 0;i < (int)m_vecControls.size(); ++i)
	{
		const CGUIControl* pControl = m_vecControls[i];
		if (pControl->HasFocus() ) return pControl->GetID();
	}
	return -1;
}

CGUIControl *CGUIWindow::GetFocusedControl() const
{
	for (int i = 0;i < (int)m_vecControls.size(); ++i)
	{
		const CGUIControl* control = m_vecControls[i];
		if (control->HasFocus())
		{
			return (CGUIControl *)control;
		}
	}
	return NULL;
}

bool CGUIWindow::OnMove(int fromControl, int moveAction)
{
	const CGUIControl *control = GetFirstFocusableControl(fromControl);
	if(!control) control = GetControl(fromControl);
	if(!control)
	{
		// No current control??
		CLog::Log(LOGERROR, "Unable to find control %i in window %i", fromControl, GetID());
		return false;
	}

	vector<int> moveHistory;
	int nextControl = fromControl;
	
	int iCtrlID = control->GetID();

	while(control)
	{
		// Grab the next control direction
		moveHistory.push_back(nextControl);
		nextControl = control->GetNextControl(moveAction);

		// Check our history - if the nextControl is in it, we can't focus it
		for(unsigned int i = 0; i < moveHistory.size(); i++)
		{
			if(nextControl == moveHistory[i])
				return false; // No control to focus so do nothing
		}
		
		control = GetFirstFocusableControl(nextControl);
		if(control)
			break; // Found a focusable control

		control = GetControl(nextControl); // Grab the next control and try again
	}

	if(!control)
		return false; // no control to focus
	
	// If we get here we have our new control so focus it (and unfocus the current control)
	CGUIMessage msg(GUI_MSG_SETFOCUS, iCtrlID, nextControl, 0);
	OnMessage(msg);
	return true;
}

const CGUIControl* CGUIWindow::GetControl(int iControl) const
{
	for (int i = 0;i < (int)m_vecControls.size(); ++i)
	{
		const CGUIControl* pControl = m_vecControls[i];
		if (pControl->GetID() == iControl) return pControl;
	}
	return NULL;
}


void CGUIWindow::SetProperty(const CStdString &key, const CStdString &value)
{
	m_mapProperties[key] = value;
}

void CGUIWindow::SetProperty(const CStdString &key, const char *value)
{
	m_mapProperties[key] = value;
}

void CGUIWindow::SetProperty(const CStdString &key, int value)
{
	CStdString strVal;
	strVal.Format("%d", value);
	SetProperty(key, strVal);
}

void CGUIWindow::SetProperty(const CStdString &key, bool value)
{
	SetProperty(key, value ? "1" : "0");
}

void CGUIWindow::SetProperty(const CStdString &key, double value)
{
	CStdString strVal;
	strVal.Format("%f", value);
	SetProperty(key, strVal);
}

CStdString CGUIWindow::GetProperty(const CStdString &key) const
{
	std::map<CStdString, CStdString, icompare>::const_iterator iter = m_mapProperties.find(key);

	if(iter == m_mapProperties.end())
		return "";

	return iter->second;
}

int CGUIWindow::GetPropertyInt(const CStdString &key) const
{
	return atoi(GetProperty(key).c_str());
}

bool CGUIWindow::GetPropertyBool(const CStdString &key) const
{
	return GetProperty(key) == "1";
}

double CGUIWindow::GetPropertyDouble(const CStdString &key) const
{
	return atof(GetProperty(key).c_str());
}

void CGUIWindow::ClearProperties()
{
	m_mapProperties.clear();
}

void CGUIWindow::SaveControlStates()
{
	if (m_saveLastControl)
		m_lastControlID = GetFocusedControlID();
}

void CGUIWindow::RestoreControlStates()
{
	if (m_saveLastControl)
	{
		// set focus to our saved control
		int focusControl = m_lastControlID ? m_lastControlID : m_dwDefaultFocusControlID;
		
		CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), focusControl, 0);
		OnMessage(msg);
	}
	else
	{
		// set the default control focus
		CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), m_dwDefaultFocusControlID, 0);
		OnMessage(msg);
	}
}

bool CGUIWindow::LoadXML(const CStdString &strPath, const CStdString &strLowerPath)
{
	TiXmlDocument xmlDoc;
	
	if(!xmlDoc.LoadFile(strPath) && !xmlDoc.LoadFile(CStdString(strPath).ToLower()) && !xmlDoc.LoadFile(strLowerPath))
	{
		CLog::Log(LOGERROR, "unable to load:%s, Line %d\n%s", strPath.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
		SetID(WINDOW_INVALID);
		return false;
	}

	return Load(xmlDoc);
}

bool CGUIWindow::Load(TiXmlDocument &xmlDoc)
{
	m_saveLastControl = true;
	m_dwDefaultFocusControlID = 0;
	m_visibleCondition = 0;

	TiXmlElement* pRootElement = xmlDoc.RootElement();
	if (strcmpi(pRootElement->Value(), "window"))
	{
		CLog::Log(LOGERROR, "file : XML file doesnt contain <window>");
		return false;
	}
	
	// Set the scaling resolution so that any control creation or initialisation can
	// be done with respect to the correct aspect ratio
	g_graphicsContext.SetScalingResolution(m_coordsRes, m_needsScaling);

	// Resolve any includes that may be present
//	g_SkinInfo.ResolveIncludes(pRootElement);
	// now load in the skin file
//	SetDefaults();	
	
//	CGUIControlFactory::GetInfoColor(pRootElement, "backgroundcolor", m_clearBackground, GetID());
//	CGUIControlFactory::GetActions(pRootElement, "onload", m_loadActions);
//	CGUIControlFactory::GetActions(pRootElement, "onunload", m_unloadActions);
//	CGUIControlFactory::GetHitRect(pRootElement, m_hitRect);

	TiXmlElement *pChild = pRootElement->FirstChildElement();
	while (pChild)
	{
		CStdString strValue = pChild->Value();
		if (strValue == "type" && pChild->FirstChild())
		{
			// if we have are a window type (ie not a dialog), and we have <type>dialog</type>
			// then make this window act like a dialog
//			if (!IsDialog() && strcmpi(pChild->FirstChild()->Value(), "dialog") == 0)
//			m_isDialog = true;
		}
		else if (strValue == "previouswindow" && pChild->FirstChild())
		{
//			m_previousWindow = CButtonTranslator::TranslateWindow(pChild->FirstChild()->Value());
		}
		else if (strValue == "defaultcontrol" && pChild->FirstChild())
		{
			const char *always = pChild->Attribute("always");
			
	//		if (always && strcmpi(always, "true") == 0)
	//		m_saveLastControl = false;
			
			m_dwDefaultFocusControlID = atoi(pChild->FirstChild()->Value());
		}
		else if (strValue == "visible" && pChild->FirstChild())
		{
			CGUIControlFactory::GetConditionalVisibility(pRootElement, m_visibleCondition);
		}
		else if (strValue == "animation" && pChild->FirstChild())
		{
/*			FRECT rect = { 0, 0, (float)g_settings.m_ResInfo[m_coordsRes].iWidth, (float)g_settings.m_ResInfo[m_coordsRes].iHeight };
			CAnimation anim;
			anim.Create(pChild, rect);
			m_animations.push_back(anim);
*/		}
		else if (strValue == "zorder" && pChild->FirstChild())
		{
//			m_renderOrder = atoi(pChild->FirstChild()->Value());
		}
		else if (strValue == "coordinates")
		{
/*			// resolve any includes within coordinates tag (such as multiple origin includes)
			g_SkinInfo.ResolveIncludes(pChild);
			TiXmlNode* pSystem = pChild->FirstChild("system");
			if (pSystem)
			{
				int iCoordinateSystem = atoi(pSystem->FirstChild()->Value());
				m_bRelativeCoords = (iCoordinateSystem == 1);
			}

			CGUIControlFactory::GetFloat(pChild, "posx", m_posX);
			CGUIControlFactory::GetFloat(pChild, "posy", m_posY);
			CGUIControlFactory::GetFloat(pChild, "left", m_posX);
			CGUIControlFactory::GetFloat(pChild, "top", m_posY);

			TiXmlElement *originElement = pChild->FirstChildElement("origin");
			while (originElement)
			{
				COrigin origin;
				g_SkinInfo.ResolveConstant(originElement->Attribute("x"), origin.x);
				g_SkinInfo.ResolveConstant(originElement->Attribute("y"), origin.y);
				if (originElement->FirstChild())
					origin.condition = g_infoManager.TranslateString(originElement->FirstChild()->Value());
				m_origins.push_back(origin);
				originElement = originElement->NextSiblingElement("origin");
			}
*/		}
		else if (strValue == "camera")
		{ 
			// z is fixed
//			g_SkinInfo.ResolveConstant(pChild->Attribute("x"), m_camera.x);
//			g_SkinInfo.ResolveConstant(pChild->Attribute("y"), m_camera.y);
//			m_hasCamera = true;
		}
		else if (strValue == "controls")
		{
			// resolve any includes within controls tag (such as whole <control> includes)
//			g_SkinInfo.ResolveIncludes(pChild);

			TiXmlElement *pControl = pChild->FirstChildElement();
			while(pControl)
			{
				if(strcmpi(pControl->Value(), "control") == 0)
				{
					LoadControl(pControl, NULL);
				}
				pControl = pControl->NextSiblingElement();
			}
		}
		else if (strValue == "allowoverlay")
		{
//			bool overlay = false;
//			if (XMLUtils::GetBoolean(pRootElement, "allowoverlay", overlay))
//				m_overlayState = overlay ? OVERLAY_STATE_SHOWN : OVERLAY_STATE_HIDDEN;
		}

		pChild = pChild->NextSiblingElement();
	}

	m_windowLoaded = true;
	OnWindowLoaded();
	return true;
}

void CGUIWindow::LoadControl(TiXmlElement* pControl, CGUIControlGroup *pGroup)
{
	// Get control type
	CGUIControlFactory factory;

	FRECT rect = { 0, 0, (float)g_settings.m_ResInfo[m_coordsRes].iWidth, (float)g_settings.m_ResInfo[m_coordsRes].iHeight };
	if(pGroup)
	{
		rect.left = pGroup->GetXPosition();
		rect.top = pGroup->GetYPosition();
		rect.right = rect.left + pGroup->GetWidth();
		rect.bottom = rect.top + pGroup->GetHeight();
	}

	CGUIControl* pGUIControl = factory.Create(GetID(), rect, pControl);
	if(pGUIControl)
	{
		float maxX = pGUIControl->GetXPosition() + pGUIControl->GetWidth();
		if (maxX > m_width)
		{
			m_width = maxX;
		}

		float maxY = pGUIControl->GetYPosition() + pGUIControl->GetHeight();
		if (maxY > m_height)
		{
			m_height = maxY;
		}

		// If we are in a group, add to the group, else add to our window
		if(pGroup)
			pGroup->AddControl(pGUIControl);
		else
			AddControl(pGUIControl);
		
		// If the new control is a group, then add it's controls
		if(pGUIControl->IsGroup())
		{
			TiXmlElement *pSubControl = pControl->FirstChildElement("control");
			while(pSubControl)
			{
				LoadControl(pSubControl, (CGUIControlGroup*)pGUIControl);
				pSubControl = pSubControl->NextSiblingElement("control");
			}
		}
	}
}

