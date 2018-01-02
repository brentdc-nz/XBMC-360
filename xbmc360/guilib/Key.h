#ifndef GUIKEY_H
#define GUIKEY_H

#define KEY_INVALID         0xFFFF

// actions that we have defined...
#define ACTION_NONE                    0
#define ACTION_MOVE_LEFT               1
#define ACTION_MOVE_RIGHT              2
#define ACTION_MOVE_UP                 3
#define ACTION_MOVE_DOWN               4
//--
#define ACTION_SELECT_ITEM             7

#define ACTION_SHOW_GUI               18 // toggle between GUI and movie or GUI and visualisation.

// Window ID defines to make the code a bit more readable
#define WINDOW_INVALID                     9999
#define WINDOW_HOME                       10000
#define WINDOW_PROGRAMS                   10001
#define WINDOW_VIDEOS                     10006
#define WINDOW_SETTINGS                   10007

#define WINDOW_FULLSCREEN_VIDEO           12005
#define WINDOW_SCREENSAVER                12900

#define WINDOW_SETTINGS_MYPICTURES        10012
#define WINDOW_SETTINGS_MYPROGRAMS        10013
#define WINDOW_SETTINGS_MYWEATHER         10014
#define WINDOW_SETTINGS_MYMUSIC           10015
#define WINDOW_SETTINGS_SYSTEM            10016
#define WINDOW_SETTINGS_MYVIDEOS          10017
#define WINDOW_SETTINGS_NETWORK           10018
#define WINDOW_SETTINGS_APPEARANCE        10019

/*!
  \ingroup actionkeys
  \brief 
  */
struct CAction
{
	WORD wID;
	DWORD m_dwButtonCode;
	CStdString strAction;
};

#endif //GUIKEY_H