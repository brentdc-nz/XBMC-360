#ifndef GUILIB_MESSAGE_H
#define GUILIB_MESSAGE_H

#include "..\utils\Stdafx.h"
#include "..\utils\StdString.h"
#include <string>

#define GUI_MSG_WINDOW_INIT     1   // Initialize window
#define GUI_MSG_WINDOW_DEINIT   2   // Deinit window

#define GUI_MSG_SETFOCUS        3   // Set focus to control param1=up/down/left/right
#define GUI_MSG_LOSTFOCUS       4   // Control lost focus
#define GUI_MSG_CLICKED         5   // Control has been clicked
#define GUI_MSG_VISIBLE         6   // Set control visible
#define GUI_MSG_HIDDEN          7   // Set control hidden

#define GUI_MSG_ENABLED         8   // Enable control
#define GUI_MSG_DISABLED        9   // Disable control

#define GUI_MSG_SCROLL_DOWN     0   // Used for scrollbar
#define GUI_MSG_SCROLL_UP       11   // Used for scrollbar

#define GUI_MSG_LABEL_RESET     12  // Clear all labels of a control // add label control (for controls supporting more then 1 label)
#define GUI_MSG_LABEL_ADD       13  // Add label control (for controls supporting more then 1 label)
#define GUI_MSG_ITEMS_RESET     14  // Clear out all the items for the control
#define GUI_MSG_ITEM_ADD        15  // Add item to control
#define GUI_MSG_LABEL_SET       16  // Set the label of a control
#define GUI_MSG_ITEM_SELECTED   17  // Ask control to return the selected item
#define GUI_MSG_ITEM_SELECT     18  // Ask control to select a specific item
#define GUI_MSG_EXECUTE         19  // User has clicked on a button with <execute> tag
#define GUI_MSG_NOTIFY_ALL      20  // Message will be send to all active and inactive(!) windows, all active modal and modeless dialogs

#define GUI_MSG_SCROLL_CHANGE   21  // A page control has changed the page number
#define GUI_MSG_PAGE_UP         22  // Page up
#define GUI_MSG_PAGE_DOWN       23  // Page down
#define GUI_MSG_MOVE            24  // Message is sent to the window from the base control class when it's
                                    // been asked to move.  dwParam1 contains direction.
#define GUI_MSG_FOCUSED         26  // A control has become focused

#define GUI_MSG_USER         1000

// Send messgae macros

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