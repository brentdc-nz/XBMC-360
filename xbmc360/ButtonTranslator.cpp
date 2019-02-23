#include "ButtonTranslator.h"
#include "utils\Log.h"
#include "utils\StringUtils.h"
#include "utils\Util.h"

using namespace std;

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

	CLog::Log(LOGINFO, "Loading D:\\keymap.xml");

	// Load the config file
	if (!xmlDoc.LoadFile("D:\\keymap.xml"))
	{
		g_LoadErrorStr.Format("D:\\keymap.xml, Line %d\n%s", xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
		return false;
	}

	TiXmlElement* pRoot = xmlDoc.RootElement();
	CStdString strValue = pRoot->Value();
	if ( strValue != "keymap")
	{
		g_LoadErrorStr.Format("Q:\\keymap.xml Doesn't contain <keymap>");
		return false;
	}

	// run through our window groups
	TiXmlNode* pWindow = pRoot->FirstChild();
	while (pWindow)
	{
		WORD wWindowID = WINDOW_INVALID;
		const char *szWindow = pWindow->Value();
		if (szWindow)
		{
			if (strcmpi(szWindow, "global") == 0)
				wWindowID = -1;
			else
				wWindowID = TranslateWindowString(szWindow);
		}
		MapWindowActions(pWindow, wWindowID);
		pWindow = pWindow->NextSibling();
	}
  
	// Done!
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
		// map gamepad actions
		TiXmlElement *pButton = pDevice->FirstChildElement();
		while (pButton)
		{
			WORD wButtonCode = TranslateGamepadString(pButton->Value());
			
			if (pButton->FirstChild()) 
				MapAction(wButtonCode, pButton->FirstChild()->Value(), map);

			pButton = pButton->NextSiblingElement();
		}
	}
	
	// add our map to our table
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
	else if (strButton.Equals("white")) wButtonCode = KEY_BUTTON_WHITE;
	else if (strButton.Equals("black")) wButtonCode = KEY_BUTTON_BLACK;
	else if (strButton.Equals("start")) wButtonCode = KEY_BUTTON_START;
	else if (strButton.Equals("back")) wButtonCode = KEY_BUTTON_BACK;
	else if (strButton.Equals("righttrigger")) wButtonCode = KEY_BUTTON_RIGHT_TRIGGER;
	else if (strButton.Equals("dpadleft")) wButtonCode = KEY_BUTTON_DPAD_LEFT;
	else if (strButton.Equals("dpadright")) wButtonCode = KEY_BUTTON_DPAD_RIGHT;
	else if (strButton.Equals("dpadup")) wButtonCode = KEY_BUTTON_DPAD_UP;
	else if (strButton.Equals("dpaddown")) wButtonCode = KEY_BUTTON_DPAD_DOWN;
	else CLog::Log(LOGERROR, "Gamepad Translator: Can't find button %s", strButton.c_str());
	
	return wButtonCode;
}

bool CButtonTranslator::TranslateActionString(const char *szAction, int &iAction)
{
	iAction = ACTION_NONE;
	CStdString strAction = szAction;
	strAction.ToLower();

	if (CUtil::IsBuiltIn(strAction)) iAction = ACTION_BUILT_IN_FUNCTION;
	else if (strAction.Equals("left")) iAction = ACTION_MOVE_LEFT;
	else if (strAction.Equals("right")) iAction = ACTION_MOVE_RIGHT;
	else if (strAction.Equals("up")) iAction = ACTION_MOVE_UP;
	else if (strAction.Equals("down")) iAction = ACTION_MOVE_DOWN;
	else if (strAction.Equals("select")) iAction = ACTION_SELECT_ITEM;
	else if (strAction.Equals("previousmenu")) iAction = ACTION_PREVIOUS_MENU;

	else if (strAction.Equals("fullscreen")) iAction = ACTION_SHOW_GUI;
	else if (strAction.Equals("codecinfo")) iAction = ACTION_SHOW_CODEC;
	else if (strAction.Equals("pause")) iAction = ACTION_PAUSE;
	else if (strAction.Equals("stop")) iAction = ACTION_STOP;
	else if (strAction.Equals("stepforward")) iAction = ACTION_STEP_FORWARD;
	else if (strAction.Equals("stepback")) iAction = ACTION_STEP_BACK;
	else if (strAction.Equals("bigstepforward")) iAction = ACTION_BIG_STEP_FORWARD;
	else if (strAction.Equals("bigstepback")) iAction = ACTION_BIG_STEP_BACK;

	else
		CLog::Log(LOGERROR, "Keymapping error: no such action '%s' defined", strAction.c_str());
	return (iAction != ACTION_NONE);
}

void CButtonTranslator::MapAction(WORD wButtonCode, const char *szAction, buttonMap &map)
{
	int iAction = ACTION_NONE;
	if (!TranslateActionString(szAction, iAction) || !wButtonCode)
		return;   // no valid action, or an invalid buttoncode

	// have a valid action, and a valid button - map it.
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
	//Windows
	else if (strWindow.Equals("home")) wWindowID = WINDOW_HOME;
	else if (strWindow.Equals("fullscreenvideo")) wWindowID = WINDOW_FULLSCREEN_VIDEO;
	else if (strWindow.Equals("myprograms")) wWindowID = WINDOW_PROGRAMS;
	else if (strWindow.Equals("myvideos")) wWindowID = WINDOW_VIDEOS;
	else if (strWindow.Equals("mymusic")) wWindowID = WINDOW_MUSIC;
	else if (strWindow.Equals("settings")) wWindowID = WINDOW_SETTINGS;
	else if (strWindow.Equals("appearancesettings")) wWindowID = WINDOW_SETTINGS_APPEARANCE;
	else if (strWindow.Equals("screensaver")) wWindowID = WINDOW_SCREENSAVER;
	else if (strWindow.Equals("systeminfo")) wWindowID = WINDOW_SYSTEM_INFORMATION;
	//Dialogs
	else if (strWindow.Equals("shutdownmenu")) wWindowID = WINDOW_DIALOG_BUTTON_MENU;
	else
		CLog::Log(LOGERROR, "Window Translator: Can't find window %s", strWindow.c_str());

	CLog::Log(LOGDEBUG, "CButtonTranslator::TranslateWindowString(%s) returned Window ID (%i)", szWindow, wWindowID);
	return wWindowID;
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

void CButtonTranslator::GetAction(WORD wWindow, const CKey &key, CAction &action)	
{
	CStdString strAction;
	
	// Try to get the action from the current window
	WORD wAction = GetActionCode(wWindow, key, strAction);

	// If it's invalid, try to get it from the global map
	if (wAction == 0)
		wAction = GetActionCode( -1, key, strAction);
	
	// Now fill our action structure
	action.SetID((int)wAction);
	action.SetActionString(strAction);
}

void CButtonTranslator::Clear()
{
	translatorMap.clear();
}
