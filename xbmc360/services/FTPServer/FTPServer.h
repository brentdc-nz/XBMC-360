#ifndef CFTPSERVER_H
#define CFTPSERVER_H

#include "utils\Thread.h"
#include "utils\StdString.h"
#include "FTPServerConn.h"
#include <vector>

class CFTPServer : public CThread
{
public:
	CFTPServer();
	~CFTPServer();

	void Start();
	void GetAllConnections(std::vector<CFTPServerConn*>& vecConns);
	void CloseConnection(int iID);

private:
	std::vector<CFTPServerConn*> m_vecConns;
	int m_iCurrentID;
	int m_iXferPort;
	int m_iXferPortStart;
	int m_iXferPortRange; 
	int m_iPort;
	CStdString m_strXboxIP;
	char m_PortsUsed[256];

	virtual void Process();
};

#endif //CFTPSERVER_H