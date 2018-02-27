#include "GUIDialogButtonMenu.h"

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
			Close();
		}
		break;
	}

	return CGUIWindow::OnMessage(message);
}