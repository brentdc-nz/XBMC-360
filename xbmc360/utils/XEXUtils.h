#ifndef H_CXEXUTILS
#define H_CXEXUTILS

#include "StdString.h"

class CXEXUtils
{
public:
	static bool GetXexTitleString(const CStdString& strFilePath, CStdString& strTitle);
	static void RunXex(CStdString strPath);
};

#endif //H_CXEXUTILS