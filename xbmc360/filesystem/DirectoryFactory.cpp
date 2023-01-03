//#include "system.h" // TODO
#include "DirectoryFactory.h"
#include "HDDirectory.h"
//#include "SpecialProtocolDirectory.h" // TODO
#include "MultiPathDirectory.h"
//#include "StackDirectory.h" // TODO
#include "FileDirectoryFactory.h"
//#include "PlaylistDirectory.h" // TODO
//#include "MusicDatabaseDirectory.h" // TODO
//#include "MusicSearchDirectory.h" // TODO
//#include "VideoDatabaseDirectory.h" // TODO
//#include "LastFMDirectory.h" // TODO
//#include "FTPDirectory.h" // TODO
//#include "HTTPDirectory.h" // TODO
//#include "DAVDirectory.h" // TODO
#include "Application.h"
#include "utils\Log.h"

#ifdef HAS_FILESYSTEM_SMB
#ifdef _WIN32PC
#include "WINSMBDirectory.h"
#else
#include "SMBDirectory.h"
#endif
#endif
#ifdef HAS_CCXSTREAM
#include "XBMSDirectory.h"
#endif
#ifdef HAS_FILESYSTEM_CDDA
#include "CDDADirectory.h"
#endif
//#include "PluginDirectory.h" // TODO
#include "SMBDirectory.h"
#ifdef HAS_FILESYSTEM
#include "ISO9660Directory.h"
#include "SMBDirectory.h"
#include "XBMSDirectory.h"
#include "CDDADirectory.h"
#include "RTVDirectory.h"
#include "SndtrkDirectory.h"
#include "DAAPDirectory.h"
#include "MemUnitDirectory.h"
#include "HTSPDirectory.h"
#endif
#ifdef HAS_UPNP
#include "UPnPDirectory.h"
#endif
#include "xbox/Network.h"
//#include "ZipDirectory.h" // TODO
//#include "RarDirectory.h" // TODO
//#include "TuxBoxDirectory.h" // TODO
//#include "HDHomeRunDirectory.h" // TODO
//#include "SlingboxDirectory.h" // TODO
//#include "MythDirectory.h" // TODO
#include "FileItem.h"
#include "URL.h"
//#include "RSSDirectory.h" // TODO

using namespace XFILE;

// Create a IDirectory object of the share type specified in \e strPath.
// param strPath Specifies the share type to access, can be a share or share with path.
// return IDirectory object to access the directories on the share.
// sa IDirectory
IDirectory* CFactoryDirectory::Create(const CStdString& strPath)
{
	CURL url(strPath);
	CFileItem item;

	IFileDirectory* pDir = CFactoryFileDirectory::Create(strPath, &item);

	if (pDir)
		return pDir;

	CStdString strProtocol = url.GetProtocol();

	if (strProtocol.size() == 0 || strProtocol == "file") return new CHDDirectory();
//	if (strProtocol == "special") return new CSpecialProtocolDirectory(); // TODO
#ifdef HAS_FILESYSTEM_CDDA
	if (strProtocol == "cdda") return new CCDDADirectory();
#endif
#ifdef HAS_FILESYSTEM
	if (strProtocol == "iso9660") return new CISO9660Directory();
	if (strProtocol == "cdda") return new CCDDADirectory();
	if (strProtocol == "soundtrack") return new CSndtrkDirectory();
#endif
//	if (strProtocol == "plugin") return new CPluginDirectory();// TODO
//	if (strProtocol == "zip") return new CZipDirectory();// TODO
//	if (strProtocol == "rar") return new CRarDirectory();// TODO
	if (strProtocol == "multipath") return new CMultiPathDirectory();
//	if (strProtocol == "stack") return new CStackDirectory();// TODO
//	if (strProtocol == "playlistmusic") return new CPlaylistDirectory();// TODO
//	if (strProtocol == "playlistvideo") return new CPlaylistDirectory();// TODO
//	if (strProtocol == "musicdb") return new CMusicDatabaseDirectory();// TODO
//	if (strProtocol == "musicsearch") return new CMusicSearchDirectory();// TODO
//	if (strProtocol == "videodb") return new CVideoDatabaseDirectory();// TODO
	if (strProtocol == "filereader") 
		return CFactoryDirectory::Create(url.GetFileName());
#ifdef HAS_XBOX_HARDWARE
	if (strProtocol.Left(3) == "mem") return new CMemUnitDirectory();
#endif

	if( g_application.getNetwork().IsAvailable(true) )
	{
//		if (strProtocol == "lastfm") return new CLastFMDirectory();// TODO
//		if (strProtocol == "tuxbox") return new CDirectoryTuxBox();// TODO
//		if (strProtocol == "ftp" ||  strProtocol == "ftpx" ||  strProtocol == "ftps") return new CFTPDirectory();// TODO
//		if (strProtocol == "http" || strProtocol == "https") return new CHTTPDirectory();// TODO
//		if (strProtocol == "dav" || strProtocol == "davs") return new CDAVDirectory();// TODO
		if (strProtocol == "smb") return new CSMBDirectory();
#ifdef HAS_FILESYSTEM
		if (strProtocol == "smb") return new CSMBDirectory();
		if (strProtocol == "daap") return new CDAAPDirectory();
		if (strProtocol == "xbms") return new CXBMSDirectory();
		if (strProtocol == "rtv") return new CRTVDirectory();
		if (strProtocol == "htsp") return new CHTSPDirectory();
#endif
#ifdef HAS_UPNP
		if (strProtocol == "upnp") return new CUPnPDirectory();
#endif
//		if (strProtocol == "hdhomerun") return new CHomeRunDirectory();// TODO
//		if (strProtocol == "sling") return new CSlingboxDirectory();// TODO
//		if (strProtocol == "myth") return new CMythDirectory();// TODO
//		if (strProtocol == "cmyth") return new CMythDirectory();// TODO
//		if (strProtocol == "rss") return new CRSSDirectory();// TODO
	}

	CLog::Log(LOGWARNING, "%s - Unsupported protocol(%s) in %s", __FUNCTION__, strProtocol.c_str(), url.Get().c_str() );
	return NULL;
}