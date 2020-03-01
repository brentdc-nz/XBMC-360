#include "FileHD.h"
#include "..\utils\Stdafx.h"

using namespace XFILE;

CFileHD::CFileHD()
{
}

CFileHD::~CFileHD()
{
}

bool CFileHD::Exists(const CStdString& strPath)
{
	if (strPath.size()==0) return false;

	FILE *fd;
	fd = fopen(strPath.c_str(), "rb");

	if (fd != NULL)
	{
		fclose(fd);
		return true;
	}

	return false;
}