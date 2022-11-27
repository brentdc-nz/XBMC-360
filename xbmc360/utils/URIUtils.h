/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#ifndef H_URIUTILS
#define H_URIUTILS

#include "utils\StdString.h"

class URIUtils
{
public:
	URIUtils(void) {};
	virtual ~URIUtils(void) {};

	static void GetDirectory(const CStdString& strFilePath, CStdString& strDirectoryPath);
	static const CStdString GetExtension(const CStdString& strFileName);
	static void GetExtension(const CStdString& strFile, CStdString& strExtension);
	static bool IsDOSPath(const CStdString &path);
	static void AddSlashAtEnd(CStdString& strFolder);
	static bool IsURL(const CStdString& strFile);
	static bool IsRemote(const CStdString& strFile);
	static bool IsMultiPath(const CStdString& strPath);
	static bool IsHD(const CStdString& strFileName);
	static const CStdString GetFileName(const CStdString& strFileNameAndPath);
	static bool HasSlashAtEnd(const CStdString& strFile);
	static void RemoveSlashAtEnd(CStdString& strFolder);
	static void RemoveExtension(CStdString& strFileName);
	static CStdString GetParentPath(const CStdString& strPath);
	static bool GetParentPath(const CStdString& strPath, CStdString& strParent);

	static void AddFileToFolder(const CStdString& strFolder,
                              const CStdString& strFile, CStdString& strResult);

	static CStdString AddFileToFolder(const CStdString &strFolder, 
                                    const CStdString &strFile)
	{
		CStdString result;
		AddFileToFolder(strFolder, strFile, result);
		return result;
	}

};

#endif //H_URIUTILS