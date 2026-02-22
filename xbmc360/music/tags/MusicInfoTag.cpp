#include "music\tags\MusicInfoTag.h"

using namespace MUSIC_INFO;

CMusicInfoTag::CMusicInfoTag(void)
{
	Clear();
}

CMusicInfoTag::CMusicInfoTag(const CMusicInfoTag& tag)
{
	*this = tag;
}

CMusicInfoTag::~CMusicInfoTag()
{}

const CMusicInfoTag& CMusicInfoTag::operator =(const CMusicInfoTag& tag)
{
	if (this == &tag) return * this;

	m_strURL = tag.m_strURL;
	m_strArtist = tag.m_strArtist;
	m_strAlbumArtist = tag.m_strAlbumArtist;
	m_strAlbum = tag.m_strAlbum;
	m_strGenre = tag.m_strGenre;
	m_strTitle = tag.m_strTitle;
	m_strComment = tag.m_strComment;
	m_strLyrics = tag.m_strLyrics;
	m_strLastPlayed = tag.m_strLastPlayed;
	m_iDuration = tag.m_iDuration;
	m_iTrack = tag.m_iTrack;
	m_bLoaded = tag.m_bLoaded;
	m_rating = tag.m_rating;
	m_listeners = tag.m_listeners;
	m_iTimesPlayed = tag.m_iTimesPlayed;
	memcpy(&m_dwReleaseDate, &tag.m_dwReleaseDate, sizeof(m_dwReleaseDate) );
	
	return *this;
}

bool CMusicInfoTag::operator !=(const CMusicInfoTag& tag) const
{
	if (this == &tag) return false;
	if (m_strURL != tag.m_strURL) return true;
	if (m_strTitle != tag.m_strTitle) return true;
	if (m_strArtist != tag.m_strArtist) return true;
	if (m_strAlbumArtist != tag.m_strAlbumArtist) return true;
	if (m_strAlbum != tag.m_strAlbum) return true;
	if (m_iDuration != tag.m_iDuration) return true;
	if (m_iTrack != tag.m_iTrack) return true;

	return false;
}

int CMusicInfoTag::GetTrackNumber() const
{
	return (m_iTrack & 0xffff);
}

int CMusicInfoTag::GetDiscNumber() const
{
	return (m_iTrack >> 16);
}

int CMusicInfoTag::GetTrackAndDiskNumber() const
{
	return m_iTrack;
}

int CMusicInfoTag::GetDuration() const
{
	return m_iDuration;
}

const CStdString& CMusicInfoTag::GetTitle() const
{
	return m_strTitle;
}

const CStdString& CMusicInfoTag::GetURL() const
{
	return m_strURL;
}

const CStdString& CMusicInfoTag::GetArtist() const
{
	return m_strArtist;
}

const CStdString& CMusicInfoTag::GetAlbum() const
{
	return m_strAlbum;
}

const CStdString& CMusicInfoTag::GetAlbumArtist() const
{
	return m_strAlbumArtist;
}

const CStdString& CMusicInfoTag::GetGenre() const
{
	return m_strGenre;
}

void CMusicInfoTag::GetReleaseDate(SYSTEMTIME& dateTime) const
{
	memcpy(&dateTime, &m_dwReleaseDate, sizeof(m_dwReleaseDate) );
}

int CMusicInfoTag::GetYear() const
{
	return m_dwReleaseDate.wYear;
}

CStdString CMusicInfoTag::GetYearString() const
{
	CStdString strReturn;
	strReturn.Format("%i", m_dwReleaseDate.wYear);

	return m_dwReleaseDate.wYear ? strReturn : "";
}

const CStdString &CMusicInfoTag::GetComment() const
{
	return m_strComment;
}

const CStdString &CMusicInfoTag::GetLyrics() const
{
	return m_strLyrics;
}

char CMusicInfoTag::GetRating() const
{
	return m_rating;
}

int CMusicInfoTag::GetPlayCount() const
{
	return m_iTimesPlayed;
}

int CMusicInfoTag::GetListeners() const
{
	return m_listeners;
}

const CStdString &CMusicInfoTag::GetLastPlayed() const
{
	return m_strLastPlayed;
}

void CMusicInfoTag::SetURL(const CStdString& strURL)
{
	m_strURL = strURL;
}

void CMusicInfoTag::SetTitle(const CStdString& strTitle)
{
	m_strTitle = Trim(strTitle);
}

void CMusicInfoTag::SetArtist(const CStdString& strArtist)
{
	m_strArtist = Trim(strArtist);
}

void CMusicInfoTag::SetAlbum(const CStdString& strAlbum)
{
	m_strAlbum = Trim(strAlbum);
}

void CMusicInfoTag::SetAlbumArtist(const CStdString& strAlbumArtist)
{
	m_strAlbumArtist = Trim(strAlbumArtist);
}

void CMusicInfoTag::SetGenre(const CStdString& strGenre)
{
	m_strGenre = Trim(strGenre);
}

void CMusicInfoTag::SetYear(int year)
{
	memset(&m_dwReleaseDate, 0, sizeof(m_dwReleaseDate) );
	m_dwReleaseDate.wYear = year;
}

void CMusicInfoTag::SetReleaseDate(SYSTEMTIME& dateTime)
{
	memcpy(&m_dwReleaseDate, &dateTime, sizeof(m_dwReleaseDate) );
}

void CMusicInfoTag::SetTrackNumber(int iTrack)
{
	m_iTrack = (m_iTrack & 0xffff0000) | (iTrack & 0xffff);
}

void CMusicInfoTag::SetPartOfSet(int iPartOfSet)
{
	m_iTrack = (m_iTrack & 0xffff) | (iPartOfSet << 16);
}

void CMusicInfoTag::SetTrackAndDiskNumber(int iTrackAndDisc)
{
	m_iTrack=iTrackAndDisc;
}

void CMusicInfoTag::SetDuration(int iSec)
{
	m_iDuration = iSec;
}

void CMusicInfoTag::SetComment(const CStdString& comment)
{
	m_strComment = comment;
}

void CMusicInfoTag::SetLyrics(const CStdString& lyrics)
{
	m_strLyrics = lyrics;
}

void CMusicInfoTag::SetRating(char rating)
{
	m_rating = rating;
}

void CMusicInfoTag::SetPlayCount(int playcount)
{
	m_iTimesPlayed = playcount;
}

void CMusicInfoTag::SetListeners(int listeners)
{
	m_listeners = listeners;
}

void CMusicInfoTag::SetLastPlayed(const CStdString& lastplayed)
{
	m_strLastPlayed = lastplayed;
}

void CMusicInfoTag::SetLoaded(bool bOnOff)
{
	m_bLoaded = bOnOff;
}

bool CMusicInfoTag::Loaded() const
{
	return m_bLoaded;
}

void CMusicInfoTag::Archive(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_strURL;
		ar << m_strTitle;
		ar << m_strArtist;
		ar << m_strAlbum;
		ar << m_strAlbumArtist;
		ar << m_strGenre;
		ar << m_iDuration;
		ar << m_iTrack;
		ar << m_bLoaded;
		ar << m_dwReleaseDate;
		ar << m_strLastPlayed;
		ar << m_strComment;
		ar << m_rating;
		ar << m_iTimesPlayed;
	}
	else
	{
		ar >> m_strURL;
		ar >> m_strTitle;
		ar >> m_strArtist;
		ar >> m_strAlbum;
		ar >> m_strAlbumArtist;
		ar >> m_strGenre;
		ar >> m_iDuration;
		ar >> m_iTrack;
		ar >> m_bLoaded;
		ar >> m_dwReleaseDate;
		ar >> m_strLastPlayed;
		ar >> m_strComment;
		ar >> m_rating;
		ar >> m_iTimesPlayed;
	}
}

void CMusicInfoTag::Clear()
{
	m_strURL.Empty();
	m_strArtist.Empty();
	m_strAlbum.Empty();
	m_strAlbumArtist.Empty();
	m_strGenre.Empty();
	m_strTitle.Empty();
	m_iDuration = 0;
	m_iTrack = 0;
	m_bLoaded = false;
	m_strLastPlayed.Empty();
	m_strComment.Empty();
	m_strLyrics.Empty();
	m_rating = '0';
	m_iTimesPlayed = 0;
	m_listeners = 0;
	memset(&m_dwReleaseDate, 0, sizeof(m_dwReleaseDate) );
}

CStdString CMusicInfoTag::Trim(const CStdString &value) const
{
	CStdString trimmedValue(value);
	trimmedValue.TrimLeft(' ');
	trimmedValue.TrimRight(" \n\r");

	return trimmedValue;
}