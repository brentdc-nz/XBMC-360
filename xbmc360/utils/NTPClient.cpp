#include "NTPClient.h"
#include "log.h"
#include "..\xbox\XBKernalExports.h"

void CNTPClient::Process()
{
//	SetPriority(THREAD_PRIORITY_LOWEST);
	CLog::Log(LOGNOTICE, "NTP: Thread Work Begun");
	
	DoTimeSync();
	
	CLog::Log(LOGNOTICE, "NTP: Thread Work Complete");

	CloseHandle(this->m_ThreadHandle);
	this->m_ThreadHandle = NULL;
}

void CNTPClient::SyncTime()
{
	CLog::Log(LOGNOTICE, "NTP: Starting Thread");

	Create();
	
	if(m_ThreadHandle)
		CLog::Log(LOGNOTICE, "NTP: Thread Created");
	else
		CLog::Log(LOGNOTICE, "NTP: Failed To Create Thread");
}

void CNTPClient::DoTimeSync()
{
	__int64 oldTime = 0;
	__int64 newTime = 0;
	
	UINT32 ret3 = NtSetSystemTime(&newTime, &oldTime);
	ret3 = NtSetSystemTime(&oldTime, &newTime);
	
	// Old time should now contain proper value
	byte NTPBuffer[48] = {0};
	NTPBuffer[0] = 0x1b;
	
	// Set at 40
	SYSTEMTIME LocalSysTime;
    GetSystemTime(&LocalSysTime);

	FILETIME ft;
	SystemTimeToFileTime(&LocalSysTime, &ft);
	
	__int64 ntpformat = 0;
	memcpy(&ntpformat, &ft, 8);

	byte* curTime = new byte[8];
	for(int x = 0; x < 8; x++)
		curTime[x] = 0;

	memcpy(curTime, &ntpformat, 8);
	CLog::Log(LOGNOTICE, "NTP: CurSysTime from 1600");
	CLog::Log(LOGNOTICE, "NTP: %02x %02x %02x %02x %02x %02x %02x %02x", curTime[0], curTime[1], curTime[2], curTime[3], curTime[4], curTime[5], curTime[6], curTime[7]);

	__int64 Diff = 0;
	byte* diffBytes = new byte[8];
	diffBytes[0] = 0x01;
	diffBytes[1] = 0x4f;
	diffBytes[2] = 0x37;
	diffBytes[3] = 0x3b;
	diffBytes[4] = 0xfd;
	diffBytes[5] = 0xe0;
	diffBytes[6] = 0x40;
	diffBytes[7] = 0x00;

	memcpy(&Diff, diffBytes, 8);

	for(int x = 0; x < 8; x++)
		curTime[x] = 0;

	memcpy(curTime, &Diff, 8);
	CLog::Log(LOGNOTICE, "NTP: 300 year is");
	CLog::Log(LOGNOTICE, "NTP: %02x %02x %02x %02x %02x %02x %02x %02x",curTime[0], curTime[1], curTime[2], curTime[3], curTime[4], curTime[5], curTime[6], curTime[7]);
	
	ntpformat-= Diff;

	for(int x = 0; x < 8; x++)
		curTime[x] = 0;

	memcpy(curTime, &ntpformat, 8);
	CLog::Log(LOGNOTICE, "NTP: CurSysTime from 1900");
	CLog::Log(LOGNOTICE, "NTP: %02x %02x %02x %02x %02x %02x %02x %02x",curTime[0], curTime[1], curTime[2], curTime[3], curTime[4], curTime[5], curTime[6], curTime[7]);

	for(int x = 0; x < 8; x++)
		NTPBuffer[40+x] = curTime[x];

	SOCKET debugSendSocket;
	SOCKADDR_IN debugSendSocketAddr;
	debugSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	BOOL bBroadcast = TRUE;

	if(setsockopt(debugSendSocket, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )
		CLog::Log(LOGERROR, "NTP: Failed to set debug send socket to 5801, error");

	debugSendSocketAddr.sin_family = AF_INET;
	debugSendSocketAddr.sin_family = AF_INET;

	XNDNS* pxndns = NULL;
	HANDLE hScktEvent = WSACreateEvent();
    XNetDnsLookup("pool.ntp.org", hScktEvent, &pxndns );
	CLog::Log(LOGNOTICE, "NTP: Waiting for dns...");
    WaitForSingleObject(hScktEvent, INFINITE);
    WSAResetEvent(hScktEvent);
   
	if(pxndns->iStatus != 0)
    {
		CLog::Log(LOGERROR, "NTP: DNS ERROR");
		XNetDnsRelease(pxndns);
        shutdown(debugSendSocket, SD_BOTH);
		closesocket(debugSendSocket);

		return;
    }
	
	CLog::Log(LOGNOTICE, "NTP: DNS OK");
	debugSendSocketAddr.sin_addr.s_addr = inet_addr("pool.ntp.org"); // TODO: Make GUI config string
    debugSendSocketAddr.sin_addr = pxndns->aina[0];
	debugSendSocketAddr.sin_port = 123;
	
	if(connect(debugSendSocket, (struct sockaddr* )&debugSendSocketAddr, sizeof(debugSendSocketAddr)) == 0)
	{
		CLog::Log(LOGNOTICE, "NTP: Connected");
		send(debugSendSocket, (char*)NTPBuffer, 48, 0);

		recv(debugSendSocket,(char*) NTPBuffer, 48, 0);
		byte* seconds = new byte[8];
		seconds[0] =0;
		seconds[1] = 0;
		seconds[2] = 0;
		seconds[3] = 0;
		seconds[4] = NTPBuffer[40];
		seconds[5] = NTPBuffer[41];
		seconds[6] = NTPBuffer[42];
		seconds[7] = NTPBuffer[43];
	
		__int64 secondsValue = 0;
		__int64 old = 0;
		memcpy(&secondsValue, seconds, 8);
		secondsValue = 10000000 * secondsValue;
			
		memcpy(curTime, &secondsValue, 8);
		CLog::Log(LOGNOTICE, "NTP: Final Set from 1900");
		CLog::Log(LOGNOTICE, "NTP: %02x %02x %02x %02x %02x %02x %02x %02x", curTime[0], curTime[1], curTime[2], curTime[3], curTime[4], curTime[5], curTime[6], curTime[7]);
		secondsValue = Diff +(secondsValue);
		curTime = new byte[8];

		for(int x=0; x<8; x++)
			curTime[x] =0;

		memcpy(curTime, &secondsValue, 8);
		CLog::Log(LOGNOTICE, "NTP: Final Set from 1600");
		CLog::Log(LOGNOTICE, "NTP: %02x %02x %02x %02x %02x %02x %02x %02x", curTime[0], curTime[1], curTime[2], curTime[3], curTime[4], curTime[5], curTime[6], curTime[7]);
		UINT32 ret2 = NtSetSystemTime(&secondsValue, &old);
		CLog::Log(LOGNOTICE, "NTP: SET RESULT : %08x", ret2);
	}
	else
		CLog::Log(LOGNOTICE, "NTP: Cant Connect");

	// Clean up pointer
	XNetDnsRelease(pxndns);
	shutdown(debugSendSocket, SD_BOTH);
	closesocket(debugSendSocket);
}