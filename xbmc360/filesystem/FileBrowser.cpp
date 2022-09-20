#include "FileBrowser.h"
#include "utils\log.h"
#include "Application.h"
#include "utils\StringUtils.h"
#include "utils\Util.h"
#include <map>
#include <sys/stat.h>


CFileBrowser::CFileBrowser()
{
}

CFileBrowser::~CFileBrowser()
{
}

CStdString CFileBrowser::GetCurrentPath()
{
	CStdString strRetVal = "";
	bool bSupress = false;

	for(unsigned int x = 0; x < m_vecCurrentFolder.size(); x++)
	{
		if(x != 0)
			strRetVal.append("\\");

		strRetVal = strRetVal + m_vecCurrentFolder.at(x);		
	}

	return strRetVal;
}

bool CFileBrowser::IsAtRoot()
{
	if(m_vecCurrentFolder.size() == 0)
		return true;
	else
		return false;
}

void CFileBrowser::UpToRoot()
{
	m_vecCurrentFolder.clear();
}

void CFileBrowser::UpDirectory()
{
	if(m_vecCurrentFolder.size() > 0)
		m_vecCurrentFolder.pop_back();
}

CStdString CFileBrowser::GetWriteFilePath(CStdString strFile)
{
	CStdString strFullPath = GetCurrentPath()+"\\" + strFile;
	return strFullPath;
}

FILE* CFileBrowser::OpenFile(CStdString strFile)
{
	FILE* pRetFile;
	string fullPath = GetCurrentPath()+"\\" + strFile;
	fopen_s(&pRetFile, fullPath.c_str(), "rb");
	
	return pRetFile;
}

std::vector<CStdString> CFileBrowser::GetFolderList()
{
	std::vector<CStdString> vecRetVal;

	if(m_vecCurrentFolder.size() == 0)
	{
		// List Drives
		vecRetVal.push_back("Game:");

		std::vector<CDrive* const> vecMountedDrives;
		g_application.getDriveManager().getMountedDrives(&vecMountedDrives);

		for(unsigned int x = 0; x < vecMountedDrives.size(); x++)
			vecRetVal.push_back(vecMountedDrives[x]->GetCleanDriveName() + ":");

		return vecRetVal;
	}
	else
	{
		vecRetVal.push_back("..");

		CStdString strPath = GetCurrentPath();

		WIN32_FIND_DATA findFileData;
		memset(&findFileData, 0, sizeof(WIN32_FIND_DATA));
		CStdString strSearchCmd = GetCurrentPath() + "\\*";
	
		HANDLE hFind = FindFirstFile(strSearchCmd.c_str(), &findFileData);
		if(hFind == INVALID_HANDLE_VALUE)
			return vecRetVal;
		do{
			string str = findFileData.cFileName;
			
			if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				vecRetVal.push_back(str);

		}while(FindNextFile(hFind, &findFileData));

		FindClose(hFind);
		return vecRetVal;
	}
}

CStdString CFileBrowser::GetFolderFTPLongDescription(CStdString strFolder)
{
	CStdString strFullPath = "";

	if(m_vecCurrentFolder.size() == 0)
		strFullPath = strFolder + ":\\";
	else
		strFullPath = GetCurrentPath() + "\\" + strFolder;

	if(CFile::Exists(strFullPath))
	{
		DWORD dwAttr = GetFileAttributes(strFullPath.c_str());
		char execute = 'x';
		char read = 'r';
		char write = 'w';
		char directory = '-';

		if(dwAttr & FILE_ATTRIBUTE_DIRECTORY)
			directory = 'd';

		if(dwAttr & FILE_ATTRIBUTE_ARCHIVE )
			execute = '-';

		if(dwAttr & FILE_ATTRIBUTE_READONLY  )
			write = '-';

		if(dwAttr & FILE_ATTRIBUTE_DEVICE   )
			read = '-';

		struct stat statDirStatus;
		stat(strFullPath.c_str(), &statDirStatus);
		char timeStr[100] = "";
		struct tm locTime;
		
		int iRetVal = localtime_s(&locTime, &statDirStatus.st_mtime);
		
		if(iRetVal)
			iRetVal = localtime_s(&locTime,  &statDirStatus.st_atime);
	
		if(iRetVal)
			iRetVal = localtime_s( &locTime, &statDirStatus.st_ctime);

		if(iRetVal)
		{
			time_t t;
			time(&t);
			iRetVal = localtime_s(&locTime, &t);
		}
		
		strftime(timeStr, 100, "%b %d %Y", &locTime);

		return CStringUtils::sprintfa("%c%c%c%c%c%c%c%c%c%c   1 root  root    %d %s %s\r\n", directory, 
			read, write, execute, read, write, execute, read, write, execute, 0, timeStr, strFolder.c_str());
	}
	else
		return CStringUtils::sprintfa("%s\r\n", strFolder.c_str());
}

void CFileBrowser::CD(CStdString strFolder)
{
	// Mattie: quickfix going up to root... ftp '/' is converted to ':'
	if(strcmp(strFolder.c_str(), ":") == 0)
	{
		UpToRoot();
		return;
	}
	
	if(strcmp(strFolder.c_str(), "..") == 0)
		UpDirectory();
	else
	{
		if(strFolder.c_str()[0] == '/')
		{
			UpToRoot();
			vector<CStdString> vecSplits;
			CStringUtils::StringSplit(strFolder, "/", &vecSplits);
			
			for(unsigned int x = 0; x < vecSplits.size(); x++)
			{
				CStdString f = vecSplits.at(x);
				if(m_vecCurrentFolder.size() == 0)
				{
					if(f.c_str()[f.size()-1] != ':')
						f = f + ":";
				}
				m_vecCurrentFolder.push_back(f);
			}
		}
		else
		{
			// Mattie: quickfix: dont allow folders that dont exist, cut trailing backslash
			// if it ends in :, add \  ie.  "hdd1: -> hdd1:\"
			if(strFolder.substr(strFolder.length() - 1, 1) == ":")
				strFolder.append("\\");
			
			// If the folder doesn't exist and there's a : in it, return  ie. hdd1:\foobar , but not foobar
			if(!CUtil::FileExists(strFolder) && (strFolder.find(":") != strFolder.npos))
				return;
			// If the folder ends in \, remove the slash  ie. hdd1:\foo\ -> hdd1:\foo, but also hdd1:\ -> hdd1:
			
			if(strFolder.compare("\\") == 0)
			{
				m_vecCurrentFolder.clear();
				return;
			}
			
			if(strFolder.substr(strFolder.length() - 1, 1) == "\\")
				strFolder = strFolder.substr(0, strFolder.length() - 1);
			
			// If the folder ends in :, call UpToRoot()
			if(strFolder.substr(strFolder.length() - 1, 1) == ":")
				UpToRoot();

			vector<CStdString> vecSplits;
			CStringUtils::StringSplit(strFolder, "\\", &vecSplits);
			
			if(vecSplits.size() > 1)
			{
				UpToRoot();
				for(unsigned int x = 0; x < vecSplits.size() ;x++) //Dont do the file
				{
					CStdString f = vecSplits.at(x);
					CD(f);
				}
			}
			else
			{
				if(m_vecCurrentFolder.size() == 0)
				{
					if(strFolder.c_str()[strFolder.size()-1] != ':')
						strFolder = strFolder + ":";
				}	
				m_vecCurrentFolder.push_back(strFolder);
			}
		}
	}
 }

std::vector<CStdString> CFileBrowser::GetFileList()
{
	CLog::Log(LOGNOTICE, "FileBrowser: In Get FileList");
	std::vector<CStdString> retVal;

	if(m_vecCurrentFolder.size() > 0)
	{
		CStdString strPath = GetCurrentPath();

		WIN32_FIND_DATA findFileData;
		memset(&findFileData, 0, sizeof(WIN32_FIND_DATA));
		CStdString strSearchCmd = GetCurrentPath() + "\\*";
	
		HANDLE hFind = FindFirstFile(strSearchCmd.c_str(), &findFileData);

		if(hFind == INVALID_HANDLE_VALUE)
			return retVal;
		do{
			CStdString str = findFileData.cFileName;
			
			if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
			}
			else
			{
				if(strcmp(str.c_str(),"..") !=0 )
					retVal.push_back(str);
			}
		} while(FindNextFile(hFind, &findFileData));

		FindClose(hFind);
	}
	return retVal;
}

CStdString CFileBrowser::GetFileFTPLongDescription(CStdString strFile)
{
	CStdString strOut = "";

	struct stat statFileStatus;
	CStdString strFullFilePath = GetCurrentPath() + "\\" + strFile;
	stat(strFullFilePath.c_str(), &statFileStatus);
		
	DWORD dwAttr = GetFileAttributes(strFullFilePath.c_str());
	char execute = 'x';
	char read = 'r';
	char write = 'w';
	char directory = '-';
	
	if(dwAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		directory = '-';
		directory = 'd';
	}

	if(dwAttr & FILE_ATTRIBUTE_ARCHIVE )
		execute = '-';

	if(dwAttr & FILE_ATTRIBUTE_READONLY  )
		write = '-';

	if(dwAttr & FILE_ATTRIBUTE_DEVICE   )
		read = '-';

	char timeStr[100] = "";
	struct tm locTime;

	int iRetVal = localtime_s(&locTime, &statFileStatus.st_mtime);
	
	if(iRetVal)
		iRetVal = localtime_s(&locTime, &statFileStatus.st_atime);

	if(iRetVal)
		iRetVal = localtime_s(&locTime, &statFileStatus.st_ctime);

	if(iRetVal)
	{
		time_t t;
		time(&t);
		iRetVal = localtime_s(&locTime, &t);
	}

	strftime(timeStr, 100, "%b %d %H:%M", &locTime);

	strOut = CStringUtils::sprintfa("%c%c%c%c%c%c%c%c%c%c   1 root  root    %d %s %s\r\n", directory, read, write, execute, 
		read, write, execute, read, write, execute, statFileStatus.st_size, timeStr, strFile.c_str());
	
	return strOut;
}
