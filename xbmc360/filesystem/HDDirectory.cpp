#include "HDDirectory.h"
#include "utils\Stdafx.h"
#include "utils\URIUtils.h"
#include "utils\Util.h"
#include "URL.h"

using namespace XFILE;

CHDDirectory::CHDDirectory(void)
{
}

CHDDirectory::~CHDDirectory(void)
{
}

bool CHDDirectory::GetDirectory(const CStdString& strPath1, CFileItemList &items)
{
	WIN32_FIND_DATA wfd;

	CStdString strPath = strPath1;
//	g_charsetConverter.utf8ToStringCharset(strPath); // TODO

	CStdString strRoot = strPath;
	CURL url(strPath);

	memset(&wfd, 0, sizeof(wfd));
	URIUtils::AddSlashAtEnd(strRoot);
	strRoot.Replace("/", "\\");
	
/* 	if (URIUtils::IsDVD(strRoot) && m_isoReader.IsScanned()) // TODO
	{
		// Reset iso reader and remount or
		// we can't access the dvd-rom
		m_isoReader.Reset();

		CIoSupport::Dismount("Cdrom0");
		CIoSupport::RemapDriveLetter('D', "Cdrom0");
	}
*/
	CStdString strSearchMask = strRoot;
	strSearchMask += "*.*";

	FILETIME localTime;
	HANDLE hFind = NULL;
	
	hFind =  FindFirstFile(strSearchMask.c_str(), &wfd);
  
	// On error, check if path exists at all, this will return true if empty folder
	if (!hFind)
      return Exists(strPath1);

	if (hFind != NULL)
	{
		do
		{
			if (wfd.cFileName[0] != 0)
			{
				if ( (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				{
					CStdString strDir = wfd.cFileName;
					if (strDir != "." && strDir != "..")
					{
						CStdString strLabel = wfd.cFileName;
//						g_charsetConverter.unknownToUTF8(strLabel); // TODO
						CFileItemPtr pItem(new CFileItem(strLabel));
						CStdString itemPath = strRoot + wfd.cFileName;
//						g_charsetConverter.unknownToUTF8(itemPath); // TODO
						pItem->m_bIsFolder = true;
						URIUtils::AddSlashAtEnd(itemPath);
						pItem->SetPath(itemPath);
						FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &localTime);
						pItem->m_dateTime = localTime;

						items.Add(pItem);
					}
				}
				else
				{
					CStdString strLabel=wfd.cFileName;
//					g_charsetConverter.unknownToUTF8(strLabel);; // TODO
					CFileItemPtr pItem(new CFileItem(strLabel));
					CStdString itemPath = strRoot + wfd.cFileName;
//					g_charsetConverter.unknownToUTF8(itemPath); // TODO
					pItem->SetPath(itemPath);
					pItem->m_bIsFolder = false;
					pItem->m_dwSize = CUtil::ToInt64(wfd.nFileSizeHigh, wfd.nFileSizeLow);
					FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &localTime);
					pItem->m_dateTime = localTime;

					items.Add(pItem);
				}
			}
		}
		while (FindNextFile((HANDLE)hFind, &wfd));

		FindClose(hFind); // Should be closed
	}
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
