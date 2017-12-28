#include "Util.h"
#include "log.h"
#include "..\ButtonTranslator.h"
#include "..\guilib\GUIWindowManager.h"

#include "..\Application.h"
#include "..\xbox\XBKernalExports.h"

typedef struct
{
	char command[20];
	char description[128];
} BUILT_IN;

const BUILT_IN commands[] = 
{
	"Help", "This help message",
	"Reboot", "Reboot the xbox (power cycle)",
	"Restart", "Restart the xbox (power cycle)",
	"ShutDown", "Shutdown the xbox",
	"Dashboard", "Run your dashboard",
	"RestartApp", "Restart XBMC",
	"Credits", "Run XBMCs Credits",
	"Reset", "Reset the xbox (warm reboot)",
	"ActivateWindow", "Activate the specified window",
	"ReplaceWindow", "Replaces the current window with the new one",
	"TakeScreenshot", "Takes a Screenshot",
	"RunScript", "Run the specified script",
	"RunXBE", "Run the specified executeable",
	"Extract", "Extracts the specified archive",
	"PlayMedia", "Play the specified media file (or playlist)",
	"SlideShow", "Run a slideshow from the specified directory",
	"RecursiveSlideShow", "Run a slideshow from the specified directory, including all subdirs",
	"ReloadSkin", "Reload XBMC's skin",
	"PlayerControl", "Control the music or video player",
	"EjectTray", "Close or open the DVD tray",
	"AlarmClock", "Prompt for a length of time and start an alarm clock",
	"CancelAlarm","Cancels an alarm",
	"Action", "Executes an action for the active window (same as in keymap)",
	"Notification", "Shows a notification on screen, specify header, then message.",
	"PlayDVD"," Plays the inserted CD or DVD media from the DVD-ROM Drive!",
	"Skin.ToggleSetting"," Toggles a skin setting on or off",
	"Skin.SetString"," Prompts and sets skin string",
	"Skin.SetPath"," Prompts and sets a skin path",
	"Skin.SetImage"," Prompts and sets a skin image",
	"Skin.SetBool"," Sets a skin setting on",
	"Skin.Reset"," Resets a skin setting to default",
	"Skin.ResetSettings"," Resets all skin settings",
	"Mute","Mute the player",
	"SetVolume","Set the current volume",
	"Dialog.Close","Close a dialog"
};

bool CUtil::IsBuiltIn(const CStdString& execString)
{
	CStdString function, param;
	SplitExecFunction(execString, function, param);
	for (int i = 0; i < sizeof(commands)/sizeof(BUILT_IN); i++)
	{
		if (function.CompareNoCase(commands[i].command) == 0)
			return true;
	}
	return false;
}

void CUtil::SplitExecFunction(const CStdString &execString, CStdString &strFunction, CStdString &strParam)
{
	strParam = "";

	int iPos = execString.Find("(");
	int iPos2 = execString.ReverseFind(")");
	if (iPos > 0 && iPos2 > 0)
	{
		strParam = execString.Mid(iPos + 1, iPos2 - iPos - 1);
		strFunction = execString.Left(iPos);
	}
	else
		strFunction = execString;

	//xbmc is the standard prefix.. so allways remove this
	//all other commands with go through in full
	if( strFunction.Left(5).Equals("xbmc.", false) )
		strFunction.Delete(0, 5);
}

int CUtil::ExecBuiltIn(const CStdString& execString)
{
	// Get the text after the "XBMC."
	CStdString execute, parameter;
	SplitExecFunction(execString, execute, parameter);
	CStdString strParameterCaseIntact = parameter;
	parameter.ToLower();
	execute.ToLower();
  
	if (execute.Equals("reboot") || execute.Equals("restart"))  //Will reboot the xbox, aka cold reboot
	{
//		g_applicationMessenger.Restart();
	}
	else if (execute.Equals("shutdown"))
	{
//		g_applicationMessenger.Shutdown();
	}
	else if (execute.Equals("activatewindow") || execute.Equals("replacewindow"))
	{
		// get the parameters
		CStdString strWindow;
		CStdString strPath;

		// split the parameter on first comma
		int iPos = parameter.Find(",");
		if (iPos == 0)
		{
			// error condition missing path
			// XMBC.ActivateWindow(1,)
			CLog::Log(LOGERROR, "Activate/ReplaceWindow called with invalid parameter: %s", parameter.c_str());
			return -7;
		}
		else if (iPos < 0)
		{
			// no path parameter
			// XBMC.ActivateWindow(5001)
			strWindow = parameter;
		}
		else
		{
			// path parameter included
			// XBMC.ActivateWindow(5001,F:\Music\)
			strWindow = parameter.Left(iPos);
			strPath = parameter.Mid(iPos + 1);
		}

		// confirm the window destination is actually a number
		// before switching
		int iWindow = g_buttonTranslator.TranslateWindowString(strWindow.c_str());
		if (iWindow != WINDOW_INVALID)
		{
			// disable the screensaver
//			g_application.ResetScreenSaverWindow();
			if (execute.Equals("activatewindow"))
				g_windowManager.ActivateWindow(iWindow/*, strPath*/);
			else  
				// ReplaceWindow
				g_windowManager.ChangeActiveWindow(iWindow/*, strPath*/);
		}
		else
		{
			CLog::Log(LOGERROR, "Activate/ReplaceWindow called with invalid destination window: %s", strWindow.c_str());
			return false;
		}
	}
	return 0;
}
