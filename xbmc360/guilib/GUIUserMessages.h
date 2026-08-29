// GUI messages outside GuiLib

#ifndef GUILIB_USERMESSAGE_H
#define GUILIB_USERMESSAGE_H

#include "GUIMessage.h"

#define GUI_MSG_PLAYBACK_ENDED          GUI_MSG_USER + 6

// Source related messages
#define GUI_MSG_REMOVED_MEDIA           GUI_MSG_USER + 1
#define GUI_MSG_UPDATE_SOURCES          GUI_MSG_USER + 2

// General playlist items changed
#define GUI_MSG_PLAYLIST_CHANGED        GUI_MSG_USER + 3

// Start Slideshow in my pictures
// Param lpVoid: CStdString* that points to the Directory
// to start the slideshow in.
#define GUI_MSG_START_SLIDESHOW         GUI_MSG_USER + 4

#define GUI_MSG_PLAYBACK_STARTED        GUI_MSG_USER + 5
#define GUI_MSG_PLAYBACK_ENDED          GUI_MSG_USER + 6

// Playback stopped by user
#define GUI_MSG_PLAYBACK_STOPPED        GUI_MSG_USER + 7

// Queue the next item for playback
#define GUI_MSG_QUEUE_NEXT_ITEM         GUI_MSG_USER + 16

//  Message is send by the playlistplayer when it starts a playlist
//  Parameter:
//  dwParam1 = Current Playlist, can be PLAYLIST_MUSIC, PLAYLIST_TEMP_MUSIC, PLAYLIST_VIDEO or PLAYLIST_TEMP_VIDEO
//  dwParam2 = Item started in the playlist
//  lpVoid = Playlistitem started playing
#define GUI_MSG_PLAYLISTPLAYER_STARTED  GUI_MSG_USER + 8

//  Message is send by playlistplayer when next/previous item is started
//  Parameter:
//  dwParam1 = Current Playlist, can be PLAYLIST_MUSIC, PLAYLIST_TEMP_MUSIC, PLAYLIST_VIDEO or PLAYLIST_TEMP_VIDEO
//  dwParam2 = LOWORD Position of the current playlistitem
//             HIWORD Position of the previous playlistitem
//  lpVoid = Current Playlistitem
#define GUI_MSG_PLAYLISTPLAYER_CHANGED  GUI_MSG_USER + 9

//  Message is send by the playlistplayer when the last item to play ended
//  Parameter:
//  dwParam1 = Current Playlist, can be PLAYLIST_MUSIC, PLAYLIST_TEMP_MUSIC, PLAYLIST_VIDEO or PLAYLIST_TEMP_VIDEO
//  dwParam2 = Playlistitem played when stopping
#define GUI_MSG_PLAYLISTPLAYER_STOPPED  GUI_MSG_USER + 10

#define GUI_MSG_LOAD_SKIN               GUI_MSG_USER + 11

// Mute activated by the user
#define GUI_MSG_MUTE_ON                 GUI_MSG_USER + 14
#define GUI_MSG_MUTE_OFF                GUI_MSG_USER + 15

// Visualisation messages when loading/unloading
#define GUI_MSG_VISUALISATION_UNLOADING GUI_MSG_USER + 17 // Sent by vis
#define GUI_MSG_VISUALISATION_LOADED    GUI_MSG_USER + 18 // Sent by vis
#define GUI_MSG_GET_VISUALISATION       GUI_MSG_USER + 19 // Request to vis for the visualisation object
#define GUI_MSG_VISUALISATION_ACTION    GUI_MSG_USER + 20 // Request the vis perform an action

// Message is sent by the background info loader when it's finished with fetching a weather location
#define GUI_MSG_WEATHER_FETCHED       GUI_MSG_USER + 24

// Message is sent to media windows to force a refresh
#define GUI_MSG_UPDATE                GUI_MSG_USER + 26

// Message sent to tell the GUI to update a single item
#define GUI_MSG_UPDATE_ITEM           GUI_MSG_USER + 29

// Message sent by filtering dialog to request a new filter be applied
#define GUI_MSG_FILTER_ITEMS          GUI_MSG_USER + 27

// Message sent by search dialog to request a new search be applied
#define GUI_MSG_SEARCH_UPDATE         GUI_MSG_USER + 28

// Message sent to tell the GUI to update a single item
#define GUI_MSG_UPDATE_ITEM           GUI_MSG_USER + 29

// Sent from filesystem if a path is known to have changed
#define GUI_MSG_UPDATE_PATH           GUI_MSG_USER + 33

#endif //GUILIB_USERMESSAGE_H