#include "GUISettings.h"
#include "utils\Log.h"
#include "utils\StringUtils.h"

// String id's of the masks
#define MASK_MINS		14044
#define MASK_SECS		14045
#define MASK_MS			14046
#define MASK_PERCENT	14047
#define MASK_KBPS		14048
#define MASK_KB			14049
#define MASK_DB			14050

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

void CSettingString::FromString(const CStdString &strValue)
{
	m_strData = strValue;
}

CStdString CSettingString::ToString()
{
	return m_strData;
}

CSettingInt::CSettingInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType)
    : CSetting(iOrder, strSetting, iLabel, iControlType)
{
	m_iData = iData;
	m_iMin = iMin;
	m_iMax = iMax;
	m_iStep = iStep;
	m_iFormat = -1;
	m_iLabelMin = -1;
	m_strFormat = "%i";

	g_guiSettings.m_LookAndFeelResolution = HDTV_720p;
}

void CSettingInt::FromString(const CStdString &strValue)
{
	SetData(atoi(strValue.c_str()));
}

CStdString CSettingInt::ToString()
{
	CStdString strValue;
	strValue.Format("%i", m_iData);
	return strValue;
}

CGUISettings g_guiSettings;

// Settings are case sensitive
CGUISettings::CGUISettings()
{
	// Appearance settings
	AddGroup(7, 480);
	AddCategory(7,"LookAndFeel", 14037);
	AddString(1, "LookAndFeel.Skin", 166, DEFAULT_SKIN, SPIN_CONTROL_TEXT);
	AddString(2, "LookAndFeel.Language", 248, "English", SPIN_CONTROL_TEXT);
	AddInt(5, "lookandfeel.skinzoom", 20109, 0, -20, 2, 20, SPIN_CONTROL_INT);

	AddCategory(7, "ScreenSaver", 360);
	AddString(1, "ScreenSaver.Mode", 356, "Dim", SPIN_CONTROL_TEXT);
	AddInt(2, "ScreenSaver.Time", 355, 3, 1, 1, 60, SPIN_CONTROL_INT_PLUS); //TODO

	AddCategory(7, "videoscreen", 131);
	AddInt(1, "videoscreen.resolution",169,(int)AUTORES, (int)HDTV_720p, 1, (int)AUTORES, SPIN_CONTROL_TEXT);
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

void CGUISettings::AddInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType)
{
	CSettingInt* pSetting = new CSettingInt(iOrder, strSetting, iLabel, iData, iMin, iStep, iMax, iControlType);

	if (!pSetting) return ;
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

int CGUISettings::GetInt(const char *strSetting)
{
	ASSERT(m_settingsMap.size());
	mapIter it = m_settingsMap.find(strSetting);
	if (it != m_settingsMap.end())
	{
		return ((CSettingInt *)(*it).second)->GetData();
	}
	
	// Assert here and write debug output
	CLog::DebugLog("Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
	return 0;
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

void CGUISettings::LoadXML(TiXmlElement *pRootElement)
{
	//Load our ssettings...
	for (mapIter it = m_settingsMap.begin(); it != m_settingsMap.end(); it++)
	{
		CStdStringArray strSplit;
		CStringUtils::SplitString((*it).first, ".", strSplit);
		if (strSplit.size() > 1)
		{
			const TiXmlNode *pChild = pRootElement->FirstChild(strSplit[0].c_str());
			if (pChild)
			{
				const TiXmlNode *pGrandChild = pChild->FirstChild(strSplit[1].c_str());
				if (pGrandChild && pGrandChild->FirstChild())
				{
					CStdString strValue = pGrandChild->FirstChild()->Value();
					if (strValue.size() )
					{
						if (strValue != "-")
						{ 
							// Update our item
							(*it).second->FromString(strValue);
							CLog::Log(LOGDEBUG, "  %s: %s", (*it).first.c_str(), (*it).second->ToString().c_str());
						}
					}
				}
			}
		}
	}

//	g_guiSettings.m_LookAndFeelResolution = (RESOLUTION)GetInt("videoscreen.resolution");
}

void CGUISettings::SaveXML(TiXmlNode *pRootNode)
{
	for (mapIter it = m_settingsMap.begin(); it != m_settingsMap.end(); it++)
	{
		CStdStringArray strSplit;
		CStringUtils::SplitString((*it).first, ".", strSplit);
		
		if (strSplit.size() > 1)
		{
			TiXmlNode *pChild = pRootNode->FirstChild(strSplit[0].c_str());
			if (!pChild)
			{
				// add our group tag
				TiXmlElement newElement(strSplit[0].c_str());
				pChild = pRootNode->InsertEndChild(newElement);
			}

			if (pChild)
			{
				// successfully added (or found) our group
				TiXmlElement newElement(strSplit[1]);
				TiXmlNode *pNewNode = pChild->InsertEndChild(newElement);
				if (pNewNode)
				{
					TiXmlText value((*it).second->ToString());
					pNewNode->InsertEndChild(value);
				}
			}
		}
	}
}

void CGUISettings::Clear()
{
	for (mapIter it = m_settingsMap.begin(); it != m_settingsMap.end(); it++)
		delete (*it).second;
	
	m_settingsMap.clear();
}