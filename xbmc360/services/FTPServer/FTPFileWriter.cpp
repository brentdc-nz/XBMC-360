#include "FTPFileWriter.h"
#include "utils\Log.h"

#include "stdio.h"

#define WRITINGPACKETMAXSIZE 1024*1024

CFTPFileWriter::CFTPFileWriter(const char * path)
{
	m_bDone = false;
	m_bLoopRunning = false;
	m_bWaitingForBytes = false;
	fopen_s(&m_pFile, path, "wb");
	Create();/*Thread(CPU3_THREAD_1);*/
}

CFTPFileWriter::~CFTPFileWriter()
{
}

void CFTPFileWriter::Process()
{
/*
	if(SETTINGS::getInstance().getFtpServerOn() == false)
	{
		Sleep(500);
		return 0;
	}
*/
	SetName("FtpFileWriter");
	m_bLoopRunning = true;
	m_bWaitingForBytes = true;

	while(!m_bDone || !m_bWaitingForBytes)
	{
		if(!m_bWaitingForBytes)
		{
			fwrite(m_CurrentBuffer, (size_t)m_CurrentBufferSize, 1, m_pFile);
		
			delete m_CurrentBuffer;
			m_bWaitingForBytes = true;
		}
		else
			Sleep(0);
	}
	m_bLoopRunning = false;
}

void CFTPFileWriter::SetBytes(byte* buffer, double length)
{
	if(length <10 || length > WRITINGPACKETMAXSIZE)
		CLog::Log(LOGERROR, "FtpFileWriter : Trouble....");

	while(!m_bWaitingForBytes)
		Sleep(0);

	m_CurrentBuffer = (byte*)malloc((size_t)length);
	memcpy(m_CurrentBuffer, buffer, (size_t)length);
	m_CurrentBufferSize = length;
	m_bWaitingForBytes = false;		
}

void CFTPFileWriter::Close()
{	
	m_bDone = true;
	
	while(m_bLoopRunning == true)
		Sleep(10);
		
	fclose(m_pFile);
	StopThread();
}