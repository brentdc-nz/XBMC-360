#include "ApplicationMessenger.h"
#include "Application.h"
#include "xbox\XBKernalExports.h"
#include "interfaces\Builtins.h"
#include "guilib\GUIWindowManager.h"

using namespace std;

CApplicationMessenger::~CApplicationMessenger()
{
	Cleanup();
}

void CApplicationMessenger::Cleanup()
{
	CSingleLock lock (m_critSection);

	while (m_vecMessages.size() > 0)
	{
		ThreadMessage* pMsg = m_vecMessages.front();

		if (pMsg->hWaitEvent)
			SetEvent(pMsg->hWaitEvent);

		delete pMsg;
		m_vecMessages.pop();
	}

	while (m_vecWindowMessages.size() > 0)
	{
		ThreadMessage* pMsg = m_vecWindowMessages.front();

		if (pMsg->hWaitEvent)
			SetEvent(pMsg->hWaitEvent);

		delete pMsg;
		m_vecWindowMessages.pop();
	}
}

void CApplicationMessenger::SendMessage(ThreadMessage& message, bool wait)
{
	ThreadMessage* msg = new ThreadMessage();
	msg->dwMessage = message.dwMessage;
	msg->dwParam1 = message.dwParam1;
	msg->dwParam2 = message.dwParam2;
	msg->hWaitEvent = message.hWaitEvent;
	msg->lpVoid = message.lpVoid;
	msg->strParam = message.strParam;

	CSingleLock lock (m_critSection);

	if (msg->dwMessage == TMSG_DIALOG_DOMODAL/* ||
      msg->dwMessage == TMSG_WRITE_SCRIPT_OUTPUT*/) // TODO
	{
		m_vecWindowMessages.push(msg);
	}
	else m_vecMessages.push(msg);

	lock.Leave();

	if (message.hWaitEvent)
	{
		WaitForSingleObject(message.hWaitEvent, INFINITE);
	}
}

void CApplicationMessenger::ProcessMessages()
{
	// Process threadmessages
	CSingleLock lock (m_critSection);

	while (m_vecMessages.size() > 0)
	{
		ThreadMessage* pMsg = m_vecMessages.front();
		// First remove the message from the queue, else the message could be processed more then once
		m_vecMessages.pop();

		// Leave here as the message might make another
		// thread call processmessages or sendmessage
		lock.Leave();

		ProcessMessage(pMsg);

		if (pMsg->hWaitEvent)
			SetEvent(pMsg->hWaitEvent);

		delete pMsg;

		// Reenter here again, to not ruin message vector
		lock.Enter();
	}
}

void CApplicationMessenger::ProcessMessage(ThreadMessage *pMsg)
{
	switch (pMsg->dwMessage)
	{
		case TMSG_SHUTDOWN:
		case TMSG_POWERDOWN:
		{
			g_application.Stop();
			Sleep(200);

			CXBKernalExports::ShutdownXbox();
			while(1){Sleep(0);}
		}
		break;

		case TMSG_REBOOT:
		{
			g_application.Stop();
			Sleep(200);
			CXBKernalExports::RebootXbox();
			while(1){Sleep(0);}
		}
		break;

		case TMSG_SWITCHTOFULLSCREEN:
			if( g_windowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO )
				g_application.SwitchToFullScreen();
		break;

		case TMSG_EXECUTE_BUILT_IN:
			CBuiltins::Execute(pMsg->strParam.c_str());
		break;

		// Window messages below here...
		case TMSG_DIALOG_DOMODAL: //doModel of window
		{
			CGUIDialog* pDialog = (CGUIDialog*)g_windowManager.GetWindow(pMsg->dwParam1);
			if (!pDialog) return;

			pDialog->DoModal();
		}
		break;

		case TMSG_NETWORKMESSAGE:
		{
			g_application.getNetwork().NetworkMessage((CNetwork::EMESSAGE)pMsg->dwParam1, pMsg->dwParam2);
		}
		break;
	}
}

void CApplicationMessenger::ProcessWindowMessages()
{
	CSingleLock lock (m_critSection);

	// Message type is window, process window messages
	while (m_vecWindowMessages.size() > 0)
	{
		ThreadMessage* pMsg = m_vecWindowMessages.front();

		// First remove the message from the queue, else the message could be processed more then once
		m_vecWindowMessages.pop();

		// Leave here in case we make more thread messages from this one
		lock.Leave();

		ProcessMessage(pMsg);
		
		if (pMsg->hWaitEvent)
			SetEvent(pMsg->hWaitEvent);

		delete pMsg;

		lock.Enter();
	}
}

void CApplicationMessenger::Shutdown()
{
	ThreadMessage tMsg = {TMSG_SHUTDOWN};
	SendMessage(tMsg);
}

void CApplicationMessenger::Reboot()
{
	ThreadMessage tMsg = {TMSG_REBOOT};
	SendMessage(tMsg);
}

void CApplicationMessenger::SwitchToFullscreen()
{
	// FIXME: Ideally this call should return upon a successfull switch but currently
	// is causing deadlocks between the DVDPlayer destructor and the rendermanager
	ThreadMessage tMsg = {TMSG_SWITCHTOFULLSCREEN};
	SendMessage(tMsg, false);
}

void CApplicationMessenger::NetworkMessage(DWORD dwMessage, DWORD dwParam)
{
	ThreadMessage tMsg = {TMSG_NETWORKMESSAGE, dwMessage, dwParam};
	SendMessage(tMsg);
}

void CApplicationMessenger::ExecBuiltIn(const CStdString &command)
{
	ThreadMessage tMsg = {TMSG_EXECUTE_BUILT_IN};
	tMsg.strParam = command;
	SendMessage(tMsg);
}