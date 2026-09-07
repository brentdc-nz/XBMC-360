#include "SkinInfo.h"
#include "utils\Log.h"
#include "Key.h"
#include "XMLUtils.h"
#include "utils\URIUtils.h"
#include "filesystem\File.h"
#include "GUIWindowManager.h"
#include "GUISettings.h"
#include <assert.h>

using namespace std;
using namespace XFILE;

CSkinInfo g_SkinInfo; // global

CSkinInfo::CSkinInfo()
{
	m_DefaultResolution = PAL_4x3;
	m_DefaultResolutionWide = INVALID;
	m_strBaseDir = "";
	m_iNumCreditLines = 0;
	m_effectsSlowDown = 1.0f;
	m_onlyAnimateToHome = true;
	m_Version = 1.0;
	m_skinzoom = 1.0f;
	m_bLegacy = false;
}

CSkinInfo::~CSkinInfo()
{
}

void CSkinInfo::Load(const CStdString& strSkinDir, bool loadIncludes)
{
	m_strBaseDir = strSkinDir;
	m_DefaultResolution = PAL_4x3;
	m_DefaultResolutionWide = INVALID;
	m_effectsSlowDown = 1.0f;
	m_skinzoom = 1.0f;
	m_Version = 1.0;

	// Load from skin.xml
	TiXmlDocument xmlDoc;
	CStdString strFile = m_strBaseDir + "\\skin.xml";

	if (xmlDoc.LoadFile(strFile))
	{
		// ok - get the default skin folder out of it...
		const TiXmlNode* root = xmlDoc.RootElement();
		if(root && root->ValueStr() == "skin")
		{
			GetResolution(root, "defaultresolution", m_DefaultResolution);

			if (!GetResolution(root, "defaultresolutionwide", m_DefaultResolutionWide))
				m_DefaultResolutionWide = m_DefaultResolution;

//			CLog::Log(LOGINFO, "Default 4:3 resolution directory is %s", URIUtils::AddFileToFolder(m_strBaseDir, GetDirFromRes(m_DefaultResolution)).c_str()); //TODO
//			CLog::Log(LOGINFO, "Default 16:9 resolution directory is %s", URIUtils::AddFileToFolder(m_strBaseDir, GetDirFromRes(m_DefaultResolutionWide)).c_str()); //TODO

			XMLUtils::GetDouble(root, "version", m_Version);
			XMLUtils::GetFloat(root, "effectslowdown", m_effectsSlowDown);
			XMLUtils::GetFloat(root, "zoom", m_skinzoom);

			// Get the legacy parameter to tweak the control behaviour for old skins such as PM3
			XMLUtils::GetBoolean(root, "legacy", m_bLegacy);   

			// Now load the credits information
			const TiXmlNode *pChild = root->FirstChild("credits");
			if (pChild)
			{ 
				// ok, run through the credits
				const TiXmlNode *pGrandChild = pChild->FirstChild("skinname");

				if (pGrandChild && pGrandChild->FirstChild())
				{
					CStdString strName = pGrandChild->FirstChild()->Value();
					swprintf(credits[0], L"%S Skin", strName.Left(44).c_str());
				}

				pGrandChild = pChild->FirstChild("name");
				m_iNumCreditLines = 1;

				while (pGrandChild && pGrandChild->FirstChild() && m_iNumCreditLines < 6)
				{
					CStdString strName = pGrandChild->FirstChild()->Value();
					swprintf(credits[m_iNumCreditLines], L"%S", strName.Left(49).c_str());
					m_iNumCreditLines++;
					pGrandChild = pGrandChild->NextSibling("name");
				}
			}

			// Now load the startupwindow information
			LoadStartupWindows(root->FirstChildElement("startupwindows"));
		}
		else
			CLog::Log(LOGERROR, "%s - %s doesnt contain <skin>", __FUNCTION__, strFile.c_str());
	}

	// Load the skin includes
	if(loadIncludes)
		LoadIncludes();
}

bool CSkinInfo::Check(const CStdString& strSkinDir)
{
	CSkinInfo info;
	info.Load(strSkinDir, false);
	
	if (info.GetVersion() < GetMinVersion())
	{
		CLog::Log(LOGERROR, "%s(%s) version is to old (%f versus %f)", __FUNCTION__, strSkinDir.c_str(), info.GetVersion(), GetMinVersion());
		return false;
	}
	
	if (!info.HasSkinFile("Home.xml") || !info.HasSkinFile("Fonts.xml"))
	{
		CLog::Log(LOGERROR, "%s(%s) does not contain Home.xml or Fonts.xml", __FUNCTION__, strSkinDir.c_str());
		return false;
	}
	return true;
}

bool CSkinInfo::HasSkinFile(const CStdString &strFile) const
{
	return CFile::Exists(GetSkinPath(strFile));
}

double CSkinInfo::GetMinVersion()
{
	return SKIN_MIN_VERSION;
}

bool CSkinInfo::ResolveConstant(const CStdString &constant, float &value) const
{
	return m_includes.ResolveConstant(constant, value);
}

bool CSkinInfo::ResolveConstant(const CStdString &constant, unsigned int &value) const
{
	float fValue;
	if (m_includes.ResolveConstant(constant, fValue))
	{
		value = (unsigned int)fValue;
		return true;
	}
	return false;
}

CStdString CSkinInfo::GetSkinPath(const CStdString& strFile, RESOLUTION *res, const CStdString& strBaseDir /* = "" */) const
{
	CStdString strPathToUse = m_strBaseDir;

	if (!strBaseDir.IsEmpty())
		strPathToUse = strBaseDir;

	// If the caller doesn't care about the resolution just use a temporary
	RESOLUTION tempRes = INVALID;
	if (!res)
		res = &tempRes;

	// First try and load from the current resolution's directory
	if (*res == INVALID)
		*res = g_graphicsContext.GetVideoResolution();

	CStdString strPath = strPathToUse;//URIUtils::AddFileToFolder(strPathToUse, GetDirFromRes(*res)); // TODO
	strPath = URIUtils::AddFileToFolder(strPath, strFile);
	
	if (CFile::Exists(strPath))
	{
		// Flat skin layout: the file lives at the skin root rather than in a
		// resolution subfolder, so the coordinates are authored in the skin's
		// default resolution (from skin.xml), not the current video resolution.
		*res = IsWide(*res) ? m_DefaultResolutionWide : m_DefaultResolution;
		return strPath;
	}

	// If we're in 1080i mode, try 720p next
	if (*res == HDTV_1080p)
	{
		*res = HDTV_720p;
		strPath = URIUtils::AddFileToFolder(strPathToUse, GetDirFromRes(*res));
		strPath = URIUtils::AddFileToFolder(strPath, strFile);
		
		if (CFile::Exists(strPath))
			return strPath;
	}

	// That failed - drop to the default widescreen resolution if where in a widemode
	if (*res == PAL_16x9 || *res == NTSC_16x9 || *res == HDTV_480p_16x9 || *res == HDTV_720p)
	{
		*res = m_DefaultResolutionWide;
		strPath = URIUtils::AddFileToFolder(strPathToUse, GetDirFromRes(*res));
		strPath = URIUtils::AddFileToFolder(strPath, strFile);
		
		if (CFile::Exists(strPath))
			return strPath;
	}

	// That failed - drop to the default resolution
	*res = m_DefaultResolution;
	strPath = URIUtils::AddFileToFolder(strPathToUse, GetDirFromRes(*res));
	strPath = URIUtils::AddFileToFolder(strPath, strFile);

	// Check if we don't have any subdirectories
	if (*res == INVALID) *res = PAL_4x3;
		return strPath;
}

CStdString CSkinInfo::GetBaseDir() const
{
	return m_strBaseDir;
}

bool CSkinInfo::GetResolution(const TiXmlNode *root, const char *tag, RESOLUTION &res) const
{
	CStdString strRes;

	if(XMLUtils::GetString(root, tag, strRes))
	{
		strRes.ToLower();

		if (strRes == "pal")
			res = PAL_4x3;
		else if (strRes == "pal16x9")
			res = PAL_16x9;
		else if (strRes == "ntsc")
			res = NTSC_4x3;
		else if (strRes == "ntsc16x9")
			res = NTSC_16x9;
		else if (strRes == "720p")
			res = HDTV_720p;
		else if (strRes == "1080p")
			res = HDTV_1080p;
		else
		{
			CLog::Log(LOGERROR, "%s invalid resolution specified for <%s>, %s", __FUNCTION__, tag, strRes.c_str());
			return false;
		}
		return true;
	}
	return false;
}

CStdString CSkinInfo::GetDirFromRes(RESOLUTION res) const
{
	CStdString strRes;

	switch (res)
	{
	case PAL_4x3:
		strRes = "PAL";
		break;
	case PAL_16x9:
		strRes = "PAL16x9";
		break;
	case NTSC_4x3:
	case HDTV_480p_4x3:
		strRes = "NTSC";
		break;
	case NTSC_16x9:
	case HDTV_480p_16x9:
		strRes = "ntsc16x9";
		break;
	case HDTV_720p:
		strRes = "720p";
		break;
	case HDTV_1080p:
		strRes = "1080p";
		break;
	case INVALID:
	default:
		strRes = "";
		break;
	}
	return strRes;
}

int CSkinInfo::GetStartWindow() const
{
	int windowID = g_guiSettings.GetInt("lookandfeel.startupwindow");
	assert(m_startupWindows.size());
	
	for (vector<CStartupWindow>::const_iterator it = m_startupWindows.begin(); it != m_startupWindows.end(); it++)
	{
		if (windowID == (*it).m_id)
			return windowID;
	}

	// Return our first one
	return m_startupWindows[0].m_id;
}

int CSkinInfo::GetFirstWindow() const
{
	int startWindow = GetStartWindow();
	
	if (HasSkinFile("Startup.xml") && (!m_onlyAnimateToHome || startWindow == WINDOW_HOME))
		startWindow = WINDOW_STARTUP_ANIM;
		
	return startWindow;
}

bool CSkinInfo::LoadStartupWindows(const TiXmlElement *startup)
{
	m_startupWindows.clear();
	
	if (startup)
	{
		// Yay, run through and grab the startup windows
		const TiXmlElement *window = startup->FirstChildElement("window");
		while (window && window->FirstChild())
		{
			int id;
			window->Attribute("id", &id);
			CStdString name = window->FirstChild()->Value();
			m_startupWindows.push_back(CStartupWindow(id + WINDOW_HOME, name));
			window = window->NextSiblingElement("window");
		}
	}

	// ok, now see if we have any startup windows
	if (!m_startupWindows.size())
	{
		// Nope - add the default ones
		m_startupWindows.push_back(CStartupWindow(WINDOW_HOME, "513"));
		m_startupWindows.push_back(CStartupWindow(WINDOW_PROGRAMS, "0"));
		m_startupWindows.push_back(CStartupWindow(WINDOW_PICTURES, "1"));
		m_startupWindows.push_back(CStartupWindow(WINDOW_MUSIC, "2"));
		m_startupWindows.push_back(CStartupWindow(WINDOW_VIDEOS, "3"));
		m_startupWindows.push_back(CStartupWindow(WINDOW_FILES, "7"));
//		m_startupWindows.push_back(CStartupWindow(WINDOW_SETTINGS_MENU, "5")); // FIXME
		m_startupWindows.push_back(CStartupWindow(WINDOW_SCRIPTS, "247"));
		m_onlyAnimateToHome = true;
	}
	else
		m_onlyAnimateToHome = false;
	return true;
}

bool CSkinInfo::IsWide(RESOLUTION res) const
{
	return (res == PAL_16x9 || res == NTSC_16x9 || res == HDTV_480p_16x9 || res == HDTV_720p || res == HDTV_1080p);
}

void CSkinInfo::GetSkinPaths(std::vector<CStdString> &paths) const
{
	RESOLUTION resToUse = INVALID;
	GetSkinPath("Home.xml", &resToUse);
	paths.push_back(URIUtils::AddFileToFolder(m_strBaseDir, GetDirFromRes(resToUse)));
	
	// See if we need to add other paths
	if (resToUse != m_DefaultResolutionWide && IsWide(resToUse))
		paths.push_back(URIUtils::AddFileToFolder(m_strBaseDir, GetDirFromRes(m_DefaultResolutionWide)));
	
	if (resToUse != m_DefaultResolution && (!IsWide(resToUse) || m_DefaultResolutionWide != m_DefaultResolution))
		paths.push_back(URIUtils::AddFileToFolder(m_strBaseDir, GetDirFromRes(m_DefaultResolution)));
}

void CSkinInfo::ResolveIncludes(TiXmlElement *node, const CStdString &type)
{
	m_includes.ResolveIncludes(node, type);
}

const INFO::CSkinVariableString* CSkinInfo::CreateSkinVariable(const CStdString& name, int context)
{
	return m_includes.CreateSkinVariable(name, context);
}

void CSkinInfo::LoadIncludes()
{
	CStdString includesPath = GetSkinPath("includes.xml");
	CLog::Log(LOGINFO, "Loading skin includes from %s", includesPath.c_str());
	m_includes.ClearIncludes();
	m_includes.LoadIncludes(includesPath);
}

