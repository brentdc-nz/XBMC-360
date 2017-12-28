#ifndef GUILIB_MESSAGE_H
#define GUILIB_MESSAGE_H

#include "..\utils\stdafx.h"
#include "..\utils\StdString.h"
#include <string>

#define GUI_MSG_WINDOW_INIT     1   // initialize window
#define GUI_MSG_WINDOW_DEINIT   2   // deinit window

#define GUI_MSG_SETFOCUS        3   // set focus to control param1=up/down/left/right
#define GUI_MSG_LOSTFOCUS       4   // control lost focus
#define GUI_MSG_CLICKED         5   // control has been clicked

#define GUI_MSG_LABEL_SET		13  // set the label of a control

#define GUI_MSG_EXECUTE			20  // user has clicked on a button with <execute> tag

#define GUI_MSG_USER         1000

class CGUIMessage
{
public:
	CGUIMessage(int dwMsg, int senderID, int controlID, int param1 = 0, int param2 = 0);

	int GetControlId() const ;
	int GetMessage() const;
	int GetSenderId() const;

	void SetLabel(const std::string& strLabel);
	const std::string& GetLabel() const;

	void SetStringParam(const std::string& strParam);
	const std::string& GetStringParam() const;

	int GetParam1() const;
	int GetParam2() const;
private:
	std::string m_strLabel;
	std::string m_strParam;
	int m_senderID;
	int m_controlID;
	int m_message;
	void* m_pointer;
	int m_param1;
	int m_param2;
};

#endif//GUILIB_MESSAGE_H