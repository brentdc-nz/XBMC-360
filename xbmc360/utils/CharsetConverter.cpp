#include "CharsetConverter.h"

CCharsetConverter g_charsetConverter; // TODO - Empty class

void CCharsetConverter::wToUTF8(const CStdStringW& strSource, CStdStringA &strDest)
{
	strDest = strSource; // TODO
}

void CCharsetConverter::utf8ToW(const CStdStringA& utf8String, CStdStringW &wString, bool bVisualBiDiFlip/*=true*/, bool forceLTRReadingOrder /*=false*/, bool* bWasFlipped/*=NULL*/)
{
	wString = utf8String; // TODO
}