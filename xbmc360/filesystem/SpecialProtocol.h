#ifndef H_SPECIALPROTOCOL
#define H_SPECIALPROTOCOL

#include <map>
#include "utils\StdString.h"

class CURL;

/* paths are as follows:

 special://xbmc/          - The main XBMC folder (i.e. where the app resides).
 special://home/          - A writeable version of the main XBMC folder
                             Linux: ~/.xbmc/
                             OS X:  ~/Library/Application Support/XBMC/
                             Win32: FIXME: Currently the same as special://home/
 special://userhome/      - A writable version of the user home directory
                             Linux, OS X: ~/.xbmc
                             Win32: home directory of user                             
 special://masterprofile/ - The master users userdata folder - usually special://home/userdata
                             Linux: ~/.xbmc/userdata/
                             OS X:  ~/Library/Application Support/XBMC/UserData/
                             Win32: ~/Application Data/XBMC/UserData/
 special://profile/       - The current users userdata folder - usually special://masterprofile/profiles/<current_profile>
                             Linux: ~/.xbmc/userdata/profiles/<current_profile>
                             OS X:  ~/Library/Application Support/XBMC/UserData/profiles/<current_profile>
                             Win32: ~/Application Data/XBMC/UserData/profiles/<current_profile>

 special://temp/          - The temporary directory.
                             Linux: ~/tmp/xbmc<username>
                             OS X:  ~/
                             Win32: ~/Application Data/XBMC/cache
*/

// Static class for path translation from our special:// URLs.
class CSpecialProtocol
{
public:
	static void SetProfilePath(const CStdString &path);
	static void SetXBMCPath(const CStdString &dir);
	static void SetHomePath(const CStdString &dir);
	static void SetUserHomePath(const CStdString &dir);
	static void SetMasterProfilePath(const CStdString &dir);
	static void SetTempPath(const CStdString &dir);

	static bool ComparePath(const CStdString &path1, const CStdString &path2);
	static bool XBMCIsHome();
	static void LogPaths();

	static CStdString TranslatePath(const CStdString &path);
	static CStdString TranslatePath(const CURL &url);

	static const int path_version = 1;

private:
	static void SetPath(const CStdString &key, const CStdString &path);
	static CStdString GetPath(const CStdString &key);

	static std::map<CStdString, CStdString> m_pathMap;
};

#define _P(x)     CSpecialProtocol::TranslatePath(x)

#endif // H_SPECIALPROTOCOL
