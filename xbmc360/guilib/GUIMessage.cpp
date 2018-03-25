#include "GUIMessage.h"

using namespace std;

CGUIMessage::CGUIMessage(int msg, int senderID, int controlID, int param1, int param2)
{
	m_message = msg;
	m_senderID = senderID;
	m_controlID = controlID;
	m_param1 = param1;
	m_param2 = param2;
	m_lpVoid = NULL;
}

CGUIMessage::CGUIMessage(int msg, int senderID, int controlID, int param1, int param2, void* lpVoid)
{
	m_message = msg;
	m_senderID = senderID;
	m_controlID = controlID;
	m_param1 = param1;
	m_param2 = param2;
	m_lpVoid = lpVoid;
}


int CGUIMessage::GetControlId() const
{
	return m_controlID;
}

int CGUIMessage::GetMessage() const
{
	return m_message;
}

int CGUIMessage::GetSenderId() const
{
	return m_senderID;
}

void CGUIMessage::SetLabel(const string& strLabel)
{
	m_strLabel = strLabel;
}

const string& CGUIMessage::GetLabel() const
{
	return m_strLabel;
}

void CGUIMessage::SetStringParam(const string& strParam)
{
	m_strParam = strParam;
}

const string& CGUIMessage::GetStringParam() const
{
	return m_strParam;
}

void CGUIMessage::SetParam1(int iParam1)
{
	m_param1 = iParam1;
}

void CGUIMessage::SetParam2(int iParam2)
{
	m_param2 = iParam2;
}

int CGUIMessage::GetParam1() const
{
	return m_param1;
}

int CGUIMessage::GetParam2() const
{
	return m_param2;
}

void* CGUIMessage::GetLPVOID() const
{
	return m_lpVoid;
}
