#include "IFile.h"
#include <errno.h>


using namespace XFILE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IFile::IFile()
{
}

IFile::~IFile()
{
}

int IFile::Stat(struct __stat64* buffer)
{
	memset(buffer, 0, sizeof(struct __stat64));
	errno = ENOENT;
	return -1;
}

bool IFile::ReadString(char *szLine, int iLineLength)
{
	if(Seek(0, SEEK_CUR) < 0) return false;

	int64_t iFilePos = GetPosition();
	int iBytesRead = Read( (unsigned char*)szLine, iLineLength - 1);
	
	if (iBytesRead <= 0)
		return false;

	szLine[iBytesRead] = 0;

	for (int i = 0; i < iBytesRead; i++)
	{
		if ('\n' == szLine[i])
		{
			if ('\r' == szLine[i + 1])
			{
				szLine[i + 1] = 0;
				Seek(iFilePos + i + 2, SEEK_SET);
			}
			else
			{
				// end of line
				szLine[i + 1] = 0;
				Seek(iFilePos + i + 1, SEEK_SET);
			}
			break;
		}
		else if ('\r' == szLine[i])
		{
			if ('\n' == szLine[i + 1])
			{
				szLine[i + 1] = 0;
				Seek(iFilePos + i + 2, SEEK_SET);
			}
			else
			{
				// end of line
				szLine[i + 1] = 0;
				Seek(iFilePos + i + 1, SEEK_SET);
			}
			break;
		}
	}
	return true;
}