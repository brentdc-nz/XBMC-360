#include "GUISettings.h"
#include "utils\log.h"

using namespace std;

struct sortsettings
{
	bool operator()(const CSetting* pSetting1, const CSetting* pSetting2)
	{
		return pSetting1->GetOrder() < pSetting2->GetOrder();
	}
};

CSettingString::CSettingString(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType)
    : CSetting(iOrder, strSetting, iLabel, iControlType)
{
	m_strData = strData;
}

CGUISettings g_guiSettings;

// Settings are case sensitive
CGUISettings::CGUISettings()
{
	// appearance settings
	AddGroup(7, 480);
	AddCategory(7,"LookAndFeel", 14037);
	AddString(1, "LookAndFeel.Skin", 166, "Project Mayhem III", SPIN_CONTROL_TEXT);
}

CGUISettings::~CGUISettings()
{
}

CSetting *CGUISettings::GetSetting(const char *strSetting)
{
	ASSERT(m_settingsMap.size());
	mapIter it = m_settingsMap.find(strSetting);
	if (it !=  m_settingsMap.end())
		return (*it).second;
	else
		return NULL;
}

void CGUISettings::AddGroup(DWORD dwGroupID, DWORD dwLabelID)
{
	CSettingsGroup *pGroup = new CSettingsGroup(dwGroupID, dwLabelID);

	if (pGroup)
		m_settingsGroups.push_back(pGroup);
}

void CGUISettings::AddCategory(DWORD dwGroupID, const char *strSetting, DWORD dwLabelID)
{
	for (unsigned int i = 0; i < m_settingsGroups.size(); i++)
	{
		if (m_settingsGroups[i]->GetGroupID() == dwGroupID)
			m_settingsGroups[i]->AddCategory(strSetting, dwLabelID);
	}
}

CSettingsGroup *CGUISettings::GetGroup(DWORD dwGroupID)
{
	for (unsigned int i = 0; i < m_settingsGroups.size(); i++)
	{
		if (m_settingsGroups[i]->GetGroupID() == dwGroupID)
			return m_settingsGroups[i];
	}

	CLog::DebugLog("Error: Requested setting group (%i) was not found.  It must be case-sensitive", dwGroupID);
	return NULL;
}

void CGUISettings::AddString(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType)
{
	CSettingString* pSetting = new CSettingString(iOrder, strSetting, iLabel, strData, iControlType);

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

void CGUISettings::SetString(const char *strSetting, const char *strData)
{
	ASSERT(m_settingsMap.size());
	mapIter it = m_settingsMap.find(strSetting);
	if (it != m_settingsMap.end())
	{
		((CSettingString *)(*it).second)->SetData(strData);
		return;
	}
	// Assert here and write debug output
	CLog::DebugLog("Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
}

// get all the settings beginning with the term "strGroup"
void CGUISettings::GetSettingsGroup(const char *strGroup, vecSettings &settings)
{
	settings.clear();
	for (mapIter it = m_settingsMap.begin(); it != m_settingsMap.end(); it++)
	{
		if ((*it).first.Left(strlen(strGroup)) == strGroup && (*it).second->GetOrder() >= 0)
			settings.push_back((*it).second);
	}
	// now order them...
	sort(settings.begin(), settings.end(), sortsettings());
}

void CGUISettings::Clear()
{
	for (mapIter it = m_settingsMap.begin(); it != m_settingsMap.end(); it++)
		delete (*it).second;
	
	m_settingsMap.clear();
}