#include "GUIDialogButtonMenu.h"
#include "..\GUILabelControl.h"
#include "..\GUIButtonControl.h"

#define CONTROL_BUTTON_LABEL  3100

CGUIDialogButtonMenu::CGUIDialogButtonMenu(void)
    : CGUIDialog(WINDOW_DIALOG_BUTTON_MENU, "DialogButtonMenu.xml")
{
}

CGUIDialogButtonMenu::~CGUIDialogButtonMenu(void)
{
}

bool CGUIDialogButtonMenu::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_CLICKED:
		{
			// Someone has been clicked - deinit...
			Close();
		}
		break;
	}

	return CGUIWindow::OnMessage(message);
}

void CGUIDialogButtonMenu::Render()
{
	// Get the label control
	CGUILabelControl *pLabel = (CGUILabelControl *)GetControl(CONTROL_BUTTON_LABEL);
	if (pLabel)
	{
		// Get the active window, and put it's label into the label control
		int iControl = GetFocusedControlID();
		const CGUIControl *pControl = GetControl(iControl);
		if (pControl && pControl->GetControlType() == CGUIControl::GUICONTROL_BUTTON)
		{
			CGUIButtonControl *pButton = (CGUIButtonControl *)pControl;
			pLabel->SetLabel(pButton->GetLabel());
		}
	}
	CGUIDialog::Render();
}