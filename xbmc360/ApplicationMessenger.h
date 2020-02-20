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
#ifndef H_CAPPLICATIONMESSENGER
#define H_CAPPLICATIONMESSENGER

#include "utils\StdString.h"
#include "utils\Stdafx.h"
#include "utils\SingleLock.h"
#include <queue>

// Defines here
#define TMSG_SHUTDOWN             300
#define TMSG_POWERDOWN            301
#define TMSG_REBOOT               306
#define TMSG_NETWORKMESSAGE       500

typedef struct
{
	DWORD dwMessage;
	DWORD dwParam1;
	DWORD dwParam2;
	CStdString strParam;
	std::vector<CStdString> params;
	HANDLE hWaitEvent;
	LPVOID lpVoid;
}
ThreadMessage;

class CApplicationMessenger
{
public:
	~CApplicationMessenger();

	void Cleanup();
	// If a message has to be send to the gui, use MSG_TYPE_WINDOW instead
	void SendMessage(ThreadMessage& msg, bool wait = false);
	void ProcessMessages(); // only call from main thread.

	void Shutdown();
	void Reboot();
	void NetworkMessage(DWORD dwMessage, DWORD dwParam = 0);

private:
	void ProcessMessage(ThreadMessage *pMsg);

	CCriticalSection m_critSection;
	std::queue<ThreadMessage*> m_vecMessages;
};

#endif //H_CAPPLICATIONMESSENGER