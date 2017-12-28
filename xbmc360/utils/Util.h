#ifndef CUTIL_H
#define CUTIL_H

#include "StdString.h"

class CUtil
{
public:
	static bool IsBuiltIn(const CStdString& execString);
	static void SplitExecFunction(const CStdString &execString, CStdString &strFunction, CStdString &strParam);
	static int ExecBuiltIn(const CStdString& execString);
};

#endif //CUTIL_H