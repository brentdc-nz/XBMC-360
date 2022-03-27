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

#include "URIUtils.h"
#include "Util.h"
#include "URL.h"

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