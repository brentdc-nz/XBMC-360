#pragma once

#include "utils\Archive.h"
#include "utils\StdString.h"

namespace MUSIC_INFO
{

class CMusicInfoTag : public IArchivable
{
public:
	CMusicInfoTag(void);
	CMusicInfoTag(const CMusicInfoTag& tag);
	virtual ~CMusicInfoTag();
	const CMusicInfoTag& operator =(const CMusicInfoTag& tag);
	bool operator !=(const CMusicInfoTag& tag) const;
	bool Loaded() const;
	const CStdString& GetTitle() const;
	const CStdString& GetURL() const;
	const CStdString& GetArtist() const;
	const CStdString& GetAlbum() const;
	const CStdString& GetAlbumArtist() const;
	const CStdString& GetGenre() const;
	int GetTrackNumber() const;
	int GetDiscNumber() const;
	int GetTrackAndDiskNumber() const;
	int GetDuration() const; // May be set even if Loaded() returns false
	int GetYear() const;

	void GetReleaseDate(SYSTEMTIME& dateTime) const;
	CStdString GetYearString() const;
	const CStdString& GetComment() const;
	const CStdString& GetLyrics() const;
	const CStdString& GetLastPlayed() const;
	char GetRating() const;
	int GetListeners() const;
	int GetPlayCount() const;

	void SetURL(const CStdString& strURL);
	void SetTitle(const CStdString& strTitle);
	void SetArtist(const CStdString& strArtist);
	void SetAlbum(const CStdString& strAlbum);
	void SetAlbumArtist(const CStdString& strAlbumArtist);
	void SetGenre(const CStdString& strGenre);
	void SetYear(int year);
	void SetReleaseDate(SYSTEMTIME& dateTime);
	void SetTrackNumber(int iTrack);
	void SetPartOfSet(int m_iPartOfSet);
	void SetTrackAndDiskNumber(int iTrackAndDisc);
	void SetDuration(int iSec);
	void SetLoaded(bool bOnOff = true);
	void SetComment(const CStdString& comment);
	void SetLyrics(const CStdString& lyrics);
	void SetRating(char rating);
	void SetListeners(int listeners);
	void SetPlayCount(int playcount);
	void SetLastPlayed(const CStdString& strLastPlayed);

	virtual void Archive(CArchive& ar);

	void Clear();

protected:
	CStdString Trim(const CStdString &value) const;

	CStdString m_strURL;
	CStdString m_strTitle;
	CStdString m_strArtist;
	CStdString m_strAlbum;
	CStdString m_strAlbumArtist;
	CStdString m_strGenre;
	CStdString m_strComment;
	CStdString m_strLyrics;
	CStdString m_strLastPlayed;
	int m_iDuration;
	int m_iTrack; // Consists of the disk number in the high 16 bits, the track number in the low 16bits
	bool m_bLoaded;
	char m_rating;
	int m_listeners;
	int m_iTimesPlayed;
	SYSTEMTIME m_dwReleaseDate;
};
}
