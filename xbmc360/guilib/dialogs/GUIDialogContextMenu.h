#include "guilib\GUIDialog.h"
#include "FileItem.h"

#ifndef GUILIB_CGUIDIALOGCONTEXTMENU_H
#define GUILIB_CGUIDIALOGCONTEXTMENU_H

enum CONTEXT_BUTTON { CONTEXT_BUTTON_CANCELLED = 0,
                      CONTEXT_BUTTON_LAUNCH,
                      CONTEXT_BUTTON_LAUNCH_IN,
                      CONTEXT_BUTTON_GAMESAVES,
                      CONTEXT_BUTTON_RENAME,
                      CONTEXT_BUTTON_DELETE,
                      CONTEXT_BUTTON_COPY,
                      CONTEXT_BUTTON_MOVE,
                      CONTEXT_BUTTON_TRAINER_OPTIONS,
                      CONTEXT_BUTTON_SCAN_TRAINERS,
                      CONTEXT_BUTTON_ADD_FAVOURITE,
                      CONTEXT_BUTTON_SETTINGS,
                      CONTEXT_BUTTON_GOTO_ROOT,
                      CONTEXT_BUTTON_PLAY_DISC,
                      CONTEXT_BUTTON_RIP_CD,
                      CONTEXT_BUTTON_RIP_TRACK,
                      CONTEXT_BUTTON_EJECT_DISC,
                      CONTEXT_BUTTON_ADD_SOURCE,
                      CONTEXT_BUTTON_EDIT_SOURCE,
                      CONTEXT_BUTTON_REMOVE_SOURCE,
                      CONTEXT_BUTTON_SET_DEFAULT,
                      CONTEXT_BUTTON_CLEAR_DEFAULT,
                      CONTEXT_BUTTON_SET_THUMB,
                      CONTEXT_BUTTON_ADD_LOCK,
                      CONTEXT_BUTTON_REMOVE_LOCK,
                      CONTEXT_BUTTON_CHANGE_LOCK,
                      CONTEXT_BUTTON_RESET_LOCK,
                      CONTEXT_BUTTON_REACTIVATE_LOCK,
                      CONTEXT_BUTTON_VIEW_SLIDESHOW,
                      CONTEXT_BUTTON_RECURSIVE_SLIDESHOW,
                      CONTEXT_BUTTON_REFRESH_THUMBS,
                      CONTEXT_BUTTON_SWITCH_MEDIA,
                      CONTEXT_BUTTON_MOVE_ITEM,
                      CONTEXT_BUTTON_MOVE_HERE,
                      CONTEXT_BUTTON_CANCEL_MOVE,
                      CONTEXT_BUTTON_MOVE_ITEM_UP,
                      CONTEXT_BUTTON_MOVE_ITEM_DOWN,
                      CONTEXT_BUTTON_SAVE,
                      CONTEXT_BUTTON_LOAD,
                      CONTEXT_BUTTON_CLEAR,
                      CONTEXT_BUTTON_QUEUE_ITEM,
                      CONTEXT_BUTTON_PLAY_ITEM,
                      CONTEXT_BUTTON_PLAY_WITH,
                      CONTEXT_BUTTON_PLAY_PARTYMODE,
                      CONTEXT_BUTTON_PLAY_PART,
                      CONTEXT_BUTTON_RESUME_ITEM,
                      CONTEXT_BUTTON_RESTART_ITEM,
                      CONTEXT_BUTTON_EDIT,
                      CONTEXT_BUTTON_EDIT_SMART_PLAYLIST,
                      CONTEXT_BUTTON_INFO,
                      CONTEXT_BUTTON_INFO_ALL,
                      CONTEXT_BUTTON_CDDB,
                      CONTEXT_BUTTON_UPDATE_LIBRARY,
                      CONTEXT_BUTTON_UPDATE_TVSHOW,
                      CONTEXT_BUTTON_SCAN,
                      CONTEXT_BUTTON_STOP_SCANNING,
                      CONTEXT_BUTTON_SET_ARTIST_THUMB,
                      CONTEXT_BUTTON_SET_SEASON_THUMB,
                      CONTEXT_BUTTON_NOW_PLAYING,
                      CONTEXT_BUTTON_CANCEL_PARTYMODE,
                      CONTEXT_BUTTON_MARK_WATCHED,
                      CONTEXT_BUTTON_MARK_UNWATCHED,
                      CONTEXT_BUTTON_SET_CONTENT,
                      CONTEXT_BUTTON_ADD_TO_LIBRARY,
                      CONTEXT_BUTTON_SONG_INFO,
                      CONTEXT_BUTTON_EDIT_PARTYMODE,
                      CONTEXT_BUTTON_LINK_MOVIE,
                      CONTEXT_BUTTON_UNLINK_MOVIE,
                      CONTEXT_BUTTON_GO_TO_ARTIST,
                      CONTEXT_BUTTON_GO_TO_ALBUM,
                      CONTEXT_BUTTON_PLAY_OTHER,
                      CONTEXT_BUTTON_SET_ACTOR_THUMB,
                      CONTEXT_BUTTON_SET_PLUGIN_THUMB,
                      CONTEXT_BUTTON_UNLINK_BOOKMARK,
                      CONTEXT_BUTTON_PLUGIN_SETTINGS,
                      CONTEXT_BUTTON_SCRIPT_SETTINGS,
                      CONTEXT_BUTTON_LASTFM_UNLOVE_ITEM,
                      CONTEXT_BUTTON_LASTFM_UNBAN_ITEM,
                      CONTEXT_BUTTON_SET_MOVIESET_THUMB,
                      CONTEXT_BUTTON_SET_MOVIESET_FANART,
                      CONTEXT_BUTTON_DELETE_PLUGIN,
                      CONTEXT_BUTTON_USER1,
                      CONTEXT_BUTTON_USER2,
                      CONTEXT_BUTTON_USER3,
                      CONTEXT_BUTTON_USER4,
                      CONTEXT_BUTTON_USER5,
                      CONTEXT_BUTTON_USER6,
                      CONTEXT_BUTTON_USER7,
                      CONTEXT_BUTTON_USER8,
                      CONTEXT_BUTTON_USER9,
                      CONTEXT_BUTTON_USER10
                    };

class CContextButtons : public std::vector< std::pair<unsigned int, CStdString> >
{
public:
	void Add(unsigned int, const CStdString &label);
	void Add(unsigned int, int label);
};

class CGUIDialogContextMenu : public CGUIDialog
{
public:
	CGUIDialogContextMenu(void);
	virtual ~CGUIDialogContextMenu(void);

	virtual bool OnMessage(CGUIMessage &message);
	virtual void DoModal(int iWindowID = WINDOW_INVALID, const CStdString &param = "");
	virtual void OnWindowLoaded();
	virtual void OnWindowUnload();
	virtual void SetPosition(float posX, float posY);

	void ClearButtons();
	int AddButton(const CStdString &label, int value = -1);
	int GetButton();
	void OffsetPosition(float offsetX, float offsetY);

	// Positions the current context menu in the middle of the focused control. If it can not
	// find it then it positions the context menu in the middle of the screen
	void PositionAtCurrentFocus();

	static bool OnContextButton(const CStdString &type, const CFileItemPtr item, CONTEXT_BUTTON button);
	static void GetContextButtons(const CStdString &type, const CFileItemPtr item, CContextButtons &buttons);

	// Show the context menu with the given choices
	// param choices the choices available for the user.
	// return -1 if no choice is made, else the chosen option.
	static int ShowAndGetChoice(const CContextButtons &choices);

	static CMediaSource *GetShare(const CStdString &type, const CFileItem *item);

protected:
	float GetWidth();
	float GetHeight();
	virtual void OnInitWindow();
	int m_clickedButton;
	CContextButtons m_buttons;
};

#endif //GUILIB_CGUIDIALOGCONTEXTMENU_H