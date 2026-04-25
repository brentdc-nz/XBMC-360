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

#include "video/VideoDatabase.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "FileItem.h"

using namespace dbiplus;

CVideoDatabase::CVideoDatabase(void)
{
	m_strDatabaseFile = "MyVideos.db";
}

CVideoDatabase::~CVideoDatabase(void)
{
}

bool CVideoDatabase::CreateTables()
{
	try
	{
		CDatabase::CreateTables();

		CLog::Log(LOGINFO, "create bookmark table");
		m_pDS->exec("CREATE TABLE bookmark ( idBookmark integer primary key, idFile integer, timeInSeconds double, totalTimeInSeconds double, thumbNailImage text, player text, playerState text, type integer)\n");

		CLog::Log(LOGINFO, "create path table");
		m_pDS->exec("CREATE TABLE path ( idPath integer primary key, strPath text)\n");

		CLog::Log(LOGINFO, "create files table");
		m_pDS->exec("CREATE TABLE files ( idFile integer primary key, idPath integer, strFilename text)\n");

		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s unable to create tables", __FUNCTION__);
	}
	return false;
}

void CVideoDatabase::SplitPath(const CStdString& strFileNameAndPath, CStdString& strPath, CStdString& strFileName)
{
	URIUtils::GetDirectory(strFileNameAndPath, strPath);
	strFileName = URIUtils::GetFileName(strFileNameAndPath);
}

int CVideoDatabase::GetPathId(const CStdString& strPath)
{
	CStdString strSQL;
	try
	{
		int idPath = -1;
		if (NULL == m_pDB.get()) return -1;
		if (NULL == m_pDS.get()) return -1;

		CStdString strPath1(strPath);
		URIUtils::AddSlashAtEnd(strPath1);

		strSQL = PrepareSQL("select idPath from path where strPath like '%s'", strPath1.c_str());
		m_pDS->query(strSQL.c_str());
		if (!m_pDS->eof())
			idPath = m_pDS->fv("idPath").get_asInt();

		m_pDS->close();
		return idPath;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s unable to getpath (%s)", __FUNCTION__, strSQL.c_str());
	}
	return -1;
}

int CVideoDatabase::AddPath(const CStdString& strPath)
{
	CStdString strSQL;
	try
	{
		int idPath;
		if (NULL == m_pDB.get()) return -1;
		if (NULL == m_pDS.get()) return -1;

		CStdString strPath1(strPath);
		URIUtils::AddSlashAtEnd(strPath1);

		strSQL = PrepareSQL("insert into path (idPath, strPath) values (NULL,'%s')", strPath1.c_str());
		m_pDS->exec(strSQL.c_str());
		idPath = (int)m_pDS->lastinsertid();
		return idPath;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s unable to addpath (%s)", __FUNCTION__, strSQL.c_str());
	}
	return -1;
}

int CVideoDatabase::AddFile(const CStdString& strFileNameAndPath)
{
	CStdString strSQL = "";
	try
	{
		int idFile;
		if (NULL == m_pDB.get()) return -1;
		if (NULL == m_pDS.get()) return -1;

		CStdString strFileName, strPath;
		SplitPath(strFileNameAndPath, strPath, strFileName);

		int idPath = GetPathId(strPath);
		if (idPath < 0)
			idPath = AddPath(strPath);

		if (idPath < 0)
			return -1;

		strSQL = PrepareSQL("select idFile from files where strFileName like '%s' and idPath=%i", strFileName.c_str(), idPath);

		m_pDS->query(strSQL.c_str());
		if (m_pDS->num_rows() > 0)
		{
			idFile = m_pDS->fv("idFile").get_asInt();
			m_pDS->close();
			return idFile;
		}
		m_pDS->close();
		strSQL = PrepareSQL("insert into files (idFile,idPath,strFileName) values(NULL, %i, '%s')", idPath, strFileName.c_str());
		m_pDS->exec(strSQL.c_str());
		idFile = (int)m_pDS->lastinsertid();
		return idFile;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s unable to addfile (%s)", __FUNCTION__, strSQL.c_str());
	}
	return -1;
}

int CVideoDatabase::GetFileId(const CStdString& strFilenameAndPath)
{
	try
	{
		if (NULL == m_pDB.get()) return -1;
		if (NULL == m_pDS.get()) return -1;
		CStdString strPath, strFileName;
		SplitPath(strFilenameAndPath, strPath, strFileName);

		int idPath = GetPathId(strPath);
		if (idPath >= 0)
		{
			CStdString strSQL;
			strSQL = PrepareSQL("select idFile from files where strFileName like '%s' and idPath=%i", strFileName.c_str(), idPath);
			m_pDS->query(strSQL.c_str());
			if (m_pDS->num_rows() > 0)
			{
				int idFile = m_pDS->fv("idFile").get_asInt();
				m_pDS->close();
				return idFile;
			}
		}
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s (%s) failed", __FUNCTION__, strFilenameAndPath.c_str());
	}
	return -1;
}

void CVideoDatabase::GetBookMarksForFile(const CStdString& strFilenameAndPath, VECBOOKMARKS& bookmarks, CBookmark::EType type, bool bAppend)
{
	try
	{
		int idFile = GetFileId(strFilenameAndPath);
		if (idFile < 0) return;
		if (!bAppend)
			bookmarks.erase(bookmarks.begin(), bookmarks.end());
		if (NULL == m_pDB.get()) return;
		if (NULL == m_pDS.get()) return;

		CStdString strSQL = PrepareSQL("select * from bookmark where idFile=%i and type=%i order by timeInSeconds", idFile, (int)type);
		m_pDS->query(strSQL.c_str());
		while (!m_pDS->eof())
		{
			CBookmark bookmark;
			bookmark.timeInSeconds = m_pDS->fv("timeInSeconds").get_asDouble();
			bookmark.totalTimeInSeconds = m_pDS->fv("totalTimeInSeconds").get_asDouble();
			bookmark.thumbNailImage = m_pDS->fv("thumbNailImage").get_asString().c_str();
			bookmark.playerState = m_pDS->fv("playerState").get_asString().c_str();
			bookmark.player = m_pDS->fv("player").get_asString().c_str();
			bookmark.type = type;
			bookmarks.push_back(bookmark);
			m_pDS->next();
		}
		m_pDS->close();
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s (%s) failed", __FUNCTION__, strFilenameAndPath.c_str());
	}
}

bool CVideoDatabase::GetResumeBookMark(const CStdString& strFilenameAndPath, CBookmark &bookmark)
{
	VECBOOKMARKS bookmarks;
	GetBookMarksForFile(strFilenameAndPath, bookmarks, CBookmark::RESUME);
	if (bookmarks.size() > 0)
	{
		bookmark = bookmarks[0];
		return true;
	}
	return false;
}

void CVideoDatabase::DeleteResumeBookMark(const CStdString &strFilenameAndPath)
{
	if (!m_pDB.get() || !m_pDS.get())
		return;

	int fileID = GetFileId(strFilenameAndPath);
	if (fileID < -1)
		return;

	try
	{
		CStdString sql = PrepareSQL("delete from bookmark where idFile=%i and type=%i", fileID, CBookmark::RESUME);
		m_pDS->exec(sql.c_str());
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "%s (%s) failed", __FUNCTION__, strFilenameAndPath.c_str());
	}
}

void CVideoDatabase::AddBookMarkToFile(const CStdString& strFilenameAndPath, const CBookmark &bookmark, CBookmark::EType type)
{
	try
	{
		int idFile = AddFile(strFilenameAndPath);
		if (idFile < 0)
			return;
		if (NULL == m_pDB.get()) return;
		if (NULL == m_pDS.get()) return;

		CStdString strSQL;
		int idBookmark = -1;
		if (type == CBookmark::RESUME) // get the same resume mark bookmark each time type
		{
			strSQL = PrepareSQL("select idBookmark from bookmark where idFile=%i and type=1", idFile);
		}
		else if (type == CBookmark::STANDARD) // get the same bookmark again, and update
		{
			double mintime = bookmark.timeInSeconds - 0.5f;
			double maxtime = bookmark.timeInSeconds + 0.5f;
			strSQL = PrepareSQL("select idBookmark from bookmark where idFile=%i and type=%i and (timeInSeconds between %f and %f) and playerState='%s'", idFile, (int)type, mintime, maxtime, bookmark.playerState.c_str());
		}

		if (type != CBookmark::EPISODE)
		{
			// get current id
			m_pDS->query(strSQL.c_str());
			if (m_pDS->num_rows() != 0)
				idBookmark = m_pDS->get_field_value("idBookmark").get_asInt();
			m_pDS->close();
		}
		// update or insert depending if it existed before
		if (idBookmark >= 0)
			strSQL = PrepareSQL("update bookmark set timeInSeconds = %f, totalTimeInSeconds = %f, thumbNailImage = '%s', player = '%s', playerState = '%s' where idBookmark = %i", bookmark.timeInSeconds, bookmark.totalTimeInSeconds, bookmark.thumbNailImage.c_str(), bookmark.player.c_str(), bookmark.playerState.c_str(), idBookmark);
		else
			strSQL = PrepareSQL("insert into bookmark (idBookmark, idFile, timeInSeconds, totalTimeInSeconds, thumbNailImage, player, playerState, type) values(NULL,%i,%f,%f,'%s','%s','%s', %i)", idFile, bookmark.timeInSeconds, bookmark.totalTimeInSeconds, bookmark.thumbNailImage.c_str(), bookmark.player.c_str(), bookmark.playerState.c_str(), (int)type);

		m_pDS->exec(strSQL.c_str());
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s (%s) failed", __FUNCTION__, strFilenameAndPath.c_str());
	}
}

void CVideoDatabase::ClearBookMarkOfFile(const CStdString& strFilenameAndPath, CBookmark& bookmark, CBookmark::EType type)
{
	try
	{
		int idFile = GetFileId(strFilenameAndPath);
		if (idFile < 0) return;
		if (NULL == m_pDB.get()) return;
		if (NULL == m_pDS.get()) return;

		double mintime = bookmark.timeInSeconds - 0.5f;
		double maxtime = bookmark.timeInSeconds + 0.5f;
		CStdString strSQL = PrepareSQL("select idBookmark from bookmark where idFile=%i and type=%i and playerState like '%s' and player like '%s' and (timeInSeconds between %f and %f)", idFile, type, bookmark.playerState.c_str(), bookmark.player.c_str(), mintime, maxtime);

		m_pDS->query(strSQL.c_str());
		if (m_pDS->num_rows() != 0)
		{
			int idBookmark = m_pDS->get_field_value("idBookmark").get_asInt();
			strSQL = PrepareSQL("delete from bookmark where idBookmark=%i", idBookmark);
			m_pDS->exec(strSQL.c_str());
		}

		m_pDS->close();
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s (%s) failed", __FUNCTION__, strFilenameAndPath.c_str());
	}
}

void CVideoDatabase::ClearBookMarksOfFile(const CStdString& strFilenameAndPath, CBookmark::EType type)
{
	try
	{
		int idFile = GetFileId(strFilenameAndPath);
		if (idFile < 0) return;
		if (NULL == m_pDB.get()) return;
		if (NULL == m_pDS.get()) return;

		CStdString strSQL = PrepareSQL("delete from bookmark where idFile=%i and type=%i", idFile, (int)type);
		m_pDS->exec(strSQL.c_str());
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s (%s) failed", __FUNCTION__, strFilenameAndPath.c_str());
	}
}
