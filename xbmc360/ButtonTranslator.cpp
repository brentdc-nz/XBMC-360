#include "ButtonTranslator.h"
#include "utils\Log.h"
#include "utils\StringUtils.h"
#include "utils\Util.h"
#include "interfaces\Builtins.h"

using namespace std;

typedef struct
{
	const char* name;
	int action;
} ActionMapping;

// Remember the fixed length names (hence max 31 char)!
static const ActionMapping actions[] = 
		{{"left"             , ACTION_MOVE_LEFT},
		{"right"             , ACTION_MOVE_RIGHT},
		{"up"                , ACTION_MOVE_UP},
        {"down"              , ACTION_MOVE_DOWN},
        {"select"            , ACTION_SELECT_ITEM},
        {"parentdir"         , ACTION_NAV_BACK}, // Backward compatibility
        {"back"              , ACTION_NAV_BACK},
        {"previousmenu"      , ACTION_PREVIOUS_MENU},
        {"pause"             , ACTION_PAUSE},
        {"stop"              , ACTION_STOP},
        {"fullscreen"        , ACTION_SHOW_GUI},
        {"stepforward"       , ACTION_STEP_FORWARD},
        {"stepback"          , ACTION_STEP_BACK},
        {"bigstepforward"    , ACTION_BIG_STEP_FORWARD},
        {"bigstepback"       , ACTION_BIG_STEP_BACK},
        {"codecinfo"         , ACTION_SHOW_CODEC},
        {"close"             , ACTION_NAV_BACK}, // Backwards compatibility
        {"contextmenu"       , ACTION_CONTEXT_MENU},
        {"volumeup"          , ACTION_VOLUME_UP},
        {"volumedown"        , ACTION_VOLUME_DOWN},
        {"mute"              , ACTION_MUTE},
        {"scrollup"          , ACTION_SCROLL_UP},
        {"scrolldown"        , ACTION_SCROLL_DOWN},
        {"analogseekforward" , ACTION_ANALOG_SEEK_FORWARD},
        {"analogseekback"    , ACTION_ANALOG_SEEK_BACK},
};

static const ActionMapping windows[] =
       {{"home"                     , WINDOW_HOME},
        {"programs"                 , WINDOW_PROGRAMS},
        {"pictures"                 , WINDOW_PICTURES},
        {"music"                    , WINDOW_MUSIC},
        {"video"                    , WINDOW_VIDEOS},
        {"videos"                   , WINDOW_VIDEOS},
        {"systeminfo"               , WINDOW_SYSTEM_INFORMATION},
        {"picturessettings"         , WINDOW_SETTINGS_MYPICTURES},
        {"programssettings"         , WINDOW_SETTINGS_MYPROGRAMS},
        {"weathersettings"          , WINDOW_SETTINGS_MYWEATHER},
        {"musicsettings"            , WINDOW_SETTINGS_MYMUSIC},
        {"systemsettings"           , WINDOW_SETTINGS_SYSTEM},
        {"videossettings"           , WINDOW_SETTINGS_MYVIDEOS},
        {"networksettings"          , WINDOW_SETTINGS_NETWORK},
        {"appearancesettings"       , WINDOW_SETTINGS_APPEARANCE},
        {"yesnodialog"              , WINDOW_DIALOG_YES_NO},
        {"progressdialog"           , WINDOW_DIALOG_PROGRESS},
        {"virtualkeyboard"          , WINDOW_DIALOG_KEYBOARD},
        {"contextmenu"              , WINDOW_DIALOG_CONTEXT_MENU},
        {"shutdownmenu"             , WINDOW_DIALOG_BUTTON_MENU},
        {"seekbar"                  , WINDOW_DIALOG_SEEK_BAR},
        {"filebrowser"              , WINDOW_DIALOG_FILE_BROWSER},
        {"networksetup"             , WINDOW_DIALOG_NETWORK_SETUP},
        {"mediasource"              , WINDOW_DIALOG_MEDIA_SOURCE},
        {"fullscreenvideo"          , WINDOW_FULLSCREEN_VIDEO},
        {"screensaver"              , WINDOW_SCREENSAVER},
};

CButtonTranslator g_buttonTranslator;
extern CStdString g_LoadErrorStr;

CButtonTranslator::CButtonTranslator()
{
}

CButtonTranslator::~CButtonTranslator()
{
}

bool CButtonTranslator::Load()
{
	// load our xml file, and fill up our mapping tables
	TiXmlDocument xmlDoc;

	CStdString strPath = "D:\\system\\keymap.xml";

	CLog::Log(LOGINFO, "Loading %s", strPath);

	// Load the config file
	if (!xmlDoc.LoadFile(strPath))
	{
		g_LoadErrorStr.Format("D:\\system\\keymap.xml, Line %d\n%s", xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
		return false;
	}

	TiXmlElement* pRoot = xmlDoc.RootElement();
	CStdString strValue = pRoot->Value();
	
	if (strValue != "keymap")
	{
		CLog::Log(LOGERROR, "%s Doesn't contain <keymap>", strPath);
		return false;
	}
	
	// Run through our window groups
	TiXmlNode* pWindow = pRoot->FirstChild();
	
	while (pWindow)
	{
		if (pWindow->Type() == TiXmlNode::ELEMENT)
		{
			int windowID = WINDOW_INVALID;
			const char *szWindow = pWindow->Value();
			
			if (szWindow)
			{
				if (strcmpi(szWindow, "global") == 0)
					windowID = -1;
				else
					windowID = TranslateWindow(szWindow);
			}
			MapWindowActions(pWindow, windowID);
		}
		pWindow = pWindow->NextSibling();
	}
	return true;
}

void CButtonTranslator::MapWindowActions(TiXmlNode *pWindow, WORD wWindowID)
{
	if (!pWindow || wWindowID == WINDOW_INVALID)
		return;
	
	buttonMap map;
	
	TiXmlNode* pDevice;
	if ((pDevice = pWindow->FirstChild("gamepad")) != NULL)
	{
		// Map gamepad actions
		TiXmlElement *pButton = pDevice->FirstChildElement();
		while (pButton)
		{
			WORD wButtonCode = TranslateGamepadString(pButton->Value());
			
			if (pButton->FirstChild()) 
				MapAction(wButtonCode, pButton->FirstChild()->Value(), map);

			pButton = pButton->NextSiblingElement();
		}
	}
	
	// Add our map to our table
	if (map.size() > 0)
		translatorMap.insert(pair<WORD, buttonMap>( wWindowID, map));
}

WORD CButtonTranslator::TranslateGamepadString(const char *szButton)
{
	if (!szButton) return 0;
	WORD wButtonCode = 0;
	CStdString strButton = szButton;
	strButton.ToLower();

	if (strButton.Equals("a")) wButtonCode = KEY_BUTTON_A;
	else if (strButton.Equals("b")) wButtonCode = KEY_BUTTON_B;
	else if (strButton.Equals("x")) wButtonCode = KEY_BUTTON_X;
	else if (strButton.Equals("y")) wButtonCode = KEY_BUTTON_Y;
	else if (strButton.Equals("rightshoulder")) wButtonCode = KEY_BUTTON_RIGHT_SHOULDER;//KEY_BUTTON_WHITE;
	else if (strButton.Equals("leftshoulder")) wButtonCode = KEY_BUTTON_LEFT_SHOULDER;//KEY_BUTTON_BLACK;
	else if (strButton.Equals("start")) wButtonCode = KEY_BUTTON_START;
	else if (strButton.Equals("back")) wButtonCode = KEY_BUTTON_BACK;
	else if (strButton.Equals("leftthumbbutton")) wButtonCode = KEY_BUTTON_LEFT_THUMB_BUTTON;
	else if (strButton.Equals("rightthumbbutton")) wButtonCode = KEY_BUTTON_RIGHT_THUMB_BUTTON;
	else if (strButton.Equals("leftthumbstick")) wButtonCode = KEY_BUTTON_LEFT_THUMB_STICK;
	else if (strButton.Equals("leftthumbstickup")) wButtonCode = KEY_BUTTON_LEFT_THUMB_STICK_UP;
	else if (strButton.Equals("leftthumbstickdown")) wButtonCode = KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
	else if (strButton.Equals("leftthumbstickleft")) wButtonCode = KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
	else if (strButton.Equals("leftthumbstickright")) wButtonCode = KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
	else if (strButton.Equals("rightthumbstick")) wButtonCode = KEY_BUTTON_RIGHT_THUMB_STICK;
	else if (strButton.Equals("rightthumbstickup")) wButtonCode = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
	else if (strButton.Equals("rightthumbstickdown")) wButtonCode = KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
	else if (strButton.Equals("rightthumbstickleft")) wButtonCode = KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
	else if (strButton.Equals("rightthumbstickright")) wButtonCode = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
	else if (strButton.Equals("righttrigger")) wButtonCode = KEY_BUTTON_RIGHT_TRIGGER;
	else if (strButton.Equals("leftanalogtrigger")) wButtonCode = KEY_BUTTON_LEFT_ANALOG_TRIGGER;
	else if (strButton.Equals("rightanalogtrigger")) wButtonCode = KEY_BUTTON_RIGHT_ANALOG_TRIGGER;
	else if (strButton.Equals("dpadleft")) wButtonCode = KEY_BUTTON_DPAD_LEFT;
	else if (strButton.Equals("dpadright")) wButtonCode = KEY_BUTTON_DPAD_RIGHT;
	else if (strButton.Equals("dpadup")) wButtonCode = KEY_BUTTON_DPAD_UP;
	else if (strButton.Equals("dpaddown")) wButtonCode = KEY_BUTTON_DPAD_DOWN;
	else CLog::Log(LOGERROR, "Gamepad Translator: Can't find button %s", strButton.c_str());
	
	return wButtonCode;
}

bool CButtonTranslator::TranslateActionString(const char *szAction, int &action)
{
	action = ACTION_NONE;
	CStdString strAction = szAction;
	strAction.ToLower();
	
	if (CBuiltins::HasCommand(strAction)) action = ACTION_BUILT_IN_FUNCTION;

	if (strAction.Equals("noop"))
		return true;
  
	for (unsigned int index=0;index < sizeof(actions)/sizeof(actions[0]);++index)
	{
		if (strAction.Equals(actions[index].name))
		{
			action = actions[index].action;
			break;
		}
	}

	if (action == ACTION_NONE)
	{
		CLog::Log(LOGERROR, "Keymapping error: no such action '%s' defined", strAction.c_str());
		return false;
	}

	return true;
}

void CButtonTranslator::MapAction(WORD wButtonCode, const char *szAction, buttonMap &map)
{
	int iAction = ACTION_NONE;
	if (!TranslateActionString(szAction, iAction) || !wButtonCode)
		return; // No valid action, or an invalid buttoncode

	// Have a valid action, and a valid button - map it.
	// check to see if we've already got this (button,action) pair defined

	buttonMap::iterator it = map.find(wButtonCode);
	if (it == map.end() || (*it).second.iID != iAction)
	{
		//char szTmp[128];
		//sprintf(szTmp,"  action:%i button:%i\n", wAction,wButtonCode);
		//OutputDebugString(szTmp);
		CButtonAction button;
		button.iID = iAction;
		button.strID = szAction;
		map.insert(pair<WORD, CButtonAction>(wButtonCode, button));
	}
}

WORD CButtonTranslator::GetActionCode(WORD wWindow, const CKey &key, CStdString &strAction)
{
	WORD wKey = (WORD)key.GetButtonCode();
	map<WORD, buttonMap>::iterator it = translatorMap.find(wWindow);
	
	if (it == translatorMap.end())
		return 0;
	
	buttonMap::iterator it2 = (*it).second.find(wKey);
	WORD wAction = 0;
	
	while (it2 != (*it).second.end())
	{
		wAction = (*it2).second.iID;
		strAction = (*it2).second.strID;
		it2 = (*it).second.end();
	}
	
	return wAction;
}

CAction CButtonTranslator::GetAction(int window, const CKey &key)
{
	CStdString strAction;
	
	// Try to get the action from the current window
	WORD wAction = GetActionCode(window, key, strAction);

	// If it's invalid, try to get it from the global map
	if (wAction == 0)
		wAction = GetActionCode( -1, key, strAction);
	
	// Now fill our action structure
//	action.SetID((int)wAction);
//	action.SetActionString(strAction);

	CAction action((int)wAction, strAction, key);
	return action;
}

int CButtonTranslator::TranslateWindow(const CStdString &window)
{
	CStdString strWindow(window);

	if (strWindow.IsEmpty()) return WINDOW_INVALID;
		strWindow.ToLower();

	// Eliminate .xml
	if (strWindow.Mid(strWindow.GetLength() - 4) == ".xml" )
		strWindow = strWindow.Mid(0, strWindow.GetLength() - 4);

	// window12345, for custom window to be keymapped
	if (strWindow.length() > 6 && strWindow.Left(6).Equals("window"))
		strWindow = strWindow.Mid(6);

	if (strWindow.Left(2) == "my")  // drop "my" prefix
		strWindow = strWindow.Mid(2);

	if (CStringUtils::IsNaturalNumber(strWindow))
	{
		// Allow a full window id or a delta id
		int iWindow = atoi(strWindow.c_str());
		
		if (iWindow > WINDOW_INVALID)
			return iWindow;
		
		return WINDOW_HOME + iWindow;
	}

	// Run through the window structure
	for (unsigned int index = 0; index < sizeof(windows) / sizeof(windows[0]); ++index)
	{
		if (strWindow.Equals(windows[index].name))
			return windows[index].action;
	}

	CLog::Log(LOGERROR, "Window Translator: Can't find window %s", strWindow.c_str());
	return WINDOW_INVALID;
}

void CButtonTranslator::Clear()
{
	translatorMap.clear();
}