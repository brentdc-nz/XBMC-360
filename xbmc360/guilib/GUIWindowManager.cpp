#include "GUIWindowManager.h"
#include "..\utils\log.h"
#include "..\utils\SingleLock.h"
#include "GraphicContext.h"
using namespace std;

CGUIWindowManager g_windowManager;

CGUIWindowManager::CGUIWindowManager(void)
{
	m_initialized = false;
}

CGUIWindowManager::~CGUIWindowManager(void)
{
}

void CGUIWindowManager::Initialize()
{
	LoadNotOnDemandWindows();
	m_initialized = true;
}

void CGUIWindowManager::SendThreadMessage(CGUIMessage& message)
{
	CSingleLock lock(m_critSection);

	CGUIMessage* msg = new CGUIMessage(message);
	m_vecThreadMessages.push_back( pair<CGUIMessage*,int>(msg,0) );
}

void CGUIWindowManager::DispatchThreadMessages()
{
	CSingleLock lock(m_critSection);
	vector< pair<CGUIMessage*,int> > messages(m_vecThreadMessages);
	m_vecThreadMessages.erase(m_vecThreadMessages.begin(), m_vecThreadMessages.end());
	lock.Leave();

	while ( messages.size() > 0 )
	{
		vector< pair<CGUIMessage*,int> >::iterator it = messages.begin();
		CGUIMessage* pMsg = it->first;
		int window = it->second;
		// first remove the message from the queue,
		// else the message could be processed more then once
		it = messages.erase(it);

//		if (window)
  //		SendMessage( *pMsg, window );
 //		else
			SendMessage( *pMsg );
		delete pMsg;
	}
}

bool CGUIWindowManager::SendMessage(CGUIMessage& message)
{
	bool handled = false;

	// Send the message to all none window targets
	for (int i = 0; i < (int) m_vecMsgTargets.size(); i++)
	{
		IMsgTargetCallback* pMsgTarget = m_vecMsgTargets[i];

		if (pMsgTarget)
		{
			if (pMsgTarget->OnMessage( message )) handled = true;
		}
	}

	// now send to the underlying window
	CGUIWindow* window = GetWindow(GetActiveWindow());
	if (window)
	{
		if (window->OnMessage(message)) handled = true;
    }

	return handled;
}

void CGUIWindowManager::Add(CGUIWindow* pWindow)
{
	if (!pWindow)
	{
		CLog::Log(LOGERROR, "Attempted to add a NULL window pointer to the window manager.");
		return;
	}
	
	// push back all the windows if there are more than one covered by this class
	CSingleLock lock(g_graphicsContext);
/*	for (int i = 0; i < pWindow->GetIDRange(); i++)
	{
		WindowMap::iterator it = m_mapWindows.find(pWindow->GetID() + i);
		if (it != m_mapWindows.end())
		{
			CLog::Log(LOGERROR, "Error, trying to add a second window with id %u "
                          "to the window manager",
                pWindow->GetID());
			return;
		}
*/		m_mapWindows.insert(pair<int, CGUIWindow *>(pWindow->GetID()/* + i*/, pWindow));
//	}
}

void CGUIWindowManager::LoadNotOnDemandWindows()
{
	CSingleLock lock(g_graphicsContext);
	for (WindowMap::iterator it = m_mapWindows.begin(); it != m_mapWindows.end(); it++)
	{
		CGUIWindow *pWindow = (*it).second;
		if (!pWindow->GetLoadOnDemand())
		{
			pWindow->FreeResources(true);
			pWindow->Initialize();
		}
	}
}

void CGUIWindowManager::UnloadNotOnDemandWindows()
{
	CSingleLock lock(g_graphicsContext);
	for (WindowMap::iterator it = m_mapWindows.begin(); it != m_mapWindows.end(); it++)
	{
		CGUIWindow *pWindow = (*it).second;
		if (!pWindow->GetLoadOnDemand())
		{
			pWindow->FreeResources(true);
		}
	}
}

CGUIWindow* CGUIWindowManager::GetWindow(int id) const
{
	if (id == WINDOW_INVALID)
		return NULL;

	CSingleLock lock(g_graphicsContext);
	WindowMap::const_iterator it = m_mapWindows.find(id);

	if (it != m_mapWindows.end())
		return (*it).second;

	return NULL;
}

int CGUIWindowManager::GetActiveWindow() const
{
	if (!m_windowHistory.empty())
		return m_windowHistory.top();
	return WINDOW_INVALID;
}

void CGUIWindowManager::PreviousWindow()
{
	// deactivate any window
	CLog::Log(LOGDEBUG,"CGUIWindowManager::PreviousWindow: Deactivate");
	int currentWindow = GetActiveWindow();
	CGUIWindow *pCurrentWindow = GetWindow(currentWindow);
	if (!pCurrentWindow)
		return;     // no windows or window history yet

	// check to see whether our current window has a <previouswindow> tag
/*	if (pCurrentWindow->GetPreviousWindow() != WINDOW_INVALID)
	{
		// TODO: we may need to test here for the
		//       whether our history should be changed

		// don't reactivate the previouswindow if it is ourselves.
		if (currentWindow != pCurrentWindow->GetPreviousWindow())
		ActivateWindow(pCurrentWindow->GetPreviousWindow());
		return;
	}
*/
	// get the previous window in our stack
	if (m_windowHistory.size() < 2)
	{
		// no previous window history yet - check if we should just activate home
		if (GetActiveWindow() != WINDOW_INVALID && GetActiveWindow() != WINDOW_HOME)
		{
			ClearWindowHistory();
			ActivateWindow(WINDOW_HOME);
		}
		return;
	}
	m_windowHistory.pop();
	int previousWindow = GetActiveWindow();
	m_windowHistory.push(currentWindow);

	CGUIWindow *pNewWindow = GetWindow(previousWindow);
	if (!pNewWindow)
	{
		CLog::Log(LOGERROR, "Unable to activate the previous window");
		ClearWindowHistory();
		ActivateWindow(WINDOW_HOME);
		return;
	}

	// ok to go to the previous window now

	// deinitialize our window
	CGUIMessage msg(GUI_MSG_WINDOW_DEINIT, 0, 0);
	pCurrentWindow->OnMessage(msg);

	// remove the current window off our window stack
	m_windowHistory.pop();

	// ok, initialize the new window
	CLog::Log(LOGDEBUG,"CGUIWindowManager::PreviousWindow: Activate new");
	CGUIMessage msg2(GUI_MSG_WINDOW_INIT, 0, 0, WINDOW_INVALID, GetActiveWindow());
	pNewWindow->OnMessage(msg2);

	return;
}

// removes and deletes the window.  Should only be called
// from the class that created the window using new.
void CGUIWindowManager::Delete(int id)
{
	CSingleLock lock(g_graphicsContext);
	CGUIWindow *pWindow = GetWindow(id);
	if (pWindow)
	{
		Remove(id);
		delete pWindow;
	}
}

void CGUIWindowManager::Remove(int id)
{
	CSingleLock lock(g_graphicsContext);
	WindowMap::iterator it = m_mapWindows.find(id);
	
	if (it != m_mapWindows.end())
	{
		m_mapWindows.erase(it);
	}
	else
	{
		CLog::Log(LOGWARNING, "Attempted to remove window %u "
                          "from the window manager when it didn't exist",
              id);
	}
}

void CGUIWindowManager::ActivateWindow(int iWindowID)
{
	// debug
	CLog::Log(LOGDEBUG, "Activating window ID: %i", iWindowID);

	// first check existence of the window we wish to activate.
	CGUIWindow *pNewWindow = GetWindow(iWindowID);
	if (!pNewWindow)
	{ 
		// nothing to see here - move along
		CLog::Log(LOGERROR, "Unable to locate window with id %d.  Check skin files", iWindowID - WINDOW_HOME);
		return;
	}

	// deactivate any window
	int currentWindow = GetActiveWindow();
	CGUIWindow *pWindow = GetWindow(currentWindow);
	if (pWindow)
	{
		//  Play the window specific deinit sound
		CGUIMessage msg(GUI_MSG_WINDOW_DEINIT, 0, 0, iWindowID);
		pWindow->OnMessage(msg);
	}
 
	// Add window to the history list (we must do this before we activate it,
	// as all messages done in WINDOW_INIT will want to be sent to the new
	// topmost window).  If we are swapping windows, we pop the old window
	// off the history stack
//	if (swappingWindows && m_windowHistory.size())
//		m_windowHistory.pop();
	AddToWindowHistory(iWindowID);

	// Send the init message
	CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0, currentWindow, iWindowID);
//	msg.SetStringParams(params);
	pNewWindow->OnMessage(msg);
}

bool CGUIWindowManager::OnAction(const CAction &action)
{
	// Have we have routed windows...
/*	if (m_vecModalWindows.size() > 0)
	{
		// ...send the action to the top most.
		CGUIWindow *window = m_vecModalWindows[m_vecModalWindows.size() - 1];
		if (!window->IsAnimating(ANIM_TYPE_WINDOW_CLOSE))
		  return window->OnAction(action);
		return true; // don't do anything with this action for now
	}
	else
	{
*/		CGUIWindow* pWindow = GetWindow(GetActiveWindow());
			if (pWindow)
			  return pWindow->OnAction(action);
//	}
	return false;
}

void CGUIWindowManager::Render()
{
	Render_Internal();
}

void CGUIWindowManager::Render_Internal()
{
	CSingleLock lock(g_graphicsContext);
	CGUIWindow* pWindow = GetWindow(GetActiveWindow());
	
	if (pWindow)
	{
		pWindow->ClearBackground();
		pWindow->Render();
	}
}

void CGUIWindowManager::AddToWindowHistory(int newWindowID)
{
	// Check the window stack to see if this window is in our history,
	// and if so, pop all the other windows off the stack so that we
	// always have a predictable "Back" behaviour for each window
	stack<int> historySave = m_windowHistory;
	while (historySave.size())
	{
		if (historySave.top() == newWindowID)
			break;
		historySave.pop();
	}
	if (!historySave.empty())
	{ 
		// found window in history
		m_windowHistory = historySave;
	}
	else
	{
		// didn't find window in history - add it to the stack
		m_windowHistory.push(newWindowID);
	}
}

void CGUIWindowManager::ClearWindowHistory()
{
	while (m_windowHistory.size())
		m_windowHistory.pop();
}

void CGUIWindowManager::DeInitialize()
{
	for (WindowMap::iterator it = m_mapWindows.begin(); it != m_mapWindows.end(); it++)
	{
		CGUIWindow* pWindow = (*it).second;
		CGUIMessage msg(GUI_MSG_WINDOW_DEINIT, 0, 0);
		pWindow->OnMessage(msg);
//		pWindow->ResetControlStates();
		pWindow->FreeResources(true);
	}

	UnloadNotOnDemandWindows();

	m_vecMsgTargets.erase( m_vecMsgTargets.begin(), m_vecMsgTargets.end() );

	m_initialized = false;
}

void CGUIWindowManager::AddMsgTarget( IMsgTargetCallback* pMsgTarget )
{
	m_vecMsgTargets.push_back( pMsgTarget );
}
