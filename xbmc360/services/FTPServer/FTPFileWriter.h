#ifndef CFTPFILEWRITER_H
#define CFTPFILEWRITER_H

#include <cstdio>
#include "..\..\utils\Thread.h"

class CFTPFileWriter : public CThread
{
public:
	CFTPFileWriter(const char * path);
	~CFTPFileWriter(void);

	virtual void Process();
	void SetBytes(byte* buffer, double dength);
	void Close();

private:
	bool m_bDone;
	FILE* m_pFile;
	bool m_bLoopRunning;
	bool m_bWaitingForBytes;
	byte* m_CurrentBuffer;
	double m_CurrentBufferSize;
};

#endif //CFTPFILEWRITER_H
