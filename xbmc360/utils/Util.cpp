#include "Util.h"
#include "Log.h"
#include "ButtonTranslator.h"
#include "guilib\GUIWindowManager.h"
#include "Application.h"
#include "URL.h"
#include "filesystem\MultiPathDirectory.h"
#include "URIUtils.h"
#include "Settings.h"
#include "guilib\LocalizeStrings.h"
//#include "RegExp.h" // TODO

typedef struct
{
	char command[20];
	char description[128];
} BUILT_IN;

const BUILT_IN commands[] = 
{
	"Help", "This help message",
	"Reboot", "Reboot the xbox (power cycle)",
	"Restart", "Restart the xbox (power cycle)",
	"ShutDown", "Shutdown the xbox",
	"Dashboard", "Run your dashboard",
	"RestartApp", "Restart XBMC",
	"Credits", "Run XBMCs Credits",
	"Reset", "Reset the xbox (warm reboot)",
	"ActivateWindow", "Activate the specified window",
	"ReplaceWindow", "Replaces the current window with the new one",
	"TakeScreenshot", "Takes a Screenshot",
	"RunScript", "Run the specified script",
	"RunXBE", "Run the specified executeable",
	"Extract", "Extracts the specified archive",
	"PlayMedia", "Play the specified media file (or playlist)",
	"SlideShow", "Run a slideshow from the specified directory",
	"RecursiveSlideShow", "Run a slideshow from the specified directory, including all subdirs",
	"ReloadSkin", "Reload XBMC's skin",
	"PlayerControl", "Control the music or video player",
	"EjectTray", "Close or open the DVD tray",
	"AlarmClock", "Prompt for a length of time and start an alarm clock",
	"CancelAlarm","Cancels an alarm",
	"Action", "Executes an action for the active window (same as in keymap)",
	"Notification", "Shows a notification on screen, specify header, then message.",
	"PlayDVD"," Plays the inserted CD or DVD media from the DVD-ROM Drive!",
	"Skin.ToggleSetting"," Toggles a skin setting on or off",
	"Skin.SetString"," Prompts and sets skin string",
	"Skin.SetPath"," Prompts and sets a skin path",
	"Skin.SetImage"," Prompts and sets a skin image",
	"Skin.SetBool"," Sets a skin setting on",
	"Skin.Reset"," Resets a skin setting to default",
	"Skin.ResetSettings"," Resets all skin settings",
	"Mute","Mute the player",
	"SetVolume","Set the current volume",
	"Dialog.Close","Close a dialog"
};

void CUtil::SplitExecFunction(const CStdString &execString, CStdString &function, vector<CStdString> &parameters)
{
	CStdString paramString;
 
	int iPos = execString.Find("(");
	int iPos2 = execString.ReverseFind(")");
	
	if (iPos > 0 && iPos2 > 0)
	{
		paramString = execString.Mid(iPos + 1, iPos2 - iPos - 1);
		function = execString.Left(iPos);
	}
	else
		function = execString;

	// Remove any whitespace, and the standard prefix (if it exists)
	function.Trim();

	if( function.Left(5).Equals("xbmc.", false) )
		function.Delete(0, 5);

	SplitParams(paramString, parameters);
}

void CUtil::SplitParams(const CStdString &paramString, std::vector<CStdString> &parameters)
{
	bool inQuotes = false;
	bool lastEscaped = false; // Only every second character can be escaped
	int inFunction = 0;
	size_t whiteSpacePos = 0;
	CStdString parameter;
	parameters.clear();

	for (size_t pos = 0; pos < paramString.size(); pos++)
	{
		char ch = paramString[pos];
		bool escaped = (pos > 0 && paramString[pos - 1] == '\\' && !lastEscaped);
		lastEscaped = escaped;

		if (inQuotes)
		{
			// If we're in a quote, we accept everything until the closing quote
			if (ch == '\"' && !escaped)
			{
				// Finished a quote - no need to add the end quote to our string
				inQuotes = false;
			}
		}
		else
		{
			// Not in a quote, so check if we should be starting one
			if (ch == '\"' && !escaped)
			{
				// Start of quote - no need to add the quote to our string
				inQuotes = true;
			}
			if (inFunction && ch == ')')
			{
				// End of a function
				inFunction--;
			}
			if (ch == '(')
			{
				// Start of function
				inFunction++;
			}

			if (!inFunction && ch == ',')
			{
				// Not in a function, so a comma signfies the end of this parameter
				if (whiteSpacePos)
					parameter = parameter.Left(whiteSpacePos);

				// Trim off start and end quotes
				if (parameter.GetLength() > 1 && parameter[0] == '\"' && parameter[parameter.GetLength() - 1] == '\"')
					parameter = parameter.Mid(1,parameter.GetLength() - 2);

				parameters.push_back(parameter);
				parameter.Empty();
				whiteSpacePos = 0;
				continue;
			}
		}

		if ((ch == '\"' || ch == '\\') && escaped)
		{
			// Escaped quote or backslash
			parameter[parameter.size()-1] = ch;
			continue;
		}

		// Whitespace handling - We skip any whitespace at the left or right of an unquoted parameter
		if (ch == ' ' && !inQuotes)
		{
			if (parameter.IsEmpty()) // skip whitespace on left
				continue;

			if (!whiteSpacePos) // Make a note of where whitespace starts on the right
				whiteSpacePos = parameter.size();
		}
		else
			whiteSpacePos = 0;

		parameter += ch;
	}

	if (inFunction || inQuotes)
		CLog::Log(LOGWARNING, "%s(%s) - end of string while searching for ) or \"", __FUNCTION__, paramString.c_str());

	if (whiteSpacePos)
		parameter = parameter.Left(whiteSpacePos);

	// Trim off start and end quotes
	if (parameter.GetLength() > 1 && parameter[0] == '\"' && parameter[parameter.GetLength() - 1] == '\"')
		parameter = parameter.Mid(1,parameter.GetLength() - 2);

	if (!parameter.IsEmpty() || parameters.size())
		parameters.push_back(parameter);
}

// Returns a filename given an url
// handles both / and \, and options in urls
const CStdString CUtil::GetFileName(const CStdString& strFileNameAndPath)
{
	// Find any slashes
	const int slash1 = strFileNameAndPath.find_last_of('/');
	const int slash2 = strFileNameAndPath.find_last_of('\\');

	// Select the last one
	int slash;

	if(slash2>slash1)
		slash = slash2;
	else
		slash = slash1;

	// Check if there is any options in the url
	const int options = strFileNameAndPath.find_first_of('?', slash+1);
	if(options < 0)
		return strFileNameAndPath.substr(slash+1);
	else
		return strFileNameAndPath.substr(slash+1, options-(slash+1));
}

CStdString CUtil::GetTitleFromPath(const CStdString& strFileNameAndPath, bool bIsFolder /* = false */)
{
	// Use above to get the filename
	CStdString path(strFileNameAndPath);
	URIUtils::RemoveSlashAtEnd(path);
	CStdString strFilename = URIUtils::GetFileName(path);

	CURL url(strFileNameAndPath);
	CStdString strHostname = url.GetHostName();

	// Windows SMB Network (SMB)
	if (url.GetProtocol() == "smb" && strFilename.IsEmpty())
	{
		if (url.GetHostName().IsEmpty())
		{
			strFilename = g_localizeStrings.Get(20171);
		}
		else
		{
			strFilename = url.GetHostName();
		}
	}

	// URLDecode since the original path may be an URL
	CURL::Decode(strFilename);
	return strFilename;
}

bool CUtil::GetParentPath(const CStdString& strPath, CStdString& strParent)
{
	strParent = "";

	CURL url(strPath);
	CStdString strFile = url.GetFileName();
/*
	if(((url.GetProtocol() == "rar") || (url.GetProtocol() == "zip")) && strFile.IsEmpty())
	{ 
		strFile = url.GetHostName();
		return GetParentPath(strFile, strParent);
	}
	else if(strFile.size() == 0)
	{
		if(url.GetProtocol() == "smb" && (url.GetHostName().size() > 0))
		{
			// We have an smb share with only server or workgroup name
			// set hostname to "" and return true.
			url.SetHostName("");
			url.GetURL(strParent);
			return true;
		}
		else if(url.GetProtocol() == "xbms" && (url.GetHostName().size() > 0))
		{
			// We have an xbms share with only server name
			// set hostname to "" and return true.
			url.SetHostName("");
			url.GetURL(strParent);
			return true;
		}
		return false;
	}
*/
	if(HasSlashAtEnd(strFile))
		strFile = strFile.Left(strFile.size() - 1);

	int iPos = strFile.ReverseFind('/');

	if(iPos < 0)
		iPos = strFile.ReverseFind('\\');

	if(iPos < 0)
	{
		url.SetFileName("");
		url.GetURL(strParent);
		return true;
	}
  
	strFile = strFile.Left(iPos);

	if(IsLocalDrive(strFile))
		AddSlashAtEnd(strFile); // We need hdd1:\, not hdd1:

	url.SetFileName(strFile);
	url.GetURL(strParent);

	return true;
}

bool CUtil::HasSlashAtEnd(const CStdString& strFile)
{
	if(strFile.size() == 0)
		return false;

	char kar = strFile.c_str()[strFile.size() - 1];

	if(kar == '/' || kar == '\\') 
		return true;

	return false;
}

void CUtil::RemoveSlashAtEnd(CStdString& strFolder)
{
	// Correct check for base url like smb://
	if(strFolder.Right(3).Equals("://"))
		return;

	if(CUtil::HasSlashAtEnd(strFolder))
		strFolder.Delete(strFolder.size() - 1);
}

void CUtil::AddSlashAtEnd(CStdString& strFolder)
{
	// Correct check for base url like smb://
	if (strFolder.Right(3).Equals("://"))
		return;

	if(!CUtil::HasSlashAtEnd(strFolder))
	{
		if(strFolder.Find("//") >= 0)
			strFolder += "/";
		else
			strFolder += "\\";
	}
}

// Returns filename extension including period of filename
const CStdString CUtil::GetExtension(const CStdString& strFileName)
{
	int period = strFileName.find_last_of('.');
	
	if(period >= 0)
	{
		if(strFileName.find_first_of('/', period+1) != -1) return "";
		if(strFileName.find_first_of('\\', period+1) != -1) return "";

		// url options could be at the end of a url
		const int options = strFileName.find_first_of('?', period+1);

		if(options >= 0)
			return strFileName.substr(period, options-period);
		else
			return strFileName.substr(period);
	}
	else
		return "";
}

void CUtil::GetExtension(const CStdString& strFile, CStdString& strExtension)
{
	strExtension = GetExtension(strFile);
}

int CUtil::GetMatchingShare(const CStdString& strPath1, VECSOURCES& vecShares, bool& bIsBookmarkName)
{
	if(strPath1.IsEmpty())
		return -1;


	return -1;
}

void CUtil::ForceForwardSlashes(CStdString& strPath)
{
	int iPos = strPath.ReverseFind('\\');

	while(iPos > 0)
	{
		strPath.at(iPos) = '/';
		iPos = strPath.ReverseFind('\\');
	}
}

void CUtil::Stat64ToStat(struct _stat *result, struct __stat64 *stat)
{
	result->st_dev = stat->st_dev;
	result->st_ino = stat->st_ino;
	result->st_mode = stat->st_mode;
	result->st_nlink = stat->st_nlink;
	result->st_uid = stat->st_uid;
	result->st_gid = stat->st_gid;
	result->st_rdev = stat->st_rdev;

	if (stat->st_size <= LONG_MAX)
		result->st_size = (_off_t)stat->st_size;
	else
	{
		result->st_size = 0;
		CLog::Log(LOGWARNING, "WARNING: File is larger than 32bit stat can handle, file size will be reported as 0 bytes");
	}

	result->st_atime = (time_t)(stat->st_atime & 0xFFFFFFFF);
	result->st_mtime = (time_t)(stat->st_mtime & 0xFFFFFFFF);
	result->st_ctime = (time_t)(stat->st_ctime & 0xFFFFFFFF);
}

void CUtil::URLEncode(CStdString& strURLData)
{
	CStdString strResult;

	// Wonder what a good value is here is, depends on how often it occurs
	strResult.reserve( strURLData.length() * 2 );

	for(int i = 0; i < (int)strURLData.size(); ++i)
	{
		int kar = (unsigned char)strURLData[i];
		if(isalnum(kar))
			strResult += kar;
		else
		{
			CStdString strTmp;
			strTmp.Format("%%%02.2x", kar);
			strResult += strTmp;
		}
	}
	strURLData = strResult;
}

bool CUtil::IsLocalDrive(const CStdString& strPath, bool bFullPath /*= false*/)
{
	VECSOURCES sources;

	return false;
}

void CUtil::AddFileToFolder(const CStdString& strFolder, const CStdString& strFile, CStdString& strResult)
{
	strResult = strFolder;
	
	// Remove the stack:// as it screws up the logic below
	if(IsStack(strFolder))
		strResult = strResult.Mid(8);

	// Add a slash to the end of the path if necessary
	if(!CUtil::HasSlashAtEnd(strResult))
	{
		if(strResult.Find("//") >= 0 )
			strResult += "/";
		else
			strResult += "\\";
	}

	// Remove any slash at the start of the file
	if(strFile.size() && strFile[0] == '/' || strFile[0] == '\\')
		strResult += strFile.Mid(1);
	else
		strResult += strFile;

	// Re-add the stack:// protocol
	if(IsStack(strFolder))
		strResult = "stack://" + strResult;
}

bool CUtil::IsStack(const CStdString& strFile)
{
	if(strFile.Left(8).Equals("stack://")) return true;
	return false;
}

bool CUtil::IsPicture(const CStdString& strFile)
{
	CStdString extension = URIUtils::GetExtension(strFile);

	if (extension.IsEmpty())
		return false;

	extension.ToLower();
	if (g_settings.GetPictureExtensions().Find(extension) != -1)
		return true;

	if (extension == ".tbn" || extension == ".dds")
		return true;

	return false;
}

void CUtil::UrlDecode(CStdString& strURLData)
{
	CStdString strResult;

	// Resulet will always be less than source */
	strResult.reserve( strURLData.length() );

	for(unsigned int i = 0; i < (int)strURLData.size(); ++i)
	{
		int kar = (unsigned char)strURLData[i];
		if (kar == '+') strResult += ' ';

		else if(kar == '%')
		{
			if(i < strURLData.size() - 2)
			{
				CStdString strTmp;
				strTmp.assign(strURLData.substr(i + 1, 2));
				int dec_num;
				sscanf(strTmp,"%x",&dec_num);
				strResult += (char)dec_num;
				i += 2;
			}
			else
				strResult += kar;
		}
		else strResult += kar;
	}
	strURLData = strResult;
}

float CUtil::CurrentCpuUsage()
{
	return (1.0f - g_application.GetIdleThread().GetRelativeUsage())*100;
}

__int64 CUtil::ToInt64(DWORD dwHigh, DWORD dwLow)
{
	__int64 n;
	n = dwHigh;
	n <<= 32;
	n += dwLow;
	return n;
}

bool CUtil::ExcludeFileOrFolder(const CStdString& strFileOrFolder, const CStdStringArray& regexps) // TODO
{
	if (strFileOrFolder.IsEmpty())
		return false;

	CStdString strExclude = strFileOrFolder;
	strExclude.MakeLower();

	return true; // WIP

/* // TODO
	CRegExp regExExcludes;

	for (unsigned int i = 0; i < regexps.size(); i++)
	{
		if (!regExExcludes.RegComp(regexps[i].c_str()))
		{
			// Invalid regexp - complain in logs
			CLog::Log(LOGERROR, "%s: Invalid exclude RegExp:'%s'", __FUNCTION__, regexps[i].c_str());
			continue;
		}
		if (regExExcludes.RegFind(strExclude) > -1)
		{
			CLog::Log(LOGDEBUG, "%s: File '%s' excluded. (Matches exclude rule RegExp:'%s')", __FUNCTION__, strExclude.c_str(), regexps[i].c_str());
			return true;
		}
	}
	return false;
*/
}

int CUtil::GetMatchingSource(const CStdString& strPath1, VECSOURCES& VECSOURCES, bool& bIsSourceName)
{
	if (strPath1.IsEmpty())
		return -1;

	CLog::Log(LOGDEBUG,"CUtil::GetMatchingSource, testing original path/name [%s]", strPath1.c_str());

	// Copy as we may change strPath
	CStdString strPath = strPath1;

	// Check for special protocols
	CURL checkURL(strPath);

	// stack://
	if (checkURL.GetProtocol() == "stack")
		strPath.Delete(0, 8); // Remove the stack protocol

	if (checkURL.GetProtocol() == "shout")
		strPath = checkURL.GetHostName();

	if (checkURL.GetProtocol() == "lastfm")
		return 1;

	if (checkURL.GetProtocol() == "tuxbox")
		return 1;

	if (checkURL.GetProtocol() == "plugin")
		return 1;

//	if (checkURL.GetProtocol() == "multipath") // TODO
//		strPath = CMultiPathDirectory::GetFirstPath(strPath); // TODO

	CLog::Log(LOGDEBUG,"CUtil::GetMatchingSource, testing for matching name [%s]", strPath.c_str());
	bIsSourceName = false;
	int iIndex = -1;
	int iLength = -1;
	
	// We first test the NAME of a source
	for (int i = 0; i < (int)VECSOURCES.size(); ++i)
	{
		CMediaSource share = VECSOURCES.at(i);
		CStdString strName = share.strName;

		// Special cases for dvds
		if (/*URIUtils::IsOnDVD(share.strPath)*/0) // TODO
		{
//			if (URIUtils::IsOnDVD(strPath)) // TODO
				return i;

			// Not a path, so we need to modify the source name
			// since we add the drive status and disc name to the source
			// "Name (Drive Status/Disc Name)"
			int iPos = strName.ReverseFind('(');
			if (iPos > 1)
				strName = strName.Mid(0, iPos - 1);
		}

		CLog::Log(LOGDEBUG,"CUtil::GetMatchingSource, comparing name [%s]", strName.c_str());

		if (strPath.Equals(strName))
		{
			bIsSourceName = true;
			return i;
		}
	}

	// Now test the paths

	// Remove user details, and ensure path only uses forward slashes
	// and ends with a trailing slash so as not to match a substring
	CURL urlDest(strPath);
	urlDest.SetOptions("");
	CStdString strDest = urlDest.GetWithoutUserDetails();
	ForceForwardSlashes(strDest);
	
	if (!URIUtils::HasSlashAtEnd(strDest))
		strDest += "/";
	
	int iLenPath = strDest.size();

	CLog::Log(LOGDEBUG,"CUtil::GetMatchingSource, testing url [%s]", strDest.c_str());

	for (int i = 0; i < (int)VECSOURCES.size(); ++i)
	{
		CMediaSource share = VECSOURCES.at(i);

		// Does it match a source name?
		if (share.strPath.substr(0,8) == "shout://")
		{
			CURL url(share.strPath);
			if (strPath.Equals(url.GetHostName()))
				return i;
		}

		// Doesnt match a name, so try the source path
		vector<CStdString> vecPaths;

		// Add any concatenated paths if they exist
		if (share.vecPaths.size() > 0)
			vecPaths = share.vecPaths;

		// Add the actual share path at the front of the vector
		vecPaths.insert(vecPaths.begin(), share.strPath);

		// Test each path
		for (int j = 0; j < (int)vecPaths.size(); ++j)
		{
			// Remove user details, and ensure path only uses forward slashes
			// and ends with a trailing slash so as not to match a substring
			CURL urlShare(vecPaths[j]);
			urlShare.SetOptions("");
			CStdString strShare = urlShare.GetWithoutUserDetails();
			ForceForwardSlashes(strShare);
			
			if (!URIUtils::HasSlashAtEnd(strShare))
				strShare += "/";
			
			int iLenShare = strShare.size();
			CLog::Log(LOGDEBUG,"CUtil::GetMatchingSource, comparing url [%s]", strShare.c_str());

			if ((iLenPath >= iLenShare) && (strDest.Left(iLenShare).Equals(strShare)) && (iLenShare > iLength))
			{
				CLog::Log(LOGDEBUG,"Found matching source at index %i: [%s], Len = [%i]", i, strShare.c_str(), iLenShare);

				// If exact match, return it immediately
				if (iLenPath == iLenShare)
				{
					// If the path EXACTLY matches an item in a concatentated path
					// set source name to true to load the full virtualpath
					bIsSourceName = false;
					
					if (vecPaths.size() > 1)
						bIsSourceName = true;
					
					return i;
				}
				iIndex = i;
				iLength = iLenShare;
			}
		}
	}

	// Return the index of the share with the longest match
	if (iIndex == -1)
	{
		// rar:// and zip://
		// If archive wasn't mounted, look for a matching share for the archive instead
		if( strPath.Left(6).Equals("rar://") || strPath.Left(6).Equals("zip://") )
		{
			// Get the hostname portion of the url since it contains the archive file
			strPath = checkURL.GetHostName();

			bIsSourceName = false;
			bool bDummy;
			return GetMatchingSource(strPath, VECSOURCES, bDummy);
		}
		CLog::Log(LOGWARNING,"CUtil::GetMatchingSource... no matching source found for [%s]", strPath1.c_str());
	}
	return iIndex;
}