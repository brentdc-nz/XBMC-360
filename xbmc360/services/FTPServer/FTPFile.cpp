#include "FTPFile.h"
#include "utils\log.h"

#define WRITINGPACKETMAXSIZE 1024*1024

CFTPFileReader::CFTPFileReader()
{
   m_pFile = NULL;
   m_bDone = false;
   m_bLoopRunning = false;
   m_bWaitingForBytes = NULL;
   m_CurrentBuffer = NULL;
   m_CurrentBufferSize = NULL;
}

CFTPFileReader::~CFTPFileReader()
{

}

void CFTPFileReader::OpenFile(const char *path)
{
	int ret;
	ret = fopen_s(&m_pFile, path, "wb");

	if(!ret)
	{
		CLog::Log(LOGERROR, "CFTPFileReader::OpenFile() -> Error Opening the file");
	}
	Create();
}

void CFTPFileReader::Process()
{
/*
	if(SETTINGS::getInstance().getFtpServerOn() == false)
	{
		Sleep(500);
		return 0;
	}
*/
	SetName("FtpFileReader");
	m_bLoopRunning = true;
	m_bWaitingForBytes = true;

	while(!m_bDone || !m_bWaitingForBytes)
	{
		if(!m_bWaitingForBytes)
		{
			fread(&m_CurrentBuffer, sizeof(m_CurrentBufferSize), 1, m_pFile);
			delete m_CurrentBuffer;
			m_bWaitingForBytes = true;
		}
		else
			Sleep(0);
	}
	m_bLoopRunning = false;
}

void CFTPFileReader::SetBytes(byte* buffer, double length)
{
	if(length <10 || length > WRITINGPACKETMAXSIZE)
		CLog::Log(LOGERROR, "FtpFileWriter : Trouble....");

	while(!m_bWaitingForBytes)
		Sleep(0);

	m_CurrentBuffer = (byte*)malloc((size_t)length);
	memcpy(m_CurrentBuffer, buffer, (size_t)length);
	m_CurrentBufferSize = length;
	m_bWaitingForBytes = false;		
	sizeof(m_CurrentBufferSize);
}

void CFTPFileReader::CloseRead()
{
	m_bDone = true;
	
	while(m_bLoopRunning == true)
		Sleep(10);
		
	fclose(m_pFile);
	StopThread();
	CLog::Log(LOGNOTICE, "Closing read");
}

CFTPFileWriter::CFTPFileWriter()
{
	m_bDone = false;
	m_bLoopRunning = false;
	m_bWaitingForBytes = false;
	m_CurrentBuffer = NULL;
	m_CurrentBufferSize = NULL;

	/*Thread(CPU3_THREAD_1);*/
}

CFTPFileWriter::~CFTPFileWriter()
{

}

void CFTPFileWriter::Open(const char *path)
{
	int ret;
	ret = fopen_s(&m_pFile, path, "wb");

	if(!ret)
	{
		CLog::Log(LOGERROR, "CFTPFileWritter::Open() -> Error Opening the file");
	}
	Create();
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
	sizeof(m_CurrentBufferSize);
}

void CFTPFileWriter::CloseWrite()
{	
	m_bDone = true;
	
	while(m_bLoopRunning == true)
		Sleep(10);
		
	fclose(m_pFile);
	StopThread();
	CLog::Log(LOGNOTICE, "Closing Write...");
}

