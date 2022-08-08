#ifndef GUILIB_MESSAGE_H
#define GUILIB_MESSAGE_H

#include "..\utils\Stdafx.h"
#include "..\utils\StdString.h"
#include <string>

#define GUI_MSG_WINDOW_INIT    1   // Initialize window
#define GUI_MSG_WINDOW_DEINIT  2   // Deinit window

#define GUI_MSG_SETFOCUS       3   // Set focus to control param1=up/down/left/right
#define GUI_MSG_LOSTFOCUS      4   // Control lost focus
#define GUI_MSG_CLICKED        5   // Control has been clicked
#define GUI_MSG_VISIBLE        6   // Set control visible
#define GUI_MSG_HIDDEN         7   // Set control hidden

#define GUI_MSG_ENABLED        8   // Enable control
#define GUI_MSG_DISABLED       9   // Disable control

#define GUI_MSG_SELECTED       10   // control = selected
#define GUI_MSG_DESELECTED     11   // control = not selected

#define GUI_MSG_SCROLL_DOWN    11   // Used for scrollbar
#define GUI_MSG_SCROLL_UP      12   // Used for scrollbar

#define GUI_MSG_LABEL_SET      13  // set the label of a control
#define GUI_MSG_LABEL_RESET    14  // clear all labels of a control // add label control (for controls supporting more then 1 label)
#define GUI_MSG_ITEMS_RESET    15  // Clear out all the items for the control
#define GUI_MSG_ITEM_SELECTED  16  // ask control 2 return the selected item
#define GUI_MSG_LABEL2_SET     17
#define GUI_MSG_SHOWRANGE      18
#define GUI_MSG_ITEM_SELECT    19 // ask control 2 select a specific item
#define GUI_MSG_ITEM_ADD       20  // Add item to control
#define GUI_MSG_FULLSCREEN     21  // should go to fullscreen window (vis or video)
#define GUI_MSG_EXECUTE        22  // user has clicked on a button with <execute> tag

#define GUI_MSG_NOTIFY_ALL     23  // message will be send to all active and inactive(!) windows, all active modal and modeless dialogs
                                  // dwParam1 must contain an additional message the windows should react on

#define GUI_MSG_REFRESH_THUMBS 22 // message is sent to all windows to refresh all thumbs

#define GUI_MSG_MOVE           24 // message is sent to the window from the base control class when it's
                                 // been asked to move.  dwParam1 contains direction.

#define GUI_MSG_LABEL_BIND     25   // bind label control (for controls supporting more then 1 label)


#define GUI_MSG_SCROLL_CHANGE  26  // A page control has changed the page number
#define GUI_MSG_PAGE_UP        27  // Page up
#define GUI_MSG_PAGE_DOWN      28  // Page down

#define GUI_MSG_SELCHANGED     29  // selection within the control has changed

#define GUI_MSG_FOCUSED        30  // a control has become focused

#define GUI_MSG_WINDOW_RESET   31  // reset window to initial state

#define GUI_MSG_PAGE_CHANGE    32  // a page control has changed the page number

#define GUI_MSG_REFRESH_LIST   33 // message sent to all listing controls telling them to refresh their item layouts



#define GUI_MSG_MOVE_OFFSET    35 // Instruct the contorl to MoveUp or MoveDown by offset amount

#define GUI_MSG_SET_TYPE       36 ///< Instruct a control to set it's type appropriately7

#define GUI_MSG_INVALIDATE     37 ///< Instruct all controls to refresh - usually due to sizing changes

#define GUI_MSG_USER           1000


// Send message macros

#define SET_CONTROL_LABEL(dwControlID, label) \
{ \
	CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), dwControlID); \
	msg.SetLabel(label); \
	OnMessage(msg); \
}

#define SET_CONTROL_HIDDEN(dwControlID) \
{ \
	CGUIMessage msg(GUI_MSG_HIDDEN, GetID(), dwControlID); \
	OnMessage(msg); \
}

#define SET_CONTROL_VISIBLE(dwControlID) \
{ \
	CGUIMessage msg(GUI_MSG_VISIBLE, GetID(), dwControlID); \
	OnMessage(msg); \
}

#define SET_CONTROL_FOCUS(dwControlID, dwParam) \
{ \
	CGUIMessage msg(GUI_MSG_SETFOCUS, GetID(), dwControlID, dwParam); \
	OnMessage(msg); \
}

#define CONTROL_ENABLE(dwControlID) \
do { \
	CGUIMessage msg(GUI_MSG_ENABLED, GetID(), dwControlID); \
	OnMessage(msg); \
} while(0);

#define CONTROL_DISABLE(dwControlID) \
do { \
	CGUIMessage msg(GUI_MSG_DISABLED, GetID(), dwControlID); \
	OnMessage(msg); \
} while(0);

class CGUIMessage
{
public:
	CGUIMessage(int dwMsg, int senderID, int controlID, int param1 = 0, int param2 = 0);
	CGUIMessage(int dwMsg, int senderID, int controlID, int param1, int param2, void* lpVoid);

	int GetControlId() const ;
	int GetMessage() const;
	int GetSenderId() const;

	void SetLabel(const std::string& strLabel);
	const std::string& GetLabel() const;

	void SetStringParam(const std::string& strParam);
	const std::string& GetStringParam() const;

	void SetParam1(int iParam1);
	void SetParam2(int iParam2);
	int GetParam1() const;
	int GetParam2() const;
	void* GetLPVOID() const;

private:
	std::string m_strLabel;
	std::string m_strParam;
	int m_senderID;
	int m_controlID;
	int m_message;
	void* m_lpVoid;
	int m_param1;
	int m_param2;
};

#endif//GUILIB_MESSAGE_H