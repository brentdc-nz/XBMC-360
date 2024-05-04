// GUI messages outside GuiLib

#ifndef GUILIB_USERMESSAGE_H
#define GUILIB_USERMESSAGE_H

#include "GUIMessage.h"

#define GUI_MSG_PLAYBACK_ENDED          GUI_MSG_USER + 6

// Source related messages
#define GUI_MSG_REMOVED_MEDIA           GUI_MSG_USER + 1
#define GUI_MSG_UPDATE_SOURCES          GUI_MSG_USER + 2

#define GUI_MSG_PLAYBACK_STARTED        GUI_MSG_USER + 5
#define GUI_MSG_PLAYBACK_ENDED          GUI_MSG_USER + 6

// Playback stopped by user
#define GUI_MSG_PLAYBACK_STOPPED        GUI_MSG_USER + 7

#define GUI_MSG_LOAD_SKIN               GUI_MSG_USER + 11

// Mute activated by the user
#define GUI_MSG_MUTE_ON                 GUI_MSG_USER + 14
#define GUI_MSG_MUTE_OFF                GUI_MSG_USER + 15

// Message is sent to media windows to force a refresh
#define GUI_MSG_UPDATE                GUI_MSG_USER + 26

// Message sent to tell the GUI to update a single item
#define GUI_MSG_UPDATE_ITEM           GUI_MSG_USER + 29

// Message sent by filtering dialog to request a new filter be applied
#define GUI_MSG_FILTER_ITEMS          GUI_MSG_USER + 27

// Message sent by search dialog to request a new search be applied
#define GUI_MSG_SEARCH_UPDATE         GUI_MSG_USER + 28

// Sent from filesystem if a path is known to have changed
#define GUI_MSG_UPDATE_PATH           GUI_MSG_USER + 33

#endif //GUILIB_USERMESSAGE_H