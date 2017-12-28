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
	return CGUIWindow::OnMessage(message);
}