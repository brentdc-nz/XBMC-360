#include "Network.h"
#include "utils\Log.h"
#include "Application.h"

// Time to wait before we give up on network init
#define WAIT_TIME 10000

CNetwork::CNetwork()
{
	m_bInited = false;
	m_bNetworkUp = false;
	m_dwLastLink = 0;
	m_dwLastState = 0;
}

CNetwork::~CNetwork()
{
}

bool CNetwork::Initialize()
{
	WSADATA WsaData;
	DWORD dwState = 0;	
	
	memset(&m_networkinfo , 0, sizeof(m_networkinfo));

	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
	xnsp.cfgSockDefaultRecvBufsizeInK = 32; // default = 16
	xnsp.cfgSockDefaultSendBufsizeInK = 32; // default = 16 

	INT iResult = XNetStartup(&xnsp);

	if(iResult != NO_ERROR)
	{
		CLog::Log(LOGERROR, "InitNetwork","XNETSTARTUP ERROR");
		return false;
	}

	// Startup Winsock
	dwState = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if(NO_ERROR != dwState)
	{
		CLog::Log(LOGERROR, __FUNCTION__" - WSAStartup failed with error %d", dwState);
		return false;
	}

	m_bInited = true;
	return true;
}

void CNetwork::Deinitialize()
{
	if(m_bNetworkUp)
		NetworkDown();
  
	m_bInited = false;

	WSACleanup();
	XNetCleanup();
}

bool CNetwork::SetupNetwork()
{
	// Setup network based on our settings
	// Network will start its init procedure but ethernet must be connected
	if(IsEthernetConnected())
	{
		CLog::Log(LOGDEBUG, "%s - Setting up network...", __FUNCTION__);
    
//TODO
		Initialize();/*g_guiSettings.GetInt("network.assignment"),
		g_guiSettings.GetString("network.ipaddress").c_str(),
		g_guiSettings.GetString("network.subnet").c_str(),
		g_guiSettings.GetString("network.gateway").c_str(),
		g_guiSettings.GetString("network.dns").c_str());
 */     
		return true;
	}
  
	// Setup failed
	CLog::Log(LOGDEBUG, "%s - Not setting up network as ethernet is not connected!", __FUNCTION__);
	return false;
}

bool CNetwork::WaitForSetup(DWORD timeout)
{
	// Wait until the net is inited
	DWORD timestamp = GetTickCount() + timeout;

	do
	{
		DWORD dwState = UpdateState();
    
		if (IsEthernetConnected() && (dwState & XNET_GET_XNADDR_DHCP || dwState & XNET_GET_XNADDR_STATIC) && 
			!(dwState & XNET_GET_XNADDR_NONE || dwState & XNET_GET_XNADDR_TROUBLESHOOT || dwState & XNET_GET_XNADDR_PENDING))
			return true;
    
		Sleep(100);
	} while (GetTickCount() < timestamp);

	CLog::Log(LOGDEBUG, "%s - Waiting for network setup failed!", __FUNCTION__);
	return false;
}

bool CNetwork::IsEthernetConnected()
{
	if(!(XNetGetEthernetLinkStatus() & XNET_ETHERNET_LINK_ACTIVE))
		return false;

	return true;
}

bool CNetwork::CheckNetwork(int count)
{
	static DWORD lastLink;  // Will hold the last link, to notice changes
	static int netRetryCounter;

	// Get our network state
	DWORD dwState = UpdateState();
	DWORD dwLink = XNetGetEthernetLinkStatus();
  
	// Check the network status every count itterations
	if(++netRetryCounter > count || lastLink != dwLink)
	{
		netRetryCounter = 0;
		lastLink = dwLink;
    
		// In case the network failed, try to set it up again
		if( !(dwLink & XNET_ETHERNET_LINK_ACTIVE) || !IsInited() || dwState & XNET_GET_XNADDR_NONE || dwState & XNET_GET_XNADDR_TROUBLESHOOT )
		{
			Deinitialize();

			if(dwLink & XNET_ETHERNET_LINK_ACTIVE)
			{
				CLog::Log(LOGWARNING, "%s - Network error. Trying re-setup", __FUNCTION__);
				SetupNetwork();
				return true;
			}
		}
	}

	return false;
}

bool CNetwork::IsAvailable(bool wait)
{
	// If network isn't up, wait for it to setup
	if( !m_bNetworkUp && wait )
		WaitForSetup(WAIT_TIME);

	return m_bNetworkUp;
}

// Update network state, call repeatedly while return value is XNET_GET_XNADDR_PENDING
DWORD CNetwork::UpdateState()
{
	CSingleLock lock (m_critSection);
  
	XNADDR xna;
	DWORD dwState = XNetGetTitleXnAddr(&xna);
	DWORD dwLink = XNetGetEthernetLinkStatus();

	if(m_dwLastLink != dwLink || m_dwLastState != dwState)
	{
		if(m_bNetworkUp)
			NetworkDown();

		m_dwLastLink = dwLink;
		m_dwLastState = dwState;

		if((dwLink & XNET_ETHERNET_LINK_ACTIVE) && (dwState & XNET_GET_XNADDR_DHCP || dwState & XNET_GET_XNADDR_STATIC) && 
			!(dwState & XNET_GET_XNADDR_NONE || dwState & XNET_GET_XNADDR_TROUBLESHOOT || dwState & XNET_GET_XNADDR_PENDING))
			NetworkUp();
    
		LogState();
	}

	return dwState;
}

void CNetwork::NetworkDown()
{
	CLog::Log(LOGDEBUG, "%s - Network service is down", __FUNCTION__);
  
//	memset(&m_networkinfo, 0, sizeof(m_networkinfo)); // TODO
	m_dwLastLink = 0;
	m_dwLastState = 0;
	m_bNetworkUp = false;

	g_application.getApplicationMessenger().NetworkMessage(SERVICES_DOWN, 0);
}

void CNetwork::NetworkUp()
{
    CLog::Log(LOGDEBUG, "%s - Network service is up", __FUNCTION__);
  
    XNADDR xnaddr;
    IN_ADDR ipaddr, lipaddr;
    memset(&xnaddr, 0, sizeof(XNADDR));
    memset(&ipaddr, 0, sizeof(IN_ADDR));
    memset(&lipaddr, 0, sizeof(IN_ADDR));
    DWORD dwState;
	
    do
    {
        dwState = XNetGetTitleXnAddr(&xnaddr);
    } while (dwState == XNET_GET_XNADDR_PENDING);

    strcpy(m_networkinfo.ip, "N/A");

    ipaddr = xnaddr.ina;

    if(memcmp(&ipaddr, &lipaddr, sizeof(IN_ADDR)) != 0)
    {
        char szip[16];
        XNetInAddrToString(xnaddr.ina, szip, 16);
        strcpy(m_networkinfo.ip, szip);
    }

    // TODO - Other network info!

    // Get the current status 
    /*	TXNetConfigStatus status;
    XNetGetConfigStatus(&status);

    // Fill local network info
    strcpy(m_networkinfo.ip, inet_ntoa(status.ip));
    strcpy(m_networkinfo.subnet, inet_ntoa(status.subnet));
    strcpy(m_networkinfo.gateway, inet_ntoa(status.gateway));
    strcpy(m_networkinfo.dhcpserver, "");
    strcpy(m_networkinfo.DNS1, inet_ntoa(status.dns1));
    strcpy(m_networkinfo.DNS2, inet_ntoa(status.dns2));

    m_networkinfo.DHCP = !(status.dhcp == 0);
    */
    m_bNetworkUp = true;
  
    g_application.getApplicationMessenger().NetworkMessage(SERVICES_UP, 0);
}

void CNetwork::NetworkMessage(EMESSAGE message, DWORD dwParam)
{
	switch(message)
	{
		case SERVICES_UP:
		{
			CLog::Log(LOGDEBUG, "%s - Starting network services",__FUNCTION__);

			g_application.StartTimeServer();
			g_application.StartFtpServer();
			// TODO: Add the other services smb, etc
		}
		break;

		case SERVICES_DOWN:
		{
			CLog::Log(LOGDEBUG, "%s - Stopping network services",__FUNCTION__);

			g_application.StopTimeServer();
//			g_application.StopFtpServer();
		}
		break;
	}
}

void CNetwork::LogState()
{
	DWORD dwLink = m_dwLastLink;
	DWORD dwState = m_dwLastState;

	if(dwLink & XNET_ETHERNET_LINK_FULL_DUPLEX)
		CLog::Log(LOGINFO, __FUNCTION__" - Link: full duplex");

	if(dwLink & XNET_ETHERNET_LINK_HALF_DUPLEX)
		CLog::Log(LOGINFO, __FUNCTION__" - Link: half duplex");

	if(dwLink & XNET_ETHERNET_LINK_100MBPS)
		CLog::Log(LOGINFO, __FUNCTION__" - Link: 100 mbps");

	if(dwLink & XNET_ETHERNET_LINK_10MBPS)
		CLog::Log(LOGINFO, __FUNCTION__" - Link: 10 mbps");
    
	if(!(dwLink & XNET_ETHERNET_LINK_ACTIVE))
		CLog::Log(LOGINFO, __FUNCTION__" - Link: none");

	if(dwState & XNET_GET_XNADDR_DNS)
		CLog::Log(LOGINFO, __FUNCTION__" - State: dns");

	if(dwState & XNET_GET_XNADDR_ETHERNET)
		CLog::Log(LOGINFO, __FUNCTION__" - State: ethernet");

	if(dwState & XNET_GET_XNADDR_NONE)
		CLog::Log(LOGINFO, __FUNCTION__" - State: none");

	if(dwState & XNET_GET_XNADDR_ONLINE)
		CLog::Log(LOGINFO, __FUNCTION__" - State: online");

	if(dwState & XNET_GET_XNADDR_PENDING)
		CLog::Log(LOGINFO, __FUNCTION__" - State: pending");

	if(dwState & XNET_GET_XNADDR_TROUBLESHOOT)
		CLog::Log(LOGINFO, __FUNCTION__" - State: error");

	if(dwState & XNET_GET_XNADDR_PPPOE)
		CLog::Log(LOGINFO, __FUNCTION__" - State: pppoe");

	if(dwState & XNET_GET_XNADDR_STATIC)
		CLog::Log(LOGINFO, __FUNCTION__" - State: static");

	if(dwState & XNET_GET_XNADDR_DHCP)
		CLog::Log(LOGINFO, __FUNCTION__" - State: dhcp");

	//TODO
//	CLog::Log(LOGINFO,  "%s - ip: %s", __FUNCTION__, m_networkinfo.ip);
//	CLog::Log(LOGINFO,  "%s - subnet: %s", __FUNCTION__, m_networkinfo.subnet);
//	CLog::Log(LOGINFO,  "%s - gateway: %s", __FUNCTION__, m_networkinfo.gateway);
//	CLog::Log(LOGINFO,  "%s - dns: %s, %s", __FUNCTION__, m_networkinfo.DNS1, m_networkinfo.DNS2);
}