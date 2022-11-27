#include "GUIDialogButtonMenu.h"
#include "guilib\GUILabelControl.h"
#include "guilib\GUIButtonControl.h"

#define CONTROL_BUTTON_LABEL  3100

CGUIDialogButtonMenu::CGUIDialogButtonMenu(int id, const CStdString &xmlFile)
: CGUIDialog(id, xmlFile)
{
}

CGUIDialogButtonMenu::~CGUIDialogButtonMenu(void)
{
}

bool CGUIDialogButtonMenu::OnMessage(CGUIMessage &message)
{
	bool bRet = CGUIDialog::OnMessage(message);
	if (message.GetMessage() == GUI_MSG_CLICKED)
	{
		// Someone has been clicked - Deinit...
		Close();
		return true;
	}
	return bRet;
}

void CGUIDialogButtonMenu::FrameMove()
{
	// Get the label control
	CGUILabelControl *pLabel = (CGUILabelControl *)GetControl(CONTROL_BUTTON_LABEL);
	
	if (pLabel)
	{
		// Get the active window, and put it's label into the label control
		const CGUIControl *pControl = GetFocusedControl();
		if (pControl && (pControl->GetControlType() == CGUIControl::GUICONTROL_BUTTON || pControl->GetControlType() == CGUIControl::GUICONTROL_TOGGLEBUTTON))
		{
			CGUIButtonControl *pButton = (CGUIButtonControl *)pControl;
			pLabel->SetLabel(pButton->GetLabel());
		}
	}

	CGUIDialog::FrameMove();
}