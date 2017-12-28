#ifndef H_CSTRINGUTILS
#define H_CSTRINGUTILS

#include "StdString.h"
#include "stdafx.h"
#include "StdString.h"

class CStringUtils
{
public:
	static LPCWSTR String2WString(CStdString strText);

	static int SplitString(const CStdString& input, const CStdString& delimiter, CStdStringArray &results, unsigned int iMaxStrings = 0);

	static int FindEndBracket(const CStdString &str, char opener, char closer, int startPos = 0);

	static bool IsNaturalNumber(const CStdString& str);
};

#endif //H_CSTRINGUTIL