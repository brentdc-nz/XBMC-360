#ifndef CRC_32_H
#define CRC_32_H

#include "stdio.h"

class CRC_32
{
public:
	CRC_32(void);
	DWORD CalcCRC(FILE* pFile);
	void Calculate (const LPBYTE buffer, UINT size, ULONG &crc);

private:
	ULONG Reflect(ULONG ref, char ch);
	ULONG Table[256];
};

#endif //CRC_32_H