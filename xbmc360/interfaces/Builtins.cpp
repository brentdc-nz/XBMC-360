#include "Application.h"
#include "ApplicationMessenger.h"
#include "Builtins.h"
#include "FileItem.h"
#include "utils\StringUtils.h"
#include "utils\Util.h"
#include "utils\Log.h"
#include "utils\URIUtils.h"
#include "ButtonTranslator.h"
#include "guilib\GUIWindowManager.h"

#include <vector>

using namespace std;

typedef struct
{
	const char *command;
	bool needsParameters;
	const char *description;
} BUILT_IN;

const BUILT_IN commands[] = {
	{ "Help",                       false,  "This help message" },
	{ "Reboot",                     false,  "Reboot the xbox (power cycle)" },
	{ "Restart",                    false,  "Restart the xbox (power cycle)" },
	{ "ShutDown",                   false,  "Shutdown the xbox" },
	{ "Dashboard",                  false,  "Run your dashboard" },
	{ "Powerdown",                  false,  "Powerdown system" },
	{ "Quit",                       false,  "Quit XBMC" },
	{ "Hibernate",                  false,  "Hibernates the system" },
	{ "Suspend",                    false,  "Suspends the system" },
	{ "RestartApp",                 false,  "Restart XBMC" },
	{ "Credits",                    false,  "Run XBMCs Credits" },
	{ "Reset",                      false,  "Reset the xbox (warm reboot)" },
	{ "Mastermode",                 false,  "Control master mode" },
	{ "ActivateWindow",             true,   "Activate the specified window" },
	{ "ReplaceWindow",              true,   "Replaces the current window with the new one" },
	{ "TakeScreenshot",             false,  "Takes a Screenshot" },
	{ "RunScript",                  true,   "Run the specified script" },
	{ "RunXBE",                     true,   "Run the specified executeable" },
	{ "RunPlugin",                  true,   "Run the specified plugin" },
	{ "Extract",                    true,   "Extracts the specified archive" },
	{ "PlayMedia",                  true,   "Play the specified media file (or playlist)" },
	{ "SlideShow",                  true,   "Run a slideshow from the specified directory" },
	{ "RecursiveSlideShow",         true,   "Run a slideshow from the specified directory, including all subdirs" },
	{ "ReloadSkin",                 false,  "Reload XBMC's skin" },
	{ "RefreshRSS",                 false,  "Reload RSS feeds from RSSFeeds.xml"},
	{ "PlayerControl",              true,   "Control the music or video player" },
	{ "Playlist.PlayOffset",        true,   "Start playing from a particular offset in the playlist" },
	{ "Playlist.Clear",             false,  "Clear the current playlist" },
	{ "EjectTray",                  false,  "Close or open the DVD tray" },
	{ "AlarmClock",                 true,   "Prompt for a length of time and start an alarm clock" },
	{ "CancelAlarm",                true,   "Cancels an alarm" },
	{ "Action",                     true,   "Executes an action for the active window (same as in keymap)" },
	{ "Notification",               true,   "Shows a notification on screen, specify header, then message, and optionally time in milliseconds and a icon." },
	{ "PlayDVD",                    false,  "Plays the inserted CD or DVD media from the DVD-ROM Drive!" },
	{ "Skin.ToggleSetting",         true,   "Toggles a skin setting on or off" },
	{ "Skin.SetString",             true,   "Prompts and sets skin string" },
	{ "Skin.SetNumeric",            true,   "Prompts and sets numeric input" },
	{ "Skin.SetPath",               true,   "Prompts and sets a skin path" },
	{ "Skin.Theme",                 true,   "Control skin theme" },
	{ "Skin.SetImage",              true,   "Prompts and sets a skin image" },
	{ "Skin.SetLargeImage",         true,   "Prompts and sets a large skin images" },
	{ "Skin.SetFile",               true,   "Prompts and sets a file" },
	{ "Skin.SetBool",               true,   "Sets a skin setting on" },
	{ "Skin.Reset",                 true,   "Resets a skin setting to default" },
	{ "Skin.ResetSettings",         false,  "Resets all skin settings" },
	{ "Mute",                       false,  "Mute the player" },
	{ "SetVolume",                  true,   "Set the current volume" },
	{ "Dialog.Close",               true,   "Close a dialog" },
	{ "System.LogOff",              false,  "Log off current user" },
	{ "System.PWMControl",          true,   "Control PWM RGB LEDs" },
	{ "Resolution",                 true,   "Change XBMC's Resolution" },
	{ "SetFocus",                   true,   "Change current focus to a different control id" }, 
	{ "BackupSystemInfo",           false,  "Backup System Informations to local hdd" },
	{ "UpdateLibrary",              true,   "Update the selected library (music or video)" },
	{ "CleanLibrary",               true,   "Clean the video library" },
	{ "ExportLibrary",              true,   "Export the video/music library" },
	{ "PageDown",                   true,   "Send a page down event to the pagecontrol with given id" },
	{ "PageUp",                     true,   "Send a page up event to the pagecontrol with given id" },
	{ "LastFM.Love",                false,  "Add the current playing last.fm radio track to the last.fm loved tracks" },
	{ "LastFM.Ban",                 false,  "Ban the current playing last.fm radio track" },
	{ "Container.Refresh",          false,  "Refresh current listing" },
	{ "Container.Update",           false,  "Update current listing. Send Container.Update(path,replace) to reset the path history" },
	{ "Container.NextViewMode",     false,  "Move to the next view type (and refresh the listing)" },
	{ "Container.PreviousViewMode", false,  "Move to the previous view type (and refresh the listing)" },
	{ "Container.SetViewMode",      true,   "Move to the view with the given id" },
	{ "Container.NextSortMethod",   false,  "Change to the next sort method" },
	{ "Container.PreviousSortMethod",false, "Change to the previous sort method" },
	{ "Container.SetSortMethod",    true,   "Change to the specified sort method" },
	{ "Container.SortDirection",    false,  "Toggle the sort direction" },
	{ "Control.Move",               true,   "Tells the specified control to 'move' to another entry specified by offset" },
	{ "Control.SetFocus",           true,   "Change current focus to a different control id" },
	{ "Control.Message",            true,   "Send a given message to a control within a given window" },
	{ "SendClick",                  true,   "Send a click message from the given control to the given window" },
	{ "LoadProfile",                true,   "Load the specified profile (note; if locks are active it won't work)" },
	{ "SetProperty",                true,   "Sets a window property for the current focused window/dialog (key,value)" },
	{ "ClearProperty",              true,   "Clears a window property for the current focused window/dialog (key,value)" },
	{ "PlayWith",                   true,   "Play the selected item with the specified core" },
	{ "WakeOnLan",                  true,   "Sends the wake-up packet to the broadcast address for the specified MAC address" },
	{ "toggledebug",                false,  "Enables/disables debug mode" },
};

bool CBuiltins::HasCommand(const CStdString& execString)
{
	CStdString function;
	vector<CStdString> parameters;

	CUtil::SplitExecFunction(execString, function, parameters);

	for (unsigned int i = 0; i < sizeof(commands)/sizeof(BUILT_IN); i++)
	{
		if (function.CompareNoCase(commands[i].command) == 0 && (!commands[i].needsParameters || parameters.size()))
			return true;
	}

	return false;
}

void CBuiltins::GetHelp(CStdString &help)
{
	help.Empty();
	for (unsigned int i = 0; i < sizeof(commands)/sizeof(BUILT_IN); i++)
	{
		help += commands[i].command;
		help += "\t";
		help += commands[i].description;
		help += "\n";
	}
}

int CBuiltins::Execute(const CStdString& execString)
{
	// Get the text after the "XBMC."
	CStdString execute;
	vector<CStdString> params;
	CUtil::SplitExecFunction(execString, execute, params);
	execute.ToLower();
	CStdString parameter = params.size() ? params[0] : "";
	CStdString strParameterCaseIntact = parameter;

	if (execute.Equals("reboot") || execute.Equals("restart")) // Will reboot the Xbox, aka cold reboot
	{
		g_application.getApplicationMessenger().Reboot();
	}
	else if (execute.Equals("shutdown"))
	{
		g_application.getApplicationMessenger().Shutdown();
	}
	else if (execute.Equals("activatewindow") || execute.Equals("replacewindow"))
	{
		// Get the parameters
		CStdString strWindow;
		CStdString strPath;
		
		if (params.size())
		{
			strWindow = params[0];
			params.erase(params.begin());
		}

		// Confirm the window destination is valid prior to switching
		int iWindow = CButtonTranslator::TranslateWindow(strWindow);
		
		if (iWindow != WINDOW_INVALID)
		{
			// Disable the screensaver
			g_application.ResetScreenSaverWindow();
			g_windowManager.ActivateWindow(iWindow, params, !execute.Equals("activatewindow"));
		}
		else
		{
			CLog::Log(LOGERROR, "Activate/ReplaceWindow called with invalid destination window: %s", strWindow.c_str());
			return false;
		}
	}
	// WIP - More to be re-added!
	else
		return -1;

	return 0;
}