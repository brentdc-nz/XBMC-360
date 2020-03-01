#include "stdafx.h"
#include "CRC_32.h"

#define BUFFERSIZE 102400

CRC_32::CRC_32(void)
{
	ULONG ulPolynomial = 0x04C11DB7;
	for(int i = 0; i <= 0xFF; i++)
	{
		Table[i] = Reflect(i, 8) << 24;

		for(int j = 0; j < 8; j++)
			Table[i] = (Table[i] << 1) ^ (Table[i] & (1 << 31) ? ulPolynomial : 0);

		Table[i] = Reflect(Table[i], 32);
	}
}

ULONG CRC_32::Reflect(DWORD ref, char ch)
{
	ULONG value = 0;
	for(int i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);

		ref >>= 1;
	}
	return value;
}

void CRC_32::Calculate(const LPBYTE buffer, UINT size, ULONG &CRC)
{
	LPBYTE pbyte = buffer;

	while(size--)
		CRC = (CRC >> 8) ^ Table[(CRC & 0xFF) ^ *pbyte++];
}

DWORD CRC_32::CalcCRC(FILE* pFile)
{   
	DWORD CRC = 0xFFFFFFFF;
	BYTE buffer[BUFFERSIZE];

	for(UINT size=1;size > 0;)
	{
		size = fread(buffer, 1, BUFFERSIZE, pFile);
		
		if(size < 0)
			break;
	
		Calculate(buffer, size, CRC);
	}

	return CRC ^ 0xFFFFFFFF;
}


    
