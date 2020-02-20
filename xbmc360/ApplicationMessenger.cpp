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

#include "ApplicationMessenger.h"
#include "Application.h"
#include "xbox\XBKernalExports.h"

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

/*	if (msg->dwMessage == TMSG_DIALOG_DOMODAL ||
      msg->dwMessage == TMSG_WRITE_SCRIPT_OUTPUT)
	{
		m_vecWindowMessages.push_back(msg);
	}
	else*/ m_vecMessages.push(msg);

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

		case TMSG_NETWORKMESSAGE:
		{
			g_application.getNetwork().NetworkMessage((CNetwork::EMESSAGE)pMsg->dwParam1, pMsg->dwParam2);
		}
		break;
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

void CApplicationMessenger::NetworkMessage(DWORD dwMessage, DWORD dwParam)
{
	ThreadMessage tMsg = {TMSG_NETWORKMESSAGE, dwMessage, dwParam};
	SendMessage(tMsg);
}
