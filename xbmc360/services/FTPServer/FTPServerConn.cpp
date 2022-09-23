#include "FTPServerConn.h"
#include "FTPFile.h"
#include "utils\log.h"
#include "utils\StringUtils.h"
#include "utils\Util.h"
#include "utils\CRC_32.h"
#include <direct.h>
using XFILE::CFile;
CFTPServerConn::CFTPServerConn()
{
	m_iID = -1;
	m_bActive = false;
	m_bGotUser = false;
	m_bIsLoggedIn = false;
	m_bPassiveMode = false;
}

CFTPServerConn::~CFTPServerConn()
{
}

void CFTPServerConn::SendReply(char* Reply)
{
	char ReplyStr[MAX_PATH+20];
	printf("    %s\n",Reply);
	sprintf_s(ReplyStr, MAX_PATH+20, "%s\r\n", Reply);
	send(m_iCommandSocket, ReplyStr, strlen(ReplyStr),0);
}

void CFTPServerConn::Process()
{
	m_bActive = true;

    char buf[MAX_PATH+10];
    char repbuf[MAX_PATH+10];
 
	// Indicate ready to accept commands
    SendReply("220 XBMC-360 FTPD ready");
	
	m_iPassiveSocket = CreateTcpipSocket(&m_iXferPort);

	if(m_iPassiveSocket < 0)
	{
		closesocket(m_iCommandSocket);
		closesocket(m_iPassiveSocket);
		m_bActive = false;
		return;
	}

	if(listen(m_iPassiveSocket, 1) == -1)
	{
		CLog::Log(LOGERROR, "FTP Server : passive socket listen failed");
		closesocket(m_iCommandSocket);
		closesocket(m_iPassiveSocket);
		m_bActive = false;
		return;
    }

	CStdString strNewPath = "";

	for(;;)
	{
		// Get FTP command from input stream
		CmdTypes FtpCommand;

		CLog::Log(LOGDEBUG, "FTPServer : Conn buf: [[[[%s]]]]", buf);
        FtpCommand = GetCommand(buf);

		CStdString commandString = buf;
		CStdString strCurPath = "";

		switch(FtpCommand)
		{
            case USER:
				CLog::Log(LOGDEBUG, "FTPServer : User %s %d %d", buf, strcmp(commandString.c_str(), "EasyUser"), commandString.length());

				if(commandString.compare(/*getFtpUser()*/"xbox") == 0) // TODO : Make config
				{
					m_bGotUser = true;

					CLog::Log(LOGNOTICE, "FTPServerConn: New FileBrowser object");
					m_pFileBrowser = new CFileBrowser();

					SendReply("331 User name okay, need password.");
				}
				else 
					SendReply("332 Need account for login.");
                break;

            case PASS:
				if(m_bGotUser && commandString.compare(/*getInstance().getFtpPass()*/"xbox") == 0) // TODO : Make config
				{
					SendReply("230 User logged in, proceed.");
					m_bIsLoggedIn = true;
				}
				else
				{
					SendReply("530 Incorrect Password");
					Sleep(2000);
				}
                break;

            case SYST:
                SendReply("215 XBMC-360 FTPd");
                break;

#if 0 // TODO
			case RAM:
				sprintf_s(repbuf, MAX_PATH+10, "250 (%4u total mb of ram free)", GetAvailableRam()/(1024*1024));
				SendReply(repbuf);
				break;
#endif
#if 0 // TODO
			case PASV:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				sprintf_s(repbuf, MAX_PATH+10, "227 Entering Passive Mode (%s,%d,%d)", m_strXboxIP.c_str(), m_iXferPort >>8, m_iXferPort & 0xff);
                for (int a = 0; a < 50; a++)
				{
					if(repbuf[a] == 0) break;
					if(repbuf[a] == '.') repbuf[a] = ',';
                }
				SendReply(repbuf);
				m_bPassiveMode = true; 
				break;
#endif
            case XPWD:
            case PWD: // Print working directory
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				sprintf_s(repbuf, MAX_PATH+10, "257 \"%s\"", GetCurrentDir().c_str());
				SendReply(repbuf);
				break;

			case CDUP:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				if(m_pFileBrowser->IsAtRoot())
					SendReply("550 CDUP command failed: already at root");
				else
				{
					m_pFileBrowser->UpDirectory();
					SendReply("250 CDUP command successful");
				}
				break;

            case NLST: // Request directory, names only
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
                Cmd_NLST(buf, false, FALSE);
                break;

			case LIST: 
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				Cmd_NLST(buf, false, FALSE);
				break;

            case STAT:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				Cmd_NLST(buf, false, TRUE);
				break;

            case DELE:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				strNewPath = GetPath(buf);

				if(unlink(strNewPath.c_str()))
					Send550Error();
				else
					SendReply("250 DELE command successful.");
				break;

            case RMD:
            case MKD:
            case XMKD:
            case XRMD:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				strNewPath = GetPath(buf);

				if(FtpCommand == MKD || FtpCommand == XMKD)
				{
					if(_mkdir(strNewPath.c_str()))
						Send550Error();
					else
						SendReply("257 Directory created");
                }
				else
				{
					CLog::Log(LOGDEBUG, "FTP Server : Delete %s", strNewPath.c_str());
					if(RemoveDirectory(strNewPath.c_str()) != 0)
						SendReply("250 RMD command successful");
					else
					{
						DWORD err = GetLastError();
						CLog::Log(LOGDEBUG, "FTPServerConn: Error %08x",err);
						Send550Error();
                    }
				}
                break;

            case RNFR:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
                strNewPath = GetPath(buf);

				if(CUtil::FileExists(strNewPath))
                    SendReply("350 File Exists");
                else
                    SendReply("550 Path permission error");
                break;
              
            case RNTO:
				{
					if(!m_bIsLoggedIn)
					{
						SendReply("530 Not logged in.");
						break;
					}
					
					// Must be immediately preceeded by RNFR!
					CStdString strRnToPath = GetPath(buf);

					if(rename(strNewPath.c_str(), strRnToPath.c_str()))
					    Send550Error();
					else
					    SendReply("250 RNTO command successful");
				}
                break;

            case ABOR:
                SendReply("226 Aborted");
                break;

			case MDTM: 				
				if(strlen(buf) == 0)
					SendReply("501 Syntax error in parameters or arguments.");
				else if(!m_bIsLoggedIn)
					SendReply("530 Not logged in.");
				else
				{
					strNewPath = GetPath(buf);
					HANDLE hFile = ::CreateFile(strNewPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
					if(hFile == INVALID_HANDLE_VALUE)
					{
						sprintf_s(repbuf, "550 \"%s\": File not found.", strNewPath.c_str());
						SendReply(repbuf);
					}
					else
					{
						FILETIME ft;
						SYSTEMTIME st;
						GetFileTime(hFile, 0, 0, &ft);
						CloseHandle(hFile);
						FileTimeToSystemTime(&ft, &st);
						sprintf_s(repbuf, "213 %04u%02u%02u%02u%02u%02u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
						SendReply(repbuf);
					}							
				}
				break;

			case MFMT:
				if(strlen(buf) == 0)
					SendReply("501 Syntax error in parameters or arguments.");
				else if(!m_bIsLoggedIn)
					SendReply("530 Not logged in.");
				else
				{
					int i;
					SYSTEMTIME st;
					st.wDayOfWeek = 0;
					st.wMilliseconds = 0;
					
					char* lpBuf = &buf[0];
		
					for (i = 0; i < 14; i++)
					{
						if((buf[i] < '0') || (buf[i] > '9'))
							break;
					}

					if((i == 14) && (buf[14] == ' '))
					{
						strncpy_s(repbuf, buf, 4);
						repbuf[4] = 0;
						st.wYear = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 4, 2);
						repbuf[2] = 0;
						st.wMonth = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 6, 2);
						st.wDay = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 8, 2);
						st.wHour = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 10, 2);
						st.wMinute = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 12, 2);
						st.wSecond = (WORD)atoi(repbuf);
						lpBuf += 15;
					}
					else
					{
						SendReply("501 Syntax error in parameters or arguments.");
						break;
					}
					strNewPath = GetPath(lpBuf);

					HANDLE hFile = CreateFileA(strNewPath.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0 );
					if(hFile == INVALID_HANDLE_VALUE)
					{
						sprintf_s(repbuf, "550 \"%s\": File not found.", strNewPath.c_str());
						SendReply(repbuf);
					}
					else
					{
						FILETIME ft;
						SystemTimeToFileTime(&st, &ft);
						BOOL b = SetFileTime(hFile, 0, 0, &ft);
						DWORD gle = GetLastError();
						CLog::Log(LOGDEBUG, "FTP Server : %d %d SYSTEMTIME %d %d %d %d %d %d", b, gle, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
						CloseHandle(hFile);
						SendReply("250 MFMT command successful.");
					}							
				}
				break;

            case CWD: // Change working directory
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				CLog::Log(LOGDEBUG, "FTPServerConn: CWD");

				if(strcmp(buf, "..") == 0)
				{
					if(m_pFileBrowser->IsAtRoot())
						SendReply("550 CDUP command failed: already at root");
					else
					{
						m_pFileBrowser->UpDirectory();
						SendReply("250 CDUP command successful");
					}
					break;
				}

				strNewPath = PathToFileSystemPath(buf);
				m_pFileBrowser->CD(strNewPath);
				
				if(strNewPath.length() >= 1)
				{
					if(strNewPath.substr(strNewPath.length() - 1, 1) == "\\")
						strNewPath = strNewPath.substr(0, strNewPath.length() - 1);
				}

				strCurPath = m_pFileBrowser->GetCurrentPath();
				if(strCurPath.length() >= 1)
				{
					if(strCurPath.substr(strCurPath.length() - 1, 1) == "\\")
						strCurPath = strCurPath.substr(0, strCurPath.length() - 1);
				}

				if(stricmp(strNewPath.c_str(), strCurPath.c_str()) == 0)
					SendReply("250 OK");
				else
				{
					sprintf_s(repbuf, "550 \"%s\": Path not found.", strNewPath.c_str());
					SendReply(repbuf);
				}
                break;

			case TYPE: // Accept file TYPE commands, but ignore
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
                SendReply("200 Type set to I");
                break;

            case NOOP:
                SendReply("200 OK");
                break;

			case PORT: // Set the TCP/IP address for transfers
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
                {
					CLog::Log(LOGDEBUG, "FTPServerConn: Port %s", buf);
                    int h1,h2,h3,h4,p1,p2;
                    char *a, *p;
                    sscanf_s(buf,"%d,%d,%d,%d,%d,%d",&h1,&h2,&h3,&h4,&p1,&p2);
					m_xfer_addr.sin_family = AF_INET;
                    a = (char *) &m_xfer_addr.sin_addr;
                    p = (char *) &m_xfer_addr.sin_port;
                    a[0] = (char)h1; a[1] = (char)h2; a[2] = (char)h3; a[3] = (char)h4;
                    p[0] = (char)p1; p[1] = (char)p2;
					m_bPassiveMode = false;
                }
                SendReply("200 PORT command successful");
                break;

			case FEAT:
                SendReply("211-Extensions supported:\r\n XCRC filename\r\n SIZE\r\n REST STREAM\r\n MDTM\r\n MFMT\r\n TVFS\r\n211 END");
				break;

			case XCRC: // Not sure this should be protected by check for isLoggedIn, so not checking since it is harmless
				strNewPath =GetPath(buf);
				Cmd_XCRC(strNewPath.c_str());
				break;

			case RETR: // Retrieve File and send it
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				/*if(NewPath == NULL)
				{
					SendReply(Conn, "550 Path permission error");
					break;
				}*/
				Cmd_RETR(buf);
				break;

			case STOR: // Store the file
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				Cmd_STOR(buf);
				break;

			case UNKNOWN_COMMAND:
				SendReply("500 command not recognized");
				break;

            case QUIT: 
				SendReply("221 goodbye");
				closesocket(m_iCommandSocket);
				closesocket(m_iPassiveSocket);
				m_bActive = false;
				return;

			case EXEC:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
                //NewPath = TranslatePath(buf);
				Cmd_EXEC(buf);
				break;

#if 0 // TODO
			case SCRN:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
	
				ScreenShot();
				SendReply("200 Screenshot taken.");
				break;
#endif

#if 0 // TODO		
			case SHTD:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				SendReply("221 goodbye");
				closesocket(CommandSocket);
				closesocket(PassiveSocket);
				active = false;
				g_application.getApplicationMessenger().Shutdown();
				break;
#endif

#if 0 // TODO
			case REBO:
				if(!m_bIsLoggedIn)
				{
					SendReply("530 Not logged in.");
					break;
				}
				SendReply("221 goodbye");
				closesocket(m_iCommandSocket);
				closesocket(m_iPassiveSocket);
				m_bActive = false;
				g_application.getApplicationMessenger().Reboot();
				break;
#endif			
			case INVALID_COMMAND: // If GetCommand returns this, the connection is broken, so clean up
				closesocket(m_iCommandSocket);
				closesocket(m_iPassiveSocket);
				m_bActive = false;
				return; // Bail
				break;

			default: // Any command not implemented, return not recognized response
				SendReply("500 command not implemented");
				break;
        } 
	}
}

int CFTPServerConn::CreateTcpipSocket(int * Port)
{
	SOCKET server;

	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons((u_short)*Port);

	server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(server == INVALID_SOCKET)
	{
		CLog::Log(LOGERROR, "FTP Server : Invalid Socket!");
		return 0;
	}

	// After setting these undocumented flags on a socket they should then run unencrypted
	BOOL bBroadcast = TRUE;

	if(setsockopt(server, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL)) != 0)
	{
		CLog::Log(LOGERROR, "FTP Server : Failed to set socket to 5802, error");
		return 0;
	}

	if(setsockopt(server, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL)) != 0)
	{
		CLog::Log(LOGERROR, "FTP Server : Failed to set socket to 5801, error");
		return 0;
	}

	if(bind(server, (const sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();
		CLog::Log(LOGERROR, "FTP Server : bind error %d",Error);
		return 0; 
	}

	// Get the assigned port number
	int size = sizeof(local);
    if(getsockname(server, (struct sockaddr *)&local,&size) < 0)
	{
		CLog::Log(LOGERROR, "FTP Server : getsockname() failed");
        return -1;
    }

    // Convert network byte order to host byte order
    *Port = ntohs(local.sin_port);

    return server;
}

CmdTypes CFTPServerConn::GetCommand(char *CmdArg) 
{
	DWORD dwMaxChars = 500;
	char InputString[500+1];
	char* psz = InputString;
	//int  CmdLen;
	char Command[6];
	int a, b;

	DWORD dw, dwBytes;
	TIMEVAL tv;
	fd_set fds;
	tv.tv_sec=180; // FTP connections only allow 180 seconds of idle
	tv.tv_usec=0;

	for(dwBytes = 0;; dwBytes++)
	{
		FD_ZERO(&fds);
		FD_SET(m_iCommandSocket, &fds);

		dw = select(0, &fds, 0, 0, &tv);
		if(dw == SOCKET_ERROR || dw == 0)
		{
			CLog::Log(LOGERROR, "FTP Server : Socket %d timed out", m_iCommandSocket);
			SendReply("221 goodbye (connection timed out)");
			return INVALID_COMMAND; // Timeout
		}

		dw = recv(m_iCommandSocket, psz, 1, 0);
		if(dw==SOCKET_ERROR || dw == 0)
		{
			CLog::Log(LOGERROR, "FTP Server : Socket %d got a network error, disconnected?", m_iCommandSocket);
			return INVALID_COMMAND; // Network error
		}

		if(*psz=='\r')
			*psz = 0;
		else if(*psz=='\n')
		{
			*psz=0;
			break;
			//return dwBytes;
		}

		if(dwBytes<dwMaxChars)
			psz++;
	}

    memset(Command, 0, sizeof(Command));
    for(a = 0; a < 5; a++)
	{
        if(!isalpha(InputString[a]))
			break;

        Command[a] = (char)toupper(InputString[a]);
    }

    b = 0;

    if(InputString[a++] == ' ')
	{
        for(b = 0; b < 500-1; b++)
		{
            if(InputString[a+b] < 32) 
				break;
            
			CmdArg[b] = InputString[a+b];
        }
    }
    CmdArg[b] = 0;

	//FTPMsg("%s %s\n", Command, CmdArg);

    // Search through the list of known commands
    for(a = 0; a < sizeof(CommandLookup)/sizeof(Lookup_t); a++)
	{
        if(strncmp(Command, CommandLookup[a].command, 4) == 0)
            return CommandLookup[a].CmdNum;
    }
   
    return UNKNOWN_COMMAND;
}

CStdString CFTPServerConn::GetPath(char* requested)
{
	std::vector<CStdString> vecParts;
	CStringUtils::StringSplit(requested, "/", &vecParts);
	
	if(vecParts.size() > 1)
	{
		// Got full path
		CStdString strRetVal = "";
		for(unsigned int x = 0; x < vecParts.size(); x++)
		{
			if(x!=0)
				strRetVal = strRetVal + "\\";
			
			strRetVal = strRetVal + vecParts.at(x);
			
			if(x == 0)
				strRetVal = strRetVal + ":";
		}
		return strRetVal;
	}
	else
	{
		// Only got file name
		return m_pFileBrowser->GetCurrentPath() + "\\" + requested;
	}
}

CStdString CFTPServerConn::GetCurrentDir()
{
	CStdString strRetVal = "\\";
	strRetVal += m_pFileBrowser->GetCurrentPath();
	strRetVal = CStringUtils::ReplaceAllA(strRetVal, "\\", "/");
	strRetVal = CStringUtils::ReplaceAllA(strRetVal, ":", "");

	return strRetVal;
}

int CFTPServerConn::ConnectTcpip(struct sockaddr_in *addr) 
{
	int iAddrlen = sizeof(struct sockaddr_in);
    int iSock;
    iSock = socket(AF_INET, SOCK_STREAM, 0);

    if(iSock < 0)
	{
		CLog::Log(LOGERROR, "FTP Server : socket() failed");
		return -1;
	}

	// After setting these undocumented flags on a socket they should then run unencrypted
	bool bBroadcast = true;

	if(setsockopt(iSock, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL)) != 0)
	{
		CLog::Log(LOGERROR, "FTP Server : Failed to set socket to 5802, error");
		return 0;
	}

	if(setsockopt(iSock, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0)
	{
		CLog::Log(LOGERROR, "FTP Server : Failed to set socket to 5801, error");
		return 0;
	}

	if(connect(iSock, (struct sockaddr *)addr, iAddrlen) < 0)
	{
		int iError = WSAGetLastError();
		CLog::Log(LOGERROR, "FTP Server : connect() failed [%d]", iError);
		return -1;
	}

	return iSock;
}

void CFTPServerConn::Send550Error()
{
	char ErrString[200];
#pragma warning(disable:4996)
	sprintf_s(ErrString, 200, "550 %s", sys_errlist[errno]);
#pragma warning(default:4996)
	SendReply(ErrString);
}

CStdString CFTPServerConn::PathToFileSystemPath(CStdString strPath)
{
	CStdString strResult(strPath);

	// Flip all slashes to backward slashes
	size_t iPos = 0;
	while((iPos = strResult.find("/", iPos)) != strResult.npos)
	{
		strResult.replace(iPos, 1, "\\");
		iPos++;
    }
	
	if(strResult.length() > 1)
	{
		// Make absolute path
		if(strResult.find_first_of("\\", 0) == 0)
			strResult = strResult.substr(1, strResult.npos - 1);
		else
		{
			if(!m_pFileBrowser->IsAtRoot())
				strResult = m_pFileBrowser->GetCurrentPath() + "\\" + strResult;
		}
		
		size_t InsertColonPos;
		if(strResult.find_first_of(":") == strResult.npos)
		{
			InsertColonPos = strResult.find_first_of("\\", 0);
			
			if(InsertColonPos != strResult.npos)
				strResult.insert(InsertColonPos, ":");
			else
				strResult.append(":");
		}
	}
	CLog::Log(LOGNOTICE, "FTP Server : PathToFileSystemPath(string %s): %s", strPath.c_str(), strResult.c_str());

	return strResult;
}

void CFTPServerConn::Cmd_XCRC(const char *filename) 
{
	FILE* pFile = NULL;
	fopen_s(&pFile, filename, "rb");

	if(!pFile)
	{
        Send550Error();
        return;
    }

	CRC_32* crc32 = new CRC_32();
	DWORD dwCRC = crc32->CalcCRC(pFile);
	char crcString[200];
	sprintf_s(crcString, 200, "250 %08X", dwCRC);
	SendReply(crcString);
	fclose(pFile);
}

// Handle the NLST command (directory)
void CFTPServerConn::Cmd_NLST(CStdString strFilename, bool bLong, bool bUseCtrlConn)
{
    int iXfer_sock;

	CLog::Log(LOGERROR, "FTP Server : Init Connection for LIST");
	
	if(bUseCtrlConn)
		iXfer_sock = m_iCommandSocket;
	else
	{
		SendReply("150 Opening connection");
	
		if(m_bPassiveMode)
		{
			CLog::Log(LOGERROR, "FTP Server : Passive Accept");
			iXfer_sock = accept(m_bPassiveMode, NULL, NULL);
			CLog::Log(LOGERROR, "FTP Server : Accepted");
		}
		else
		{
			// Create TCP/IP connection for data link
			CLog::Log(LOGERROR, "FTP Server : Active socket creation");
			iXfer_sock = ConnectTcpip(&m_xfer_addr);
			
			if(iXfer_sock < 0)
			{
				Send550Error();
				return;
			}
		}
	}

	bool bListAll = true;

	bLong = true;
	std::vector<CStdString> vecFolders = m_pFileBrowser->GetFolderList();
	
	for(unsigned int x = 0; x < vecFolders.size(); x++)
	{
		CStdString strOut = "";
		CStdString strFolder = vecFolders.at(x);
		strFolder = CStringUtils::ReplaceAllA(strFolder, ":", "");
		
		if(bLong)
			strOut = m_pFileBrowser->GetFolderFTPLongDescription(strFolder);
		else
			strOut = CStringUtils::sprintfa("%s\r\n", strFolder.c_str());

		send(iXfer_sock, strOut.c_str(), strlen(strOut.c_str()), 0);
		CLog::Log(LOGERROR, "FTP Server : FTPSERVERCONN", "PASSIVE MODE IS BROKEN AT ROOT- THIS DEBUG MESSAGE IS A HACK FIX-  PLEASE FIX ME.  KTHX");
	}
	
	std::vector<CStdString> vecFiles = m_pFileBrowser->GetFileList();
	CStdString strFullPath = m_pFileBrowser->GetCurrentPath();

	for(unsigned int x = 0; x < vecFiles.size(); x++)
	{
		CStdString strOut = "";
		CStdString strFile = vecFiles.at(x);
		
		if(bLong)
			strOut = m_pFileBrowser->GetFileFTPLongDescription(strFile);
		else
			strOut = CStringUtils::sprintfa("%s\r\n", strFile.c_str());

		send(iXfer_sock, strOut.c_str(), strlen(strOut.c_str()), 0);
	}

	if(!bUseCtrlConn)
	{
		closesocket(iXfer_sock);
		SendReply("226 Transfer Complete");
	}
}

void CFTPServerConn::Cmd_STOR(const char *filename) 
{
    int iXfer_sock;
    int iSize;
	CStdString strFullPath = m_pFileBrowser->GetWriteFilePath(filename);

	// Check to see if the file can be opened for writing
	FILE *pFile = NULL;
	fopen_s(&pFile, strFullPath.c_str(), "wb");
	
	if(pFile == NULL)
	{
		Send550Error();
		return;
	}

	fclose(pFile);

	SendReply("150 Opening BINARY mode data connection");

    if(m_bPassiveMode)
        iXfer_sock = accept(m_iPassiveSocket, NULL, NULL);
    else
	{
        // Create TCP/IP connection for data link
        iXfer_sock = ConnectTcpip(&m_xfer_addr);
		if(iXfer_sock < 0)
		{
            Send550Error();
            return;
        }
	}

	long Offset = 0;
	long Read = 0;
	long transferSize = XFERSIZE;
	CStdString strPath(filename);
	
	if(strPath.find("Flash:") == 0)
	{
		transferSize = 0x4000;
		CLog::Log(LOGNOTICE, "FTPServerConn : Writingto flash");
	}

	CFTPFileWriter writer;
	writer.Open(strPath.c_str());

    // Transfer file
    for(iSize = 1; iSize >= 0;)
	{
        // Get from socket.
		while(iSize > 0 &&  Offset < transferSize)
		{
			iSize = recv(iXfer_sock, XferBuffer + Offset, transferSize - Offset, 0);
	//		CFTPServer::getInstance().AddBytes(iSize, true); // TODO
			
			if(iSize < 0)
				CLog::Log(LOGNOTICE, "FTPServerConn : read failed (recv < 0)?");
			
			Read += iSize;
			Offset += iSize;
		}

        if(Read <= 0) break;
	
		writer.SetBytes(((byte*)&XferBuffer), Read);

		Read = 0;
		Offset = 0;
    }

    if(iSize < 0)
	{
		CLog::Log(LOGNOTICE, "FTPServerConn : file save failed %s", filename);
        Send550Error();
    }
	else
		SendReply("226 Transfer Complete");

    closesocket(iXfer_sock);
	writer.CloseWrite();
    // For some reason, the file ends up readonly - should fix that
    //fclose(file);
}

void CFTPServerConn::Cmd_RETR(const char *filename) 
{
    int iXfer_sock;
    int iSize;

    // Check to see if the file can be opened for reading
	FILE* pFile =  m_pFileBrowser->OpenFile(filename);
 	if(!pFile)
	{
		Send550Error();
		return;
    }

    // File opened succesfully, so make the connection
    SendReply("150 Opening BINARY mode data connection");

    if(m_bPassiveMode)
        iXfer_sock = accept(m_iPassiveSocket, NULL, NULL);
    else
	{
        // Create TCP/IP connection for data link
        iXfer_sock = ConnectTcpip(&m_xfer_addr);
        if(iXfer_sock < 0)
		{
            Send550Error();
            return;
        }
    }

    // Transfer file
    for(iSize = 1; iSize > 0;)
	{
		iSize = fread(XferBuffer, 1, XFERSIZE, pFile);
		//FTPMsg("Got size %d", iSize);
        //size = read(pFile, XferBuffer, sizeof(XferBuffer));

		if(iSize < 0)
			break;

		int iToSend = iSize;
		double dSent = 1;
		int iOffset = 0;
		
		while(iToSend > 0)
		{
			// Write buffer to socket.
			dSent = send(iXfer_sock, XferBuffer+iOffset, iToSend, 0);
//			CFTPServer::getInstance().AddBytes(dSent, false); // TODO
			iToSend -= (int)dSent;
			iOffset += (int)dSent;

			if(dSent < 1)
			{
				CLog::Log(LOGERROR, "Send failed");
				SendReply("426 Broken pipe") ;
				iSize = -1;
				iToSend = 0;
			}
		}
    }
    
    if(iSize < 0)
        Send550Error();
    else
        SendReply("226 Transfer Complete");

    closesocket(iXfer_sock);
    fclose(pFile);
}

void CFTPServerConn::Cmd_EXEC(const char *filename) 
{
	//MATTIE: fix automatic path from ftp command
	//i.e. "\hdd1\default.xex" becomes "hdd1:\default.xex"
	CStdString strRealPath(filename);
	size_t iPos = 0;
	size_t iInsertColonPos = 0;
	
	while((iPos = strRealPath.find("/", iPos)) != strRealPath.npos)
	{
		strRealPath.replace(iPos, 1, "\\");
		iPos++;
	}
	
	if(strRealPath.find_first_of("\\", 0) == 0)
	{
		strRealPath = strRealPath.substr(1, strRealPath.npos - 1);
		iInsertColonPos = strRealPath.find_first_of("\\", 0);
		
		if (iInsertColonPos != strRealPath.npos)
			strRealPath.insert(iInsertColonPos, ":");
	}
	
	CLog::Log(LOGNOTICE, "FTPServerConn : FTP EXEC: filename: %s", filename);
	CLog::Log(LOGNOTICE, "FTPServerConn : FTP EXEC: realpath: %s", strRealPath.c_str());

	if (CUtil::FileExists(strRealPath))
	{
		SendReply("221 goodbye");
		closesocket(m_iCommandSocket);
		closesocket(m_iPassiveSocket);

		XLaunchNewImage(strRealPath.c_str(),0);
	}
	else
		SendReply("550 File not found");
}