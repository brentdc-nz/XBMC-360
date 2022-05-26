#include "Settings.h"
#include "Application.h"
#include "utils\Log.h"
#include "GUISettings.h"
#include "guilib\XMLUtils.h"
#include "utils\URIUtils.h"
#include "AdvancedSettings.h"
#include "URL.h"

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
	m_strVideoExtensions = ".m4v|.3gp|.nsv|.ts|.ty|.strm|.pls|.rm|.rmvb|.m3u|.ifo|.mov|.qt|.divx|.xvid|.bivx|.vob|.nrg|.img|.iso|.pva|.wmv|.asf|.asx|.ogm|.m2v|.avi|.bin|.dat|.mpg|.mpeg|.mp4|.mkv|.avc|.vp3|.svq3|.nuv|.viv|.dv|.fli|.flv|.rar|.001|.wpl|.zip|.vdr|.dvr-ms";
	m_strAudioExtensions = ".nsv|.m4a|.flac|.aac|.strm|.pls|.rm|.mpa|.wav|.wma|.ogg|.mp3|.mp2|.m3u|.mod|.amf|.669|.dmf|.dsm|.far|.gdm|.imf|.it|.m15|.med|.okt|.s3m|.stm|.sfx|.ult|.uni|.xm|.sid|.ac3|.dts|.cue|.aif|.aiff|.wpl|.ape|.mac|.mpc|.mp+|.mpp|.shn|.zip|.rar|.wv|.nsf|.spc|.gym|.adplug|.adx|.dsp|.adp|.ymf|.ast|.afc|.hps|.xsp|.xwav|.waa|.wvs|.wam|.gcm|.idsp|.mpdsp|.mss|.spt|.rsd";
	m_strPictureExtensions = ".png|.jpg|.jpeg|.bmp|.gif|.ico|.tif|.tiff|.tga|.pcx|.cbz|.zip|.cbr|.rar|.m3u";

	m_logFolder = "D:\\"; // Log file location //TODO: Set in App:Create
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
	m_vecProgramSources.clear();
	m_vecVideoSources.clear();
	m_vecMusicSources.clear();
	m_vecPictureSources.clear();

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
//	else if(CFile::Exists(strXMLFile))
//		CLog::Log(LOGERROR, "%s Error loading %s: Line %d, %s", __FUNCTION__, strXMLFile.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());

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

	//
	//TODO
	//

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

bool CSettings::SetSources(TiXmlNode *root, const char *section, const VECSOURCES &shares) //TODO: Thumbnails
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

//			if(!share.m_strThumbnailImage.IsEmpty())
//				XMLUtils::SetPath(&source, "thumbnail", share.m_strThumbnailImage);

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

	return doc.SaveFile(/*GetSourcesFile()*/SOURCES_FILE); //TODO
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

		if(pThumbnailNode) // TODO
		{
//			if(pThumbnailNode->FirstChild())
//				share.m_strThumbnailImage = pThumbnailNode->FirstChild()->Value();
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