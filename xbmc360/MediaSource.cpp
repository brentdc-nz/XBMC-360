#include "MediaSource.h"
//#include "settings\AdvancedSettings.h" // TODO
#include "utils\Util.h"
#include "utils\URIUtils.h"
#include "URL.h"
#include "FileSystem\MultiPathDirectory.h"

using namespace std;
using namespace XFILE;

bool CMediaSource::IsWritable() const
{
	return false;//return CUtil::SupportsWriteFileOperations(strPath); // TODO
}

void CMediaSource::FromNameAndPaths(const CStdString &category, const CStdString &name, const vector<CStdString> &paths)
{
	vecPaths = paths;
	
	if (paths.size() == 0)
	{
		// No paths - return
		strPath.Empty();
	}
	else if (paths.size() == 1)
	{
		// Only one valid path? make it the strPath
		strPath = paths[0];
	}
	else
	{
		// Multiple valid paths?
		strPath = CMultiPathDirectory::ConstructMultiPath(vecPaths);
	}

	strName = name;
//	m_iLockMode = LOCK_MODE_EVERYONE; // TODO: Locking
	m_strLockCode = "0";
	m_iBadPwdCount = 0;
	m_iHasLock = 0;

	if (URIUtils::IsMultiPath(strPath))
		m_iDriveType = SOURCE_TYPE_VPATH;
	else if (strPath.Left(4).Equals("udf:"))
	{
		m_iDriveType = SOURCE_TYPE_VIRTUAL_DVD;
		strPath = "D:\\";
	}
	else if (strPath.Left(11).Equals("soundtrack:"))
		m_iDriveType = SOURCE_TYPE_LOCAL;
//	else if (URIUtils::IsISO9660(strPath)) // TODO
//		m_iDriveType = SOURCE_TYPE_VIRTUAL_DVD;
//	else if (URIUtils::IsDVD(strPath))  // TODO
//		m_iDriveType = SOURCE_TYPE_DVD;
	else if (URIUtils::IsRemote(strPath))
		m_iDriveType = SOURCE_TYPE_REMOTE;
	else if (URIUtils::IsHD(strPath))
		m_iDriveType = SOURCE_TYPE_LOCAL;
	else
		m_iDriveType = SOURCE_TYPE_UNKNOWN;
	
	// Check - Convert to url and back again to make sure strPath is accurate
	// in terms of what we expect
	URIUtils::AddSlashAtEnd(strPath);
	strPath = CURL(strPath).Get();
}

bool CMediaSource::operator==(const CMediaSource &share) const
{
	// NOTE: We may wish to filter this through CURL to enable better "fuzzy" matching
	if (strPath != share.strPath)
		return false;
	
	if (strName != share.strName)
		return false;
	
	return true;
}

void AddOrReplace(VECSOURCES& sources, const CMediaSource& source)
{
	unsigned int i;
	
	for( i=0;i<sources.size();++i )
	{
		if (sources[i].strPath.Equals(source.strPath)) 
		{
			sources[i] = source;
			break;
		}
	}

	if (i == sources.size())
		sources.push_back(source);
}