#include "URIUtils.h"
#include "Util.h"
#include "URL.h"
#include "filesystem\MultiPathDirectory.h"
#include "GUISettings.h"

using namespace std;
using namespace XFILE;

void URIUtils::GetDirectory(const CStdString& strFilePath, CStdString& strDirectoryPath)
{
	// Will from a full filename return the directory the file resides in.
	// Keeps the final slash at end

	int iPos1 = strFilePath.ReverseFind('/');
	int iPos2 = strFilePath.ReverseFind('\\');

	if (iPos2 > iPos1)
	{
		iPos1 = iPos2;
	}

	if (iPos1 > 0)
	{
		strDirectoryPath = strFilePath.Left(iPos1 + 1); // Include the slash
	}
}

// Returns filename extension including period of filename
const CStdString URIUtils::GetExtension(const CStdString& strFileName)
{
	if(IsURL(strFileName))
	{
		CURL url(strFileName);
		return GetExtension(url.GetFileName());
	}

	int period = strFileName.find_last_of('.');

	if(period >= 0)
	{
		if( strFileName.find_first_of('/', period+1) != string::npos ) return "";
		if( strFileName.find_first_of('\\', period+1) != string::npos ) return "";

		return strFileName.substr(period);
	}
	else
		return "";
}

void URIUtils::GetExtension(const CStdString& strFile, CStdString& strExtension)
{
	strExtension = GetExtension(strFile);
}

bool URIUtils::IsDOSPath(const CStdString &path)
{
	if(path.size() > 1 && path[1] == ':' && isalpha(path[0]))
		return true;

	// BDC - Xbox 360 is different e.g.
	//       Need a more elegant to detect
	std::string strTestPath = path;
	if (strTestPath.find("Hdd1:") != std::string::npos)
		return true;
	else if (strTestPath.find("HddX:") != std::string::npos)
		return true;
	else if (strTestPath.find("Flash:") != std::string::npos)
		return true;

	return false;
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
*/
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

// Returns a filename given an url
// Handles both / and \, and options in urls
const CStdString URIUtils::GetFileName(const CStdString& strFileNameAndPath)
{
	if(IsURL(strFileNameAndPath))
	{
		CURL url(strFileNameAndPath);
		return GetFileName(url.GetFileName());
	}

	// Find any slashes
	const int slash1 = strFileNameAndPath.find_last_of('/');
	const int slash2 = strFileNameAndPath.find_last_of('\\');

	// Select the last one
	int slash;

	if(slash2>slash1)
		slash = slash2;
	else
		slash = slash1;

	return strFileNameAndPath.substr(slash+1);
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

void URIUtils::RemoveSlashAtEnd(CStdString& strFolder)
{
	if (IsURL(strFolder))
	{
		CURL url(strFolder);
		CStdString file = url.GetFileName();
		
		if (!file.IsEmpty() && file != strFolder)
		{
			RemoveSlashAtEnd(file);
			url.SetFileName(file);
			strFolder = url.Get();
			return;
		}
		if(url.GetHostName().IsEmpty())
			return;
	}

	while (HasSlashAtEnd(strFolder))
		strFolder.Delete(strFolder.size() - 1);
}

void URIUtils::RemoveExtension(CStdString& strFileName)
{
	if(IsURL(strFileName))
	{
		CURL url(strFileName);
		strFileName = url.GetFileName();
		RemoveExtension(strFileName);
		url.SetFileName(strFileName);
		strFileName = url.Get();
		return;
	}

	int iPos = strFileName.ReverseFind(".");
	
	// Extension found
	if (iPos > 0)
	{
		CStdString strExtension;
		GetExtension(strFileName, strExtension);
		strExtension.ToLower();
		strExtension += "|";

		CStdString strFileMask;
		strFileMask = g_settings.GetPictureExtensions();
		strFileMask += "|" + g_settings.GetAudioExtensions();
		strFileMask += "|" + g_settings.GetVideoExtensions();
		strFileMask += "|.py|.xml|.milk|.xpr|.cdg";
		strFileMask += "|";

		if (strFileMask.Find(strExtension) >= 0)
			strFileName = strFileName.Left(iPos);
	}
}

CStdString URIUtils::GetParentPath(const CStdString& strPath)
{
	CStdString strReturn;
	GetParentPath(strPath, strReturn);
	return strReturn;
}

bool URIUtils::GetParentPath(const CStdString& strPath, CStdString& strParent)	// TODO - Remove Zip/Rar stuff
{
	strParent = "";

	CURL url(strPath);
	CStdString strFile = url.GetFileName();

	if ( ((url.GetProtocol() == "rar") || (url.GetProtocol() == "zip")) && strFile.IsEmpty())
	{
		strFile = url.GetHostName();
		return GetParentPath(strFile, strParent);
	}
	else if (url.GetProtocol() == "stack")	// TODO
	{
/*		CStackDirectory dir;
		CFileItemList items;
		dir.GetDirectory(strPath,items);
		GetDirectory(items[0]->GetPath(),items[0]->m_strDVDLabel);
		
		if (items[0]->m_strDVDLabel.Mid(0,6).Equals("rar://") || items[0]->m_strDVDLabel.Mid(0,6).Equals("zip://"))
			GetParentPath(items[0]->m_strDVDLabel, strParent);
		else
			strParent = items[0]->m_strDVDLabel;
		
		for( int i=1;i<items.Size();++i)
		{
			GetDirectory(items[i]->GetPath(),items[i]->m_strDVDLabel);
			
			if (items[0]->m_strDVDLabel.Mid(0,6).Equals("rar://") || items[0]->m_strDVDLabel.Mid(0,6).Equals("zip://"))
				items[i]->SetPath(GetParentPath(items[i]->m_strDVDLabel));
			else
				items[i]->SetPath(items[i]->m_strDVDLabel);

			GetCommonPath(strParent,items[i]->GetPath());
		}
*/		return true;
	}
	else if (url.GetProtocol() == "multipath")
	{
		// Get the parent path of the first item
		return GetParentPath(CMultiPathDirectory::GetFirstPath(strPath), strParent);
	}
	else if (url.GetProtocol() == "plugin")
	{
		if (!url.GetOptions().IsEmpty())
		{
			url.SetOptions("");
			strParent = url.Get();
			return true;
		}

		if (!url.GetFileName().IsEmpty())
		{
			url.SetFileName("");
			strParent = url.Get();
			return true;
		}
		
		if (!url.GetHostName().IsEmpty())
		{
			url.SetHostName("");
			strParent = url.Get();
			return true;
		}

		return true;  // Already at root
	}
	else if (url.GetProtocol() == "special")
	{
		if (HasSlashAtEnd(strFile) )
			strFile = strFile.Left(strFile.size() - 1);
		
		if(strFile.ReverseFind('/') < 0)
			return false;
	}
	else if (strFile.size() == 0)
	{
		if (url.GetHostName().size() > 0)
		{
			// We have an share with only server or workgroup name
			// set hostname to "" and return true to get back to root
			url.SetHostName("");
			strParent = url.Get();
			return true;
		}
		return false;
	}

	if (HasSlashAtEnd(strFile) )
	{
		strFile = strFile.Left(strFile.size() - 1);
	}

	int iPos = strFile.ReverseFind('/');

	if (iPos < 0)
	{
		iPos = strFile.ReverseFind('\\');
	}

	if (iPos < 0)
	{
		url.SetFileName("");
		strParent = url.Get();
		return true;
	}

	strFile = strFile.Left(iPos);

	AddSlashAtEnd(strFile);

	url.SetFileName(strFile);
	strParent = url.Get();

	return true;
}

void URIUtils::AddSlashAtEnd(CStdString& strFolder)
{
	if (IsURL(strFolder))
	{
		CURL url(strFolder);
		CStdString file = url.GetFileName();
		
		if(!file.IsEmpty() && file != strFolder)
		{
			AddSlashAtEnd(file);
			url.SetFileName(file);
			strFolder = url.Get();
		}
		return;
	}

	if (!HasSlashAtEnd(strFolder))
	{
		if (IsDOSPath(strFolder))
			strFolder += '\\';
		else
			strFolder += '/';
	}
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