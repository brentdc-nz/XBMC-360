#include "GUIDialog.h"
#include "GUIWindowManager.h"
#include "..\utils\SingleLock.h"
#include "..\utils\Log.h"
#include "GUIAudioManager.h"

CGUIDialog::CGUIDialog(int id, const CStdString &xmlFile)
    : CGUIWindow(id, xmlFile)
{
	m_bRunning = false;
	m_bModal = true;
}

CGUIDialog::~CGUIDialog(void)
{
}

void CGUIDialog::DoModal(DWORD dwParentId, int iWindowID /*= WINDOW_INVALID */)
{
	// Lock graphic context here as it is sometimes called from non rendering threads
	// maybe we should have a critical section per window instead??
	CSingleLock lock(g_graphicsContext);

	// Set running before it's added to the window manager, else the auto-show code
	// could show it as well if we are in a different thread from
	// the main rendering thread (this should really be handled via
	// a thread message though IMO)
	m_bRunning = true;
	g_windowManager.RouteToWindow(this);

	// Play the window specific init sound
	g_audioManager.PlayWindowSound(GetID(), SOUND_INIT);

	// Active this window...
	CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0, WINDOW_INVALID, iWindowID);
	OnMessage(msg);

	lock.Leave();
}

void CGUIDialog::Show()
{
	// Lock graphic context here as it is sometimes called from non rendering threads
	// maybe we should have a critical section per window instead??
	CSingleLock lock(g_graphicsContext);

	m_bModal = false;
 
	m_bRunning = true;
	g_windowManager.AddModeless(this);

	// Play the window specific init sound
	g_audioManager.PlayWindowSound(GetID(), SOUND_INIT);

	// Activate this window...
	CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0);
	OnMessage(msg);
}

bool CGUIDialog::OnBack(int actionID)
{
	Close();
	return true;
}

bool CGUIDialog::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_DEINIT:
		{
//			CGUIWindow *pWindow = m_gWindowManager.GetWindow(m_gWindowManager.GetActiveWindow()); //TODO
//			if (pWindow && pWindow->GetOverlayState()!=OVERLAY_STATE_PARENT_WINDOW)
//			m_gWindowManager.ShowOverlay(pWindow->GetOverlayState()==OVERLAY_STATE_SHOWN);

			CGUIWindow::OnMessage(message);

			// If we were running, make sure we remove ourselves from the window manager
			if (m_bRunning)
			{
				g_windowManager.RemoveDialog(GetID());
				m_bRunning = false;
//				m_dialogClosing = false; //TODO
			}
			return true;
		}

		case GUI_MSG_WINDOW_INIT:
		{
			CGUIWindow::OnMessage(message);
			return true;
		}
	}
	return CGUIWindow::OnMessage(message);
}

void CGUIDialog::Close(bool forceClose /*= false*/)
{
	// Lock graphic context here as it is sometimes called from non rendering threads
	// maybe we should have a critical section per window instead??
	CSingleLock lock(g_graphicsContext);

	if (!m_bRunning) return;

	CLog::DebugLog("Dialog::Close called");

	// Don't close if we should be animating
/*	if (!forceClose && HasAnimation(ANIM_TYPE_WINDOW_CLOSE))
	{
		if (!m_dialogClosing && !IsAnimating(ANIM_TYPE_WINDOW_CLOSE))
		{
			QueueAnimation(ANIM_TYPE_WINDOW_CLOSE);
			m_dialogClosing = true;
		}
		return;
	}
*/
	// Play the window specific deinit sound
	g_audioManager.PlayWindowSound(GetID(), SOUND_DEINIT);

	CGUIMessage msg(GUI_MSG_WINDOW_DEINIT, 0, 0);

#if 0 // FIXME : Why is it calling the base class?
	OnMessage(msg);
#endif
	CGUIDialog::OnMessage(msg);
}

void CGUIDialog::Render()
{
	CGUIWindow::Render();
	
	// TODO:

	// Check to see if we should close at this point
	// We check after the controls have finished rendering, as we may have to close due to
	// the controls rendering after the window has finished it's animation
//	if (m_dialogClosing && !IsAnimating(ANIM_TYPE_WINDOW_CLOSE))
//		Close(true);
}