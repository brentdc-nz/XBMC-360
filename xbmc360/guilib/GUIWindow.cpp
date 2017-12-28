#include "GUIWindow.h"
#include "..\utils\log.h"
#include "..\Application.h"
#include "GUIWindowManager.h"
#include "GUIControlFactory.h"

#include "GUID3DTexture.h"

CGUIWindow::CGUIWindow(int id, const CStdString &xmlFile)
{
	SetID(id);
	m_dwIDRange = 1;
	m_xmlFile = xmlFile;
	m_dwDefaultFocusControlID = 0;
	m_saveLastControl = false;
	m_lastControlID = 0;
	m_visibleCondition = 0;
	m_WindowAllocated = false;
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
		return false;     // can't load if we have no skin yet
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
	if (m_windowLoaded)
		return true;	// no point loading if it's already there

	CLog::Log(LOGINFO, "Loading skin file: %s", strFileName.c_str());
//	TiXmlDocument xmlDoc;
	
	// Find appropriate skin folder + resolution to load from
	CStdString strPath;
	CStdString strLowerPath;
//	if (bContainsPath) //MARTY FIXME WIP
		strPath = strFileName;
//	else
//		strPath = g_SkinInfo.GetSkinPath(strFileName, &resToUse);

//	if (!bContainsPath)
//		m_coordsRes = resToUse;

	bool ret = LoadXML(strPath.c_str(), strLowerPath.c_str());

  return ret;
}

bool CGUIWindow::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
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

void CGUIWindow::AddControl(CGUIControl* pControl)
{
	m_vecControls.push_back(pControl);
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
	// will try and be allocated from 2 different threads, which causes nasty things
	// to occur.
	if (!m_WindowAllocated) return;

	for (int i = 0; i < (int)m_vecControls.size(); i++)
	{
		CGUIControl *pControl = m_vecControls[i];
		if (pControl)
		{
//			pControl->UpdateEffectState(currentTime);
			pControl->DoRender();
		}
	}
//	m_hasRendered = true;
}

bool CGUIWindow::OnAction(const CAction &action)
{
	 for (ivecControls i = m_vecControls.begin();i != m_vecControls.end(); ++i)
	{
		CGUIControl* pControl = *i;
		if (pControl->HasFocus())
		{
			return pControl->OnAction(action);
		}
	}

	// no control has focus?
	// set focus to the default control then
	CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), m_dwDefaultFocusControlID);
	OnMessage(msg);
	return false;
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

	m_WindowAllocated = true;
}

void CGUIWindow::FreeResources(bool forceUnload /*= FALSE */)
{
	m_WindowAllocated = false;
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
//	OnWindowUnload();

	for (int i = 0; i < (int)m_vecControls.size(); ++i)
	{
		CGUIControl* pControl = m_vecControls[i];
		delete pControl;
	}

	m_vecControls.erase(m_vecControls.begin(), m_vecControls.end());
	m_windowLoaded = false;
	m_dynamicResourceAlloc = true;
}

int CGUIWindow::GetFocusedControl() const
{
	for (int i = 0;i < (int)m_vecControls.size(); ++i)
	{
		const CGUIControl* pControl = m_vecControls[i];
		if (pControl->HasFocus() ) return pControl->GetID();
	}
	return -1;
}

void CGUIWindow::SaveControlStates()
{
	if (m_saveLastControl)
		m_lastControlID = GetFocusedControl();
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

	CStdString strPathroot = g_graphicsContext.GetMediaDir();
	CStdString strPathFull;
	strPathFull = strPathroot + strPath;

	if ( !xmlDoc.LoadFile(strPathFull)/* && !xmlDoc.LoadFile(CStdString(strPath).ToLower()) && !xmlDoc.LoadFile(strLowerPath)*/)
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
	
	// set the scaling resolution so that any control creation or initialisation can
	// be done with respect to the correct aspect ratio
//	g_graphicsContext.SetScalingResolution(/*m_coordsRes, m_needsScaling*/ true);

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
			while (pControl)
			{
				if (strcmpi(pControl->Value(), "control") == 0)
				{
					LoadControl(pControl);
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

void CGUIWindow::LoadControl(TiXmlElement* pControl)
{
	// get control type
	CGUIControlFactory factory;

	FRECT rect = { 0, 0, 0, 0 };
/*	FRECT rect = { 0, 0, (float)g_settings.m_ResInfo[m_coordsRes].iWidth, (float)g_settings.m_ResInfo[m_coordsRes].iHeight };
	if (pGroup)
	{
		rect.left = pGroup->GetXPosition();
		rect.top = pGroup->GetYPosition();
		rect.right = rect.left + pGroup->GetWidth();
		rect.bottom = rect.top + pGroup->GetHeight();
	}
*/
	CGUIControl* pGUIControl = factory.Create(GetID(), rect, pControl);
	if (pGUIControl)
	{
	/*	float maxX = pGUIControl->GetXPosition() + pGUIControl->GetWidth();
		if (maxX > m_width)
		{
			m_width = maxX;
		}

		float maxY = pGUIControl->GetYPosition() + pGUIControl->GetHeight();
		if (maxY > m_height)
		{
			m_height = maxY;
		}
		// if we are in a group, add to the group, else add to our window
		if (pGroup)
			pGroup->AddControl(pGUIControl);
		else
*/			AddControl(pGUIControl);
		// if the new control is a group, then add it's controls
/*		if (pGUIControl->IsGroup())
		{
			TiXmlElement *pSubControl = pControl->FirstChildElement("control");
			while (pSubControl)
			{
				LoadControl(pSubControl, (CGUIControlGroup *)pGUIControl);
				pSubControl = pSubControl->NextSiblingElement("control");
			}
		}*/
	}
}

