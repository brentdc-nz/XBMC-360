#pragma once

#include "guilib\IMsgTargetCallback.h"
#include "playlists\PlayList.h"

#define PLAYLIST_NONE    -1
#define PLAYLIST_MUSIC   0
#define PLAYLIST_VIDEO   1

namespace PLAYLIST
{

/*
 \ Manages playlist playing.
 */
enum REPEAT_STATE { REPEAT_NONE = 0, REPEAT_ONE, REPEAT_ALL };

class CPlayListPlayer : public IMsgTargetCallback
{
public:
	CPlayListPlayer(void);
	virtual ~CPlayListPlayer(void);
	virtual bool OnMessage(CGUIMessage &message);
	void PlayNext(int offset = 1, bool bAutoPlay = false);
	void PlayPrevious();
	void Play();
	void Play(int iSong, bool bAutoPlay = false, bool bPlayPrevious = false);
	int GetNextSong(int offset) const; ///< Returns the song index that is offset away from the current song
	int GetNextSong();

	void ClearPlaylist(int iPlaylist);
	CPlayList& GetPlaylist(int iPlaylist);
	const CPlayList& GetPlaylist(int iPlaylist) const;
	void Reset();
	void Clear();

	void SetCurrentPlaylist(int iPlaylist);
	int GetCurrentPlaylist() const;
	int GetCurrentSong() const;
	void SetCurrentSong(int iSong);

	// Add items via the playlist player
	void Add(int iPlaylist, CPlayList& playlist);
	void Add(int iPlaylist, CFileItemList& items);

protected:
	bool Repeated(int iPlaylist) const;
	bool RepeatedOne(int iPlaylist) const;
	void ReShuffle(int iPlaylist, int iPosition);

	bool m_bPlayedFirstFile;
	bool m_bPlaybackStarted;
	int m_iFailedSongs;
	DWORD m_failedSongsStart;
	int m_iCurrentSong;
	int m_iCurrentPlayList;
	CPlayList* m_PlaylistMusic;
	CPlayList* m_PlaylistVideo;
	CPlayList* m_PlaylistEmpty;
	REPEAT_STATE m_repeatState[2];
};

}

extern PLAYLIST::CPlayListPlayer g_playlistPlayer;