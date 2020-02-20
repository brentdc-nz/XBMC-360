#ifndef CNETWORK_H
#define CNETWORK_H

#include "..\utils\SingleLock.h"

class CNetwork
{
public:
	enum EMESSAGE
	{
		SERVICES_UP,
		SERVICES_DOWN
	};

	CNetwork();
	~CNetwork();

	bool Initialize();
	void Deinitialize();

	bool SetupNetwork();
	bool WaitForSetup(DWORD timeout);
	bool IsEthernetConnected();
	bool CheckNetwork(int count);

	// Updates and returns current network state
	// will return pending if network is not up and running
	// should really be called once in a while should network be unplugged
	DWORD UpdateState();
	bool IsAvailable(bool wait = false);
	bool IsInited() { return m_bInited; }

	// Callback from application controlled thread to handle any setup
	void NetworkMessage(EMESSAGE message, DWORD dwParam);

private:
	bool m_bInited; // True if initalized() has been called
	bool m_bNetworkUp;  /* true if network is available */
	DWORD m_dwLastLink; // Will hold the last link, to notice changes
	DWORD m_dwLastState;  // Will hold the last state, to notice changes

	void LogState();

	void NetworkDown();
	void NetworkUp();
	CCriticalSection  m_critSection;
};

#endif //CNETWORK_H