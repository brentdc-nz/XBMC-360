#include "MediaManager.h"
#include "utils\Log.h"
#include "guilib\LocalizeStrings.h"
#include "Application.h"

using namespace std;

const char MEDIA_SOURCES_XML[] = { "Q:\\system\\mediasources.xml" }; //FIXME - Path different on 360

class CMediaManager g_mediaManager;

CMediaManager::CMediaManager()
{
}

void CMediaManager::GetLocalDrives(VECSOURCES &localDrives, bool includeQ)
{
	// Get local mounted drives
	std::vector<CDrive* const> vecMountedDrives;
	g_application.getDriveManager().getMountedDrives(&vecMountedDrives);

	for(unsigned int i = 0; i < vecMountedDrives.size(); i++)
	{
		CMediaSource share;
		share.strPath = vecMountedDrives[i]->getRootPath();
		share.strName = "Drive ";//g_localizeStrings.Get(21438); // TODO
		share.strName += vecMountedDrives[i]->GetCleanDriveName();

		if(vecMountedDrives[i]->GetDriveType() == DEVICE_CDROM0)
			int j = 0;
		else
			share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;

		localDrives.push_back(share);
	}
}
