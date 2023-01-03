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

// 0xF000 -> 0xF200 is reserved for the keyboard; a keyboard press is either
#define KEY_VKEY            0xF000 // A virtual key/functional key e.g. cursor left
#define KEY_ASCII           0xF100 // A printable character in the range of TRUE ASCII (from 0 to 127) // FIXME make it clean and pure unicode! remove the need for KEY_ASCII
#define KEY_UNICODE         0xF200 // Another printable character whose range is not included in this KEY code

#define REMOTE_0                    58  // Remote keys 0-9. are used by multiple windows
#define REMOTE_1                    59  // For example in videoFullScreen.xml window id=2005 you can
#define REMOTE_2                    60  // Enter time (mmss) to jump to particular point in the movie
#define REMOTE_3                    61
#define REMOTE_4                    62  // With spincontrols you can enter 3digit number to quickly set
#define REMOTE_5                    63  // Spincontrol to desired value
#define REMOTE_6                    64
#define REMOTE_7                    65
#define REMOTE_8                    66
#define REMOTE_9                    67


//====================
//		Actions
//====================

// Actions that we have defined...
#define ACTION_NONE                    0
#define ACTION_MOVE_LEFT               1
#define ACTION_MOVE_RIGHT              2
#define ACTION_MOVE_UP                 3
#define ACTION_MOVE_DOWN               4

#define ACTION_PAGE_UP                 5
#define ACTION_PAGE_DOWN               6

#define ACTION_SELECT_ITEM             7
#define ACTION_PREVIOUS_MENU          10
#define ACTION_PAUSE                  12
#define ACTION_STOP                   13

#define ACTION_SHOW_GUI               18 // Toggle between GUI and movie or GUI and visualisation.
#define ACTION_STEP_FORWARD           20 // Seek +1% in the movie. Can be used in videoFullScreen.xml window id=2005
#define ACTION_STEP_BACK              21 // Seek -1% in the movie. Can be used in videoFullScreen.xml window id=2005
#define ACTION_BIG_STEP_FORWARD       22 // Seek +10% in the movie. Can be used in videoFullScreen.xml window id=2005
#define ACTION_BIG_STEP_BACK          23 // Seek -10% in the movie. Can be used in videoFullScreen.xml window id=2005
#define ACTION_SHOW_CODEC             27 // Show information about file. Can be used in videoFullScreen.xml window id=2005

#define ACTION_NAV_BACK				  92
#define ACTION_CONTEXT_MENU           117 // Pops up the context menu
#define ACTION_BUILT_IN_FUNCTION	  122
#define ACTION_ENTER                  135

// Stuff for virtual keyboard shortcuts
#define ACTION_SHIFT              118
#define ACTION_SYMBOLS            119
#define ACTION_CURSOR_LEFT        120
#define ACTION_CURSOR_RIGHT       121

#define ACTION_NEXT_CONTROL           181
#define ACTION_PREV_CONTROL           182

#define ACTION_BACKSPACE          110
#define ACTION_SCROLL_UP          111
#define ACTION_SCROLL_DOWN        112
#define ACTION_ANALOG_FORWARD     113
#define ACTION_ANALOG_REWIND      114

#define ACTION_PARENT_DIR              9

#define ACTION_FILTER_CLEAR           150
#define ACTION_FILTER_SMS2            151
#define ACTION_FILTER_SMS3            152
#define ACTION_FILTER_SMS4            153
#define ACTION_FILTER_SMS5            154
#define ACTION_FILTER_SMS6            155
#define ACTION_FILTER_SMS7            156
#define ACTION_FILTER_SMS8            157
#define ACTION_FILTER_SMS9            158

//====================
//		Windows
//====================

// Window ID defines to make the code a bit more readable
#define WINDOW_INVALID                     9999
#define WINDOW_HOME                       10000
#define WINDOW_PROGRAMS                   10001
#define WINDOW_PICTURES                   10002
#define WINDOW_SETTINGS                   10004
#define WINDOW_MUSIC                      10005
#define WINDOW_VIDEOS                     10006
#define WINDOW_SYSTEM_INFORMATION         10007

#define WINDOW_VIDEO_FILES                10024

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
#define WINDOW_DIALOG_PROGRESS            10101
#define WINDOW_DIALOG_CONTEXT_MENU        10106
#define WINDOW_DIALOG_NUMERIC             10109
#define WINDOW_DIALOG_BUTTON_MENU         10111
#define WINDOW_DIALOG_SEEK_BAR            10115
#define WINDOW_DIALOG_MEDIA_SOURCE        10116
#define WINDOW_DIALOG_FILE_BROWSER        10126
#define WINDOW_DIALOG_NETWORK_SETUP       10128
#define WINDOW_DIALOG_SLIDER              10145
#define WINDOW_DIALOG_KEYBOARD            10103
#define WINDOW_DIALOG_OK                  12002

#define WINDOW_MUSIC_FILES                10501
#define WINDOW_FILES                      10003

#define WINDOW_START                      12998 // First window to load
#define WINDOW_STARTUP_ANIM               12999 // For startup animations

#define ACTION_NEXT_LETTER            140 // Jump through a list or container by letter
#define ACTION_PREV_LETTER            141

#define ACTION_JUMP_SMS2              142 // Jump direct to a particular letter using SMS-style input
#define ACTION_JUMP_SMS3              143
#define ACTION_JUMP_SMS4              144
#define ACTION_JUMP_SMS5              145
#define ACTION_JUMP_SMS6              146
#define ACTION_JUMP_SMS7              147
#define ACTION_JUMP_SMS8              148
#define ACTION_JUMP_SMS9              149

#define ACTION_FIRST_PAGE             159
#define ACTION_LAST_PAGE              160

#define ACTION_NEXT_ITEM              14
#define ACTION_PREV_ITEM              15

#include "utils\Log.h" //REMOVE ME

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

	// Is this an action from the mouse
	// return true if this is a mouse action, false otherwise
#ifdef _HAS_MOUSE
	bool IsMouse() const { return (m_id >= ACTION_MOUSE_START && m_id <= ACTION_MOUSE_END); };
#else
	bool IsMouse() const { return false; };
#endif

	float GetAmount(unsigned int index = 0) const { 
		CLog::Log(LOGERROR,"CAction::GetAmount not yet implemented!!");

		return 0;
	};

private:
	int m_id;
	CStdString m_strAction;
};

#endif //GUIKEY_H