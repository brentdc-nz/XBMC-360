#ifndef H_CSTRINGUTILS
#define H_CSTRINGUTILS

#include "StdString.h"
#include "Stdafx.h"
#include "StdString.h"
#include "TimeUtils.h"

class CStringUtils
{
public:
	static void StringtoWString(CStdString strText, std::wstring &strResult);
	static int SplitString(const CStdString& input, const CStdString& delimiter, CStdStringArray &results, unsigned int iMaxStrings = 0);
	static void StringSplit(CStdString str, CStdString delim, std::vector<CStdString>* results, bool bShowEmptyEntries = false);
	static int FindEndBracket(const CStdString &str, char opener, char closer, int startPos = 0);
	static bool IsNaturalNumber(const CStdString& str);
	static CStdString SecondsToTimeString(long seconds, TIME_FORMAT format = TIME_FORMAT_GUESS);
	static CStdString ReplaceAllA(CStdString s, CStdString sub, CStdString other);
	static CStdString sprintfa(const char *format, ...);
	static CStdString MakeLowercase(CStdString strTmp);
};

#endif //H_CSTRINGUTIL