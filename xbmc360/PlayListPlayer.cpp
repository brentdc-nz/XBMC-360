#include "PlayListPlayer.h"
#include "Application.h"
#include "utils/log.h"
#include "playlists\PlayList.h"
#include "guilib\GUIWindowManager.h"
#include "guilib\GUIUserMessages.h"
#include "utils\TimeUtils.h"
#include "guilib\dialogs\GUIDialogOK.h"

using namespace PLAYLIST;

CPlayListPlayer g_playlistPlayer;

CPlayListPlayer::CPlayListPlayer(void)
{
	m_PlaylistMusic = new CPlayList;
	m_PlaylistVideo = new CPlayList;
	m_PlaylistEmpty = new CPlayList;

	m_iCurrentSong = -1;
	m_iCurrentPlayList = PLAYLIST_NONE;
	m_bPlayedFirstFile = false;
	m_bPlaybackStarted = false;

	for (int i = 0; i < 2; i++)
		m_repeatState[i] = REPEAT_NONE;

	m_iFailedSongs = 0;
	m_failedSongsStart = 0;
}

CPlayListPlayer::~CPlayListPlayer(void)
{
	Clear();
	delete m_PlaylistMusic;
	delete m_PlaylistVideo;
	delete m_PlaylistEmpty;
}

bool CPlayListPlayer::OnMessage(CGUIMessage &message)
{
	switch (message.GetMessage())
	{
		case GUI_MSG_NOTIFY_ALL:
		if (message.GetParam1() == GUI_MSG_UPDATE_ITEM && message.GetItem())
		{
			// Update our item if necessary
			CPlayList &playlist = GetPlaylist(m_iCurrentPlayList);
			CFileItemPtr item = std::static_pointer_cast<CFileItem>(message.GetItem());
			playlist.UpdateItem(item.get());
		}
		break;
		case GUI_MSG_PLAYBACK_STOPPED:
		{
			if (m_iCurrentPlayList != PLAYLIST_NONE && m_bPlaybackStarted)
			{
				CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_STOPPED, 0, 0, m_iCurrentPlayList, m_iCurrentSong);
				g_windowManager.SendThreadMessage(msg);
				Reset();
				m_iCurrentPlayList = PLAYLIST_NONE;
				return true;
			}
		}
		break;
	}
	return false;
}

// Play next entry in current playlist
void CPlayListPlayer::PlayNext(int offset, bool bAutoPlay)
{
	int iSong = GetNextSong(offset);
	CPlayList& playlist = GetPlaylist(m_iCurrentPlayList);

	// Stop playing
	if ((iSong < 0) || (iSong >= playlist.size()) || (playlist.GetPlayable() <= 0))
	{
		CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_STOPPED, 0, 0, m_iCurrentPlayList, m_iCurrentSong);
		g_windowManager.SendThreadMessage(msg);
		Reset();
		m_iCurrentPlayList = PLAYLIST_NONE;
		return;
	}

	if (bAutoPlay)
	{
		CFileItemPtr item = playlist[iSong];

//		if ( item->IsShoutCast() ) // TODO BREBT
//			return;
	}
	Play(iSong, bAutoPlay);
	//g_partyModeManager.OnSongChange();
}

// Play previous entry in current playlist
void CPlayListPlayer::PlayPrevious()
{
	if (m_iCurrentPlayList == PLAYLIST_NONE)
		return;

	CPlayList& playlist = GetPlaylist(m_iCurrentPlayList);

	if (playlist.size() <= 0) return ;

	int iSong = m_iCurrentSong;

	if (!RepeatedOne(m_iCurrentPlayList))
		iSong--;

	if (iSong < 0)
		iSong = playlist.size() - 1;

	Play(iSong, false, true);
}

void CPlayListPlayer::Play()
{
	if (m_iCurrentPlayList == PLAYLIST_NONE)
		return;

	CPlayList& playlist = GetPlaylist(m_iCurrentPlayList);
	if (playlist.size() <= 0) return;

	Play(0);
}

// Start playing entry \e iSong in current playlist
// param iSong Song in playlist
void CPlayListPlayer::Play(int iSong, bool bAutoPlay /* = false */, bool bPlayPrevious /* = false */)
{
	if (m_iCurrentPlayList == PLAYLIST_NONE)
		return;

	CPlayList& playlist = GetPlaylist(m_iCurrentPlayList);

	if (playlist.size() <= 0) return;
	if (iSong < 0) iSong = 0;
	if (iSong >= playlist.size()) iSong = playlist.size() - 1;

	// Check if the item itself is a playlist, and can be expanded
	// only allow a few levels, this could end up in a loop
	// if they refer to each other in a loop
	for(int i=0; i<5; i++)
	{
		if(!playlist.Expand(iSong))
			break;
	}

	int iPreviousSong = m_iCurrentSong;
	m_iCurrentSong = iSong;
	CFileItemPtr item = playlist[m_iCurrentSong];
	playlist.SetPlayed(true);

	m_bPlaybackStarted = false;

	DWORD playAttempt = CTimeUtils::timeGetTime();
	
	if (!g_application.PlayFile(*item, bAutoPlay))
	{
		CLog::Log(LOGERROR,"Playlist Player: skipping unplayable item: %i, path [%s]", m_iCurrentSong, item->GetPath().c_str());
		playlist.SetUnPlayable(m_iCurrentSong);

		// Abort on 100 failed CONSECTUTIVE songs
		if (!m_iFailedSongs)
			m_failedSongsStart = playAttempt;

		m_iFailedSongs++;
		
		if ((m_iFailedSongs >= /*g_advancedSettings.m_playlistRetries*/10 && /*g_advancedSettings.m_playlistRetries*/10 >= 0) //TODO BRENT
			|| ((CTimeUtils::timeGetTime() - m_failedSongsStart  >= (unsigned int)/*g_advancedSettings.m_playlistTimeout*/10 * 1000) && /*g_advancedSettings.m_playlistTimeout*/10)) //TODO BRENT
		{
			CLog::Log(LOGDEBUG,"Playlist Player: one or more items failed to play... aborting playback");

			// Open error dialog
			CGUIDialogOK::ShowAndGetInput(16026, 16027, 16029, 0);

			CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_STOPPED, 0, 0, m_iCurrentPlayList, m_iCurrentSong);
			g_windowManager.SendThreadMessage(msg);
			Reset();
			GetPlaylist(m_iCurrentPlayList).Clear();
			m_iCurrentPlayList = PLAYLIST_NONE;
			m_iFailedSongs = 0;
			m_failedSongsStart = 0;

			return;
		}

		// How many playable items are in the playlist?
		if (playlist.GetPlayable() > 0)
		{
			if (bPlayPrevious)
				PlayPrevious();
			else
				PlayNext();

			return;
		}
		// None? then abort playback
		else
		{
			CLog::Log(LOGDEBUG,"Playlist Player: no more playable items... aborting playback");
			CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_STOPPED, 0, 0, m_iCurrentPlayList, m_iCurrentSong);
			g_windowManager.SendThreadMessage(msg);
			Reset();
			m_iCurrentPlayList = PLAYLIST_NONE;

			return;
		}
	}

	// TODO - move the above failure logic and the below success logic
	//        to callbacks instead so we don't rely on the return value
	//        of PlayFile()

	// Consecutive error counter so reset if the current item is playing
	m_iFailedSongs = 0;
	m_failedSongsStart = 0;
	m_bPlaybackStarted = true;
	m_bPlayedFirstFile = true;

	if (/*!item->IsShoutCast()*/1) //TODO BRENT
	{
		if (iPreviousSong < 0)
		{
			CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_STARTED, 0, 0, m_iCurrentPlayList, m_iCurrentSong, item);
			g_windowManager.SendThreadMessage( msg );
		}
		else
		{
			CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_CHANGED, 0, 0, m_iCurrentPlayList, MAKELONG(m_iCurrentSong, iPreviousSong), item);
			g_windowManager.SendThreadMessage(msg);
		}
	}
}

int CPlayListPlayer::GetNextSong(int offset) const
{
	if (m_iCurrentPlayList == PLAYLIST_NONE)
		return -1;

	const CPlayList& playlist = GetPlaylist(m_iCurrentPlayList);
	
	if (playlist.size() <= 0)
		return -1;

	int song = m_iCurrentSong;

	// Party mode
//	if (g_partyModeManager.IsEnabled() && GetCurrentPlaylist() == PLAYLIST_MUSIC) //TODO BRENT
//		return song + offset;

	// Wrap around in the case of repeating
	if (RepeatedOne(m_iCurrentPlayList))
		return song;

	song += offset;

	if (song >= playlist.size() && Repeated(m_iCurrentPlayList))
		song %= playlist.size();

	return song;
}

int CPlayListPlayer::GetNextSong()
{
	if (m_iCurrentPlayList == PLAYLIST_NONE)
		return -1;

	CPlayList& playlist = GetPlaylist(m_iCurrentPlayList);
	
	if (playlist.size() <= 0)
		return -1;
	
	int iSong = m_iCurrentSong;

	// Party mode
//	if (g_partyModeManager.IsEnabled() && GetCurrentPlaylist() == PLAYLIST_MUSIC) //TODO BRENT
//		return iSong + 1;

	// If repeat one, keep playing the current song if its valid
	if (RepeatedOne(m_iCurrentPlayList))
	{
		// Otherwise immediately abort playback
		if (m_iCurrentSong >= 0 && m_iCurrentSong < playlist.size() && playlist[m_iCurrentSong]->GetPropertyBOOL("unplayable"))
		{
			CLog::Log(LOGERROR,"Playlist Player: RepeatOne stuck on unplayable item: %i, path [%s]", m_iCurrentSong, playlist[m_iCurrentSong]->GetPath().c_str());
			CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_STOPPED, 0, 0, m_iCurrentPlayList, m_iCurrentSong);
			g_windowManager.SendThreadMessage(msg);
			Reset();
			m_iCurrentPlayList = PLAYLIST_NONE;
			
			return -1;
		}
		return iSong;
	}

	// If we've gone beyond the playlist and repeat all is enabled,
	// then we clear played status and wrap around
	iSong++;
	
	if (iSong >= playlist.size() && Repeated(m_iCurrentPlayList))
		iSong = 0;

	return iSong;
}

void CPlayListPlayer::ClearPlaylist(int iPlaylist)	
{
	// Clear our applications playlist file
	g_application.m_strPlayListFile.Empty();

	CPlayList& playlist = GetPlaylist(iPlaylist);
	playlist.Clear();

	// Its likely that the playlist changed
	CGUIMessage msg(GUI_MSG_PLAYLIST_CHANGED, 0, 0);
	g_windowManager.SendMessage(msg);
}

// Get the playlist object specified in nPlayList
// param nPlayList Values can be:
// - PLAYLIST_MUSIC - Playlist from music playlist window
// - PLAYLIST_VIDEO - Playlist from music playlist window
// return A reference to the CPlayList object
CPlayList& CPlayListPlayer::GetPlaylist(int iPlaylist)
{
	switch ( iPlaylist )
	{
		case PLAYLIST_MUSIC:
			return *m_PlaylistMusic;
		break;
		case PLAYLIST_VIDEO:
			return *m_PlaylistVideo;
		break;
		default:
			m_PlaylistEmpty->Clear();
			return *m_PlaylistEmpty;
		break;
	}
}

const CPlayList& CPlayListPlayer::GetPlaylist(int iPlaylist) const
{
	switch ( iPlaylist )
	{
		case PLAYLIST_MUSIC:
			return *m_PlaylistMusic;
		break;
		case PLAYLIST_VIDEO:
			return *m_PlaylistVideo;
		break;
		default:
			// NOTE: This playlist may not be empty if the caller of the non-const version alters it!
			return *m_PlaylistEmpty;
		break;
	}
}



// Change the current song in playlistplayer.
// param iSong Song in playlist
void CPlayListPlayer::SetCurrentSong(int iSong)
{
	if (iSong >= -1 && iSong < GetPlaylist(m_iCurrentPlayList).size())
		m_iCurrentSong = iSong;
}

// Returns to current song in active playlist.
// return Current song
int CPlayListPlayer::GetCurrentSong() const
{
	return m_iCurrentSong;
}

// Returns the active playlist.
// Active playlist
// Return values can be:
// - PLAYLIST_NONE No playlist active
// - PLAYLIST_MUSIC Playlist from music playlist window
// - PLAYLIST_VIDEO Playlist from music playlist window
int CPlayListPlayer::GetCurrentPlaylist() const
{
	return m_iCurrentPlayList;
}

// Set active playlist.
// param iPlayList Playlist to set active
// Values can be:
// - PLAYLIST_NONE - No playlist active
// - PLAYLIST_MUSIC - Playlist from music playlist window
// - PLAYLIST_VIDEO - Playlist from music playlist window
void CPlayListPlayer::SetCurrentPlaylist(int iPlaylist)
{
	if (iPlaylist == m_iCurrentPlayList)
		return;

	// Changing the current playlist while party mode is on
	// disables party mode
//	if (g_partyModeManager.IsEnabled()) // TODO BRENT
//		g_partyModeManager.Disable();

	m_iCurrentPlayList = iPlaylist;
	m_bPlayedFirstFile = false;
}

void CPlayListPlayer::Add(int iPlaylist, CPlayList& playlist)
{
	if (iPlaylist < PLAYLIST_MUSIC || iPlaylist > PLAYLIST_VIDEO)
		return;
	
	CPlayList& list = GetPlaylist(iPlaylist);
	
	int iSize = list.size();
	list.Add(playlist);
	
	if (list.IsShuffled())
		ReShuffle(iPlaylist, iSize);
}

void CPlayListPlayer::Add(int iPlaylist, CFileItemList& items)
{
	if (iPlaylist < PLAYLIST_MUSIC || iPlaylist > PLAYLIST_VIDEO)
		return;

	CPlayList& list = GetPlaylist(iPlaylist);
	int iSize = list.size();
	list.Add(items);

	if (list.IsShuffled())
		ReShuffle(iPlaylist, iSize);
}

// Returns true if iPlaylist is repeated
// param iPlaylist Playlist to be asked
bool CPlayListPlayer::Repeated(int iPlaylist) const
{
	if (iPlaylist >= PLAYLIST_MUSIC && iPlaylist <= PLAYLIST_VIDEO)
		return (m_repeatState[iPlaylist] == REPEAT_ALL);
	
	return false;
}

// Returns true if iPlaylist repeats one song
// param iPlaylist Playlist to be asked
bool CPlayListPlayer::RepeatedOne(int iPlaylist) const
{
	if (iPlaylist >= PLAYLIST_MUSIC && iPlaylist <= PLAYLIST_VIDEO)
		return (m_repeatState[iPlaylist] == REPEAT_ONE);
	
	return false;
}

void CPlayListPlayer::ReShuffle(int iPlaylist, int iPosition)
{
	// Playlist has not played yet so shuffle the entire list
	// (this only really works for new video playlists)
	if (!GetPlaylist(iPlaylist).WasPlayed())
	{
		GetPlaylist(iPlaylist).Shuffle();
	}

	// We're trying to shuffle new items into the curently playing playlist
	// so we shuffle starting at two positions below the current item
	else if (iPlaylist == m_iCurrentPlayList)
	{
		if (
			(g_application.IsPlayingAudio() && iPlaylist == PLAYLIST_MUSIC) ||
			(g_application.IsPlayingVideo() && iPlaylist == PLAYLIST_VIDEO)
			)
		{
			g_playlistPlayer.GetPlaylist(iPlaylist).Shuffle(m_iCurrentSong + 2);
		}
	}
	// Otherwise, shuffle from the passed position
	// which is the position of the first new item added
	else
		g_playlistPlayer.GetPlaylist(iPlaylist).Shuffle(iPosition);
}

// Resets the playlistplayer, but the active playlist stays the same.
void CPlayListPlayer::Reset()
{
	m_iCurrentSong = -1;
	m_bPlayedFirstFile = false;
	m_bPlaybackStarted = false;

	// its likely that the playlist changed
	CGUIMessage msg(GUI_MSG_PLAYLIST_CHANGED, 0, 0);
	g_windowManager.SendMessage(msg);
}

void CPlayListPlayer::Clear()
{
	if (m_PlaylistMusic)
		m_PlaylistMusic->Clear();

	if (m_PlaylistVideo)
		m_PlaylistVideo->Clear();

	if (m_PlaylistEmpty)
		m_PlaylistEmpty->Clear();
}