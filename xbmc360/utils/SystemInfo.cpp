#include "SystemInfo.h"

CSysInfo g_sysinfo;

CSysInfo::CSysInfo(void) : CInfoLoader(5 * 60 * 1000)
{
}

CSysInfo::~CSysInfo()
{
}

CStdString CSysInfo::GetUserAgent()
{
	CStdString result;
	result = "XBMC/360 (Xbox 360; http://www.xbmc.org)";
	return result;
}

bool CSysInfo::DoWork()
{
	return true;
}

CStdString CSysInfo::TranslateInfo(int info) const
{
	return "";
}
