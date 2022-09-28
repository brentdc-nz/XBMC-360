#include "Settings.h"
#include "Application.h"
#include "utils\Log.h"
#include "GUISettings.h"
#include "guilib\XMLUtils.h"
#include "utils\URIUtils.h"
#include "AdvancedSettings.h"
#include "URL.h"
#include "filesystem\File.h"
#include "utils/Util.h"
#include "guilib/GUIInfoManager.h"
#include "guilib/GUIWindowManager.h"
#include "LangInfo.h"

using namespace DIRECTORY;

class CSettings g_settings;
extern CStdString g_LoadErrorStr;

CSettings::CSettings()
{
}

CSettings::~CSettings()
{
}

//TODO Wolf3s: Some extensions are not working properly or not executing. need more tests and work.
void CSettings::LoadExtensions()
{
	m_strVideoExtensions = ".m4v|.3gp|.nsv|.ts|.ty|.strm|.pls|.rm|.rmvb|.m3u|.ifo|.mov|.qt|.divx|.xvid|.bivx|.vob|.nrg|.img|.iso|.pva|.wmv|.asf|.asx|.ogm|.m2v|.avi|.bin|.dat|.mpg|.mpeg|.mp4|.mkv|.avc|.vp3|.svq3|.nuv|.viv|.dv|.fli|.flv|.rar|.001|.wpl|.zip|.vdr|.dvr-ms";
	m_strAudioExtensions = ".nsv|.m4a|.flac|.aac|.strm|.pls|.rm|.mpa|.wav|.wma|.ogg|.mp3|.mp2|.m3u|.mod|.amf|.669|.dmf|.dsm|.far|.gdm|.imf|.it|.m15|.med|.okt|.s3m|.stm|.sfx|.ult|.uni|.xm|.sid|.ac3|.dts|.cue|.aif|.aiff|.wpl|.ape|.mac|.mpc|.mp+|.mpp|.shn|.zip|.rar|.wv|.nsf|.spc|.gym|.adplug|.adx|.dsp|.adp|.ymf|.ast|.afc|.hps|.xsp|.xwav|.waa|.wvs|.wam|.gcm|.idsp|.mpdsp|.mss|.spt|.rsd";
	m_strPictureExtensions = ".png|.jpg|.jpeg|.bmp|.gif|.ico|.tif|.tiff|.tga|.pcx|.cbz|.zip|.cbr|.rar|.m3u";
}

void CSettings::Initialize()
{
	LoadExtensions();
	m_iSystemTimeTotalUp = 0;
}

void CSettings::ClearSources()
{
	// Clear sources, then load xml file...
	m_vecProgramSources.clear();
	m_vecVideoSources.clear();
	m_vecMusicSources.clear();
	m_vecPictureSources.clear();
}

bool CSettings::Load()
{
	// Load settings file...
	CLog::Log(LOGNOTICE, "loading %s", GetSettingsFile());

	if(!LoadSettings(GetSettingsFile()))
	{
		CLog::Log(LOGERROR, "Unable to load %s, creating new %s with default values", GetSettingsFile(), GetSettingsFile());
		Save();
		return false;
	}

	ClearSources();

	CStdString strXMLFile = GetSourcesFile(); 

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
		GetSources(pRootElement, "programs", m_vecProgramSources);
		GetSources(pRootElement, "videos", m_vecVideoSources);
		GetSources(pRootElement, "music", m_vecMusicSources);
		GetSources(pRootElement, "pictures", m_vecPictureSources);
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

//	(pRootElement, "loglevel", g_stSettings.m_iLogLevel, LOGWARNING, LOGDEBUG, LOGNONE); //TODO

	// General settings
	TiXmlElement *pElement = pRootElement->FirstChildElement("general");
	if (pElement)
	{
		GetInteger(pElement, "systemtotaluptime", m_iSystemTimeTotalUp, 0, 0, INT_MAX);
	}

	g_guiSettings.LoadXML(pRootElement);	

	// Advanced settings
	g_advancedSettings.Load();

	return true;
}

CStdString CSettings::GetSettingsFile() const
{
  CStdString settings;
  if (m_currentProfile == 0)
    settings = "special://masterprofile/guisettings.xml";
  else
    settings = "special://profile/guisettings.xml";
  return settings;
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
	if (!SaveSettings(GetUserDataFolder()))
	{
		CLog::Log(LOGERROR, "Unable to save settings to special:\\xbmc-360\settings.xml");
	}
}

CStdString CSettings::GetUserDataItem(const CStdString& strFile) const
{
  CStdString folder;
  folder = "special://profile/"+strFile;
  //check if item exists in the profile
  //(either for folder or for a file (depending on slashAtEnd of strFile)
  //otherwise return path to masterprofile
  if ( (URIUtils::HasSlashAtEnd(folder) && !CDirectory::Exists(folder)) || !CFile::Exists(folder))
    folder = "special://masterprofile/"+strFile;
  return folder;
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

	g_guiSettings.SaveXML(pRoot);

	return xmlDoc.SaveFile(strSettingsFile);
}

bool CSettings::AddShare(const CStdString &type, const CMediaSource &share)
{
	VECSOURCES *pShares = GetSourcesFromType(type);
	if(!pShares) return false;

	// Translate dir and add to our current shares
	CStdString strPath1 = share.strPath;
	strPath1.ToUpper();
	
	if(strPath1.IsEmpty())
	{
		CLog::Log(LOGERROR, "Unable to add empty path");
		return false;
	}

	CMediaSource shareToAdd = share;
	pShares->push_back(shareToAdd);

	return SaveSources();
}

bool CSettings::LoadProfile(unsigned int index)
{
  unsigned int oldProfile = m_currentProfile;
  m_currentProfile = index;
  bool bSourcesXML=true;
  CStdString strOldSkin = g_guiSettings.GetString("lookandfeel.skin");
  CStdString strOldFont = g_guiSettings.GetString("lookandfeel.font");
  CStdString strOldTheme = g_guiSettings.GetString("lookandfeel.skintheme");
  CStdString strOldColors = g_guiSettings.GetString("lookandfeel.skincolors");

  if (Load())
  {
#ifdef WIP
    CreateProfileFolders();
    // initialize our charset converter
    g_charsetConverter.reset();
#endif
    // Load the langinfo to have user charset <-> utf-8 conversion
    CStdString strLanguage = g_guiSettings.GetString("locale.language");
    strLanguage[0] = toupper(strLanguage[0]);

    CStdString strLangInfoPath;
    strLangInfoPath.Format("special://xbmc-360/language/%s/langinfo.xml", strLanguage.c_str());
    CLog::Log(LOGINFO, "load language info file: %s", strLangInfoPath);
#ifdef WIP	
	g_langInfo.Load(strLangInfoPath);
#endif
#ifdef _XBOX
    CStdString strKeyboardLayoutConfigurationPath;
    strKeyboardLayoutConfigurationPath.Format("special://xbmc-360/language/%s/keyboardmap.xml", strLanguage.c_str());
    CLog::Log(LOGINFO, "load keyboard layout configuration info file: %s", strKeyboardLayoutConfigurationPath.c_str());
#ifdef WIP
    g_keyboardLayoutConfiguration.Load(strKeyboardLayoutConfigurationPath);
#endif
#endif
#ifdef WIP
    CButtonTranslator::GetInstance().Load();
#endif
    g_localizeStrings.Load("special://xbmc-360/language/", strLanguage);

    g_infoManager.ResetCache();
    g_infoManager.ResetLibraryBools();

    // always reload the skin - we need it for the new language strings
    g_application.LoadSkin(g_guiSettings.GetString("lookandfeel.skin"));
#ifdef WIP
    if (m_currentProfile != 0)
    {
      TiXmlDocument doc;
      if (doc.LoadFile(URIUtils::AddFileToFolder(GetUserDataFolder(),"guisettings.xml")))
        g_guiSettings.LoadMasterLock(doc.RootElement());
	}
   
#ifdef HAS_XBOX_HARDWARE //TODO: Add cooler class
    if (g_guiSettings.GetBool("system.autotemperature"))
    {
      CLog::Log(LOGNOTICE, "start fancontroller");
      CFanController::Instance()->Start(g_guiSettings.GetInt("system.targettemperature"), g_guiSettings.GetInt("system.minfanspeed"));
    }
    else if (g_guiSettings.GetBool("system.fanspeedcontrol"))
    {
      CLog::Log(LOGNOTICE, "setting fanspeed");
      CFanController::Instance()->SetFanSpeed(g_guiSettings.GetInt("system.fanspeed"));
    }
    g_application.StartLEDControl(false);
#endif
#endif
    // to set labels - shares are reloaded
#ifdef WIP    
	CDetectDVDMedia::UpdateState();
#endif    
	// init windows
    CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_WINDOW_RESET);

    g_windowManager.SendMessage(msg);
#ifdef WIP    
	CUtil::DeleteMusicDatabaseDirectoryCache();
    CUtil::DeleteVideoDatabaseDirectoryCache();
#endif
    return true;
  }

  m_currentProfile = oldProfile;

  return false;
}

CStdString CSettings::GetProfileUserDataFolder() const
{
  CStdString folder;
  if (m_currentProfile == 0)
    return GetUserDataFolder();

  URIUtils::AddFileToFolder(GetUserDataFolder(), GetCurrentProfile().getDirectory(), folder);

  return folder;
}

CStdString CSettings::GetUserDataFolder() const
{
  return GetMasterProfile().getDirectory();
}

CStdString CSettings::GetSkinFolder() const
{
  CStdString folder;

  // Get the Current Skin Path
  return GetSkinFolder(g_guiSettings.GetString("lookandfeel.skin"));
}

CStdString CSettings::GetSkinFolder(const CStdString &skinName) const
{
  CStdString folder;

  // Get the Current Skin Path
  URIUtils::AddFileToFolder("special://home/skin/", skinName, folder);
  if ( ! CDirectory::Exists(folder) )
    URIUtils::AddFileToFolder("special://xbmc-360/skin/", skinName, folder);

  return folder;
}

CStdString CSettings::GetSourcesFile() const
{
  CStdString folder;
  if (GetCurrentProfile().hasSources())
    URIUtils::AddFileToFolder(GetProfileUserDataFolder(),"sources.xml",folder);
  else
    URIUtils::AddFileToFolder(GetUserDataFolder(),"sources.xml",folder);
  return folder;
}

void CSettings::CreateProfileFolders()
{
#ifdef WIP
  CDirectory::Create(GetDatabaseFolder());
  CDirectory::Create(GetCDDBFolder());

  // Thumbnails/
  CDirectory::Create(GetThumbnailsFolder());
  CDirectory::Create(GetMusicThumbFolder());
  CDirectory::Create(GetMusicArtistThumbFolder());
  CDirectory::Create(GetLastFMThumbFolder());
  CDirectory::Create(GetVideoThumbFolder());
  CDirectory::Create(GetVideoFanartFolder());
  CDirectory::Create(GetMusicFanartFolder());
  CDirectory::Create(GetBookmarksThumbFolder());
  CDirectory::Create(GetProgramsThumbFolder());
  CDirectory::Create(GetPicturesThumbFolder());
  CDirectory::Create(GetGameSaveThumbFolder());

  CLog::Log(LOGINFO, "thumbnails folder: %s", GetThumbnailsFolder().c_str());
  for (unsigned int hex=0; hex < 16; hex++)
  {
    CStdString strHex;
    strHex.Format("%x",hex);
    CDirectory::Create(URIUtils::AddFileToFolder(GetPicturesThumbFolder(), strHex));
    CDirectory::Create(URIUtils::AddFileToFolder(GetMusicThumbFolder(), strHex));
    CDirectory::Create(URIUtils::AddFileToFolder(GetVideoThumbFolder(), strHex));
    CDirectory::Create(URIUtils::AddFileToFolder(GetProgramsThumbFolder(), strHex));
 
  }
   CDirectory::Create("special://profile/visualisations");
#endif
}

static CProfile emptyProfile;

const CProfile &CSettings::GetMasterProfile() const
{
  if (GetNumProfiles())
    return m_vecProfiles[0];
  CLog::Log(LOGERROR, "%s - master profile requested while none exists", __FUNCTION__);
  return emptyProfile;
}

const CProfile &CSettings::GetCurrentProfile() const
{
  if (m_currentProfile < m_vecProfiles.size())
    return m_vecProfiles[m_currentProfile];
  CLog::Log(LOGERROR, "%s - last profile index (%u) is outside the valid range (%u)", __FUNCTION__, m_currentProfile, m_vecProfiles.size());
  return emptyProfile;
}

void CSettings::UpdateCurrentProfileDate()
{
  if (m_currentProfile < m_vecProfiles.size())
    m_vecProfiles[m_currentProfile].setDate();
}

const CProfile *CSettings::GetProfile(unsigned int index) const
{
  if (index < GetNumProfiles())
    return &m_vecProfiles[index];
  return NULL;
}

CProfile *CSettings::GetProfile(unsigned int index)
{
  if (index < GetNumProfiles())
    return &m_vecProfiles[index];
  return NULL;
}

unsigned int CSettings::GetNumProfiles() const
{
  return m_vecProfiles.size();
}

int CSettings::GetProfileIndex(const CStdString &name) const
{
  for (unsigned int i = 0; i < m_vecProfiles.size(); i++)
    if (m_vecProfiles[i].getName().Equals(name))
      return i;
  return -1;
}

void CSettings::AddProfile(const CProfile &profile)
{
  m_vecProfiles.push_back(profile);
}

void CSettings::LoadMasterForLogin()
{
  // save the previous user
  m_lastUsedProfile = m_currentProfile;
  if (m_currentProfile != 0)
    LoadProfile(0);
}

VECSOURCES *CSettings::GetSourcesFromType(const CStdString &type)
{
	CStdString strType(type);

	CStringUtils::MakeLowercase(strType);

	if(strType == "programs" || strType == "myprograms")
		return &m_vecProgramSources;
	else if(strType== "videos")
		return &m_vecVideoSources;
	else if(strType == "music")
		return &m_vecMusicSources;
	else if(strType == "pictures")
		return &m_vecPictureSources;

	return NULL;
}

bool CSettings::DeleteSource(const CStdString &strType, const CStdString strName, const CStdString strPath, bool virtualSource)
{
	VECSOURCES *pShares = GetSourcesFromType(strType);
	if(!pShares) return false;

	bool found(false);

	for(IVECSOURCES it = pShares->begin(); it != pShares->end(); it++)
	{
		if((*it).strName == strName && (*it).strPath == strPath)
		{
			CLog::Log(LOGDEBUG,"Found source, removing!");
			pShares->erase(it);
			found = true;
			break;
		}
	}

	if(virtualSource)
		return found;

	return SaveSources();
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

bool CSettings::SetSources(TiXmlNode *root, const char *section, const VECSOURCES &shares)
{
	TiXmlElement sectionElement(section);
	TiXmlNode *sectionNode = root->InsertEndChild(sectionElement);
	
	if(sectionNode)
	{
		for(unsigned int i = 0; i < shares.size(); i++)
		{
			const CMediaSource &share = shares[i];
				
			TiXmlElement source("source");

			XMLUtils::SetString(&source, "name", share.strName);

			for(unsigned int i = 0; i < share.vecPaths.size(); i++)
				XMLUtils::SetPath(&source, "path", share.vecPaths[i]);

			if(!share.m_strThumbnailImage.IsEmpty())
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
	SetSources(pRoot, "programs", m_vecProgramSources);
	SetSources(pRoot, "videos", m_vecVideoSources);
	SetSources(pRoot, "music", m_vecMusicSources);
	SetSources(pRoot, "pictures", m_vecPictureSources);

	return doc.SaveFile(GetUserDataFolder());
}

bool CSettings::GetSource(const CStdString &category, const TiXmlNode *source, CMediaSource &share)
{
	CLog::Log(LOGDEBUG,"---- SOURCE START ----");
	const TiXmlNode *pNodeName = source->FirstChild("name");

	CStdString strName;
	if(pNodeName && pNodeName->FirstChild())
	{
		strName = pNodeName->FirstChild()->Value();
		CLog::Log(LOGDEBUG,"Found name: %s", strName.c_str());
	}

	// Get multiple paths
	vector<CStdString> vecPaths;
	const TiXmlElement *pPathName = source->FirstChildElement("path");
	
	while(pPathName)
	{
		if(pPathName->FirstChild())
		{
			CStdString strPath = pPathName->FirstChild()->Value();

			if(!strPath.IsEmpty())
			{
				CLog::Log(LOGDEBUG,"-> Translated to path: %s", strPath.c_str());
			}
			else
			{
				CLog::Log(LOGERROR,"-> Skipping invalid token: %s", strPath.c_str());
				pPathName = pPathName->NextSiblingElement("path");
				continue;
			}
			URIUtils::AddSlashAtEnd(strPath);
			vecPaths.push_back(strPath);
		}
		pPathName = pPathName->NextSiblingElement("path");
	}

	const TiXmlNode *pThumbnailNode = source->FirstChild("thumbnail");

	if(!strName.IsEmpty() && vecPaths.size() > 0)
	{
		vector<CStdString> verifiedPaths;

		// Disallowed for files, or theres only a single path in the vector
		if(vecPaths.size() == 1)
			verifiedPaths.push_back(vecPaths[0]);
		else // Multiple paths?
		{
			// Validate the paths
			for(int j = 0; j < (int)vecPaths.size(); ++j)
			{
				CURL url(vecPaths[j]);
				bool bIsInvalid = false;

				// For my programs
				if(category.Equals("programs") || category.Equals("myprograms"))
				{
					// Only allow HD and plugins
					if(url.IsLocal())
						verifiedPaths.push_back(vecPaths[j]);
					else
						bIsInvalid = true;
				}		
				else
					verifiedPaths.push_back(vecPaths[j]); // For others allow everything (if the user does something silly, we can't stop them)

			// Error message
			if(bIsInvalid)
				CLog::Log(LOGERROR,"Invalid path type (%s) for multipath source", vecPaths[j].c_str());
			}

			// No valid paths? skip to next source
			if(verifiedPaths.size() == 0)
			{
				CLog::Log(LOGERROR,"Missing or invalid <name> and/or <path> in source");
				return false;
			}
		}

		share.FromNameAndPaths(category, strName, verifiedPaths);

		if(pThumbnailNode)
		{
			if(pThumbnailNode->FirstChild())
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