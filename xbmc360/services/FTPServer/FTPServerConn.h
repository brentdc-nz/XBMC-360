#ifndef CFTPSERVERCONN_H
#define CFTPSERVERCONN_H

#include "utils\Thread.h"
#include "utils\StdString.h"
#include "..\..\filesystem\FileBrowser.h"

// FTP Command tokens
typedef enum {
	USER, PASS, CWD,  PORT, 
	QUIT, PASV, ABOR, DELE,
	RMD,  XRMD, MKD,  XMKD,
	PWD,  XPWD, LIST, NLST, 
	SYST, TYPE, MODE, RETR, 
	STOR, REST, RNFR, RNTO,
	STAT, NOOP, MDTM, xSIZE, 
	EXEC, REBO, CDUP, RAM,
	FEAT, XCRC, MFMT, SCRN,
	SHTD,
    UNKNOWN_COMMAND, INVALID_COMMAND = -1
} CmdTypes;

struct command_list
{
    char *command;
    CmdTypes CmdNum;
};

#define XFERSIZE 1024*1024

typedef struct
{
    char *command;
    CmdTypes CmdNum;
} Lookup_t;

static const Lookup_t CommandLookup[] =
{
	"USER", USER, "PASS", PASS, "CWD",  CWD,  "PORT", PORT, 
	"QUIT", QUIT, "PASV", PASV, "ABOR", ABOR, "DELE", DELE,
	"RMD",  RMD,  "XRMD", XRMD, "MKD",  MKD,  "XMKD", XMKD,
	"PWD",  PWD,  "XPWD", XPWD, "LIST", LIST, "NLST", NLST,  
	"SYST", SYST, "TYPE", TYPE, "MODE", MODE, "RETR", RETR,
	"STOR", STOR, "REST", REST, "RNFR", RNFR, "RNTO", RNTO,
	"STAT", STAT, "NOOP", NOOP, "MDTM", MDTM, "SIZE", xSIZE,
	"EXEC", EXEC, "REBO", REBO, "CDUP", CDUP, "RAM", RAM,
	"FEAT", FEAT, "XCRC", XCRC, "MFMT", MFMT, "SCREENSHOT", SCRN,
	"SHUTDOWN", SHTD
};

class CFTPServerConn : public CThread
{
public:
	CFTPServerConn();
	~CFTPServerConn();

	virtual void Process();

	void SetID(int iID) { m_iID = iID; }
	int GetID() { return m_iID; }

	int m_iCommandSocket;
    int m_iXferPort;
	CStdString m_strXboxIP;

private:
	void SendReply(char* reply);
	int CreateTcpipSocket(int* iPort);
	CmdTypes GetCommand(char *CmdArg);
	CStdString GetPath(char* requested);
	CStdString GetCurrentDir();
	int ConnectTcpip(struct sockaddr_in *addr);
	void Send550Error();
	CStdString PathToFileSystemPath(CStdString strPath);
	
	void Cmd_XCRC(const char *filename);
	void Cmd_NLST(CStdString strFilename, bool bLong, bool bUseCtrlConn);
	void Cmd_STOR(const char *filename);
	void Cmd_RETR(const char *filename);
	void Cmd_EXEC(const char *filename);

	int m_iID;
	bool m_bActive;
	bool m_bGotUser;
	bool m_bIsLoggedIn;
	int m_iPassiveSocket;

	char XferBuffer[XFERSIZE];
    struct sockaddr_in m_xfer_addr;
    bool m_bPassiveMode;

	CFileBrowser* m_pFileBrowser;
};

#endif //CFTPSERVERCONN_H