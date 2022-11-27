#ifndef CMEDIAMANAGER_H
#define CMEDIAMANAGER_H

#include "Settings.h" // for VECSOURCES
#include "URL.h"

class CMediaManager
{
public:
	CMediaManager();

	void GetLocalDrives(VECSOURCES &localDrives, bool includeQ = true);
};

extern class CMediaManager g_mediaManager;

#endif //CMEDIAMANAGER_H