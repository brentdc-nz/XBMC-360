#include "Settings.h"
#include "Application.h"
#include "utils\Log.h"
#include "GUISettings.h"

CSettings g_settings;

extern CStdString g_LoadErrorStr;

CSettings::CSettings()
{
}

CSettings::~CSettings()
{
}

void CSettings::Save() const
{
	if (g_application.IsStopping())
	{
		//don't save settings when we're busy stopping the application
		//a lot of screens try to save settings on deinit and deinit is called
		//for every screen when the application is stopping.
		return;
	}
	if (!SaveSettings("D:\\settings.xml"))
	{
		CLog::Log(LOGERROR, "Unable to save settings to D:\\settings.xml");
	}
}

bool CSettings::SaveSettings(const CStdString& strSettingsFile) const
{
	TiXmlDocument xmlDoc;
	TiXmlElement xmlRootElement("settings");

	TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
	if (!pRoot) return false;
	// write our tags one by one - just a big list for now (can be flashed up later)

	//
	//TODO
	//

	g_guiSettings.SaveXML(pRoot);

	return xmlDoc.SaveFile(strSettingsFile);
}

bool CSettings::Load()
{
	// load settings file...
	CLog::Log(LOGNOTICE, "loading D:\\settings.xml");

	if (!LoadSettings("D:\\settings.xml"))
	{
		CLog::Log(LOGERROR, "Unable to load T:\\settings.xml, creating new T:\\settings.xml with default values");
		Save();
		return false;
	}

	return true;
}

bool CSettings::LoadSettings(const CStdString& strSettingsFile)
{
	// load the xml file
	TiXmlDocument xmlDoc;
	if (!xmlDoc.LoadFile(strSettingsFile))
	{
		g_LoadErrorStr.Format("%s, Line %d\n%s", strSettingsFile.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
		return false;
	}

	TiXmlElement* pRootElement = xmlDoc.RootElement();
	CStdString strValue;

	if (pRootElement)
		strValue = pRootElement->Value();

	if (strcmpi(pRootElement->Value(), "settings") != 0)
	{
		g_LoadErrorStr.Format("%s Doesn't contain <settings>", strSettingsFile.c_str());
		return false;
	}

//	(pRootElement, "loglevel", g_stSettings.m_iLogLevel, LOGWARNING, LOGDEBUG, LOGNONE); //TODO

	g_guiSettings.LoadXML(pRootElement);	

	return true;
}

void CSettings::GetInteger(const TiXmlElement* pRootElement, const CStdString& strTagName, int& iValue, const int iDefault, const int iMin, const int iMax)
{
	const TiXmlNode *pChild = pRootElement->FirstChild(strTagName.c_str());
	if (pChild)
	{
		iValue = atoi( pChild->FirstChild()->Value() );
		if ((iValue < iMin) || (iValue > iMax)) iValue = iDefault;
	}
	else
		iValue = iDefault;

	CLog::Log(LOGDEBUG, "  %s: %d", strTagName.c_str(), iValue);
}