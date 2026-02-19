#ifndef H_CGUIVIEWSTATEMUSIC
#define H_CGUIVIEWSTATEMUSIC

#include "GUIViewState.h"

class CGUIViewStateWindowMusic : public CGUIViewState
{
public:
	CGUIViewStateWindowMusic(const CFileItemList& items) : CGUIViewState(items) {}
protected:
	virtual int GetPlaylist();
	virtual bool UnrollArchives();
	virtual bool AutoPlayNextItem();
	virtual CStdString GetLockType();
	virtual CStdString GetExtensions();
};

/* TODO - CGUIViewStateMusicSearch not yet implemented (requires MusicDatabase)
class CGUIViewStateMusicSearch : public CGUIViewStateWindowMusic
{
public:
	CGUIViewStateMusicSearch(const CFileItemList& items);

protected:
	virtual void SaveViewState();
};
*/

/* TODO - CGUIViewStateMusicDatabase not yet implemented (requires MusicDatabase)
class CGUIViewStateMusicDatabase : public CGUIViewStateWindowMusic
{
public:
	CGUIViewStateMusicDatabase(const CFileItemList& items);

protected:
	virtual void SaveViewState();
};
*/

/* TODO - CGUIViewStateMusicSmartPlaylist not yet implemented (requires SmartPlaylist)
class CGUIViewStateMusicSmartPlaylist : public CGUIViewStateWindowMusic
{
public:
	CGUIViewStateMusicSmartPlaylist(const CFileItemList& items);

protected:
	virtual void SaveViewState();
};
*/

/* TODO - CGUIViewStateMusicPlaylist not yet implemented (requires Playlist)
class CGUIViewStateMusicPlaylist : public CGUIViewStateWindowMusic
{
public:
	CGUIViewStateMusicPlaylist(const CFileItemList& items);

protected:
	virtual void SaveViewState();
};
*/

/* TODO - CGUIViewStateWindowMusicNav not yet implemented (requires MusicDatabase, VideoDatabase, PluginDirectory)
class CGUIViewStateWindowMusicNav : public CGUIViewStateWindowMusic
{
public:
	CGUIViewStateWindowMusicNav(const CFileItemList& items);

protected:
	virtual void SaveViewState();
	virtual VECSOURCES& GetSources();

private:
	void AddOnlineShares();
};
*/

class CGUIViewStateWindowMusicSongs : public CGUIViewStateWindowMusic
{
public:
	CGUIViewStateWindowMusicSongs(const CFileItemList& items);

protected:
	virtual void SaveViewState();
	virtual VECSOURCES& GetSources();
};

/* TODO - CGUIViewStateWindowMusicPlaylist not yet implemented (requires PlaylistPlayer)
class CGUIViewStateWindowMusicPlaylist : public CGUIViewStateWindowMusic
{
public:
	CGUIViewStateWindowMusicPlaylist(const CFileItemList& items);

protected:
	virtual void SaveViewState();
	virtual int GetPlaylist();
	virtual bool AutoPlayNextItem();
	virtual bool HideParentDirItems();
	virtual VECSOURCES& GetSources();
};
*/

/* TODO - CGUIViewStateMusicShoutcast not yet implemented (requires Shoutcast)
class CGUIViewStateMusicShoutcast : public CGUIViewStateWindowMusic
{
public:
	CGUIViewStateMusicShoutcast(const CFileItemList& items);

protected:
	virtual bool AutoPlayNextItem();
	virtual void SaveViewState();
};
*/

/* TODO - CGUIViewStateMusicLastFM not yet implemented (requires LastFM)
class CGUIViewStateMusicLastFM : public CGUIViewStateWindowMusic
{
public:
	CGUIViewStateMusicLastFM(const CFileItemList& items);

protected:
	virtual bool AutoPlayNextItem();
	virtual void SaveViewState();
};
*/

#endif //H_CGUIVIEWSTATEMUSIC