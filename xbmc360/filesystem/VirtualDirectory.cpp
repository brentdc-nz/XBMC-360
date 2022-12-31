//#include "system.h" // TODO
#include "VirtualDirectory.h"
//#include "DirectoryFactory.h"
#include "utils\Util.h"
//#include "settings\Profile.h" // TODO
#include "Directory.h"
//#include "DirectoryCache.h" 
//#include "storage\DetectDVDType.h" // TODO
#include "FileSystem\File.h"
#include "FileItem.h"
#include "utils\URIUtils.h"

using namespace XFILE;

namespace XFILE
{

CVirtualDirectory::CVirtualDirectory(void)
{
	m_allowPrompting = true; // By default, prompting is allowed.
	m_cacheDirectory = DIR_CACHE_ONCE; // By default, caching is done.
	m_allowNonLocalSources = true;
}

CVirtualDirectory::~CVirtualDirectory(void)
{
}

// Add shares to the virtual directory
// VECSOURCES Shares to add
// CMediaSource, VECSOURCES
void CVirtualDirectory::SetSources(const VECSOURCES& vecSources)
{
	m_vecSources = vecSources;
}

// Retrieve the shares or the content of a directory.
// param strPath Specifies the path of the directory to retrieve or pass an empty string to get the shares.
// param items Content of the directory.
// return Returns \e true, if directory access is successfull.
// note If \e strPath is an empty string, the share \e items have thumbnails and icons set, else the thumbnails
//  and icons have to be set manually.
bool CVirtualDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
	return GetDirectory(strPath,items,true);
}

bool CVirtualDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items, bool bUseFileDirectories)
{
	VECSOURCES shares;
	GetSources(shares);
	
	if (!strPath.IsEmpty() && strPath != "files://")
		return CDirectory::GetDirectory(strPath, items, m_strFileMask, bUseFileDirectories, m_allowPrompting, m_cacheDirectory, m_extFileInfo);

	// If strPath is blank, clear the list (to avoid parent items showing up)
	if (strPath.IsEmpty())
		items.Clear();

	// Return the root listing
	items.SetPath(strPath);

	// Grab our shares
	for (unsigned int i = 0; i < shares.size(); ++i)
	{
		CMediaSource& share = shares[i];
		CFileItemPtr pItem(new CFileItem(share));
		
//		if (pItem->IsLastFM() || pItem->IsShoutCast() || (pItem->GetPath().Left(14).Equals("musicsearch://"))) // TODO
//			pItem->SetCanQueue(false);
		
		CStdString strPathUpper = pItem->GetPath();
		strPathUpper.ToUpper();

		CStdString strIcon;
		// We have the real DVD-ROM, set icon on disktype
		if (share.m_iDriveType == CMediaSource::SOURCE_TYPE_DVD && share.m_strThumbnailImage.IsEmpty())
		{
//			CUtil::GetDVDDriveIcon( pItem->GetPath(), strIcon ); // TODO
			// CDetectDVDMedia::SetNewDVDShareUrl() caches disc thumb as special://temp/dvdicon.tbn
			CStdString strThumb = "special://temp/dvdicon.tbn";
			
			if (XFILE::CFile::Exists(strThumb))
				pItem->SetThumbnailImage(strThumb);
		}
/*		else if (strPathUpper.Left(11) == "SOUNDTRACK:") // TODO
			strIcon = "DefaultHardDisk.png";
		else if (pItem->IsLastFM()
			|| pItem->IsShoutCast()
			|| pItem->IsVideoDb()
			|| pItem->IsMusicDb()
			|| pItem->IsPlugin()
			|| pItem->IsPluginRoot()
			|| pItem->GetPath() == "special://musicplaylists/"
			|| pItem->GetPath() == "special://videoplaylists/"
			|| pItem->GetPath() == "musicsearch://")
		strIcon = "DefaultFolder.png";*/
		else if (pItem->IsRemote())
			strIcon = "DefaultNetwork.png";
/*		else if (pItem->IsISO9660())
			strIcon = "DefaultDVDRom.png";
		else if (pItem->IsDVD())
			strIcon = "DefaultDVDRom.png";
		else if (pItem->IsCDDA())
			strIcon = "DefaultCDDA.png";
*/		else
			strIcon = "DefaultHardDisk.png";

		pItem->SetIconImage(strIcon);
	
//		if (share.m_iHasLock == 2 && g_settings.GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE) // TODO
//			pItem->SetOverlayImage(CGUIListItem::ICON_OVERLAY_LOCKED);
//		else
			pItem->SetOverlayImage(CGUIListItem::ICON_OVERLAY_NONE);

		items.Add(pItem);
	}
	return true;
}

// Is the share \e strPath in the virtual directory.
// param strPath Share to test
// return Returns \e true, if share is in the virtual directory.
// note The parameter \e strPath can not be a share with directory. Eg. "iso9660://dir" will return \e false.
// It must be "iso9660://".
bool CVirtualDirectory::IsSource(const CStdString& strPath, VECSOURCES *sources, CStdString *name) const
{
	CStdString strPathCpy = strPath;
	strPathCpy.TrimRight("/");
	strPathCpy.TrimRight("\\");

	// Just to make sure there's no mixed slashing in share/default defines
	// ie. f:/video and f:\video was not be recognised as the same directory,
	// resulting in navigation to a lower directory then the share.
	if(URIUtils::IsDOSPath(strPathCpy))
		strPathCpy.Replace("/", "\\");

	VECSOURCES shares;
	if (sources)
		shares = *sources;
	else
		GetSources(shares);
	
	for (int i = 0; i < (int)shares.size(); ++i)
	{
		const CMediaSource& share = shares.at(i);
		CStdString strShare = share.strPath;
		strShare.TrimRight("/");
		strShare.TrimRight("\\");
		
		if(URIUtils::IsDOSPath(strShare))
			strShare.Replace("/", "\\");
		if (strShare == strPathCpy)
		{
			if (name)
				*name = share.strName;
			
			return true;
		}
	}
	return false;
}

// Is the share \e path in the virtual directory.
// param path Share to test
// return Returns \e true, if share is in the virtual directory.
// note The parameter \e path CAN be a share with directory. Eg. "iso9660://dir" will
// return the same as "iso9660://".
bool CVirtualDirectory::IsInSource(const CStdString &path) const
{
	bool isSourceName;
	VECSOURCES shares;
	GetSources(shares);
	
	int iShare = CUtil::GetMatchingSource(path, shares, isSourceName);
//	if (URIUtils::IsOnDVD(path)) // TODO
	{
		// Check to see if our share path is still available and of the same type, as it changes during autodetect
		// and GetMatchingSource() is too naive at it's matching
		for (unsigned int i = 0; i < shares.size(); i++)
		{
			CMediaSource &share = shares[i];
///			if (URIUtils::IsOnDVD(share.strPath) && share.strPath.Equals(path.Left(share.strPath.GetLength()))) // TODO
				return true;
		}
		return false;
	}
	// TODO: May need to handle other special cases that GetMatchingSource() fails on
	(iShare > -1);
}

void CVirtualDirectory::GetSources(VECSOURCES &shares) const
{
	shares = m_vecSources;

	// Add our plug n play shares
	if (m_allowNonLocalSources)
	{
	//	CUtil::AutoDetectionGetSource(shares); // TODO
	}

	// and update our dvd share
	for (unsigned int i = 0; i < shares.size(); ++i)
	{
		CMediaSource& share = shares[i];
		if (share.m_iDriveType == CMediaSource::SOURCE_TYPE_DVD)
		{
//			share.strStatus = MEDIA_DETECT::CDetectDVDMedia::GetDVDLabel(); // TODO
//			share.strPath = MEDIA_DETECT::CDetectDVDMedia::GetDVDPath(); // TODO
		}
	}
}
}