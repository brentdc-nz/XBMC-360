#include "GUIDialogSubMenu.h"

CGUIDialogSubMenu::CGUIDialogSubMenu(int id, const CStdString &xmlFile)
	: CGUIDialog(id, xmlFile)
{
}

CGUIDialogSubMenu::~CGUIDialogSubMenu(void)
{
}

bool CGUIDialogSubMenu::OnMessage(CGUIMessage &message)
{
	if (message.GetMessage() == GUI_MSG_CLICKED)
	{
		// Someone has been clicked - deinit...
		CGUIDialog::OnMessage(message);
		Close();
		return true;
	}
	return CGUIDialog::OnMessage(message);
}
