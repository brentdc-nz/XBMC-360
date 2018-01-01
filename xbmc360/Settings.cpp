#include "Settings.h"
#include "Application.h"
#include "utils\Log.h"

CSettings g_settings;

CSettings::CSettings()
{
}

CSettings::~CSettings()
{
}

void CSettings::Save() const
{
	if (g_application.IsStoppoing())
	{
		//don't save settings when we're busy stopping the application
		//a lot of screens try to save settings on deinit and deinit is called
		//for every screen when the application is stopping.
		return ;
	}
	if (!SaveSettings("T:\\settings.xml"))
	{
		CLog::Log(LOGERROR, "Unable to save settings to T:\\settings.xml");
	}
}

bool CSettings::SaveSettings(const CStdString& strSettingsFile) const
{
	return true;
}