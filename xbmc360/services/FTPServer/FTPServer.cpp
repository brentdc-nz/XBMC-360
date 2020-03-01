#include "FTPServer.h"
#include "..\..\utils\log.h"
#include "winsockx.h"

CFTPServer::CFTPServer()
{
	m_iCurrentID = 0;
}

CFTPServer::~CFTPServer()
{
}

void CFTPServer::Start()
{
	XNADDR addr;
	XNetGetTitleXnAddr(&addr);

	char ip[16];
	sprintf_s(ip, 16, "%d.%d.%d.%d", (byte)addr.ina.S_un.S_un_b.s_b1,
		(byte)addr.ina.S_un.S_un_b.s_b2,
		(byte)addr.ina.S_un.S_un_b.s_b3,
		(byte)addr.ina.S_un.S_un_b.s_b4);

	CLog::Log(LOGNOTICE, "FTP Server : XNetGetTitleXnAddr returned %s", ip);
	
	CStdString sIp = ip;
	m_strXboxIP = sIp; //cl.xboxip;	
	m_iPort = 21; //TODO: Make a config

	Create(false/*CPU2_THREAD_1*/);
}

void CFTPServer::GetAllConnections(std::vector<CFTPServerConn*> &vecConns)
{
	// TODO: lock and release the connectiosn here
	for(int i = 0; i < (int)m_vecConns.size(); i++)
	{
		CFTPServerConn* pConnection = m_vecConns[i];

		if(pConnection)
			vecConns.push_back(pConnection);
	}
}

void CFTPServer::CloseConnection(int iID)
{
	for(int i = 0; i < (int)m_vecConns.size(); i++)
	{
		CFTPServerConn* pConnection = m_vecConns[i];

		if(pConnection)
		{
			if(pConnection->GetID() == iID)
			{
//				pConnection->CloseConection(); // TODO: Fix this leak asap!
				delete pConnection;
			}
		}
	}
}

void CFTPServer::Process()
{
	SetName("FTP Server");
	SOCKET server;

	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons((u_short)m_iPort);

    m_iXferPortStart = m_iXferPortRange = m_iXferPort = 0;
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(server == INVALID_SOCKET)
	{
		CLog::Log(LOGERROR, "FTP Server: Invalid socket!");
		return;
	}

	// After setting these undocumented flags on a socket they should then run unencrypted
	BOOL bBroadcast = TRUE;

	if(setsockopt(server, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL)) != 0)
	{
		CLog::Log(LOGERROR, "FTP Server: Failed to set socket to 5802, error");
//		return;
	}

	if(setsockopt(server, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL)) != 0)
	{
		CLog::Log(LOGERROR, "FTP Server: Failed to set socket to 5801, error");
//		return;
	}

	if(bind(server, (const sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();
		CLog::Log(LOGERROR, "FTP Server: Bind error %d", Error);
		return; 
	}

	while(listen(server, SOMAXCONN) != SOCKET_ERROR)
	{
		SOCKET client;
		int length;

		length = sizeof(local);
		CLog::Log(LOGNOTICE, "FTP Server: Trying to accept");
		client = accept(server, (sockaddr*)&local, &length);

		CFTPServerConn* pConn = new CFTPServerConn();

		pConn->SetID(m_iCurrentID);
		pConn->m_iCommandSocket = client;
		CLog::Log(LOGNOTICE, "FTP Server: New Connection, XFERPORT : %d", m_iXferPort);
		CLog::Log(LOGNOTICE, "FTP Server: Xbox Ip : %s", m_strXboxIP.c_str());
		pConn->m_iXferPort = m_iXferPort;
		pConn->m_strXboxIP = m_strXboxIP;
		CLog::Log(LOGNOTICE, "FTP Server: Xbox Ip : %s", pConn->m_strXboxIP.c_str());
        m_PortsUsed[m_iXferPort] = 1;

		m_vecConns.push_back(pConn);
		m_iCurrentID++;

		pConn->Create(false/*CPU2_THREAD_1*/);
//		SetThreadPriority(conn->hThread,THREAD_PRIORITY_HIGHEST);

        // Cycle through port numbers.
		m_iXferPort = m_iXferPortStart;
		for(;m_iXferPort < m_iXferPortRange; m_iXferPort++)
		{
            // Find an unused port numbeer 
            // This code only relevant if a port range was specified
            if (!m_PortsUsed[m_iXferPort & 255])
				break;
        }
	}
}