#ifndef CUTIL_H
#define CUTIL_H

#include "StdString.h"
#include "stdafx.h"

class CUtil
{
public:
	static bool IsBuiltIn(const CStdString& execString);
	static void SplitExecFunction(const CStdString &execString, CStdString &strFunction, CStdString &strParam);
	static int ExecBuiltIn(const CStdString& execString);
	static bool FileExists(CStdString strFullPath) { return (GetFileAttributes(strFullPath.c_str()) != 0xFFFFFFFF); } // TODO : Remove once filesystem is more sorted out
};

#endif //CUTIL_H