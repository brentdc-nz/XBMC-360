#ifndef CNETWORK_H
#define CNETWORK_H

#include "utils\CriticalSection.h"

struct network_info
{
	char ip[32];
	char gateway[32];
	char subnet[32];
	char DNS1[32];
	char DNS2[32];
	bool DHCP;
	char dhcpserver[32];
	char mac[32];
	// XNet tells a title whether a gateway and DNS servers are set up, but not their addresses
	bool hasGateway;
	bool hasDNS;
};

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

	enum INTERNET_STATE
	{
		INTERNET_UNKNOWN,       // Not checked yet
		INTERNET_CONNECTED,
		INTERNET_NO_DNS,        // Gets out to the Internet, but names don't resolve
		INTERNET_NOT_CONNECTED
	};

	// Whether the Internet can be reached. Asking starts a check in the background
	// when the last one is older than 30 seconds; until it ends, the last result
	INTERNET_STATE GetInternetState();
	void OnInternetChecked(INTERNET_STATE state);

	struct network_info m_networkinfo;

private:
	bool m_bInited; // True if initalized() has been called
	bool m_bNetworkUp; // True if network is available
	DWORD m_dwLastLink; // Will hold the last link, to notice changes
	DWORD m_dwLastState;  // Will hold the last state, to notice changes

	INTERNET_STATE m_internetState;
	DWORD m_dwInternetChecked; // When the last check started
	bool m_bCheckingInternet;

	void LogState();

	void NetworkDown();
	void NetworkUp();
	CCriticalSection  m_critSection;
};

#endif //CNETWORK_H