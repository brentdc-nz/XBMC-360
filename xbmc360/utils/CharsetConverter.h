#ifndef CCHARSET_CONVERTER
#define CCHARSET_CONVERTER

#include "utils/StdString.h"

class CCharsetConverter // TODO - Empty class
{
public:
	void wToUTF8(const CStdStringW& strSource, CStdStringA &strDest);
	void utf8ToW(const CStdStringA& utf8String, CStdStringW &wString, bool bVisualBiDiFlip = true, bool forceLTRReadingOrder = false, bool* bWasFlipped = NULL);
};

extern CCharsetConverter g_charsetConverter;

#endif //CCHARSET_CONVERTER
