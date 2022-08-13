#ifndef H_MEDIAMANAGER
#define H_MEDIAMANAGER

#include "utils\StdString.h"

class CMediaSource
{
public:
	enum eSourceType
	{
		SOURCE_TYPE_UNKNOWN      = 0,
		SOURCE_TYPE_LOCAL        = 1,
		SOURCE_TYPE_DVD          = 2,
	};
	CMediaSource()
	{
		m_iDriveType = SOURCE_TYPE_UNKNOWN;
	};
	virtual ~CMediaSource()
	{
	};

	void FromNameAndPaths(const CStdString &category, const CStdString &name, const std::vector<CStdString> &paths);

	CStdString strName; // Name of the share, can be choosen freely
	CStdString strPath; // Path of the share, eg. iso9660:// or F:
	CStdString m_strThumbnailImage;
	std::vector<CStdString> vecPaths;
	eSourceType m_iDriveType;
};

typedef std::vector<CMediaSource> VECSOURCES;
typedef std::vector<CMediaSource>::iterator IVECSOURCES;

//======================================================

class CNetworkLocation
{
public:
	CNetworkLocation() { id = 0; };
	int id;
	CStdString strPath;
};

//======================================================

class CMediaManager
{
public:
	CMediaManager();

	void GetLocalDrives(VECSOURCES &localDrives);
};

extern class CMediaManager g_mediaManager;

#endif //H_MEDIAMANAGER