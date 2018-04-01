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

#define GUI_MSG_LABEL_ADD       12  // Add label control (for controls supporting more then 1 label)
#define GUI_MSG_LABEL_SET		13  // Set the label of a control
#define GUI_MSG_ITEM_SELECTED   15  // Ask control 2 return the selected item
#define GUI_MSG_EXECUTE			20  // User has clicked on a button with <execute> tag

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