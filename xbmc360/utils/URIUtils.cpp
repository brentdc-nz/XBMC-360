#include "URIUtils.h"
#include "Util.h"
#include "URL.h"
#include "filesystem\MultiPathDirectory.h"

using namespace std;
using namespace DIRECTORY;

bool URIUtils::IsDOSPath(const CStdString &path)
{
	if(path.size() > 1 && path[1] == ':' && isalpha(path[0]))
		return true;

	return false;
}

void URIUtils::AddSlashAtEnd(CStdString& strFolder)
{
/*	if(IsURL(strFolder))
	{
		CURL url(strFolder);
		CStdString file = url.GetFileName();
		if(!file.IsEmpty() && file != strFolder)
		{
			AddSlashAtEnd(file);
			url.SetFileName(file);
			strFolder = url.Get();
			return;
		}
	}
*/
	if(!HasSlashAtEnd(strFolder))
	{
		if(CUtil::IsLocalDrive(strFolder, true))
			strFolder += '\\';
		else if(IsDOSPath(strFolder))
			strFolder += '\\';
		else
			strFolder += '/';
	}
}

bool URIUtils::IsURL(const CStdString& strFile)
{
	return strFile.Find("://") >= 0;
}

bool URIUtils::IsRemote(const CStdString& strFile) //TODO: finish other type checks
{
/*	if(IsMemCard(strFile) || IsCDDA(strFile) || IsISO9660(strFile) || IsPlugin(strFile) || IsMusicDb(strFile) || IsVideoDb(strFile))
		return false;

	if(IsSpecial(strFile))
		return IsRemote(CSpecialProtocol::TranslatePath(strFile));

	if(IsStack(strFile))
		return IsRemote(CStackDirectory::GetFirstStackedFile(strFile));
*/
	if(IsMultiPath(strFile))
	{
		// Virtual paths need to be checked separately
		vector<CStdString> paths;
		if(CMultiPathDirectory::GetPaths(strFile, paths))
		{
			for(unsigned int i = 0; i < paths.size(); i++)
				if(IsRemote(paths[i])) return true;
		}
		return false;
	}

	CURL url(strFile);
/*	
	if(IsInArchive(strFile))
		return IsRemote(url.GetHostName());
*/
	if(!url.IsLocal())
		return true;

	return false;
}

bool URIUtils::IsMultiPath(const CStdString& strPath)
{
	return strPath.Left(10).Equals("multipath:");
}

bool URIUtils::IsHD(const CStdString& strFileName)
{
	CURL url(strFileName);

	return url.IsLocal();
}

bool URIUtils::HasSlashAtEnd(const CStdString& strFile)
{
	if(strFile.size() == 0)
		return false;
	
	char kar = strFile.c_str()[strFile.size() - 1];

	if(kar == '/' || kar == '\\')
		return true;

	return false;
}

void URIUtils::AddFileToFolder(const CStdString& strFolder, const CStdString& strFile, CStdString& strResult)
{
	if(IsURL(strFolder))
	{
		CURL url(strFolder);
		if(url.GetFileName() != strFolder)
		{
			AddFileToFolder(url.GetFileName(), strFile, strResult);
			url.SetFileName(strResult);
			strResult = url.Get();
			return;
		}
	}

	strResult = strFolder;
	if(!strResult.IsEmpty())
		AddSlashAtEnd(strResult);

	// Remove any slash at the start of the file
	if(strFile.size() && (strFile[0] == '/' || strFile[0] == '\\'))
		strResult += strFile.Mid(1);
	else
		strResult += strFile;

	// Correct any slash directions
	if(!IsDOSPath(strFolder))
		strResult.Replace('\\', '/');
	else
		strResult.Replace('/', '\\');
}