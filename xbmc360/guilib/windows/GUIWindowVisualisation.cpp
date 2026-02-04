#include "GUIWindowVisualisation.h"
#include "guilib\GUIVisualisationControl.h"
#include "Application.h"
#include "guilib\GUIInfoManager.h"
#include "ButtonTranslator.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\GUIUserMessages.h"
#include "Settings.h"
#include "AdvancedSettings.h"

#define TRANSISTION_COUNT   50  // 1 Second
#define TRANSISTION_LENGTH 200  // 4 Seconds
#define START_FADE_LENGTH  100  // 2 Seconds on startup

#define CONTROL_VIS          2

CGUIWindowVisualisation::CGUIWindowVisualisation(void)
	: CGUIWindow(WINDOW_VISUALISATION, "MusicVisualisation.xml")
{
	m_dwInitTimer = 0;
	m_dwLockedTimer = 0;
	m_bShowPreset = false;
}

CGUIWindowVisualisation::~CGUIWindowVisualisation(void)
{
}

bool CGUIWindowVisualisation::OnAction(const CAction &action)
{
	switch (action.GetID())
	{
		case ACTION_SHOW_GUI:
		// Save the settings
		g_settings.Save();
		g_windowManager.PreviousWindow();
		return true;
		break;
	}
	return CGUIWindow::OnAction(action);
}

bool CGUIWindowVisualisation::OnMessage(CGUIMessage& message)// TODO
{
	return CGUIWindow::OnMessage(message);
}

void CGUIWindowVisualisation::FrameMove() // TODO
{
	g_application.ResetScreenSaver();

	CGUIWindow::FrameMove();
}

void CGUIWindowVisualisation::AllocResources(bool forceLoad) // TODO
{
	CGUIWindow::AllocResources(forceLoad);
}

void CGUIWindowVisualisation::FreeResources(bool forceUnload) // TODO
{
	CGUIWindow::FreeResources(forceUnload);
}

