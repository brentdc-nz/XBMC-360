#pragma once
/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include "Database.h"
#include "video/Bookmark.h"

class CFileItem;

class CVideoDatabase : public CDatabase
{
public:
	CVideoDatabase(void);
	virtual ~CVideoDatabase(void);

	// Bookmark methods
	void GetBookMarksForFile(const CStdString& strFilenameAndPath, VECBOOKMARKS& bookmarks, CBookmark::EType type = CBookmark::STANDARD, bool bAppend=false);
	void AddBookMarkToFile(const CStdString& strFilenameAndPath, const CBookmark &bookmark, CBookmark::EType type = CBookmark::STANDARD);
	bool GetResumeBookMark(const CStdString& strFilenameAndPath, CBookmark &bookmark);
	void DeleteResumeBookMark(const CStdString &strFilenameAndPath);
	void ClearBookMarkOfFile(const CStdString& strFilenameAndPath, CBookmark& bookmark, CBookmark::EType type = CBookmark::STANDARD);
	void ClearBookMarksOfFile(const CStdString& strFilenameAndPath, CBookmark::EType type = CBookmark::STANDARD);

protected:
	int GetPathId(const CStdString& strPath);
	int AddPath(const CStdString& strPath);
	int GetFileId(const CStdString& strFilenameAndPath);
	int AddFile(const CStdString& strFilenameAndPath);

	void SplitPath(const CStdString& strFileNameAndPath, CStdString& strPath, CStdString& strFileName);

	virtual bool CreateTables();
	virtual int GetMinVersion() const { return 1; }
};
