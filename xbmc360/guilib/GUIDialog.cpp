#include "include.h"
#include "GUIDialog.h"
#include "GUIWindowManager.h"
#include "GUILabelControl.h"
#include "GUIAudioManager.h"
#include "utils/SingleLock.h"
#include "utils/TimeUtils.h"
#include "Application.h"
#include "ApplicationMessenger.h"

CGUIDialog::CGUIDialog(int id, const CStdString &xmlFile)
	: CGUIWindow(id, xmlFile)
{
	m_bModal = true;
	m_bRunning = false;
	m_dialogClosing = false;
	m_renderOrder = 1;
	m_autoClosing = false;
	m_enableSound = true;
}

CGUIDialog::~CGUIDialog(void)
{
}

void CGUIDialog::OnWindowLoaded()
{
	CGUIWindow::OnWindowLoaded();

	// Clip labels to extents
	if (m_children.size())
	{
		CGUIControl* pBase = m_children[0];

		for (iControls p = m_children.begin() + 1; p != m_children.end(); ++p)
		{
			if ((*p)->GetControlType() == CGUIControl::GUICONTROL_LABEL)
			{
				CGUILabelControl* pLabel = (CGUILabelControl*)(*p);

				if (!pLabel->GetWidth())
				{
					float spacing = (pLabel->GetXPosition() - pBase->GetXPosition()) * 2;
					pLabel->SetWidth(pBase->GetWidth() - spacing);
				}
			}
		}
	}
}

bool CGUIDialog::OnAction(const CAction &action)
{
	// Keyboard or controller movement should prevent autoclosing
	if (!action.IsMouse() && m_autoClosing)
		SetAutoClose(m_showDuration);

	return CGUIWindow::OnAction(action);
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
			CGUIWindow *pWindow = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
			if (pWindow)
				g_windowManager.ShowOverlay(pWindow->GetOverlayState());

			CGUIWindow::OnMessage(message);
			// If we were running, make sure we remove ourselves from the window manager
			if (m_bRunning)
			{
				g_windowManager.RemoveDialog(GetID());
				m_bRunning = false;
				m_dialogClosing = false;
				m_autoClosing = false;
			}
			return true;
		}
		case GUI_MSG_WINDOW_INIT:
		{
			CGUIWindow::OnMessage(message);
			m_showStartTime = CTimeUtils::GetFrameTime();
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

	// Play the window specific deinit sound
	if(!m_dialogClosing && m_enableSound)
		g_audioManager.PlayWindowSound(GetID(), SOUND_DEINIT);

	// Don't close if we should be animating
	if (!forceClose && HasAnimation(ANIM_TYPE_WINDOW_CLOSE))
	{
		if (!m_dialogClosing && !IsAnimating(ANIM_TYPE_WINDOW_CLOSE))
		{
			QueueAnimation(ANIM_TYPE_WINDOW_CLOSE);
			m_dialogClosing = true;
		}
		return;
	}

	CGUIMessage msg(GUI_MSG_WINDOW_DEINIT, 0, 0);
	OnMessage(msg);
}

void CGUIDialog::DoModal_Internal(int iWindowID /*= WINDOW_INVALID */, const CStdString &param /* = "" */)
{
	// Lock graphic context here as it is sometimes called from non rendering threads
	// maybe we should have a critical section per window instead??
	CSingleLock lock(g_graphicsContext);

	if (!g_windowManager.Initialized())
		return; // Don't do anything

	m_dialogClosing = false;
	m_bModal = true;
	
	// set running before it's added to the window manager, else the auto-show code
	// could show it as well if we are in a different thread from
	// the main rendering thread (this should really be handled via
	// a thread message though IMO)
	m_bRunning = true;
	g_windowManager.RouteToWindow(this);

	// Play the window specific init sound
	if (m_enableSound)
		g_audioManager.PlayWindowSound(GetID(), SOUND_INIT);

	// Active this window...
	CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0, WINDOW_INVALID, iWindowID);
	msg.SetStringParam(param);
	OnMessage(msg);

	if (!m_windowLoaded)
		Close(true);

	lock.Leave();

	while (m_bRunning && !g_application.IsStopping())
	{
		g_windowManager.ProcessRenderLoop();
	}
}

void CGUIDialog::DoModalThreadSafe()
{
	// We make sure we're threadsafe by sending via the application messenger
	ThreadMessage tMsg = {TMSG_DIALOG_DOMODAL, GetID(), g_windowManager.GetActiveWindow()};
	
	// First ensure we don't hold the graphics lock
	int numLocks = ExitCriticalSection(g_graphicsContext);
	g_application.getApplicationMessenger().SendMessage(tMsg, true);
	RestoreCriticalSection(g_graphicsContext, numLocks);
}

void CGUIDialog::Show_Internal()
{
	// Lock graphic context here as it is sometimes called from non rendering threads
	// maybe we should have a critical section per window instead??
	CSingleLock lock(g_graphicsContext);

	if (m_bRunning && !m_dialogClosing && !IsAnimating(ANIM_TYPE_WINDOW_CLOSE)) return;

	if (!g_windowManager.Initialized())
		return; // Don't do anything

	m_bModal = false;

	// Set running before it's added to the window manager, else the auto-show code
	// could show it as well if we are in a different thread from
	// the main rendering thread (this should really be handled via
	// a thread message though IMO)
	m_bRunning = true;
	m_dialogClosing = false;
	g_windowManager.AddModeless(this);

	// Play the window specific init sound
	if (m_enableSound)
		g_audioManager.PlayWindowSound(GetID(), SOUND_INIT);

	// Active this window...
	CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0, 0);
	OnMessage(msg);
}

void CGUIDialog::DoModal(int iWindowID /*= WINDOW_INVALID */, const CStdString &param)
{
	DoModal_Internal(iWindowID, param);
}

void CGUIDialog::Show()
{
	Show_Internal();
}

bool CGUIDialog::RenderAnimation(unsigned int time)
{
	CGUIWindow::RenderAnimation(time);
	return m_bRunning;
}

void CGUIDialog::FrameMove()
{
	if (m_autoClosing && m_showStartTime + m_showDuration < CTimeUtils::GetFrameTime() && !m_dialogClosing)
		Close();

	CGUIWindow::FrameMove();
}

void CGUIDialog::Render()
{
	CGUIWindow::Render();

	// Check to see if we should close at this point
	// We check after the controls have finished rendering, as we may have to close due to
	// the controls rendering after the window has finished it's animation
	// we call the base class instead of this class so that we can find the change
	if (m_dialogClosing && !CGUIWindow::IsAnimating(ANIM_TYPE_WINDOW_CLOSE))
	{
		Close(true);
	}
}

bool CGUIDialog::IsAnimating(ANIMATION_TYPE animType)
{
	if (animType == ANIM_TYPE_WINDOW_CLOSE)
		return m_dialogClosing;

	return CGUIWindow::IsAnimating(animType);
}

void CGUIDialog::SetDefaults()
{
	CGUIWindow::SetDefaults();
	m_renderOrder = 1;
}

void CGUIDialog::SetAutoClose(unsigned int timeoutMs)
{
	m_autoClosing = true;
	m_showDuration = timeoutMs;

	if (m_bRunning)
		m_showStartTime = CTimeUtils::GetFrameTime();
}