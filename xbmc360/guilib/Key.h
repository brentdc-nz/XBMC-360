#ifndef GUIKEY_H
#define GUIKEY_H

//====================
//		Keys
//====================

#define KEY_INVALID         0xFFFF

// Analogue - don't change order
#define KEY_BUTTON_A                        256
#define KEY_BUTTON_B                        257
#define KEY_BUTTON_X                        258
#define KEY_BUTTON_Y                        259
#define KEY_BUTTON_LEFT_SHOULDER            260 // ex-black
#define KEY_BUTTON_RIGHT_SHOULDER           261 // ex-white
#define KEY_BUTTON_LEFT_TRIGGER             262
#define KEY_BUTTON_RIGHT_TRIGGER            263

#define KEY_BUTTON_DPAD_UP                  270
#define KEY_BUTTON_DPAD_DOWN                271
#define KEY_BUTTON_DPAD_LEFT                272
#define KEY_BUTTON_DPAD_RIGHT               273

#define KEY_BUTTON_START                    274
#define KEY_BUTTON_BACK                     275

//====================
//		Actions
//====================

// Actions that we have defined...
#define ACTION_NONE                    0
#define ACTION_MOVE_LEFT               1
#define ACTION_MOVE_RIGHT              2
#define ACTION_MOVE_UP                 3
#define ACTION_MOVE_DOWN               4

#define ACTION_SELECT_ITEM             7
#define ACTION_PREVIOUS_MENU          10
#define ACTION_PAUSE                  12
#define ACTION_STOP                   13

#define ACTION_SHOW_GUI               18 // toggle between GUI and movie or GUI and visualisation.
#define ACTION_STEP_FORWARD           20 // seek +1% in the movie. Can be used in videoFullScreen.xml window id=2005
#define ACTION_STEP_BACK              21 // seek -1% in the movie. Can be used in videoFullScreen.xml window id=2005
#define ACTION_BIG_STEP_FORWARD       22 // seek +10% in the movie. Can be used in videoFullScreen.xml window id=2005
#define ACTION_BIG_STEP_BACK          23 // seek -10% in the movie. Can be used in videoFullScreen.xml window id=2005
#define ACTION_SHOW_CODEC             27 // show information about file. Can be used in videoFullScreen.xml window id=2005

#define ACTION_NAV_BACK				  92
#define ACTION_CONTEXT_MENU           117 // pops up the context menu
#define ACTION_BUILT_IN_FUNCTION	  122

//====================
//		Windows
//====================

// Window ID defines to make the code a bit more readable
#define WINDOW_INVALID                     9999
#define WINDOW_HOME                       10000
#define WINDOW_PROGRAMS                   10001
#define WINDOW_SETTINGS                   10004
#define WINDOW_MUSIC                      10005
#define WINDOW_VIDEOS                     10006
#define WINDOW_PICTURES                   10007
#define WINDOW_SYSTEM_INFORMATION         10008

#define WINDOW_SETTINGS_MYPICTURES        10012
#define WINDOW_SETTINGS_MYPROGRAMS        10013
#define WINDOW_SETTINGS_MYWEATHER         10014
#define WINDOW_SETTINGS_MYMUSIC           10015
#define WINDOW_SETTINGS_SYSTEM            10016
#define WINDOW_SETTINGS_MYVIDEOS          10017
#define WINDOW_SETTINGS_NETWORK           10018
#define WINDOW_SETTINGS_APPEARANCE        10019

#define WINDOW_FULLSCREEN_VIDEO           12005
#define WINDOW_SCREENSAVER                12900

// Dialogs
#define WINDOW_DIALOG_YES_NO              10100
#define WINDOW_DIALOG_CONTEXT_MENU        10106
#define WINDOW_DIALOG_BUTTON_MENU         10111
#define WINDOW_DIALOG_SEEK_BAR            10115
#define WINDOW_DIALOG_MEDIA_SOURCE        10116
#define WINDOW_DIALOG_FILE_BROWSER        10126

class CKey
{
public:
	CKey(int iButtonCode){ m_iButtonCode = iButtonCode; };
	CKey::~CKey(void){};

	int GetButtonCode()  const { return m_iButtonCode; };

private:
	int m_iButtonCode;
};

class CAction
{
public:
	CAction() { m_id = ACTION_NONE; m_strAction = ""; }
	CAction(int iID) { m_id = iID; }
	int GetID() const { return m_id; };
	void SetID(int id) { m_id = id; };
	CStdString GetActionString() { return m_strAction; };
	void SetActionString(CStdString strAction) { m_strAction = strAction; };

private:
	int m_id;
	CStdString m_strAction;
};

#endif //GUIKEY_H