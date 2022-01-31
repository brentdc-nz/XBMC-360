#include "MediaManager.h"
#include "Application.h"
#include "guilib\LocalizeStrings.h"
#include "URL.h"
#include "filesystem\MultiPathDirectory.h"

class CMediaManager g_mediaManager;

void CMediaSource::FromNameAndPaths(const CStdString &category, const CStdString &name, const std::vector<CStdString> &paths)
{
	vecPaths = paths;
	if(paths.size() == 0)
	{
		// No paths
		strPath.Empty();
	}
	else if(paths.size() == 1)
	{
		// Only one valid path? Make it the strPath
		strPath = paths[0];
	}
	else
	{
		// Multiple valid paths?
#if 1//def MULTIPATH
		// use new multipath:// protocol
		DIRECTORY::CMultiPathDirectory dir;
		strPath = dir.ConstructMultiPath(vecPaths);
#else
		// Use older virtualpath:// protocol
		strPath.Format("virtualpath://%s/%s", category.c_str(), name.c_str());
#endif
	}

	strName = name;
/*	m_iBufferSize = 0;

	if(CUtil::IsVirtualPath(strPath) || CUtil::IsMultiPath(strPath))
		m_iDriveType = SHARE_TYPE_VPATH;
	else if(strPath.Left(4).Equals("udf:"))
	{
		m_iDriveType = SHARE_TYPE_VIRTUAL_DVD;
		strPath = "D:\\";
	}
	else if(strPath.Left(11).Equals("soundtrack:"))
		m_iDriveType = SHARE_TYPE_LOCAL;
	else if(CUtil::IsISO9660(strPath))
		m_iDriveType = SHARE_TYPE_VIRTUAL_DVD;
	else if(CUtil::IsDVD(strPath))
		m_iDriveType = SHARE_TYPE_DVD;
	else if(CUtil::IsRemote(strPath))
		m_iDriveType = SHARE_TYPE_REMOTE;
	else if(CUtil::IsHD(strPath))
		m_iDriveType = SHARE_TYPE_LOCAL;
	else
		m_iDriveType = SHARE_TYPE_UNKNOWN;
*/
	// Check: Convert to url and back again to make sure strPath is accurate
	// in terms of what we expect
	CURL url(strPath);
	url.GetURL(strPath);
}

//======================================================

CMediaManager::CMediaManager()
{
}

void CMediaManager::GetLocalDrives(VECSOURCES &localDrives)
{
	// Get local mounted drives
	std::vector<CDrive* const> vecMountedDrives;
	g_application.getDriveManager().getMountedDrives(&vecMountedDrives);

	for(unsigned int i = 0; i < vecMountedDrives.size(); i++)
	{
		CMediaSource share;
		share.strPath = vecMountedDrives[i]->getRootPath();
//		share.strName = "Drive ";//g_localizeStrings.Get(21438);
		share.strName += vecMountedDrives[i]->GetCleanDriveName();

		if(vecMountedDrives[i]->GetDriveType() == DEVICE_CDROM0)
			int j = 0;
		else
			share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;

		localDrives.push_back(share);
	}

/*
  // Local shares
  CMediaSource share;
  share.strPath = "C:\\";
  share.strName.Format(g_localizeStrings.Get(21438),'C');
  share.m_ignore = true;
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  localDrives.push_back(share);


  share.strPath = "D:\\";
  share.strName = g_localizeStrings.Get(218);
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_DVD;
  localDrives.push_back(share);


  share.strPath = "E:\\";
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  share.strName.Format(g_localizeStrings.Get(21438),'E');
  localDrives.push_back(share);


  for (int driveCount=EXTEND_PARTITION_BEGIN;driveCount<=(EXTEND_PARTITION_BEGIN+EXTEND_PARTITIONS_LIMIT-1);driveCount++)
  {
    if (CIoSupport::DriveExists(CIoSupport::GetExtendedPartitionDriveLetter(driveCount-EXTEND_PARTITION_BEGIN)))
    {
      CMediaSource share;
      share.strPath.Format("%c:\\", CIoSupport::GetExtendedPartitionDriveLetter(driveCount-EXTEND_PARTITION_BEGIN));
      CLog::Log(LOGNOTICE, "  Local Source Drive %c:", CIoSupport::GetExtendedPartitionDriveLetter(driveCount-EXTEND_PARTITION_BEGIN));
      share.strName.Format(g_localizeStrings.Get(21438),CIoSupport::GetExtendedPartitionDriveLetter(driveCount-EXTEND_PARTITION_BEGIN));
      share.m_ignore = true;
      localDrives.push_back(share);
    }
  }
  if (includeQ)
  {
    CMediaSource share;
    share.strPath = "Q:\\";
    share.strName.Format(g_localizeStrings.Get(21438),'Q');
    share.m_ignore = true;
    localDrives.push_back(share);
  }*/

}