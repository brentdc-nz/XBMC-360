#ifndef H_CGUIVIEWSTATEVIDEOS
#define H_CGUIVIEWSTATEVIDEOS

#include "GUIViewState.h"

class CGUIViewStateWindowVideo : public CGUIViewState
{
public:
	CGUIViewStateWindowVideo(const CFileItemList& items) : CGUIViewState(items) {}

protected:
	virtual CStdString GetLockType();
	virtual bool UnrollArchives();
	virtual int GetPlaylist();
	virtual CStdString GetExtensions();
};

class CGUIViewStateWindowVideoFiles : public CGUIViewStateWindowVideo
{
public:
	CGUIViewStateWindowVideoFiles(const CFileItemList& items);

protected:
	virtual void SaveViewState();
	virtual VECSOURCES& GetSources();
};

/* TODO - CGUIViewStateWindowVideoNav not yet implemented (requires VideoDatabaseDirectory, PluginDirectory)
class CGUIViewStateWindowVideoNav : public CGUIViewStateWindowVideo
{
public:
	CGUIViewStateWindowVideoNav(const CFileItemList& items);
	virtual bool AutoPlayNextItem();

protected:
	virtual void SaveViewState();
	virtual VECSOURCES& GetSources();
};
*/

/* TODO - CGUIViewStateWindowVideoPlaylist not yet implemented (requires PlaylistPlayer)
class CGUIViewStateWindowVideoPlaylist : public CGUIViewStateWindowVideo
{
public:
	CGUIViewStateWindowVideoPlaylist(const CFileItemList& items);

protected:
	virtual void SaveViewState();
	virtual bool HideExtensions();
	virtual bool HideParentDirItems();
	virtual VECSOURCES& GetSources();
};
*/

/* TODO - CGUIViewStateVideoMovies not yet implemented (requires VideoDatabase)
class CGUIViewStateVideoMovies : public CGUIViewStateWindowVideo
{
public:
	CGUIViewStateVideoMovies(const CFileItemList& items);
protected:
	virtual void SaveViewState();
};
*/

/* TODO - CGUIViewStateVideoMusicVideos not yet implemented (requires VideoDatabase)
class CGUIViewStateVideoMusicVideos : public CGUIViewStateWindowVideo
{
public:
	CGUIViewStateVideoMusicVideos(const CFileItemList& items);
protected:
	virtual void SaveViewState();
};
*/

/* TODO - CGUIViewStateVideoTVShows not yet implemented (requires VideoDatabase)
class CGUIViewStateVideoTVShows : public CGUIViewStateWindowVideo
{
public:
	CGUIViewStateVideoTVShows(const CFileItemList& items);
protected:
	virtual void SaveViewState();
};
*/

/* TODO - CGUIViewStateVideoEpisodes not yet implemented (requires VideoDatabase)
class CGUIViewStateVideoEpisodes : public CGUIViewStateWindowVideo
{
public:
	CGUIViewStateVideoEpisodes(const CFileItemList& items);
protected:
	virtual void SaveViewState();
};
*/

#endif //H_CGUIVIEWSTATEVIDEOS