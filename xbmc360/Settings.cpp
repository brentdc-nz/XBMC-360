#include "Settings.h"
#include "Application.h"
#include "utils\Log.h"
#include "GUISettings.h"
#include "guilib\XMLUtils.h"
#include "utils\URIUtils.h"
#include "AdvancedSettings.h"
#include "URL.h"
#include "filesystem\File.h"
#include "guilib\GUIInfoManager.h"

class CSettings g_settings;
extern CStdString g_LoadErrorStr;

CSettings::CSettings()
{
}

CSettings::~CSettings()
{
}

void CSettings::Initialize()
{
	for(int i = HDTV_1080p; i <= PAL60_16x9; i++)
	{
		g_graphicsContext.ResetScreenParameters((RESOLUTION)i);
		g_graphicsContext.ResetOverscan((RESOLUTION)i, m_ResInfo[i].Overscan);
	}

	m_nVolumeLevel = 0;
	m_dynamicRangeCompressionLevel = 0;
	m_iPreMuteVolumeLevel = 0;
	m_bMute = false;

	m_strVideoExtensions = ".m4v|.3gp|.nsv|.ts|.ty|.strm|.pls|.rm|.rmvb|.m3u|.ifo|.mov|.qt|.divx|.xvid|.bivx|.vob|.nrg|.img|.iso|.pva|.wmv|.asf|.asx|.ogm|.m2v|.avi|.bin|.dat|.mpg|.mpeg|.mp4|.mkv|.avc|.vp3|.svq3|.nuv|.viv|.dv|.fli|.flv|.rar|.001|.wpl|.zip|.vdr|.dvr-ms";
	m_strAudioExtensions = ".nsv|.m4a|.flac|.aac|.strm|.pls|.rm|.mpa|.wav|.wma|.ogg|.mp3|.mp2|.m3u|.mod|.amf|.669|.dmf|.dsm|.far|.gdm|.imf|.it|.m15|.med|.okt|.s3m|.stm|.sfx|.ult|.uni|.xm|.sid|.ac3|.dts|.cue|.aif|.aiff|.wpl|.ape|.mac|.mpc|.mp+|.mpp|.shn|.zip|.rar|.wv|.nsf|.spc|.gym|.adplug|.adx|.dsp|.adp|.ymf|.ast|.afc|.hps|.xsp|.xwav|.waa|.wvs|.wam|.gcm|.idsp|.mpdsp|.mss|.spt|.rsd";
	m_strPictureExtensions = ".png|.jpg|.jpeg|.bmp|.gif|.ico|.tif|.tiff|.tga|.pcx|.cbz|.zip|.cbr|.rar|.m3u";

	m_logFolder = "D:\\"; // Log file location //TODO: Set in App:Create
	m_iSystemTimeTotalUp = 0;
}

bool CSettings::Load()
{
	// Load settings file...
	CLog::Log(LOGNOTICE, "loading %s", SETTINGS_FILE);

	if(!LoadSettings(SETTINGS_FILE))
	{
		CLog::Log(LOGERROR, "Unable to load %s, creating new %s with default values", SETTINGS_FILE, SETTINGS_FILE);
		Save();
		return false;
	}

	// Clear sources, then load xml file...
	m_fileSources.clear();
	m_musicSources.clear();
	m_pictureSources.clear();
	m_programSources.clear();
	m_videoSources.clear();

	CStdString strXMLFile = SOURCES_FILE; //GetSourcesFile(); // TODO

	CLog::Log(LOGNOTICE, "%s", strXMLFile.c_str());
	TiXmlDocument xmlDoc;
	TiXmlElement *pRootElement = NULL;

	if(xmlDoc.LoadFile(strXMLFile))
	{
		pRootElement = xmlDoc.RootElement();
		CStdString strValue;

		if(pRootElement)
			strValue = pRootElement->Value();

		if(strValue != "sources")
			CLog::Log(LOGERROR, "%s sources.xml file does not contain <sources>", __FUNCTION__);
	}
	else if(XFILE::CFile::Exists(strXMLFile))
		CLog::Log(LOGERROR, "%s Error loading %s: Line %d, %s", __FUNCTION__, strXMLFile.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());

	if(pRootElement)
	{
		// Parse sources...
		GetSources(pRootElement, "programs", m_programSources);
		GetSources(pRootElement, "pictures", m_pictureSources);
		GetSources(pRootElement, "files", m_fileSources);
		GetSources(pRootElement, "music", m_musicSources);
		GetSources(pRootElement, "video", m_videoSources);
	}	

	return true;
}

bool CSettings::LoadSettings(const CStdString& strSettingsFile)
{
	// load the xml file
	TiXmlDocument xmlDoc;

	if(!xmlDoc.LoadFile(strSettingsFile))
	{
		g_LoadErrorStr.Format("%s, Line %d\n%s", strSettingsFile.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
		return false;
	}

	TiXmlElement *pRootElement = xmlDoc.RootElement();
	if(strcmpi(pRootElement->Value(), "settings") != 0)
	{
		g_LoadErrorStr.Format("%s\nDoesn't contain <settings>", strSettingsFile.c_str());
		return false;
	}

	// My Music settings
	TiXmlElement *pElement = pRootElement->FirstChildElement("mymusic");
	if (pElement)
	{
		// TODO
	}

	// My Videos settings
	pElement = pRootElement->FirstChildElement("myvideos");
	if (pElement)
	{
		// TODO
	}

	pElement = pRootElement->FirstChildElement("viewstates");
	if (pElement)
	{
		// TODO
	}

	// General settings
	pElement = pRootElement->FirstChildElement("general");
	if (pElement)
	{
		GetInteger(pElement, "systemtotaluptime", m_iSystemTimeTotalUp, 0, 0, INT_MAX);
	}

	pElement = pRootElement->FirstChildElement("defaultvideosettings");
	if (pElement)
	{
		// TODO
	}

	// Audio settings
	pElement = pRootElement->FirstChildElement("audio");
	if (pElement)
	{
		GetInteger(pElement, "volumelevel", m_nVolumeLevel, VOLUME_MAXIMUM, VOLUME_MINIMUM, VOLUME_MAXIMUM);
		GetInteger(pElement, "dynamicrangecompression", m_dynamicRangeCompressionLevel, VOLUME_DRC_MINIMUM, VOLUME_DRC_MINIMUM, VOLUME_DRC_MAXIMUM);
		// TODO
	}

	g_guiSettings.LoadXML(pRootElement);	

	// Advanced settings
	g_advancedSettings.Load();

	return true;
}

void CSettings::Save() const
{
	if(g_application.IsStopping())
	{
		// Don't save settings when we're busy stopping the application
		// a lot of screens try to save settings on deinit and deinit is called
		// for every screen when the application is stopping.
		return;
	}
	if (!SaveSettings(SETTINGS_FILE))
	{
		CLog::Log(LOGERROR, "Unable to save settings to D:\\settings.xml");
	}
}

bool CSettings::SaveSettings(const CStdString& strSettingsFile) const
{
	TiXmlDocument xmlDoc;
	TiXmlElement xmlRootElement("settings");

	TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
	if(!pRoot) return false;

	// Write our tags one by one, just a big list for now (can be flashed up later)

	// General settings
	TiXmlElement generalNode("general");
	TiXmlNode *pNode = pRoot->InsertEndChild(generalNode);
	if(!pNode) return false;
	XMLUtils::SetInt(pNode, "systemtotaluptime", m_iSystemTimeTotalUp);

	// Audio settings
	TiXmlElement volumeNode("audio");
	pNode = pRoot->InsertEndChild(volumeNode);
	if (!pNode) return false;
	XMLUtils::SetInt(pNode, "volumelevel", m_nVolumeLevel);
	XMLUtils::SetInt(pNode, "dynamicrangecompression", m_dynamicRangeCompressionLevel);

	g_guiSettings.SaveXML(pRoot);

	return xmlDoc.SaveFile(strSettingsFile);
}

bool CSettings::UpdateShare(const CStdString &type, const CStdString oldName, const CMediaSource &share)
{
	VECSOURCES *pShares = GetSourcesFromType(type);

	if (!pShares) return false;

	// update our current share list
	CMediaSource* pShare=NULL;
	for (IVECSOURCES it = pShares->begin(); it != pShares->end(); it++)
	{
		if ((*it).strName == oldName)
		{
			(*it).strName = share.strName;
			(*it).strPath = share.strPath;
			(*it).vecPaths = share.vecPaths;
			pShare = &(*it);
			break;
		}
	}

	if (!pShare)
		return false;

	// Update our XML file as well
	return SaveSources();
}

bool CSettings::AddShare(const CStdString &type, const CMediaSource &share)
{
	VECSOURCES *pShares = GetSourcesFromType(type);
	if (!pShares) return false;

	// Translate dir and add to our current shares
	CStdString strPath1 = share.strPath;
	strPath1.ToUpper();

	if(strPath1.IsEmpty())
	{
		CLog::Log(LOGERROR, "unable to add empty path");
		return false;
	}

	CMediaSource shareToAdd = share;

/* // TODO - No special shares yet
	if (strPath1.at(0) == '$')
	{
		shareToAdd.strPath = CUtil::TranslateSpecialSource(strPath1);
		
		if (!share.strPath.IsEmpty())
			CLog::Log(LOGDEBUG, "%s Translated (%s) to Path (%s)",__FUNCTION__ ,strPath1.c_str(),shareToAdd.strPath.c_str());
		else
		{
			CLog::Log(LOGDEBUG, "%s Skipping invalid special directory token: %s",__FUNCTION__,strPath1.c_str());
			return false;
		}
	}
*/
	pShares->push_back(shareToAdd);

	if (!share.m_ignore)
	{
		return SaveSources();
	}
	return true;
}

VECSOURCES *CSettings::GetSourcesFromType(const CStdString &type)
{
	if (type == "programs" || type == "myprograms")
		return &m_programSources;
	else if (type == "files")
		return &m_fileSources;
	else if (type == "music")
		return &m_musicSources;
	else if (type == "video")
		return &m_videoSources;
	else if (type == "pictures")
		return &m_pictureSources;

	return NULL;
}

bool CSettings::DeleteSource(const CStdString &strType, const CStdString strName, const CStdString strPath, bool virtualSource)
{
	VECSOURCES *pShares = GetSourcesFromType(strType);
	if (!pShares) return false;

	bool found(false);

	for (IVECSOURCES it = pShares->begin(); it != pShares->end(); it++)
	{
		if ((*it).strName == strName && (*it).strPath == strPath)
		{
			CLog::Log(LOGDEBUG,"found share, removing!");
			pShares->erase(it);
			found = true;
			break;
		}
	}

	if (virtualSource)
		return found;

	return SaveSources();
}

void CSettings::Clear()
{
	m_programSources.clear();
	m_pictureSources.clear();
	m_fileSources.clear();
	m_musicSources.clear();
	m_videoSources.clear();
	m_skinBools.clear();
	m_skinStrings.clear();
}

int CSettings::TranslateSkinString(const CStdString &setting)
{
	CStdString settingName;
	settingName.Format("%s.%s", g_guiSettings.GetString("lookandfeel.skin").c_str(), setting);
	
	// Run through and see if we have this setting
	for (map<int, CSkinString>::const_iterator it = m_skinStrings.begin(); it != m_skinStrings.end(); it++)
	{
		if (settingName.Equals((*it).second.name))
			return (*it).first;
	}
	
	// Didn't find it - insert it
	CSkinString skinString;
	skinString.name = settingName;
	m_skinStrings.insert(pair<int, CSkinString>(m_skinStrings.size() + m_skinBools.size(), skinString));
	
	return m_skinStrings.size() + m_skinBools.size() - 1;
}

const CStdString &CSettings::GetSkinString(int setting) const
{
	map<int, CSkinString>::const_iterator it = m_skinStrings.find(setting);

	if (it != m_skinStrings.end())
		return (*it).second.value;

	return CStringUtils::EmptyString;
}

void CSettings::SetSkinString(int setting, const CStdString &label)
{
	map<int, CSkinString>::iterator it = m_skinStrings.find(setting);
	
	if (it != m_skinStrings.end())
	{
		(*it).second.value = label;
		return;
	}

	assert(false);
	CLog::Log(LOGFATAL, "%s : Unknown setting requested", __FUNCTION__);
}

void CSettings::ResetSkinSetting(const CStdString &setting)
{
	CStdString settingName;
	settingName.Format("%s.%s", g_guiSettings.GetString("lookandfeel.skin").c_str(), setting);
	
	// Run through and see if we have this setting as a string
	for (map<int, CSkinString>::iterator it = m_skinStrings.begin(); it != m_skinStrings.end(); it++)
	{
		if (settingName.Equals((*it).second.name))
		{
			(*it).second.value = "";
			return;
		}
	}
	
	// and now check for the skin bool
	for (map<int, CSkinBool>::iterator it = m_skinBools.begin(); it != m_skinBools.end(); it++)
	{
		if (settingName.Equals((*it).second.name))
		{
			(*it).second.value = false;
			return;
		}
	}
}

int CSettings::TranslateSkinBool(const CStdString &setting)
{
	CStdString settingName;
	settingName.Format("%s.%s", g_guiSettings.GetString("lookandfeel.skin").c_str(), setting);
	
	// Run through and see if we have this setting
	for (map<int, CSkinBool>::const_iterator it = m_skinBools.begin(); it != m_skinBools.end(); it++)
	{
		if (settingName.Equals((*it).second.name))
			return (*it).first;
	}
	
	// Didn't find it - insert it
	CSkinBool skinBool;
	skinBool.name = settingName;
	skinBool.value = false;
	m_skinBools.insert(pair<int, CSkinBool>(m_skinBools.size() + m_skinStrings.size(), skinBool));
	
	return m_skinBools.size() + m_skinStrings.size() - 1;
}

bool CSettings::GetSkinBool(int setting) const
{
	map<int, CSkinBool>::const_iterator it = m_skinBools.find(setting);
	
	if (it != m_skinBools.end())
		return (*it).second.value;

	// Default is to return false
	return false;
}

void CSettings::SetSkinBool(int setting, bool set)
{
	map<int, CSkinBool>::iterator it = m_skinBools.find(setting);
	
	if (it != m_skinBools.end())
	{
		(*it).second.value = set;
		return;
	}

	assert(false);
	CLog::Log(LOGFATAL,"%s : Unknown setting requested", __FUNCTION__);
}

void CSettings::ResetSkinSettings()
{
	CStdString currentSkin = g_guiSettings.GetString("lookandfeel.skin") + ".";

	// Clear all the settings and strings from this skin.
	map<int, CSkinBool>::iterator it = m_skinBools.begin();
	while (it != m_skinBools.end())
	{
		CStdString skinName = (*it).second.name;
		if (skinName.Left(currentSkin.size()) == currentSkin)
			(*it).second.value = false;

		it++;
	}
	
	map<int, CSkinString>::iterator it2 = m_skinStrings.begin();
	while (it2 != m_skinStrings.end())
	{
		CStdString skinName = (*it2).second.name;
		if (skinName.Left(currentSkin.size()) == currentSkin)
			(*it2).second.value = "";

		it2++;
	}
	g_infoManager.ResetCache();
}

void CSettings::GetSources(const TiXmlElement* pRootElement, const CStdString& strTagName, VECSOURCES& items)
{
	CLog::Log(LOGDEBUG, "Parsing <%s> tag", strTagName.c_str());

	items.clear();
	const TiXmlNode *pChild = pRootElement->FirstChild(strTagName.c_str());

	if(pChild)
	{
		pChild = pChild->FirstChild();
		while(pChild > 0)
		{
			CStdString strValue = pChild->Value();
			if(strValue == "source")
			{
				CMediaSource share;
				if(GetSource(strTagName, pChild, share))
				{
					items.push_back(share);
				}
				else
				{
					CLog::Log(LOGERROR, "Missing or invalid <name> and/or <path> in source");
				}
			}
			pChild = pChild->NextSibling();
		}
	}
	else
	{
		CLog::Log(LOGDEBUG, "  <%s> tag is missing or sources.xml is malformed", strTagName.c_str());
	}
}

bool CSettings::SetSources(TiXmlNode *root, const char *section, const VECSOURCES &shares) //TODO: Thumbnails
{
	TiXmlElement sectionElement(section);
	TiXmlNode *sectionNode = root->InsertEndChild(sectionElement);
	
	if (sectionNode)
	{
		XMLUtils::SetPath(sectionNode, "default", /*defaultPath*/"todo"); // TODO
		for (unsigned int i = 0; i < shares.size(); i++)
		{
			const CMediaSource &share = shares[i];
			
			if (share.m_ignore)
				continue;
			
			TiXmlElement source("source");

			XMLUtils::SetString(&source, "name", share.strName);

			for (unsigned int i = 0; i < share.vecPaths.size(); i++)
				XMLUtils::SetPath(&source, "path", share.vecPaths[i]);

/* TODO - No locking yet
			if (share.m_iHasLock)
			{
				XMLUtils::SetInt(&source, "lockmode", share.m_iLockMode);
				XMLUtils::SetString(&source, "lockcode", share.m_strLockCode);
				XMLUtils::SetInt(&source, "badpwdcount", share.m_iBadPwdCount);
			}
*/			
			if (!share.m_strThumbnailImage.IsEmpty())
				XMLUtils::SetPath(&source, "thumbnail", share.m_strThumbnailImage);

			sectionNode->InsertEndChild(source);
		}
	}
	return true;
}

bool CSettings::SaveSources()
{
	TiXmlDocument doc;
	TiXmlElement xmlRootElement("sources");
	TiXmlNode *pRoot = doc.InsertEndChild(xmlRootElement);

	if(!pRoot) return false;

	// Ok, now run through and save each sources section
	SetSources(pRoot, "programs", m_programSources);
	SetSources(pRoot, "video", m_videoSources);
	SetSources(pRoot, "music", m_musicSources);
	SetSources(pRoot, "pictures", m_pictureSources);
	SetSources(pRoot, "files", m_fileSources);

	return doc.SaveFile(/*GetSourcesFile()*/SOURCES_FILE); //TODO
}

bool CSettings::GetSource(const CStdString &category, const TiXmlNode *source, CMediaSource &share)
{
	CLog::Log(LOGDEBUG,"    ---- SOURCE START ----");

	const TiXmlNode *pNodeName = source->FirstChild("name");
	CStdString strName;
	
	if (pNodeName && pNodeName->FirstChild())
	{
		strName = pNodeName->FirstChild()->Value();
		CLog::Log(LOGDEBUG,"    Found name: %s", strName.c_str());
	}
	
	// Get multiple paths
	vector<CStdString> vecPaths;
	const TiXmlElement *pPathName = source->FirstChildElement("path");
	
	while (pPathName)
	{
		if (pPathName->FirstChild())
		{
			int pathVersion = 0;
			pPathName->Attribute("pathversion", &pathVersion);
			CStdString strPath = pPathName->FirstChild()->Value();
//			strPath = CSpecialProtocol::ReplaceOldPath(strPath, pathVersion); // TODO - No special paths yet
			
			// Make sure there are no virtualpaths or stack paths defined in xboxmediacenter.xml
			CLog::Log(LOGDEBUG,"    Found path: %s", strPath.c_str());
			
			if (/*!URIUtils::IsStack(strPath)*/1) // TODO
			{
				// Translate special tags
				if (!strPath.IsEmpty() && strPath.at(0) == '$')
				{
					CStdString strPathOld(strPath);
					strPath = strPath;//CUtil::TranslateSpecialSource(strPath); // TODO - No special paths yet
					
					if (!strPath.IsEmpty())
					{
						CLog::Log(LOGDEBUG,"    -> Translated to path: %s", strPath.c_str());
					}
					else
					{
						CLog::Log(LOGERROR,"    -> Skipping invalid token: %s", strPathOld.c_str());
						pPathName = pPathName->NextSiblingElement("path");
						continue;
					}
				}
				URIUtils::AddSlashAtEnd(strPath);
				vecPaths.push_back(strPath);
			}
			else
				CLog::Log(LOGERROR,"    Invalid path type (%s) in source", strPath.c_str());
		}
		pPathName = pPathName->NextSiblingElement("path");
	}

	const TiXmlNode *pLockMode = source->FirstChild("lockmode");
	const TiXmlNode *pLockCode = source->FirstChild("lockcode");
	const TiXmlNode *pBadPwdCount = source->FirstChild("badpwdcount");
	const TiXmlNode *pThumbnailNode = source->FirstChild("thumbnail");

	if (!strName.IsEmpty() && vecPaths.size() > 0)
	{
		vector<CStdString> verifiedPaths;
		
		// Disallowed for files, or theres only a single path in the vector
		if ((category.Equals("files")) || (vecPaths.size() == 1))
			verifiedPaths.push_back(vecPaths[0]);
		// Multiple paths?
		else
		{
			// Validate the paths
			for (int j = 0; j < (int)vecPaths.size(); ++j)
			{
				CURL url(vecPaths[j]);
				CStdString protocol = url.GetProtocol();
				bool bIsInvalid = false;

				// For my programs
				if (category.Equals("programs") || category.Equals("myprograms"))
				{
					// Only allow HD and plugins
					if (url.IsLocal() || protocol.Equals("plugin"))
						verifiedPaths.push_back(vecPaths[j]);
					else
					bIsInvalid = true;
				}
				// For others allow everything (if the user does something silly, we can't stop them)
				else
					verifiedPaths.push_back(vecPaths[j]);

				// Error message
				if (bIsInvalid)
					CLog::Log(LOGERROR,"    Invalid path type (%s) for multipath source", vecPaths[j].c_str());
			}

			// No valid paths? skip to next source
			if (verifiedPaths.size() == 0)
			{
				CLog::Log(LOGERROR,"    Missing or invalid <name> and/or <path> in source");
				return false;
			}
		}

		share.FromNameAndPaths(category, strName, verifiedPaths);

/* // TODO - No locking yet
		share.m_iBadPwdCount = 0;
		if (pLockMode)
		{
			share.m_iLockMode = LockType(atoi(pLockMode->FirstChild()->Value()));
			share.m_iHasLock = 2;
		}

		if (pLockCode)
		{
			if (pLockCode->FirstChild())
				share.m_strLockCode = pLockCode->FirstChild()->Value();
		}

		if (pBadPwdCount)
		{
			if (pBadPwdCount->FirstChild())
				share.m_iBadPwdCount = atoi( pBadPwdCount->FirstChild()->Value() );
		}
*/
		if (pThumbnailNode)
		{
			if (pThumbnailNode->FirstChild())
				share.m_strThumbnailImage = pThumbnailNode->FirstChild()->Value();
		}

		return true;
	}
	return false;
}

void CSettings::GetInteger(const TiXmlElement* pRootElement, const CStdString& strTagName, int& iValue, const int iDefault, const int iMin, const int iMax)
{
	const TiXmlNode *pChild = pRootElement->FirstChild(strTagName.c_str());
	if(pChild)
	{
		iValue = atoi( pChild->FirstChild()->Value() );
		if((iValue < iMin) || (iValue > iMax)) iValue = iDefault;
	}
	else
		iValue = iDefault;

	CLog::Log(LOGDEBUG, "  %s: %d", strTagName.c_str(), iValue);
}