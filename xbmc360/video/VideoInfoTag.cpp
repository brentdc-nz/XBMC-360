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

#include "video\VideoInfoTag.h"

#include <map>

#include "guilib\LocalizeStrings.h"

void CVideoInfoTag::Reset()
{
  m_strDirector = "";
  m_strWritingCredits = "";
  m_strGenre = "";
  m_strCountry = "";
  m_strTagLine = "";
  m_strPlotOutline = "";
  m_strPlot = "";
  m_strTitle = "";
  m_strOriginalTitle = "";
  m_strSortTitle = "";
  m_strVotes = "";
  m_cast.clear();
  m_strSet = "";
  m_strFile = "";
  m_strPath = "";
  m_strIMDBNumber = "";
  m_strMPAARating = "";
  m_strPremiered = "";
  m_strStatus = "";
  m_strProductionCode = "";
  m_strFirstAired = "";
  m_strStudio = "";
  m_strAlbum = "";
  m_strArtist = "";
  m_strTrailer = "";
  m_iTop250 = 0;
  m_iYear = 0;
  m_iSeason = -1;
  m_iEpisode = -1;
  m_iSpecialSortSeason = -1;
  m_iSpecialSortEpisode = -1;
  m_fRating = 0.0f;
  m_iDbId = -1;
  m_iFileId = -1;
  m_iBookmarkId = -1;
  m_iTrack = -1;
  m_strRuntime = "";
  m_lastPlayed = "";
  m_strShowLink = "";
  m_playCount = 0;
  m_fEpBookmark = 0;
}

bool CVideoInfoTag::HasStreamDetails() const
{
  return m_streamDetails.HasItems();
}

bool CVideoInfoTag::IsEmpty() const
{
  return (m_strTitle.IsEmpty() &&
          m_strFile.IsEmpty() &&
          m_strPath.IsEmpty());
}

const CStdString CVideoInfoTag::GetCast(bool bIncludeRole /*= false*/) const
{
	CStdString strLabel;
	for (iCast it = m_cast.begin(); it != m_cast.end(); ++it)
	{
		CStdString character;
		if (it->strRole.IsEmpty() || !bIncludeRole)
			character.Format("%s\n", it->strName.c_str());
		else
			character.Format("%s %s %s\n", it->strName.c_str(), g_localizeStrings.Get(20347).c_str(), it->strRole.c_str());
		strLabel += character;
	}
	return strLabel.TrimRight("\n");
}
