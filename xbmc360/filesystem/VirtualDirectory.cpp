#include "VirtualDirectory.h"
#include "Directory.h"
#include "..\utils\log.h"
#include "..\utils\util.h"
#include "..\MediaManager.h"

using namespace DIRECTORY;

CVirtualDirectory::CVirtualDirectory(void)
{
}

CVirtualDirectory::~CVirtualDirectory(void)
{
}

bool CVirtualDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
	return GetDirectory(strPath, items, true);
}

bool CVirtualDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items, bool bUseFileDirectories)
{
	if(!m_vecShares)
	{
		items.SetPath(strPath);
		return true;
	}

	CStdString strPath2 = strPath;
	CStdString strPath3 = strPath;
	strPath2 += "/";
	strPath3 += "\\";

	VECSOURCES shares;
	GetShares(shares);

	if(!strPath.IsEmpty()) //TODO
	{
		bool bIsLoalDrive = false;
		bool bIsBookmarkName = false;

		bIsLoalDrive = CUtil::IsLocalDrive(strPath, true);	
		int iIndex = CUtil::GetMatchingShare(strPath, shares, bIsBookmarkName);

		// Added exception for various local hd items
		// function doesn't work for http/shout streams with options
		if(iIndex > -1 || (bIsLoalDrive == true) 
		|| strPath.Left(8).Equals("shout://") 
		|| strPath.Left(8).Equals("https://") 
		|| strPath.Left(7).Equals("http://") 
		|| strPath.Left(7).Equals("daap://") 
		|| strPath.Left(7).Equals("upnp://"))
		{
			return CDirectory::GetDirectory(strPath, items, m_strFileMask, bUseFileDirectories, m_allowPrompting, m_cacheDirectory);
		}

		// What do with an invalid path?
		// return false so the calling window can deal with the error accordingly
		// otherwise the root bookmark listing is returned which seems incorrect but was the previous behaviour
		CLog::Log(LOGERROR,"CVirtualDirectory::GetDirectory(%s) matches no valid bookmark, getting root bookmark list instead", strPath.c_str());
		return false;
	}

	// If strPath is blank, return the root bookmark listing
	items.Clear();
	items.SetPath(strPath);

	// Grab our shares
	for(unsigned int i = 0; i < shares.size(); ++i)
	{
		CMediaSource& share = shares[i];
		CFileItem* pItem = new CFileItem(share);
		CStdString strPathUpper = pItem->GetPath();
		strPathUpper.ToUpper();

		CStdString strIcon;// = share.m_strThumbnailImage; //TODO
/*		if(share.m_strThumbnailImage.IsEmpty())
*/		{
			// We have the real DVD-ROM, set icon on disktype
/*			if(share.m_iDriveType == CMediaSource::SOURCE_TYPE_DVD)
				CUtil::GetDVDDriveIcon(pItem->m_strPath, strIcon);
			else if(pItem->IsRemote())
				strIcon = "defaultNetwork.png";
			else if(pItem->IsISO9660())
				strIcon = "defaultDVDRom.png";
			else if(pItem->IsDVD())
				strIcon = "defaultDVDRom.png";
			else if(pItem->IsCDDA())
				strIcon = "defaultCDDA.png";
			else
*/				strIcon = "defaultHardDisk.png";
		}

		pItem->SetIconImage(strIcon);

		items.Add(pItem);
	}
	return true;
}

void CVirtualDirectory::SetShares(VECSOURCES& vecShares)
{
	m_vecShares = &vecShares;
}

void CVirtualDirectory::GetShares(VECSOURCES& shares) const
{
	shares = *m_vecShares;

	//TODO
}

//	Is the share strPath in the virtual directory
bool CVirtualDirectory::IsShare(const CStdString& strPath) const
{
	CStdString strPathCpy = strPath;
	strPathCpy.TrimRight("/");
	strPathCpy.TrimRight("\\");

	// Just to make sure there's no mixed slashing in share/default defines
	// ie. f:/video and f:\video was not be recognised as the same directory,
	// resulting in navigation to a lower directory then the share.
	strPathCpy.Replace("/", "\\");

	VECSOURCES shares;
	GetShares(shares);
	
	for(int i = 0; i < (int)shares.size(); ++i)
	{
		const CMediaSource& share = shares.at(i);
		CStdString strShare = share.strPath;
		strShare.TrimRight("/");
		strShare.TrimRight("\\");
		strShare.Replace("/", "\\");

		if(strShare == strPathCpy)
			return true;
	}
	return false;
}