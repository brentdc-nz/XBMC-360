#include "GUISettings.h"
#include "utils\Log.h"
#include "utils\StringUtils.h"
#include "MediaManager.h"
#include "guilib\LocalizeStrings.h"
#include "xbox\XBVideoConfig.h"
#include "xbox\XBTimeZone.h"

using namespace std;

// String id's of the masks
#define MASK_MINS     14044
#define MASK_SECS     14045
#define MASK_MS       14046
#define MASK_PERCENT  14047
#define MASK_KBPS     14048
#define MASK_KB       14049
#define MASK_DB       14050

#define TEXT_OFF 351

class CGUISettings g_guiSettings;

struct sortsettings
{
	bool operator()(const CSetting* pSetting1, const CSetting* pSetting2)
	{
		return pSetting1->GetOrder() < pSetting2->GetOrder();
	}
};

void CSettingBool::FromString(const CStdString &strValue)
{
	m_bData = (strValue == "true");
}

CStdString CSettingBool::ToString()
{
	return m_bData ? "true" : "false";
}

CSettingSeparator::CSettingSeparator(int iOrder, const char *strSetting)
	: CSetting(iOrder, strSetting, 0, SEPARATOR_CONTROL)
{
}

CSettingFloat::CSettingFloat(int iOrder, const char *strSetting, int iLabel, float fData, float fMin, float fStep, float fMax, int iControlType)
    : CSetting(iOrder, strSetting, iLabel, iControlType)
{
	m_fData = fData;
	m_fMin = fMin;
	m_fStep = fStep;
	m_fMax = fMax;
}

void CSettingFloat::FromString(const CStdString &strValue)
{
	SetData((float)atof(strValue.c_str()));
}

CStdString CSettingFloat::ToString()
{
	CStdString strValue;
	strValue.Format("%f", m_fData);

	return strValue;
}

CSettingInt::CSettingInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType, const char *strFormat)
	: CSetting(iOrder, strSetting, iLabel, iControlType)
{
	m_iData = iData;
	m_iMin = iMin;
	m_iMax = iMax;
	m_iStep = iStep;
	m_iFormat = -1;
	m_iLabelMin = -1;
	
	if (strFormat)
		m_strFormat = strFormat;
	else
		m_strFormat = "%i";
}

CSettingInt::CSettingInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType, int iFormat, int iLabelMin)
    : CSetting(iOrder, strSetting, iLabel, iControlType)
{
	m_iData = iData;
	m_iMin = iMin;
	m_iMax = iMax;
	m_iStep = iStep;
	m_iLabelMin = iLabelMin;
	m_iFormat = iFormat;
	
	if (m_iFormat < 0)
		m_strFormat = "%i";
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

void CSettingHex::FromString(const CStdString &strValue)
{
	int iHexValue;
	if (sscanf(strValue, "%x", (unsigned int *)&iHexValue))
		SetData(iHexValue);
}

CStdString CSettingHex::ToString()
{
	CStdString strValue;
	strValue.Format("%x", m_iData);

	return strValue;
}

CSettingString::CSettingString(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType, bool bAllowEmpty, int iHeadingString)
    : CSetting(iOrder, strSetting, iLabel, iControlType)
{
	m_strData = strData;
	m_bAllowEmpty = bAllowEmpty;
	m_iHeadingString = iHeadingString;
}

void CSettingString::FromString(const CStdString &strValue)
{
	m_strData = strValue;
}

CStdString CSettingString::ToString()
{
	return m_strData;
}

CSettingPath::CSettingPath(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType, bool bAllowEmpty, int iHeadingString)
	: CSettingString(iOrder, strSetting, iLabel, strData, iControlType, bAllowEmpty, iHeadingString)
{
}

void CSettingsGroup::GetCategories(vecSettingsCategory &vecCategories)
{
	vecCategories.clear();
	
	for (unsigned int i = 0; i < m_vecCategories.size(); i++)
	{
		vecSettings settings;

		// Check whether we actually have these settings available.
		g_guiSettings.GetSettingsGroup(m_vecCategories[i]->m_strCategory, settings);
		
		if (settings.size())
			vecCategories.push_back(m_vecCategories[i]);
	}
}

// Settings are case sensitive
CGUISettings::CGUISettings(void)
{
}

CGUISettings::~CGUISettings(void)
{
	Clear();
}

void CGUISettings::Initialize()
{

	m_LookAndFeelResolution = HDTV_720p; //FIXME AND REMOVE!!!

	// My Weather settings
	AddGroup(2, 8);
	AddCategory(2, "weather", 16000);
	AddString(1, "weather.location1", 14019, "40.71:-74.01~New York, United States of America", BUTTON_CONTROL_STANDARD);
	AddString(2, "weather.location2", 14020, "51.52:-0.11~London, United Kingdom", BUTTON_CONTROL_STANDARD);
	AddString(3, "weather.location3", 14021, "35.69:139.69~Tokyo, Japan", BUTTON_CONTROL_STANDARD);

	// System settings
	AddGroup(4, 13000);

	AddCategory(4, "videooutput", 21373);
	AddInt(1, "videooutput.aspect", 21374, VIDEO_NORMAL, VIDEO_NORMAL, 1, VIDEO_WIDESCREEN, SPIN_CONTROL_TEXT);

	// Appearance settings
	AddGroup(7, 480);
	AddCategory(7,"lookandfeel", 166);
	AddString(1, "lookandfeel.skin",166,DEFAULT_SKIN, SPIN_CONTROL_TEXT);
	AddInt(5, "lookandfeel.skinzoom",20109, 0, -20, 2, 20, SPIN_CONTROL_INT, MASK_PERCENT);

	AddCategory(7, "locale", 14090);
	AddString(1, "locale.language",248,"english", SPIN_CONTROL_TEXT);
	AddString(2, "locale.country", 20026, "UK (24h)", SPIN_CONTROL_TEXT);
	AddInt(7, "locale.timezone", 14074, 0, 0, 1, g_timezone.GetNumberOfTimeZones(), SPIN_CONTROL_TEXT);

	AddCategory(7, "videoscreen", 131);
	AddInt(1, "videoscreen.resolution",169,(int)AUTORES, (int)HDTV_1080p, 1, (int)AUTORES, SPIN_CONTROL_TEXT);

	AddCategory(7, "screensaver", 360);
	AddInt(1, "screensaver.time", 355, 3, 1, 1, 60, SPIN_CONTROL_INT_PLUS, MASK_MINS);
	AddString(2, "screensaver.mode", 356, "Plasma", SPIN_CONTROL_TEXT);
}

void CGUISettings::AddGroup(int groupID, int labelID)
{
	CSettingsGroup *pGroup = new CSettingsGroup(groupID, labelID);
	if (pGroup)
		m_SettingsGroups.push_back(pGroup);
}

void CGUISettings::AddCategory(int groupID, const char *strSetting, int labelID)
{
	for (unsigned int i = 0; i < m_SettingsGroups.size(); i++)
	{
		if (m_SettingsGroups[i]->GetGroupID() == groupID)
			m_SettingsGroups[i]->AddCategory(CStdString(strSetting).ToLower(), labelID);
	}
}

CSettingsGroup *CGUISettings::GetGroup(int groupID)
{
	for (unsigned int i = 0; i < m_SettingsGroups.size(); i++)
	{
		if (m_SettingsGroups[i]->GetGroupID() == groupID)
			return m_SettingsGroups[i];
	}
	
	CLog::Log(LOGDEBUG, "Error: Requested setting group (%i) was not found.  "
				"It must be case-sensitive", groupID);
            
	return NULL;
}

void CGUISettings::AddSeparator(int iOrder, const char *strSetting)
{
	CSettingSeparator *pSetting = new CSettingSeparator(iOrder, CStdString(strSetting).ToLower());

	if (!pSetting) return;
		m_SettingsMap.insert(pair<CStdString, CSetting*>(CStdString(strSetting).ToLower(), pSetting));
}

void CGUISettings::AddBool(int iOrder, const char *strSetting, int iLabel, bool bData, int iControlType)
{
	CSettingBool* pSetting = new CSettingBool(iOrder, CStdString(strSetting).ToLower(), iLabel, bData, iControlType);

	if (!pSetting) return;
		m_SettingsMap.insert(pair<CStdString, CSetting*>(CStdString(strSetting).ToLower(), pSetting));
}

bool CGUISettings::GetBool(const char *strSetting) const
{
	ASSERT(m_SettingsMap.size());
	CStdString lower(strSetting);
	lower.ToLower();
	constMapIter it = m_SettingsMap.find(lower);
	
	if (it != m_SettingsMap.end())
	{
		// Old category
		return ((CSettingBool*)(*it).second)->GetData();
	}
	
	// Forward compatibility for new skins (skins use this setting)
// TODO
/*	if (lower == "input.enablemouse")
		return GetBool("lookandfeel.enablemouse");
*/
	// Assert here and write debug output
	CLog::Log(LOGDEBUG,"Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
	return false;
}

void CGUISettings::SetBool(const char *strSetting, bool bSetting)
{
	ASSERT(m_SettingsMap.size());
	mapIter it = m_SettingsMap.find(CStdString(strSetting).ToLower());
	
	if (it != m_SettingsMap.end())
	{
		// Old category
		((CSettingBool*)(*it).second)->SetData(bSetting);
		return;
	}

	// Assert here and write debug output
	CLog::Log(LOGDEBUG,"Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
}

void CGUISettings::ToggleBool(const char *strSetting)
{
	ASSERT(m_SettingsMap.size());
	mapIter it = m_SettingsMap.find(CStdString(strSetting).ToLower());

	if (it != m_SettingsMap.end())
	{
		// Old category
		((CSettingBool*)(*it).second)->SetData(!((CSettingBool *)(*it).second)->GetData());
		return;
	}

	// Assert here and write debug output
	CLog::Log(LOGDEBUG,"Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
}

void CGUISettings::AddFloat(int iOrder, const char *strSetting, int iLabel, float fData, float fMin, float fStep, float fMax, int iControlType)
{
	CSettingFloat* pSetting = new CSettingFloat(iOrder, CStdString(strSetting).ToLower(), iLabel, fData, fMin, fStep, fMax, iControlType);

	if (!pSetting) return ;
		m_SettingsMap.insert(pair<CStdString, CSetting*>(CStdString(strSetting).ToLower(), pSetting));
}

float CGUISettings::GetFloat(const char *strSetting) const
{
	ASSERT(m_SettingsMap.size());
	constMapIter it = m_SettingsMap.find(CStdString(strSetting).ToLower());

	if (it != m_SettingsMap.end())
		return ((CSettingFloat *)(*it).second)->GetData();

	// Assert here and write debug output
	CLog::Log(LOGDEBUG,"Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
	return 0.0f;
}

void CGUISettings::SetFloat(const char *strSetting, float fSetting)
{
	ASSERT(m_SettingsMap.size());
	mapIter it = m_SettingsMap.find(CStdString(strSetting).ToLower());
	
	if (it != m_SettingsMap.end())
	{
		((CSettingFloat *)(*it).second)->SetData(fSetting);
		return;
	}

	// Assert here and write debug output
	ASSERT(false);
	CLog::Log(LOGDEBUG,"Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
}

void CGUISettings::AddInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType, const char *strFormat)
{
	CSettingInt* pSetting = new CSettingInt(iOrder, CStdString(strSetting).ToLower(), iLabel, iData, iMin, iStep, iMax, iControlType, strFormat);

	if (!pSetting) return;
		m_SettingsMap.insert(pair<CStdString, CSetting*>(CStdString(strSetting).ToLower(), pSetting));
}

void CGUISettings::AddInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType, int iFormat, int iLabelMin/*=-1*/)
{
	CSettingInt* pSetting = new CSettingInt(iOrder, CStdString(strSetting).ToLower(), iLabel, iData, iMin, iStep, iMax, iControlType, iFormat, iLabelMin);

	if (!pSetting) return;
		m_SettingsMap.insert(pair<CStdString, CSetting*>(CStdString(strSetting).ToLower(), pSetting));
}

void CGUISettings::AddHex(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType, const char *strFormat)
{
	CSettingHex* pSetting = new CSettingHex(iOrder, CStdString(strSetting).ToLower(), iLabel, iData, iMin, iStep, iMax, iControlType, strFormat);

	if (!pSetting) return;
		m_SettingsMap.insert(pair<CStdString, CSetting*>(CStdString(strSetting).ToLower(), pSetting));
}

int CGUISettings::GetInt(const char *strSetting) const
{
	ASSERT(m_SettingsMap.size());
	constMapIter it = m_SettingsMap.find(CStdString(strSetting).ToLower());

	if (it != m_SettingsMap.end())
		return ((CSettingInt *)(*it).second)->GetData();

	// Assert here and write debug output
	CLog::Log(LOGERROR,"Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
	return 0;
}

void CGUISettings::SetInt(const char *strSetting, int iSetting)
{
	ASSERT(m_SettingsMap.size());
	mapIter it = m_SettingsMap.find(CStdString(strSetting).ToLower());
	
	if (it != m_SettingsMap.end())
	{
		((CSettingInt *)(*it).second)->SetData(iSetting);
			if (stricmp(strSetting, "videoscreen.resolution") == 0)
				g_guiSettings.m_LookAndFeelResolution = (RESOLUTION)iSetting;
		return;
	}

	// Assert here and write debug output
	ASSERT(false);
}

void CGUISettings::AddString(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType, bool bAllowEmpty, int iHeadingString)
{
	CSettingString* pSetting = new CSettingString(iOrder, CStdString(strSetting).ToLower(), iLabel, strData, iControlType, bAllowEmpty, iHeadingString);

	if (!pSetting) return ;
		m_SettingsMap.insert(pair<CStdString, CSetting*>(CStdString(strSetting).ToLower(), pSetting));
}

void CGUISettings::AddPath(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType, bool bAllowEmpty, int iHeadingString)
{
	CSettingPath* pSetting = new CSettingPath(iOrder, CStdString(strSetting).ToLower(), iLabel, strData, iControlType, bAllowEmpty, iHeadingString);

	if (!pSetting) return ;
		m_SettingsMap.insert(pair<CStdString, CSetting*>(CStdString(strSetting).ToLower(), pSetting));
}

const CStdString &CGUISettings::GetString(const char *strSetting, bool bPrompt) const
{
	ASSERT(m_SettingsMap.size());
	constMapIter it = m_SettingsMap.find(CStdString(strSetting).ToLower());
	
	if (it != m_SettingsMap.end())
	{
		CSettingString* result = ((CSettingString *)(*it).second);
		if (result->GetData() == "select folder" || result->GetData() == "select writable folder")
		{
			CStdString strData = "";
			if (bPrompt)
			{
// TODO: Need CGUIDialogFileBrowser
/*				VECSOURCES shares;
				g_mediaManager.GetLocalDrives(shares);
				
				if (CGUIDialogFileBrowser::ShowAndGetDirectory(shares,g_localizeStrings.Get(result->GetLabel()),strData,result->GetData() == "select writable folder"))
				{
					result->SetData(strData);
					g_settings.Save();
				}
				else
					return CStringUtils::EmptyString;
*/			}
			else
				return CStringUtils::EmptyString;
		}
		return result->GetData();
	}

	// Assert here and write debug output
	CLog::Log(LOGDEBUG,"Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);

	// hardcoded return value so that compiler is happy
	return CStringUtils::EmptyString;
}

void CGUISettings::SetString(const char *strSetting, const char *strData)
{
	ASSERT(m_SettingsMap.size());
	mapIter it = m_SettingsMap.find(CStdString(strSetting).ToLower());

	if (it != m_SettingsMap.end())
	{
		((CSettingString *)(*it).second)->SetData(strData);
		return;
	}

	// Assert here and write debug output
	ASSERT(false);
	CLog::Log(LOGDEBUG,"Error: Requested setting (%s) was not found.  It must be case-sensitive", strSetting);
}

CSetting *CGUISettings::GetSetting(const char *strSetting)
{
	ASSERT(m_SettingsMap.size());
	mapIter it = m_SettingsMap.find(CStdString(strSetting).ToLower());
	
	if (it != m_SettingsMap.end())
		return (*it).second;
	else
		return NULL;
}

// Get all the settings beginning with the term "strGroup"
void CGUISettings::GetSettingsGroup(const char *strGroup, vecSettings &settings)
{
	vecSettings unorderedSettings;

	for (mapIter it = m_SettingsMap.begin(); it != m_SettingsMap.end(); it++)
	{
		if ((*it).first.Left(strlen(strGroup)).Equals(strGroup) && (*it).second->GetOrder() > 0 && !(*it).second->IsAdvanced())
			unorderedSettings.push_back((*it).second);
	}

	// Now order them...
	sort(unorderedSettings.begin(), unorderedSettings.end(), sortsettings());

	// Remove any instances of 2 separators in a row
	bool lastWasSeparator(false);
	
	for (vecSettings::iterator it = unorderedSettings.begin(); it != unorderedSettings.end(); it++)
	{
		CSetting *setting = *it;
		
		// Only add separators if we don't have 2 in a row
		if (setting->GetType() == SETTINGS_TYPE_SEPARATOR)
		{
			if (!lastWasSeparator)
				settings.push_back(setting);
			
			lastWasSeparator = true;
		}
		else
		{
			lastWasSeparator = false;
			settings.push_back(setting);
		}
	}
}

void CGUISettings::LoadXML(TiXmlElement *pRootElement, bool hideSettings /* = false */)
{
	// Load our stuff...
	for (mapIter it = m_SettingsMap.begin(); it != m_SettingsMap.end(); it++)
	{
		LoadFromXML(pRootElement, it, hideSettings);
	}
	
	// Get hardware based stuff...
	CLog::Log(LOGNOTICE, "Getting hardware information now...");

	if (g_videoConfig.HasLetterbox())
		SetInt("videooutput.aspect", VIDEO_LETTERBOX);
	else if (g_videoConfig.HasWidescreen())
		SetInt("videooutput.aspect", VIDEO_WIDESCREEN);
	else
		SetInt("videooutput.aspect", VIDEO_NORMAL);

	SetBool("videooutput.hd480p", g_videoConfig.Has480p());
	SetBool("videooutput.hd720p", g_videoConfig.Has720p());

	SetInt("locale.timezone", g_timezone.GetTimeZoneIndex());
//	SetBool("locale.usedst", g_timezone.GetDST()); // TODO

	g_guiSettings.m_LookAndFeelResolution = (RESOLUTION)GetInt("videoscreen.resolution");
	CLog::Log(LOGNOTICE, "Checking resolution %i", g_guiSettings.m_LookAndFeelResolution);

//	g_videoConfig.PrintInfo(); // TODO

	if( (g_guiSettings.m_LookAndFeelResolution == AUTORES) ||
    (!g_videoConfig.IsValidResolution(g_guiSettings.m_LookAndFeelResolution)))
	{
		RESOLUTION newRes = g_videoConfig.GetBestMode();

		if (g_guiSettings.m_LookAndFeelResolution == AUTORES)
		{
			// "videoscreen.resolution" will stay at AUTORES, m_LookAndFeelResolution will be the real mode
			CLog::Log(LOGNOTICE, "Setting autoresolution mode %i", newRes);
			g_guiSettings.m_LookAndFeelResolution = newRes;
		}
		else
		{
			CLog::Log(LOGNOTICE, "Setting safe mode %i", newRes);
			SetInt("videoscreen.resolution", newRes);
		}
	}
}

void CGUISettings::LoadFromXML(TiXmlElement *pRootElement, mapIter &it, bool advanced /* = false */)
{
	CStdStringArray strSplit;
	CStringUtils::SplitString((*it).first, ".", strSplit);

	if (strSplit.size() > 1)
	{
		const TiXmlNode *pChild = pRootElement->FirstChild(strSplit[0].c_str());
		if (pChild)
		{
			const TiXmlElement *pGrandChild = pChild->FirstChildElement(strSplit[1].c_str());
			
			if (pGrandChild && pGrandChild->FirstChild())
			{
				CStdString strValue = pGrandChild->FirstChild()->Value();
				if (strValue.size())
				{
					if (strValue != "-")
					{
						// Update our item
						if ((*it).second->GetType() == SETTINGS_TYPE_PATH)
						{
							// Check our path
							int pathVersion = 0;
							pGrandChild->Attribute("pathversion", &pathVersion);
//							strValue = CSpecialProtocol::ReplaceOldPath(strValue, pathVersion); // TODO: Not required I think?
						}
						(*it).second->FromString(strValue);

						if (advanced)
							(*it).second->SetAdvanced();
					}
				}
			}
		}
	}
}

void CGUISettings::SaveXML(TiXmlNode *pRootNode)
{
	for (mapIter it = m_SettingsMap.begin(); it != m_SettingsMap.end(); it++)
	{
		// Don't save advanced settings
		CStdString first = (*it).first;
		if ((*it).second->IsAdvanced())
			continue;

		CStdStringArray strSplit;
		CStringUtils::SplitString((*it).first, ".", strSplit);
		
		if (strSplit.size() > 1)
		{
			TiXmlNode *pChild = pRootNode->FirstChild(strSplit[0].c_str());
			if (!pChild)
			{
				// Add our group tag
				TiXmlElement newElement(strSplit[0].c_str());
				pChild = pRootNode->InsertEndChild(newElement);
			}

			if (pChild)
			{
				// Successfully added (or found) our group
				TiXmlElement newElement(strSplit[1]);
				
				if ((*it).second->GetType() == SETTINGS_TYPE_PATH)
					newElement.SetAttribute("pathversion", /*CSpecialProtocol::path_version*/1); // TODO: Not required I think?
				
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
	for (mapIter it = m_SettingsMap.begin(); it != m_SettingsMap.end(); it++)
		delete (*it).second;
	
	m_SettingsMap.clear();
	
	for (unsigned int i = 0; i < m_SettingsGroups.size(); i++)
		delete m_SettingsGroups[i];
	
	m_SettingsGroups.clear();
}