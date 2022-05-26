#include "HDDirectory.h"
#include "..\utils\Stdafx.h"
#include "..\utils\URIUtils.h"

using namespace DIRECTORY;

CHDDirectory::CHDDirectory(void)
{
}

CHDDirectory::~CHDDirectory(void)
{
}

bool CHDDirectory::GetDirectory(const CStdString& strPath1, CFileItemList &items)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind;

	CStdString strPath = strPath1;
	CStdString strRoot = strPath;

	memset(&wfd, 0, sizeof(wfd));
	URIUtils::AddSlashAtEnd(strRoot);
	strRoot.Replace("/", "\\");

	CStdString strSearchMask = strRoot;
	strSearchMask += "*.*";

	hFind = FindFirstFile(strSearchMask.c_str(), &wfd);

	// On error, check if path exists at all, this will return true if empty folder
	if (INVALID_HANDLE_VALUE == hFind) 
		return Exists(strPath1);
   
	// List all the files in the directory with some info about them.
	do
	{
		if (wfd.cFileName[0] != 0)
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				CStdString strLabel=wfd.cFileName;
				CFileItem* pItem(new CFileItem(strLabel));
				CStdString itemPath = strRoot + wfd.cFileName;
				pItem->m_bIsFolder = true;
				URIUtils::AddSlashAtEnd(itemPath);
				pItem->SetPath(itemPath);
//			    FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &localTime);
//				pItem->m_dateTime=localTime;
				
				items.Add(pItem);
			}
			else
			{
				CStdString strLabel=wfd.cFileName;
				CFileItem* pItem(new CFileItem(strLabel));
				CStdString itemPath = strRoot + wfd.cFileName;
				pItem->SetPath(itemPath);
				pItem->m_bIsFolder = false;
//				pItem->m_dwSize = CUtil::ToInt64(wfd.nFileSizeHigh, wfd.nFileSizeLow);
//				FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &localTime);
//				pItem->m_dateTime=localTime;

				if(IsAllowed(wfd.cFileName))
					items.Add(pItem);
			}
		}
	}
	while (FindNextFile(hFind, &wfd) != 0);

	FindClose(hFind);

	return true;
}

bool CHDDirectory::Exists(const char* strPath)
{
	if (!strPath || !*strPath)
		return false;

	CStdString strReplaced = strPath;
//	g_charsetConverter.utf8ToStringCharset(strReplaced); //TODO
	
	strReplaced.Replace("/","\\");

	URIUtils::AddSlashAtEnd(strReplaced);

	DWORD attributes = GetFileAttributes(strReplaced.c_str());
		
	if (FILE_ATTRIBUTE_DIRECTORY & attributes)
		return true;
		
	return false;
}
