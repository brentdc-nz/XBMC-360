#include "ButtonTranslator.h"
#include "utils\log.h"
#include "guilib\Key.h"
#include "utils\StringUtils.h"

CButtonTranslator g_buttonTranslator;

CButtonTranslator::CButtonTranslator()
{}

CButtonTranslator::~CButtonTranslator()
{}

WORD CButtonTranslator::TranslateWindowString(const char *szWindow)
{
	WORD wWindowID = WINDOW_INVALID;
	CStdString strWindow = szWindow;
	if (strWindow.IsEmpty()) return wWindowID;
	strWindow.ToLower();

	// window12345, for custom window to be keymapped
	if (strWindow.length() > 6 && strWindow.Left(6).Equals("window"))
		strWindow = strWindow.Mid(6);
	if (CStringUtils::IsNaturalNumber(strWindow))
	{
		// allow a full window id or a delta id
		int iWindow = atoi(strWindow.c_str());
		if (iWindow > WINDOW_INVALID) 
			wWindowID = iWindow;
		else 
			wWindowID = WINDOW_HOME + iWindow;
	}
	else if (strWindow.Equals("home")) wWindowID = WINDOW_HOME;
	else if (strWindow.Equals("myprograms")) wWindowID = WINDOW_PROGRAMS;
	else if (strWindow.Equals("myvideos")) wWindowID = WINDOW_VIDEOS;
	else if (strWindow.Equals("settings")) wWindowID = WINDOW_SETTINGS;
	else if (strWindow.Equals("appearancesettings")) wWindowID = WINDOW_SETTINGS_APPEARANCE;
	else
		CLog::Log(LOGERROR, "Window Translator: Can't find window %s", strWindow.c_str());

	//CLog::Log(LOGDEBUG, "CButtonTranslator::TranslateWindowString(%s) returned Window ID (%i)", szWindow, wWindowID);
	return wWindowID;
}