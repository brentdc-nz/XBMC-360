#include "GUIDialogBoxBase.h"
#include "LocalizeStrings.h"

CGUIDialogBoxBase::CGUIDialogBoxBase(int id, const CStdString &xmlFile)
	: CGUIDialog(id, xmlFile)
{
	m_bConfirmed = false;
}

CGUIDialogBoxBase::~CGUIDialogBoxBase(void)
{
}

bool CGUIDialogBoxBase::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_INIT:
		{
			CGUIDialog::OnMessage(message);
			m_bConfirmed = false;
			return true;
		}
		break;
	}
	return CGUIDialog::OnMessage(message);
}

bool CGUIDialogBoxBase::IsConfirmed() const
{
	return m_bConfirmed;
}

void CGUIDialogBoxBase::SetHeading(const string& strLine)
{
	Initialize();
	CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), 1);
	msg.SetLabel(strLine);
	OnMessage(msg);
}

void CGUIDialogBoxBase::SetHeading(int iString)
{
	Initialize();
	CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), 1);
	
	if(iString)
		msg.SetLabel(g_localizeStrings.Get(iString));
	else
		msg.SetLabel("");

	OnMessage(msg);
}

void CGUIDialogBoxBase::SetLine(int iLine, const string& strLine)
{
	Initialize();
	CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), iLine + 2);
	msg.SetLabel(strLine);
	OnMessage(msg);
}

void CGUIDialogBoxBase::SetLine(int iLine, int iString)
{
	Initialize();
	CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), iLine + 2);
	
	if(iString)
		msg.SetLabel(g_localizeStrings.Get(iString));
	else
		msg.SetLabel("");

	OnMessage(msg);
}

void CGUIDialogBoxBase::SetChoice(int iButton, int iString) // iButton == 0 for no, 1 for yes
{
	Initialize();
	CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), 10+iButton);

	if(iString)
		msg.SetLabel(g_localizeStrings.Get(iString));
	else
		msg.SetLabel("");

	OnMessage(msg);
}

void CGUIDialogBoxBase::SetChoice(int iButton, const string& strString) // iButton == 0 for no, 1 for yes
{
	Initialize();
	CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), 10+iButton);
	msg.SetLabel(strString);
	OnMessage(msg);
}

void CGUIDialogBoxBase::OnInitWindow()
{
	// Set focus to default
	m_lastControlID = m_defaultControl;
	CGUIDialog::OnInitWindow();
}
