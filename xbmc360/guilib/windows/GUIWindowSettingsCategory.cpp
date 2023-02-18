#include "GUIWindowSettingsCategory.h"
#include "guilib\GUIControlGroupList.h"

#define CONTROL_GROUP_BUTTONS           0
#define CONTROL_GROUP_SETTINGS          1
#define CONTROL_SETTINGS_LABEL          2
#define CATEGORY_GROUP_ID               3
#define SETTINGS_GROUP_ID               5
#define CONTROL_DEFAULT_BUTTON          7
#define CONTROL_DEFAULT_RADIOBUTTON     8
#define CONTROL_DEFAULT_SPIN            9
#define CONTROL_DEFAULT_CATEGORY_BUTTON 10
#define CONTROL_DEFAULT_SEPARATOR       11
#define CONTROL_DEFAULT_EDIT            12
#define CONTROL_START_BUTTONS           -40
#define CONTROL_START_CONTROL           -20

CGUIWindowSettingsCategory::CGUIWindowSettingsCategory(void)
	: CGUIWindow(WINDOW_SETTINGS_MYPICTURES, "SettingsCategory.xml")
{
	// Set the correct ID range...
	m_idRange = 8;
}

CGUIWindowSettingsCategory::~CGUIWindowSettingsCategory(void)
{
}

bool CGUIWindowSettingsCategory::OnBack(int actionID)
{
	g_settings.Save();
	m_lastControlID = 0; // Don't save the control as we go to a different window each time
	
	return CGUIWindow::OnBack(actionID);
}

bool CGUIWindowSettingsCategory::OnMessage(CGUIMessage &message)
{
	// TODO

	return CGUIWindow::OnMessage(message);
}

void CGUIWindowSettingsCategory::SetupControls()
{
	// Cleanup first, if necessary
	FreeControls();
}

void CGUIWindowSettingsCategory::FreeControls()
{
	// Clear the category group
	CGUIControlGroupList *control = (CGUIControlGroupList *)GetControl(CATEGORY_GROUP_ID);
	
	if (control)
	{
		control->FreeResources();
		control->ClearAll();
	}
	
//	m_vecSections.clear();
//	FreeSettingsControls();
}

void CGUIWindowSettingsCategory::OnInitWindow()
{
/*	m_iNetworkAssignment = g_guiSettings.GetInt("network.assignment");
	m_strNetworkIPAddress = g_guiSettings.GetString("network.ipaddress");
	m_strNetworkSubnet = g_guiSettings.GetString("network.subnet");
	m_strNetworkGateway = g_guiSettings.GetString("network.gateway");
	m_strNetworkDNS = g_guiSettings.GetString("network.dns");
	m_strOldTrackFormat = g_guiSettings.GetString("musicfiles.trackformat");
	m_strOldTrackFormatRight = g_guiSettings.GetString("musicfiles.trackformatright");
	m_NewResolution = INVALID;
*/
	SetupControls();
	
	CGUIWindow::OnInitWindow();
}
