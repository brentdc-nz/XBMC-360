#include "AdvancedSettings.h"
#include "guilib\tinyxml\tinyxml.h"
#include "utils\Log.h"
#include "guilib\XMLUtils.h"
#include "Settings.h"


CAdvancedSettings g_advancedSettings;

CAdvancedSettings::CAdvancedSettings()
{
	m_logLevel = LOG_LEVEL_NORMAL;

	m_videoUseTimeSeeking = true;
	m_videoTimeSeekForwardBig = 600;
	m_videoTimeSeekBackwardBig = -600;
	m_videoTimeSeekForward = 30;
	m_videoTimeSeekBackward = -30;
	m_videoPercentSeekForwardBig = 10;
	m_videoPercentSeekBackwardBig = -10;
	m_videoPercentSeekForward = 2;
	m_videoPercentSeekBackward = -2;
	m_videoPPFFmpegDeint = "linblenddeint";
	m_videoPPFFmpegPostProc = "ha:128:7,va,dr";
	m_iSkipLoopFilter = 0;
}

bool CAdvancedSettings::Load()
{
	// NOTE: This routine should NOT set the default of any of these parameters
	//       it should instead use the versions of GetString/Integer/Float that
	//       don't take defaults in.  Defaults are set in the constructor above
	CStdString advancedSettingsXML;

	advancedSettingsXML = g_settings.GetUserDataItem("advancedsettings.xml"); //TODO
	TiXmlDocument advancedXML;
	if(!CFile::Exists(advancedSettingsXML))
	{
		// Tell the user it doesn't exist
		CLog::Log(LOGNOTICE, "No advancedsettings.xml to load (%s)", advancedSettingsXML.c_str());
		return false;
	}

	if(!advancedXML.LoadFile(advancedSettingsXML))
	{
		CLog::Log(LOGERROR, "Error loading %s, Line %d\n%s", advancedSettingsXML.c_str(), advancedXML.ErrorRow(), advancedXML.ErrorDesc());
		return false;
	}

	TiXmlElement *pRootElement = advancedXML.RootElement();
	if(!pRootElement || strcmpi(pRootElement->Value(),"advancedsettings") != 0)
	{
		CLog::Log(LOGERROR, "Error loading %s, no <advancedsettings> node", advancedSettingsXML.c_str());
		return false;
	}

	// Succeeded - tell the user it worked
	CLog::Log(LOGNOTICE, "Loaded advancedsettings.xml from %s", advancedSettingsXML.c_str());

	TiXmlElement *pElement = pRootElement->FirstChildElement("loglevel");
/*	if(pElement)
	{
		// Read the loglevel setting, so set the setting advanced to hide it in GUI
		// as altering it will do nothing - we don't write to advancedsettings.xml
		XMLUtils::GetInt(pRootElement, "loglevel", m_logLevelHint, LOG_LEVEL_NONE, LOG_LEVEL_MAX);
		CSettingBool *setting = (CSettingBool *)g_guiSettings.GetSetting("debug.showloginfo");
		if(setting)
		{
			const char* hide;
			if(!((hide = pElement->Attribute("hide")) && strnicmp("false", hide, 4) == 0))
				setting->SetAdvanced();
		}
		g_advancedSettings.m_logLevel = std::max(g_advancedSettings.m_logLevel, g_advancedSettings.m_logLevelHint);
		CLog::SetLogLevel(g_advancedSettings.m_logLevel);
	}
*/
	pElement = pRootElement->FirstChildElement("audio");
	if(pElement)
	{
		// WIP
	}

	pElement = pRootElement->FirstChildElement("video");
	if (pElement)
	{
		XMLUtils::GetBoolean(pElement, "usetimeseeking", m_videoUseTimeSeeking);

		XMLUtils::GetInt(pElement, "timeseekforward", m_videoTimeSeekForward, 0, 6000);
		XMLUtils::GetInt(pElement, "timeseekbackward", m_videoTimeSeekBackward, -6000, 0);
		XMLUtils::GetInt(pElement, "timeseekforwardbig", m_videoTimeSeekForwardBig, 0, 6000);
		XMLUtils::GetInt(pElement, "timeseekbackwardbig", m_videoTimeSeekBackwardBig, -6000, 0);

		XMLUtils::GetInt(pElement, "percentseekforward", m_videoPercentSeekForward, 0, 100);
		XMLUtils::GetInt(pElement, "percentseekbackward", m_videoPercentSeekBackward, -100, 0);
		XMLUtils::GetInt(pElement, "percentseekforwardbig", m_videoPercentSeekForwardBig, 0, 100);
		XMLUtils::GetInt(pElement, "percentseekbackwardbig", m_videoPercentSeekBackwardBig, -100, 0);

		XMLUtils::GetString(pElement,"ppffmpegdeinterlacing",m_videoPPFFmpegDeint);
		XMLUtils::GetString(pElement,"ppffmpegpostprocessing",m_videoPPFFmpegPostProc);
	}

	XMLUtils::GetInt(pRootElement,"skiploopfilter", m_iSkipLoopFilter, -16, 48);

	return true;
}

void CAdvancedSettings::Clear()
{
}