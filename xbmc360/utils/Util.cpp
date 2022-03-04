#include "Util.h"
#include "Log.h"
#include "..\ButtonTranslator.h"
#include "..\guilib\GUIWindowManager.h"
#include "..\Application.h"
#include "..\URL.h"
#include "..\filesystem\MultiPathDirectory.h"

namespace MathUtils
{
	inline int round_int(double x)
	{
		return (x > 0) ? (int)floor(x + 0.5) : (int)ceil(x - 0.5);
	}

	inline double rint(double x)
	{
		return floor(x+.5);
	}

	void hack()
	{
		// Stupid hack to keep compiler from dropping these
		// functions as unused
		MathUtils::round_int(0.0);
		MathUtils::rint(0.0);
	}

} // CMathUtils namespace

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

bool CUtil::IsBuiltIn(const CStdString& execString)
{
	CStdString function, param;
	SplitExecFunction(execString, function, param);
	for (int i = 0; i < sizeof(commands)/sizeof(BUILT_IN); i++)
	{
		if (function.CompareNoCase(commands[i].command) == 0)
			return true;
	}
	return false;
}

void CUtil::SplitExecFunction(const CStdString &execString, CStdString &strFunction, CStdString &strParam)
{
	strParam = "";

	int iPos = execString.Find("(");
	int iPos2 = execString.ReverseFind(")");
	if (iPos > 0 && iPos2 > 0)
	{
		strParam = execString.Mid(iPos + 1, iPos2 - iPos - 1);
		strFunction = execString.Left(iPos);
	}
	else
		strFunction = execString;

	//xbmc is the standard prefix.. so allways remove this
	//all other commands with go through in full
	if( strFunction.Left(5).Equals("xbmc.", false) )
		strFunction.Delete(0, 5);
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
	CStdString strFilename = GetFileName(strFileNameAndPath);

	// Now remove the extension if needed
/*	if(g_guiSettings.GetBool("filelists.hideextensions") && !bIsFolder)
	{
		RemoveExtension(strFilename);
		return strFilename;
	}
*/
	return strFilename;
}

int CUtil::ExecBuiltIn(const CStdString& execString)
{
	// Get the text after the "XBMC."
	CStdString execute, parameter;
	SplitExecFunction(execString, execute, parameter);
	CStdString strParameterCaseIntact = parameter;
	parameter.ToLower();
	execute.ToLower();
  
	if(execute.Equals("reboot") || execute.Equals("restart")) // Will reboot the Xbox, aka cold reboot
	{
		g_application.getApplicationMessenger().Reboot();
	}
	else if(execute.Equals("shutdown"))
	{
		g_application.getApplicationMessenger().Shutdown();
	}
	else if (execute.Equals("activatewindow") || execute.Equals("replacewindow"))
	{
		// Get the parameters
		CStdString strWindow;
		CStdString strPath;

		// Split the parameter on first comma
		int iPos = parameter.Find(",");
		if(iPos == 0)
		{
			// Error condition missing path
			// XMBC.ActivateWindow(1,)
			CLog::Log(LOGERROR, "Activate/ReplaceWindow called with invalid parameter: %s", parameter.c_str());
			return -7;
		}
		else if(iPos < 0)
		{
			// No path parameter
			// XBMC.ActivateWindow(5001)
			strWindow = parameter;
		}
		else
		{
			// Path parameter included
			// XBMC.ActivateWindow(5001,F:\Music\)
			strWindow = parameter.Left(iPos);
			strPath = parameter.Mid(iPos + 1);
		}

		// Confirm the window destination is actually a number
		// before switching
		int iWindow = g_buttonTranslator.TranslateWindowString(strWindow.c_str());
		if(iWindow != WINDOW_INVALID)
		{
			// Disable the screensaver
			g_application.ResetScreenSaverWindow();
			if(execute.Equals("activatewindow"))
				g_windowManager.ActivateWindow(iWindow/*, strPath*/);
			else  
				// Replace Window
				g_windowManager.ChangeActiveWindow(iWindow/*, strPath*/);
		}
		else
		{
			CLog::Log(LOGERROR, "Activate/ReplaceWindow called with invalid destination window: %s", strWindow.c_str());
			return false;
		}
	}
	return 0;
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

	CLog::Log(LOGDEBUG,"CUtil::GetMatchingShare, testing original path/name [%s]", strPath1.c_str());

	// Copy as we may change strPath
	CStdString strPath = strPath1;

	// Check for special protocols
	CURL checkURL(strPath);

	if(checkURL.GetProtocol() == "multipath")
		strPath = DIRECTORY::CMultiPathDirectory::GetFirstPath(strPath);

	CLog::Log(LOGDEBUG,"CUtil::GetMatchingShare, testing for matching name [%s]", strPath.c_str());
	
	bIsBookmarkName = false;
	int iIndex = -1;
	int iLength = -1;
	
	// We first test the NAME of a bookmark
	for(int i = 0; i < (int)vecShares.size(); ++i)
	{
		CMediaSource share = vecShares.at(i);
		CStdString strName = share.strName;
/*
		// Special cases for DVDs
		if(IsOnDVD(share.strPath)) //TODO
		{
			if(IsOnDVD(strPath))
				return i;

			// Not a path, so we need to modify the bookmark name
			// since we add the drive status and disc name to the bookmark

			// "Name (Drive Status/Disc Name)"
			int iPos = strName.ReverseFind('(');
			if(iPos > 1)
				strName = strName.Mid(0, iPos - 1);
		}
*/
		CLog::Log(LOGDEBUG,"CUtil::GetMatchingShare, comparing name [%s]", strName.c_str());

		if(strPath.Equals(strName))
		{
			bIsBookmarkName = true;
			return i;
		}
	}

	// Now test the paths

	// Remove user details, and ensure path only uses forward slashes
	// and ends with a trailing slash so as not to match a substring
	CURL urlDest(strPath);
	CStdString strDest;
	urlDest.GetURLWithoutUserDetails(strDest);
	ForceForwardSlashes(strDest);

	if(!HasSlashAtEnd(strDest))
		strDest += "/";

	int iLenPath = strDest.size();

	CLog::Log(LOGDEBUG,"CUtil::GetMatchingShare, testing url [%s]", strDest.c_str());

	for(int i = 0; i < (int)vecShares.size(); ++i)
	{
		CMediaSource share = vecShares.at(i);

		// Doesnt match a name, so try the bookmark path
		vector<CStdString> vecPaths;

		// Add any concatenated paths if they exist
		if(share.vecPaths.size() > 0)
			vecPaths = share.vecPaths;

		// Add the actual share path at the front of the vector
		vecPaths.insert(vecPaths.begin(), share.strPath);

		// Test each path
		for(int j = 0; j < (int)vecPaths.size(); ++j)
		{
			// Remove user details, and ensure path only uses forward slashes
			// and ends with a trailing slash so as not to match a substring
			CURL urlShare(vecPaths[j]);
			CStdString strShare;
			urlShare.GetURLWithoutUserDetails(strShare);
			ForceForwardSlashes(strShare);

			if(!HasSlashAtEnd(strShare))
				strShare += "/";
			
			int iLenShare = strShare.size();
			
			CLog::Log(LOGDEBUG,"CUtil::GetMatchingShare, comparing url [%s]", strShare.c_str());

			if((iLenPath >= iLenShare) && (strDest.Left(iLenShare).Equals(strShare)) && (iLenShare > iLength))
			{
				CLog::Log(LOGDEBUG,"Found matching bookmark at index %i: [%s], Len = [%i]", i, strShare.c_str(), iLenShare);

				// If exact match, return it immediately
				if(iLenPath == iLenShare)
				{
					// If the path EXACTLY matches an item in a concatentated path
					// set bookmark name to true to load the full virtualpath
					bIsBookmarkName = false;

					if(vecPaths.size() > 1)
						bIsBookmarkName = true;

					return i;
				}
				iIndex = i;
				iLength = iLenShare;
			}
		}
	}

	// Return the index of the share with the longest match

	if(iIndex == -1)
		CLog::Log(LOGWARNING,"CUtil::GetMatchingShare... no matching bookmark found for [%s]", strPath1.c_str());

	return iIndex;
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
	g_mediaManager.GetLocalDrives(sources);

	CStdString strTmpPath = strPath;

	if(bFullPath)
	{
		int iPos = strTmpPath.Find(":\\");
		strTmpPath.Delete(iPos + 2, strTmpPath.size() - iPos);
	}

	for(int i = 0; i < (int)sources.size(); ++i)
	{
		CStdString strTmp = sources[i].strPath;	

		CUtil::RemoveSlashAtEnd(strTmp);
		CUtil::RemoveSlashAtEnd(strTmpPath);

		if(strTmp == strTmpPath)
			return true;
	}
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