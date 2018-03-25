#include "GUIDialog.h"
#include "GUIWindowManager.h"
#include "..\utils\SingleLock.h"
#include "..\utils\Log.h"

CGUIDialog::CGUIDialog(int id, const CStdString &xmlFile)
    : CGUIWindow(id, xmlFile)
{
	m_bRunning = false;
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

	// Active this window...
	CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0, WINDOW_INVALID, iWindowID);
	OnMessage(msg);

	lock.Leave();
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

			// if we were running, make sure we remove ourselves from the window manager
			if (/*m_bRunning*/1)
			{
				if (/*m_bModal*/1)
				{
					g_windowManager.UnRoute(GetID());
				}
				else
				{
//					g_windowManager.RemoveModeless( GetID() );
				}

//				m_pParentWindow = NULL;
				m_bRunning = false;
//				m_dialogClosing = false;
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

void CGUIDialog::Render()
{
	CGUIWindow::Render();
	
	// Check to see if we should close at this point
	// We check after the controls have finished rendering, as we may have to close due to
	// the controls rendering after the window has finished it's animation
//	if (m_dialogClosing && !IsAnimating(ANIM_TYPE_WINDOW_CLOSE))
//		Close(true);
}

bool CGUIDialog::OnBack(int actionID)
{
	Close();
	return true;
}

void CGUIDialog::Close(bool forceClose /*= false*/)
{
	//Lock graphic context here as it is sometimes called from non rendering threads
	//maybe we should have a critical section per window instead??
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
	//  Play the window specific deinit sound
//	g_audioManager.PlayWindowSound(GetID(), SOUND_DEINIT);

	CGUIMessage msg(GUI_MSG_WINDOW_DEINIT, 0, 0);

#if 0 // FIXME : Why is it calling the base class?
	OnMessage(msg);
#endif
	CGUIDialog::OnMessage(msg);
}
