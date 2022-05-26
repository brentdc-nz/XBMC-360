#include "XEXUtils.h"
#include "Application.h"
#include "Util.h"

bool CXEXUtils::GetXexTitleString(const CStdString& strFilePath, CStdString& strTitle)
{
	// TODO


	// Failed so return file name
	strTitle = CUtil::GetFileName(strFilePath);
	return false;
}

void CXEXUtils::RunXex(CStdString strPath)
{
	g_application.Stop();

	XLaunchNewImage(strPath.c_str(), NULL);
}