#include "playlists/PlayListFactory.h"
#include "utils/URIUtils.h"
#include "FileItem.h"

using namespace PLAYLIST;

CPlayList* CPlayListFactory::Create(const CStdString& filename)
{
	CFileItem item(filename,false);
	return Create(item);
}

CPlayList* CPlayListFactory::Create(const CFileItem& item)
{
	return NULL;
}

bool CPlayListFactory::IsPlaylist(const CFileItem& item)
{
	CStdString strMimeType = "";//item.GetMimeType(); //TODO BRENT
//	strMimeType.ToLower();

/* These are abit uncertain 
	if(strMimeType == "video/x-ms-asf"
	|| strMimeType == "video/x-ms-asx"
	|| strMimeType == "video/x-ms-wmv"
	|| strMimeType == "video/x-ms-wma"
	|| strMimeType == "video/x-ms-wfs"
	|| strMimeType == "video/x-ms-wvx"
	|| strMimeType == "video/x-ms-wax"
	|| strMimeType == "video/x-ms-asf")
	return true;
*/

	// Online m3u8 files are hls:// -- do not treat as playlist
//	if (item.IsInternetStream() && item.IsType(".m3u8")) //TODO : BRENT
//		return false;

	if(strMimeType == "audio/x-pn-realaudio"
	|| strMimeType == "playlist"
	|| strMimeType == "audio/x-mpegurl")
	return true;

	return IsPlaylist(item.GetPath());
}

bool CPlayListFactory::IsPlaylist(const CStdString& filename)
{
	CStdString extension = URIUtils::GetExtension(filename);
	extension.ToLower();

	if (extension == ".m3u") return true;
	if (extension == ".b4s") return true;
	if (extension == ".pls") return true;
	if (extension == ".strm") return true;
	if (extension == ".wpl") return true;
	if (extension == ".asx") return true;
	if (extension == ".ram") return true;
	if (extension == ".url") return true;
	if (extension == ".pxml") return true;

	return false;
}

