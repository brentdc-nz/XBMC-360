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
#include "video/VideoInfoTag.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/StringUtils.h"
#include "utils/TimeUtils.h"
#include "XBDateTime.h"
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
		m_pDS->exec("CREATE TABLE files ( idFile integer primary key, idPath integer, strFilename text, playCount integer, lastPlayed text)\n");

		CLog::Log(LOGINFO, "create settings table");
		m_pDS->exec("CREATE TABLE settings ( idFile integer, Deinterlace bool,"
		            "ViewMode integer,ZoomAmount float, PixelRatio float,"
		            "AudioStream integer, SubtitleStream integer,"
		            "SubtitleDelay float, SubtitlesOn bool,"
		            "PostProcess bool, AudioDelay float)\n");
		m_pDS->exec("CREATE UNIQUE INDEX ix_settings ON settings ( idFile )\n");

		CLog::Log(LOGINFO, "create streamdetails table");
		m_pDS->exec("CREATE TABLE streamdetails (idFile integer, iStreamType integer, "
		            "strVideoCodec text, fVideoAspect float, iVideoWidth integer, iVideoHeight integer, "
		            "strAudioCodec text, iAudioChannels integer, strAudioLanguage text, strSubtitleLanguage text, iVideoDuration integer)");
		m_pDS->exec("CREATE INDEX ix_streamdetails ON streamdetails (idFile)");

		CLog::Log(LOGINFO, "create stacktimes table");
		m_pDS->exec("CREATE TABLE stacktimes (idFile integer, times text)\n");
		m_pDS->exec("CREATE UNIQUE INDEX ix_stacktimes ON stacktimes ( idFile )\n");

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
	if (URIUtils::IsStack(strFileNameAndPath) || strFileNameAndPath.Mid(0,6).Equals("rar://") || strFileNameAndPath.Mid(0,6).Equals("zip://"))
	{
		URIUtils::GetParentPath(strFileNameAndPath, strPath);
		strFileName = strFileNameAndPath;
	}
	else
	{
		URIUtils::GetDirectory(strFileNameAndPath, strPath);
		strFileName = URIUtils::GetFileName(strFileNameAndPath);
	}
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

int CVideoDatabase::GetFileId(const CFileItem &item)
{
	return GetFileId(item.GetPath());
}

int CVideoDatabase::AddFile(const CFileItem &item)
{
	return AddFile(item.GetPath());
}

int CVideoDatabase::GetPlayCount(const CFileItem &item)
{
	int id = GetFileId(item);
	if (id < 0)
		return 0; // Not in db, so not watched

	try
	{
		if (NULL == m_pDB.get()) return -1;
		if (NULL == m_pDS.get()) return -1;

		CStdString strSQL = PrepareSQL("select playCount from files WHERE idFile=%i", id);
		int count = 0;
		if (m_pDS->query(strSQL.c_str()))
		{
			if (m_pDS->num_rows() == 1)
				count = m_pDS->fv(0).get_asInt();
			m_pDS->close();
		}
		return count;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s failed", __FUNCTION__);
	}
	return -1;
}

void CVideoDatabase::SetPlayCount(const CFileItem &item, int count, const CStdString &date)
{
	int id = AddFile(item);
	if (id < 0)
		return;

	// and mark as watched
	try
	{
		if (NULL == m_pDB.get()) return;
		if (NULL == m_pDS.get()) return;

		CStdString strSQL;
		if (count)
		{
			if (date.IsEmpty())
				strSQL = PrepareSQL("update files set playCount=%i,lastPlayed='%s' where idFile=%i", count, CDateTime::GetCurrentDateTime().GetAsDBDateTime().c_str(), id);
			else
				strSQL = PrepareSQL("update files set playCount=%i,lastPlayed='%s' where idFile=%i", count, date.c_str(), id);
		}
		else
		{
			if (date.IsEmpty())
				strSQL = PrepareSQL("update files set playCount=NULL,lastPlayed=NULL where idFile=%i", id);
			else
				strSQL = PrepareSQL("update files set playCount=NULL,lastPlayed='%s' where idFile=%i", date.c_str(), id);
		}

		m_pDS->exec(strSQL.c_str());

		// TODO: We only need to announce changes to video items in the library
		//if (item.HasVideoInfoTag() && item.GetVideoInfoTag()->m_iDbId > 0)
		//{
		//	// Only provide the "playcount" value if it has actually changed
		//	if (item.GetVideoInfoTag()->m_playCount != count)
		//	{
		//		CVariant data;
		//		data["playcount"] = count;
		//		ANNOUNCEMENT::CAnnouncementManager::Announce(ANNOUNCEMENT::VideoLibrary, "xbmc", "OnUpdate", CFileItemPtr(new CFileItem(item)), data);
		//	}
		//	else
		//		ANNOUNCEMENT::CAnnouncementManager::Announce(ANNOUNCEMENT::VideoLibrary, "xbmc", "OnUpdate", CFileItemPtr(new CFileItem(item)));
		//}
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s failed", __FUNCTION__);
	}
}

void CVideoDatabase::IncrementPlayCount(const CFileItem &item)
{
	SetPlayCount(item, GetPlayCount(item) + 1);
}

void CVideoDatabase::UpdateLastPlayed(const CFileItem &item)
{
	SetPlayCount(item, GetPlayCount(item), CDateTime::GetCurrentDateTime().GetAsDBDateTime());
}

/// \brief Sets the settings for a particular video file
void CVideoDatabase::SetVideoSettings(const CStdString& strFilenameAndPath, const CVideoSettings &setting)
{
	try
	{
		if (NULL == m_pDB.get()) return;
		if (NULL == m_pDS.get()) return;
		int idFile = AddFile(strFilenameAndPath);
		if (idFile < 0)
			return;
		CStdString strSQL;
		strSQL.Format("select * from settings where idFile=%i", idFile);
		m_pDS->query(strSQL.c_str());
		if (m_pDS->num_rows() > 0)
		{
			m_pDS->close();
			// update the item
			strSQL = PrepareSQL("update settings set Deinterlace=%i,ViewMode=%i,ZoomAmount=%f,PixelRatio=%f,"
			                   "AudioStream=%i,SubtitleStream=%i,SubtitleDelay=%f,SubtitlesOn=%i,"
			                   "PostProcess=%i,AudioDelay=%f where idFile=%i",
			                   setting.m_InterlaceMethod, setting.m_ViewMode, setting.m_CustomZoomAmount, setting.m_CustomPixelRatio,
			                   setting.m_AudioStream, setting.m_SubtitleStream, setting.m_SubtitleDelay, setting.m_SubtitleOn,
			                   setting.m_PostProcess, setting.m_AudioDelay, idFile);
			m_pDS->exec(strSQL.c_str());
			return;
		}
		else
		{
			// add the items
			m_pDS->close();
			strSQL = PrepareSQL("INSERT INTO settings (idFile,Deinterlace,ViewMode,ZoomAmount,PixelRatio,"
			                   "AudioStream,SubtitleStream,SubtitleDelay,SubtitlesOn,"
			                   "PostProcess,AudioDelay) "
			                   "VALUES (%i,%i,%i,%f,%f,%i,%i,%f,%i,%i,%f)",
			                   idFile, setting.m_InterlaceMethod, setting.m_ViewMode, setting.m_CustomZoomAmount, setting.m_CustomPixelRatio,
			                   setting.m_AudioStream, setting.m_SubtitleStream, setting.m_SubtitleDelay, setting.m_SubtitleOn,
			                   setting.m_PostProcess, setting.m_AudioDelay);
			m_pDS->exec(strSQL.c_str());
		}
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s (%s) failed", __FUNCTION__, strFilenameAndPath.c_str());
	}
}

/// \brief GetVideoSettings() obtains any saved video settings for the current file.
/// \retval Returns true if the settings exist, false otherwise.
bool CVideoDatabase::GetVideoSettings(const CStdString &strFilenameAndPath, CVideoSettings &settings)
{
	try
	{
		// obtain the FileID (if it exists)
		int idFile = GetFileId(strFilenameAndPath);
		if (idFile < 0) return false;
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;
		// ok, now obtain the settings for this file
		CStdString strSQL = PrepareSQL("select * from settings where settings.idFile = '%i'", idFile);
		m_pDS->query(strSQL.c_str());
		if (m_pDS->num_rows() > 0)
		{
			// get the video settings info
			settings.m_InterlaceMethod = (EINTERLACEMETHOD)m_pDS->fv("Deinterlace").get_asInt();
			settings.m_ViewMode = m_pDS->fv("ViewMode").get_asInt();
			settings.m_CustomZoomAmount = m_pDS->fv("ZoomAmount").get_asFloat();
			settings.m_CustomPixelRatio = m_pDS->fv("PixelRatio").get_asFloat();
			settings.m_AudioStream = m_pDS->fv("AudioStream").get_asInt();
			settings.m_SubtitleStream = m_pDS->fv("SubtitleStream").get_asInt();
			settings.m_SubtitleDelay = m_pDS->fv("SubtitleDelay").get_asFloat();
			settings.m_SubtitleOn = m_pDS->fv("SubtitlesOn").get_asBool();
			settings.m_PostProcess = m_pDS->fv("PostProcess").get_asBool();
			settings.m_AudioDelay = m_pDS->fv("AudioDelay").get_asFloat();
			settings.m_SubtitleCached = false;
			m_pDS->close();
			return true;
		}
		m_pDS->close();
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s failed", __FUNCTION__);
	}
	return false;
}

void CVideoDatabase::SetStreamDetailsForFile(const CStreamDetails& details, const CStdString &strFileNameAndPath)
{
	// AddFile checks to make sure the file isn't already in the DB first
	int idFile = AddFile(strFileNameAndPath);
	if (idFile < 0)
		return;
	SetStreamDetailsForFileId(details, idFile);
}

void CVideoDatabase::SetStreamDetailsForFileId(const CStreamDetails& details, int idFile)
{
	if (idFile < 0)
		return;

	try
	{
		BeginTransaction();
		m_pDS->exec(PrepareSQL("DELETE FROM streamdetails WHERE idFile = %i", idFile));

		for (int i = 1; i <= details.GetVideoStreamCount(); i++)
		{
			m_pDS->exec(PrepareSQL("INSERT INTO streamdetails "
			            "(idFile, iStreamType, strVideoCodec, fVideoAspect, iVideoWidth, iVideoHeight, iVideoDuration) "
			            "VALUES (%i,%i,'%s',%f,%i,%i,%i)",
			            idFile, (int)CStreamDetail::VIDEO,
			            details.GetVideoCodec(i).c_str(), details.GetVideoAspect(i),
			            details.GetVideoWidth(i), details.GetVideoHeight(i), details.GetVideoDuration(i)));
		}
		for (int i = 1; i <= details.GetAudioStreamCount(); i++)
		{
			m_pDS->exec(PrepareSQL("INSERT INTO streamdetails "
			            "(idFile, iStreamType, strAudioCodec, iAudioChannels, strAudioLanguage) "
			            "VALUES (%i,%i,'%s',%i,'%s')",
			            idFile, (int)CStreamDetail::AUDIO,
			            details.GetAudioCodec(i).c_str(), details.GetAudioChannels(i),
			            details.GetAudioLanguage(i).c_str()));
		}
		for (int i = 1; i <= details.GetSubtitleStreamCount(); i++)
		{
			m_pDS->exec(PrepareSQL("INSERT INTO streamdetails "
			            "(idFile, iStreamType, strSubtitleLanguage) "
			            "VALUES (%i,%i,'%s')",
			            idFile, (int)CStreamDetail::SUBTITLE,
			            details.GetSubtitleLanguage(i).c_str()));
		}

		CommitTransaction();
	}
	catch (...)
	{
		RollbackTransaction();
		CLog::Log(LOGERROR, "%s (%i) failed", __FUNCTION__, idFile);
	}
}

void CVideoDatabase::EraseVideoSettings()
{
	try
	{
		CLog::Log(LOGINFO, "Deleting settings information for all movies");
		m_pDS->exec("delete from settings");
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s failed", __FUNCTION__);
	}
}

/// \brief GetStackTimes() obtains any saved video times for the stacked file
/// \retval Returns true if the stack times exist, false otherwise.
bool CVideoDatabase::GetStackTimes(const CStdString &filePath, std::vector<int> &times)
{
	try
	{
		// obtain the FileID (if it exists)
		int idFile = GetFileId(filePath);
		if (idFile < 0) return false;
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;
		// ok, now obtain the settings for this file
		CStdString strSQL = PrepareSQL("select times from stacktimes where idFile=%i\n", idFile);
		m_pDS->query(strSQL.c_str());
		if (m_pDS->num_rows() > 0)
		{ // get the video settings info
			CStdStringArray timeString;
			int timeTotal = 0;
			CStringUtils::SplitString(m_pDS->fv("times").get_asString(), ",", timeString);
			times.clear();
			for (unsigned int i = 0; i < timeString.size(); i++)
			{
				times.push_back(atoi(timeString[i].c_str()));
				timeTotal += atoi(timeString[i].c_str());
			}
			m_pDS->close();
			return (timeTotal > 0);
		}
		m_pDS->close();
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s failed", __FUNCTION__);
	}
	return false;
}

/// \brief Sets the stack times for a particular video file
void CVideoDatabase::SetStackTimes(const CStdString& filePath, std::vector<int> &times)
{
	try
	{
		if (NULL == m_pDB.get()) return;
		if (NULL == m_pDS.get()) return;
		int idFile = AddFile(filePath);
		if (idFile < 0)
			return;

		// delete any existing items
		m_pDS->exec(PrepareSQL("delete from stacktimes where idFile=%i", idFile));

		// add the items
		CStdString timeString;
		timeString.Format("%i", times[0]);
		for (unsigned int i = 1; i < times.size(); i++)
		{
			CStdString time;
			time.Format(",%i", times[i]);
			timeString += time;
		}
		m_pDS->exec(PrepareSQL("insert into stacktimes (idFile,times) values (%i,'%s')\n", idFile, timeString.c_str()));
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s (%s) failed", __FUNCTION__, filePath.c_str());
	}
}

bool CVideoDatabase::GetPlayCounts(CFileItemList &items)
{
	int pathID = GetPathId(items.GetPath());
	if (pathID < 0)
		return false; // path (and thus files) aren't in the database

	try
	{
		// error!
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		// TODO: also test a single query for the above and below
		CStdString sql = PrepareSQL("select strFilename,playCount from files where idPath=%i", pathID);
		if (RunQuery(sql) <= 0)
			return false;

		items.SetFastLookup(true); // note: it's possibly quicker the other way around (map on db returned items)?
		while (!m_pDS->eof())
		{
			CStdString path;
			ConstructPath(path, items.GetPath(), m_pDS->fv(0).get_asString());
			CFileItemPtr item = items.Get(path);
			if (item)
				item->GetVideoInfoTag()->m_playCount = m_pDS->fv(1).get_asInt();
			m_pDS->next();
		}
		return true;
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s failed", __FUNCTION__);
	}
	return false;
}

int CVideoDatabase::RunQuery(const CStdString &sql)
{
	unsigned int time = CTimeUtils::GetTimeMS();
	int rows = -1;
	if (m_pDS->query(sql.c_str()))
	{
		rows = m_pDS->num_rows();
		if (rows == 0)
			m_pDS->close();
	}
	CLog::Log(LOGDEBUG, "%s took %d ms for %d items query: %s", __FUNCTION__, CTimeUtils::GetTimeMS() - time, rows, sql.c_str());
	return rows;
}

void CVideoDatabase::ConstructPath(CStdString& strDest, const CStdString& strPath, const CStdString& strFileName)
{
	if (URIUtils::IsStack(strFileName) || URIUtils::IsInArchive(strFileName))
		strDest = strFileName;
	else
		URIUtils::AddFileToFolder(strPath, strFileName, strDest);
}
