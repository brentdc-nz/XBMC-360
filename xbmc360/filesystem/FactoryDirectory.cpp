#include "FactoryDirectory.h"
#include "IFileDirectory.h"
#include "FactoryFileDirectory.h"
#include "..\URL.h"
#include "..\Application.h"

#include "HDDirectory.h"
#include "MultiPathDirectory.h"

using namespace DIRECTORY;

//	Create a IDirectory object of the share type specified in strPath
//	strPath Specifies the share type to access, can be a share or share with path
//	IDirectory object to access the directories on the share.

IDirectory* CFactoryDirectory::Create(const CStdString& strPath)
{
	CURL url(strPath);

	CFileItem item;
	IFileDirectory* pDir = CFactoryFileDirectory::Create(strPath, &item);

	if(pDir)
		return pDir;

	CStdString strProtocol = url.GetProtocol();
	if(strProtocol.size() == 0 || strProtocol == "file") return new CHDDirectory();
	if(strProtocol == "multipath") return new CMultiPathDirectory();
/*	if(strProtocol == "iso9660") return new CISO9660Directory();
	if(strProtocol == "cdda") return new CCDDADirectory();
	if(strProtocol == "soundtrack") return new CSndtrkDirectory();
	if(strProtocol == "zip") return new CZipDirectory();
	if(strProtocol == "rar") return new CRarDirectory();
	if(strProtocol == "virtualpath") return new CVirtualPathDirectory();
	if(strProtocol == "stack") return new CStackDirectory();
	if(strProtocol == "musicdb") return new CMusicDatabaseDirectory();
	if(strProtocol == "playlistmusic") return new CPlaylistDirectory();
	if(strProtocol == "playlistvideo") return new CPlaylistDirectory();
	if(strProtocol.Left(3) == "mem") return new CMemUnitDirectory();
*/
	if(g_application.getNetwork().IsAvailable())
	{
//		if(strProtocol == "smb") return new CSMBDirectory();
//		if(strProtocol == "daap") return new CDAAPDirectory();
//		if(strProtocol == "upnp") return new CUPnPDirectory();
//		if(strProtocol == "shout") return new CShoutcastDirectory();
//		if(strProtocol == "lastfm") return new CLastFMDirectory();
//		if(strProtocol == "xbms") return new CXBMSDirectory();
//		if(strProtocol == "ftp" || strProtocol == "ftpx") return new CFTPDirectory();
//		if(strProtocol == "rtv") return new CRTVDirectory();
	}

	return NULL;
}