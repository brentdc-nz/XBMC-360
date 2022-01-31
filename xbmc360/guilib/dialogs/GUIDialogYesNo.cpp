#include "GUIDialogYesNo.h"
#include "..\GUIWindowManager.h"

#define BUTTON_YES 11
#define BUTTON_NO  10

CGUIDialogYesNo::CGUIDialogYesNo()
	: CGUIDialogBoxBase(WINDOW_DIALOG_YES_NO, "DialogYesNo.xml")
{
	m_bConfirmed = false;
}

CGUIDialogYesNo::~CGUIDialogYesNo()
{
}

bool CGUIDialogYesNo::OnMessage(CGUIMessage& message)
{
	switch(message.GetMessage())
	{
		case GUI_MSG_CLICKED:
		{
			int iControl = message.GetSenderId();

			if (iControl == BUTTON_NO)
			{
				m_bConfirmed = false;
				Close();
				return true;
			}

			if (iControl == BUTTON_YES)
			{
				m_bConfirmed = true;
				Close();
				return true;
			}
		}
		break;
	}

	return CGUIDialogBoxBase::OnMessage(message);
}

bool CGUIDialogYesNo::OnAction(const CAction& action)
{
	if(action.GetID() == ACTION_PREVIOUS_MENU)
	{
		m_bCanceled = true;
		m_bConfirmed = false;
		Close();
		return true;
	}

	return CGUIDialogBoxBase::OnAction(action);
}

bool CGUIDialogYesNo::ShowAndGetInput(int heading, int line0, int line1, int line2, int iNoLabel, int iYesLabel)
{
	bool bDummy;
	return ShowAndGetInput(heading,line0,line1,line2,iNoLabel,iYesLabel, bDummy);
}

bool CGUIDialogYesNo::ShowAndGetInput(int heading, int line0, int line1, int line2, int iNoLabel, int iYesLabel, bool& bCanceled)
{
	CGUIDialogYesNo *dialog = (CGUIDialogYesNo *)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
	if(!dialog) return false;
	
	dialog->SetHeading(heading);
	dialog->SetLine(0, line0);
	dialog->SetLine(1, line1);
	dialog->SetLine(2, line2);
	
	if(iNoLabel != -1)
		dialog->SetChoice(0,iNoLabel);

	if(iYesLabel != -1)
		dialog->SetChoice(1,iYesLabel);

	dialog->m_bCanceled = false;
	dialog->DoModal();
	bCanceled = dialog->m_bCanceled;

	return (dialog->IsConfirmed()) ? true : false;
}