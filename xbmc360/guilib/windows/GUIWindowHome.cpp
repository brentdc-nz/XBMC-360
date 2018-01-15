#include "GUIWindowHome.h"

CGUIWindowHome::CGUIWindowHome(void) : CGUIWindow(WINDOW_HOME, "Home.xml")
{
	m_loadOnDemand = false;
}

CGUIWindowHome::~CGUIWindowHome(void)
{
}

void CGUIWindowHome::Render()
{

	CGUIWindow::Render();
}