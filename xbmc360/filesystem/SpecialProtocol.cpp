#include "SpecialProtocol.h"
#include "URL.h"
#include "utils\URIUtils.h"
#include "utils\Log.h"
#include "guilib\GraphicContext.h"
#include "Settings.h"

using namespace std;

map<CStdString, CStdString> CSpecialProtocol::m_pathMap;

void CSpecialProtocol::SetProfilePath(const CStdString &dir)
{
	SetPath("profile", dir);
	CLog::Log(LOGNOTICE, "special://profile/ is mapped to: %s", GetPath("profile").c_str());
}

void CSpecialProtocol::SetXBMCPath(const CStdString &dir)
{
	SetPath("xbmc", dir);
}

void CSpecialProtocol::SetHomePath(const CStdString &dir)
{
	SetPath("home", dir);
}

void CSpecialProtocol::SetUserHomePath(const CStdString &dir)
{
	SetPath("userhome", dir);
}

void CSpecialProtocol::SetMasterProfilePath(const CStdString &dir)
{
	SetPath("masterprofile", dir);
}

void CSpecialProtocol::SetTempPath(const CStdString &dir)
{
	SetPath("temp", dir);
}

bool CSpecialProtocol::XBMCIsHome()
{
	return TranslatePath("special://xbmc") == TranslatePath("special://home");
}

bool CSpecialProtocol::ComparePath(const CStdString &path1, const CStdString &path2)
{
	return TranslatePath(path1) == TranslatePath(path2);
}

CStdString CSpecialProtocol::TranslatePath(const CStdString &path)
{
	CURL url(path);
	if (!url.GetProtocol().Equals("special"))
		return path;

	return TranslatePath(url);
}

CStdString CSpecialProtocol::TranslatePath(const CURL &url)
{
	// Check for special-protocol, if not, return
	if (!url.GetProtocol().Equals("special"))
		return url.Get();

	CStdString FullFileName = url.GetFileName();

	CStdString translatedPath;
	CStdString FileName;
	CStdString RootDir;

	// Split up into the special://root and the rest of the filename
	int pos = FullFileName.Find('/');
	if (pos != -1 && pos > 1)
	{
		RootDir = FullFileName.Left(pos);

		if (pos < FullFileName.GetLength())
			FileName = FullFileName.Mid(pos + 1);
	}
	else
		RootDir = FullFileName;

	if (RootDir.Equals("subtitles"))
		URIUtils::AddFileToFolder("D:\\subtitle\\", FileName, translatedPath);
	else if (RootDir.Equals("userdata"))
		URIUtils::AddFileToFolder(g_settings.GetUserDataFolder(), FileName, translatedPath);
	else if (RootDir.Equals("database"))
		URIUtils::AddFileToFolder(g_settings.GetUserDataFolder() + "\\Database", FileName, translatedPath);
	else if (RootDir.Equals("thumbnails"))
		URIUtils::AddFileToFolder(g_settings.GetUserDataFolder() + "\\Thumbnails", FileName, translatedPath);
	else if (RootDir.Equals("musicplaylists"))
		URIUtils::AddFileToFolder(g_settings.GetUserDataFolder() + "\\playlists\\music", FileName, translatedPath);
	else if (RootDir.Equals("videoplaylists"))
		URIUtils::AddFileToFolder(g_settings.GetUserDataFolder() + "\\playlists\\video", FileName, translatedPath);
	else if (RootDir.Equals("skin"))
		URIUtils::AddFileToFolder(g_graphicsContext.GetMediaDir(), FileName, translatedPath);
	else if (RootDir.Equals("xbmc"))
		URIUtils::AddFileToFolder(GetPath("xbmc"), FileName, translatedPath);
	else if (RootDir.Equals("home"))
		URIUtils::AddFileToFolder(GetPath("home"), FileName, translatedPath);
	else if (RootDir.Equals("userhome"))
		URIUtils::AddFileToFolder(GetPath("userhome"), FileName, translatedPath);
	else if (RootDir.Equals("temp"))
		URIUtils::AddFileToFolder(GetPath("temp"), FileName, translatedPath);
	else if (RootDir.Equals("profile"))
		URIUtils::AddFileToFolder(GetPath("profile"), FileName, translatedPath);
	else if (RootDir.Equals("masterprofile"))
		URIUtils::AddFileToFolder(GetPath("masterprofile"), FileName, translatedPath);

	// Check if we need to recurse in
	if (URIUtils::IsSpecial(translatedPath))
	{
		// We need to recurse in, as there may be multiple translations required
		return TranslatePath(translatedPath);
	}

	return translatedPath;
}

void CSpecialProtocol::LogPaths()
{
	CLog::Log(LOGNOTICE, "special://xbmc/ is mapped to: %s", GetPath("xbmc").c_str());
	CLog::Log(LOGNOTICE, "special://masterprofile/ is mapped to: %s", GetPath("masterprofile").c_str());
	CLog::Log(LOGNOTICE, "special://home/ is mapped to: %s", GetPath("home").c_str());
	CLog::Log(LOGNOTICE, "special://temp/ is mapped to: %s", GetPath("temp").c_str());
}

// Private routines, to ensure we only set/get an appropriate path
void CSpecialProtocol::SetPath(const CStdString &key, const CStdString &path)
{
	m_pathMap[key] = path;
}

CStdString CSpecialProtocol::GetPath(const CStdString &key)
{
	map<CStdString, CStdString>::iterator it = m_pathMap.find(key);
	
	if (it != m_pathMap.end())
		return it->second;
		
	// Not yet initialised (early boot) - return empty rather than assert
	return "";
}
