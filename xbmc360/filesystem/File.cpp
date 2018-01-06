#include "File.h"
#include "..\utils\Log.h"

#include "FileHD.h"

using namespace XFILE;

bool CFile::Exists(const CStdString& strFileName)
{
	if (strFileName.IsEmpty()) return false;
		
	//
	// TODO - Check other future protocols i.e. ftp, http, etc
	//

	//Check HDD
	if(CFileHD::Exists(strFileName)) return true;

	CLog::Log(LOGERROR, __FUNCTION__" - Unhandled exception checking %s", strFileName.c_str());
	return false;
}