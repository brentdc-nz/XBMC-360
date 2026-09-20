#include "GUIWindowSettings.h"
#include "guilib\GUIUserMessages.h"
#include "guilib\GUIWindowManager.h"

CGUIWindowSettings::CGUIWindowSettings(void) : CGUIWindow(WINDOW_SETTINGS_MENU, "Settings.xml")
{
}

CGUIWindowSettings::~CGUIWindowSettings(void)
{
}

bool CGUIWindowSettings::OnMessage(CGUIMessage& message)
{
	// TODO

	return CGUIWindow::OnMessage(message);
}