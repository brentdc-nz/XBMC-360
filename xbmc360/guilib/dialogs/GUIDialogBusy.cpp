#include "GUIDialogBusy.h"
#include "ApplicationRenderer.h"

CGUIDialogBusy::CGUIDialogBusy(void)
: CGUIDialog(WINDOW_DIALOG_BUSY, "DialogBusy.xml")
{
	m_loadOnDemand = true;
}

CGUIDialogBusy::~CGUIDialogBusy(void)
{
}

bool CGUIDialogBusy::OnMessage(CGUIMessage& message)
{
	switch(message.GetMessage())
	{
		case GUI_MSG_WINDOW_INIT:
		{
			CGUIDialog::OnMessage(message);
			return true;
		}
		break;

		case GUI_MSG_WINDOW_DEINIT:
		{
		}
		break;
	}
	return CGUIDialog::OnMessage(message);
}

void CGUIDialogBusy::OnWindowLoaded()
{
	CGUIDialog::OnWindowLoaded();
}

void CGUIDialogBusy::Render()
{
	// Only render if system is busy
	if(g_ApplicationRenderer.IsBusy() || IsAnimating(ANIM_TYPE_WINDOW_CLOSE))
	{
		CGUIDialog::Render();
	}
}
