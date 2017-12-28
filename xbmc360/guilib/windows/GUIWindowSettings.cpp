#include "GUIWindowSettings.h"
#include "..\GUIUserMessage.h"
#include "..\..\Application.h"
#include "..\GUIWindowManager.h"

CGUIWindowSettimgs::CGUIWindowSettimgs(void) : CGUIWindow(WINDOW_SETTINGS, "Settings.xml")
{
	m_loadOnDemand = false;
}

CGUIWindowSettimgs::~CGUIWindowSettimgs(void)
{
}

void CGUIWindowSettimgs::Render()
{

	CGUIWindow::Render();
}

bool CGUIWindowSettimgs::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_LOAD_SKIN:
		{
			// Reload the skin.  Save the current focused control, and refocus it
			// when done.
//			unsigned iCtrlID = GetFocusedControl();
//			CGUIMessage msg(GUI_MSG_ITEM_SELECTED, GetID(), iCtrlID, 0, 0, NULL);
//			g_graphicsContext.SendMessage(msg);
//			int iWindowID = GetID();
			g_application.LoadSkin(/*g_guiSettings.GetString("LookAndFeel.Skin")*/"PM3\\");
			g_windowManager.ActivateWindow(iWindowID);
//			SET_CONTROL_FOCUS(iCtrlID, 0);
//			CGUIMessage msgSelect(GUI_MSG_ITEM_SELECT, GetID(), iCtrlID, msg.GetParam1(), msg.GetParam2());
//			OnMessage(msgSelect);
			break;
		}
		break;
	}

	return CGUIWindow::OnMessage(message);
}