#ifndef CFTPFILEWRITER_H
#define CFTPFILEWRITER_H
#include <cstdio> //Both Stdio´s??
#include <stdio.h>
#include "..\..\utils\Thread.h"

//Foward Class To CThread
class CFTPFile : public CThread
{
public:
	CFTPFile() {}
	~CFTPFile() {}

};

class CFTPFileReader : public CFTPFile
{
public:	
	CFTPFileReader();
	~CFTPFileReader();
	void OpenFile(const char *path);
	virtual void Process();
	void SetBytes(byte* buffer, double dength);
	void CloseRead();
private:
	FILE *m_pFile;
	bool m_bDone;
	bool m_bLoopRunning;
	bool m_bWaitingForBytes;
	byte* m_CurrentBuffer;
	double m_CurrentBufferSize; 
};

class CFTPFileWriter : public CFTPFile
{
public:
	CFTPFileWriter();
	~CFTPFileWriter(void);
	void Open(const char *path);
	virtual void Process();
	void SetBytes(byte* buffer, double length);
	void CloseWrite();

private:
	bool m_bDone;
	FILE* m_pFile;
	bool m_bLoopRunning;
	bool m_bWaitingForBytes;
	byte* m_CurrentBuffer;
	double m_CurrentBufferSize;
};

#endif //CFTPFILEWRITER_H
