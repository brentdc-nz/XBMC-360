#include "GUISettings.h"
#include "utils\log.h"

using namespace std;

CSettingString::CSettingString(const char *strSetting, const char *strData)
    : CSetting(strSetting)
{
	m_strData = strData;
}

CGUISettings g_guiSettings;

// Settings are case sensitive
CGUISettings::CGUISettings()
{
	// appearance settings
	AddString(/*1,*/ "LookAndFeel.Skin"/*,166*/,"Project Mayhem III"/*, SPIN_CONTROL_TEXT*/); //TODO - Add commented stuff when settings window is added
}

CGUISettings::~CGUISettings()
{
}

void CGUISettings::AddString(const char *strSetting, const char *strData)
{
	CSettingString* pSetting = new CSettingString(strSetting, strData);

	if (!pSetting) return;
	m_settingsMap.insert(pair<CStdString, CSetting*>(strSetting, pSetting));
}

CStdString CGUISettings::GetString(const char *strSetting)
{
	ASSERT(m_settingsMap.size());
	mapIter it = m_settingsMap.find(strSetting);
	if (it != m_settingsMap.end())
	{
		return ((CSettingString *)(*it).second)->GetData();
	}

	// Assert here and write debug output
	CLog::Log(LOGERROR, "Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
	return "";
}

void CGUISettings::Clear()
{
	for (mapIter it = m_settingsMap.begin(); it != m_settingsMap.end(); it++)
		delete (*it).second;
	
	m_settingsMap.clear();
}