#include "GUIViewStateMusic.h"
//#include "utils/log.h" // TODO - Log not fully ported
#include "PlayListPlayer.h"
#include "guilib\GUIBaseContainer.h" // for VIEW_TYPE_*
//#include "video/VideoDatabase.h" // TODO - VideoDatabase not yet implemented
#include "Settings.h"
//#include "settings/AdvancedSettings.h" // TODO - AdvancedSettings not yet implemented
#include "FileItem.h"
//#include "Util.h" // TODO - Util not fully ported
//#include "LocalizeStrings.h" // TODO - LocalizeStrings not fully ported
//#include "FileSystem/MusicDatabaseDirectory.h" // TODO - MusicDatabaseDirectory not yet implemented
//#include "FileSystem/VideoDatabaseDirectory.h" // TODO - VideoDatabaseDirectory not yet implemented
//#include "FileSystem/PluginDirectory.h" // TODO - PluginDirectory not yet implemented
#include "ViewState.h"
#include "guilib\key.h"

//using namespace XFILE; // TODO - Not yet implemented
//using namespace MUSICDATABASEDIRECTORY; // TODO - Not yet implemented

int CGUIViewStateWindowMusic::GetPlaylist()
{
	return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusic::UnrollArchives()
{
	return false; //g_guiSettings.GetBool("filelists.unrollarchives"); // TODO - g_guiSettings not yet implemented
}

bool CGUIViewStateWindowMusic::AutoPlayNextItem()
{
	return true; // TODO - g_guiSettings not yet implemented
	//return g_guiSettings.GetBool("musicplayer.autoplaynextitem") &&
	//       !g_guiSettings.GetBool("musicplayer.queuebydefault");
}

CStdString CGUIViewStateWindowMusic::GetLockType()
{
	return "music";
}

CStdString CGUIViewStateWindowMusic::GetExtensions()
{
	return g_settings.GetAudioExtensions();
}

/* TODO - CGUIViewStateMusicSearch not yet implemented (requires MusicDatabase)
CGUIViewStateMusicSearch::CGUIViewStateMusicSearch(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
	CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.librarytrackformat");
	if (strTrackLeft.IsEmpty())
		strTrackLeft = g_guiSettings.GetString("musicfiles.trackformat");
	CStdString strTrackRight=g_guiSettings.GetString("musicfiles.librarytrackformatright");
	if (strTrackRight.IsEmpty())
		strTrackRight = g_guiSettings.GetString("musicfiles.trackformatright");

	CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
	if (strAlbumLeft.IsEmpty())
		strAlbumLeft = "%B"; // album
	CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
	if (strAlbumRight.IsEmpty())
		strAlbumRight = "%A"; // artist

	if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
	{
		AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D", "%L", "%A"));
		SetSortMethod(SORT_METHOD_TITLE_IGNORE_THE);
	}
	else
	{
		AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D", "%L", "%A"));
		SetSortMethod(SORT_METHOD_TITLE);
	}

	SetViewAsControl(g_settings.m_viewStateMusicNavSongs.m_viewMode);
	SetSortOrder(g_settings.m_viewStateMusicNavSongs.m_sortOrder);
	LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSearch::SaveViewState()
{
	SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, &g_settings.m_viewStateMusicNavSongs);
}
*/

/* TODO - CGUIViewStateMusicDatabase not yet implemented (requires MusicDatabaseDirectory)
CGUIViewStateMusicDatabase::CGUIViewStateMusicDatabase(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
	CMusicDatabaseDirectory dir;
	NODE_TYPE NodeType=dir.GetDirectoryChildType(items.GetPath());

	CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.librarytrackformat");
	if (strTrackLeft.IsEmpty())
		strTrackLeft = g_guiSettings.GetString("musicfiles.trackformat");
	CStdString strTrackRight=g_guiSettings.GetString("musicfiles.librarytrackformatright");
	if (strTrackRight.IsEmpty())
		strTrackRight = g_guiSettings.GetString("musicfiles.trackformatright");

	CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
	if (strAlbumLeft.IsEmpty())
		strAlbumLeft = "%B"; // album
	CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
	if (strAlbumRight.IsEmpty())
		strAlbumRight = "%A"; // artist

	CLog::Log(LOGDEBUG,"Album format left  = [%s]", strAlbumLeft.c_str());
	CLog::Log(LOGDEBUG,"Album format right = [%s]", strAlbumRight.c_str());

	switch (NodeType)
	{
	case NODE_TYPE_OVERVIEW:
		{
			AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", "%L", ""));
			SetSortMethod(SORT_METHOD_NONE);
			SetViewAsControl(DEFAULT_VIEW_LIST);
			SetSortOrder(SORT_ORDER_NONE);
		}
		break;
	case NODE_TYPE_TOP100:
		{
			AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", "%L", ""));
			SetSortMethod(SORT_METHOD_NONE);
			SetViewAsControl(DEFAULT_VIEW_LIST);
			SetSortOrder(SORT_ORDER_NONE);
		}
		break;
	case NODE_TYPE_GENRE:
		{
			AddSortMethod(SORT_METHOD_GENRE, 551, LABEL_MASKS("%F", "", "%G", ""));
			SetSortMethod(SORT_METHOD_GENRE);
			SetViewAsControl(DEFAULT_VIEW_LIST);
			SetSortOrder(SORT_ORDER_ASC);
		}
		break;
	case NODE_TYPE_YEAR:
		{
			AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%F", "", "%Y", ""));
			SetSortMethod(SORT_METHOD_LABEL);
			SetViewAsControl(DEFAULT_VIEW_LIST);
			SetSortOrder(SORT_ORDER_ASC);
		}
		break;
	case NODE_TYPE_ARTIST:
		{
			if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
			{
				AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 551, LABEL_MASKS("%F", "", "%A", ""));
				SetSortMethod(SORT_METHOD_ARTIST_IGNORE_THE);
			}
			else
			{
				AddSortMethod(SORT_METHOD_ARTIST, 551, LABEL_MASKS("%F", "", "%A", ""));
				SetSortMethod(SORT_METHOD_ARTIST);
			}
			SetViewAsControl(g_settings.m_viewStateMusicNavArtists.m_viewMode);
			SetSortOrder(g_settings.m_viewStateMusicNavArtists.m_sortOrder);
		}
		break;
	case NODE_TYPE_ALBUM_COMPILATIONS:
	case NODE_TYPE_ALBUM:
	case NODE_TYPE_YEAR_ALBUM:
		{
			if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
				AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));
			else
				AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

			if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
				AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));
			else
				AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

			AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

			SetSortMethod(g_settings.m_viewStateMusicNavAlbums.m_sortMethod);
			SetViewAsControl(g_settings.m_viewStateMusicNavAlbums.m_viewMode);
			SetSortOrder(g_settings.m_viewStateMusicNavAlbums.m_sortOrder);
		}
		break;
	case NODE_TYPE_ALBUM_RECENTLY_ADDED:
		{
			AddSortMethod(SORT_METHOD_NONE, 552, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));
			SetSortMethod(SORT_METHOD_NONE);
			SetViewAsControl(g_settings.m_viewStateMusicNavAlbums.m_viewMode);
			SetSortOrder(SORT_ORDER_NONE);
		}
		break;
	case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
		{
			AddSortMethod(SORT_METHOD_NONE, 552, LABEL_MASKS(strTrackLeft, strTrackRight));
			SetSortMethod(SORT_METHOD_NONE);
			SetViewAsControl(g_settings.m_viewStateMusicNavSongs.m_viewMode);
			SetSortOrder(SORT_ORDER_NONE);
		}
		break;
	case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
		{
			AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));
			SetSortMethod(SORT_METHOD_NONE);
			SetViewAsControl(g_settings.m_viewStateMusicNavAlbums.m_viewMode);
			SetSortOrder(SORT_ORDER_NONE);
		}
		break;
	case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
		{
			AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
			SetSortMethod(SORT_METHOD_NONE);
			SetViewAsControl(g_settings.m_viewStateMusicNavAlbums.m_viewMode);
			SetSortOrder(SORT_ORDER_NONE);
		}
		break;
	case NODE_TYPE_ALBUM_TOP100:
		{
			AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));
			SetSortMethod(SORT_METHOD_NONE);
			SetViewAsControl(DEFAULT_VIEW_LIST);
			SetSortOrder(SORT_ORDER_NONE);
		}
		break;
	case NODE_TYPE_SINGLES:
		{
			if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
			{
				AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));
				AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));
				AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
			}
			else
			{
				AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));
				AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));
				AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
			}
			AddSortMethod(SORT_METHOD_DURATION, 555, LABEL_MASKS("%T - %A", "%D"));
			AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));

			SetSortMethod(g_settings.m_viewStateMusicNavSongs.m_sortMethod);
			SetViewAsControl(g_settings.m_viewStateMusicNavSongs.m_viewMode);
			SetSortOrder(g_settings.m_viewStateMusicNavSongs.m_sortOrder);
		}
		break;
	case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
	case NODE_TYPE_ALBUM_TOP100_SONGS:
	case NODE_TYPE_YEAR_SONG:
	case NODE_TYPE_SONG:
		{
			AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));
			if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
			{
				AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));
				AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T - %A", "%D"));
				AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));
				AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
			}
			else
			{
				AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));
				AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T - %A", "%D"));
				AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));
				AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
			}
			AddSortMethod(SORT_METHOD_DURATION, 555, LABEL_MASKS("%T - %A", "%D"));
			AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));

			if (dir.IsAllItem(items.GetPath()))
				SetSortMethod(g_guiSettings.GetBool("filelists.ignorethewhensorting") ? SORT_METHOD_ALBUM_IGNORE_THE : SORT_METHOD_ALBUM);
			else
				SetSortMethod(g_settings.m_viewStateMusicNavSongs.m_sortMethod);

			SetViewAsControl(g_settings.m_viewStateMusicNavSongs.m_viewMode);
			SetSortOrder(g_settings.m_viewStateMusicNavSongs.m_sortOrder);
		}
		break;
	case NODE_TYPE_SONG_TOP100:
		{
			AddSortMethod(SORT_METHOD_NONE, 554, LABEL_MASKS(strTrackLeft, strTrackRight));
			SetSortMethod(SORT_METHOD_NONE);
			SetViewAsControl(g_settings.m_viewStateMusicNavSongs.m_viewMode);
			SetSortOrder(SORT_ORDER_NONE);
		}
		break;
	default:
		break;
	}

	LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicDatabase::SaveViewState()
{
	CMusicDatabaseDirectory dir;
	NODE_TYPE NodeType=dir.GetDirectoryChildType(m_items.GetPath());

	switch (NodeType)
	{
		case NODE_TYPE_ARTIST:
			SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, &g_settings.m_viewStateMusicNavArtists);
			break;
		case NODE_TYPE_ALBUM_COMPILATIONS:
		case NODE_TYPE_ALBUM:
		case NODE_TYPE_YEAR_ALBUM:
			SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, &g_settings.m_viewStateMusicNavAlbums);
			break;
		case NODE_TYPE_ALBUM_RECENTLY_ADDED:
		case NODE_TYPE_ALBUM_TOP100:
		case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
			SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
			break;
		case NODE_TYPE_SINGLES:
		case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
		case NODE_TYPE_SONG:
		case NODE_TYPE_YEAR_SONG:
			SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, &g_settings.m_viewStateMusicNavSongs);
			break;
		case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
		case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
		case NODE_TYPE_SONG_TOP100:
			SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
			break;
		default:
			SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
			break;
	}
}
*/

/* TODO - CGUIViewStateMusicSmartPlaylist not yet implemented (requires SmartPlayList)
CGUIViewStateMusicSmartPlaylist::CGUIViewStateMusicSmartPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
	if (items.GetContent() == "songs" || items.GetContent() == "mixed")
	{
		CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
		CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");

		AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS(strTrackLeft, strTrackRight));
		AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));
		if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
		{
			AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));
			AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T - %A", "%D"));
			AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));
			AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
		}
		else
		{
			AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));
			AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T - %A", "%D"));
			AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));
			AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
		}
		AddSortMethod(SORT_METHOD_DURATION, 555, LABEL_MASKS("%T - %A", "%D"));
		AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));
		SetSortMethod(SORT_METHOD_PLAYLIST_ORDER);

		SetViewAsControl(g_settings.m_viewStateMusicNavSongs.m_viewMode);
		SetSortOrder(g_settings.m_viewStateMusicNavSongs.m_sortOrder);
	}
	else if (items.GetContent() == "albums")
	{
		CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
		if (strAlbumLeft.IsEmpty())
			strAlbumLeft = "%B";
		CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
		if (strAlbumRight.IsEmpty())
			strAlbumRight = "%A";

		AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));
		if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
			AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));
		else
			AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

		if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
			AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));
		else
			AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

		AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

		SetSortMethod(SORT_METHOD_PLAYLIST_ORDER);
		SetViewAsControl(g_settings.m_viewStateMusicNavAlbums.m_viewMode);
		SetSortOrder(g_settings.m_viewStateMusicNavAlbums.m_sortOrder);
	}
	else
	{
		CLog::Log(LOGERROR,"Music Smart Playlist must be one of songs, mixed or albums");
	}

	LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSmartPlaylist::SaveViewState()
{
	SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, &g_settings.m_viewStateMusicNavSongs);
}
*/

/* TODO - CGUIViewStateMusicPlaylist not yet implemented (requires Playlist)
CGUIViewStateMusicPlaylist::CGUIViewStateMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
	CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
	CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");

	AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS(strTrackLeft, strTrackRight));
	AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));
	if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
	{
		AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));
		AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T - %A", "%D"));
		AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));
		AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
	}
	else
	{
		AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));
		AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T - %A", "%D"));
		AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));
		AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
	}
	AddSortMethod(SORT_METHOD_DURATION, 555, LABEL_MASKS("%T - %A", "%D"));
	AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));

	SetSortMethod(SORT_METHOD_PLAYLIST_ORDER);
	SetViewAsControl(g_settings.m_viewStateMusicFiles.m_viewMode);
	SetSortOrder(g_settings.m_viewStateMusicFiles.m_sortOrder);

	LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

void CGUIViewStateMusicPlaylist::SaveViewState()
{
	SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES);
}
*/

/* TODO - CGUIViewStateWindowMusicNav not yet implemented (requires MusicDatabase, VideoDatabase, PluginDirectory)
CGUIViewStateWindowMusicNav::CGUIViewStateWindowMusicNav(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
	if (items.IsVirtualDirectoryRoot())
	{
		AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "%I", "%L", ""));
		SetSortMethod(SORT_METHOD_NONE);
		SetViewAsControl(DEFAULT_VIEW_LIST);
		SetSortOrder(SORT_ORDER_NONE);
	}
	else
	{
		if (items.IsVideoDb() && items.Size() > (!g_guiSettings.GetBool("filelists.showparentdiritems")?0:1))
		{
			XFILE::VIDEODATABASEDIRECTORY::CQueryParams params;
			XFILE::CVideoDatabaseDirectory::GetQueryParams(items[!g_guiSettings.GetBool("filelists.showparentdiritems")?0:1]->GetPath(),params);
			if (params.GetMVideoId() != -1)
			{
				if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
					AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 556, LABEL_MASKS("%T", "%Y"));
				else
					AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%T", "%Y"));
				AddSortMethod(SORT_METHOD_YEAR,562, LABEL_MASKS("%T", "%Y"));
				if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
				{
					AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE,557, LABEL_MASKS("%A - %T", "%Y"));
					AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE,558, LABEL_MASKS("%B - %T", "%Y"));
				}
				else
				{
					AddSortMethod(SORT_METHOD_ARTIST,557, LABEL_MASKS("%A - %T", "%Y"));
					AddSortMethod(SORT_METHOD_ALBUM,558, LABEL_MASKS("%B - %T", "%Y"));
				}
				CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
				CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");
				AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));
			}
			else
			{
				AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%F", "%D", "%L", ""));
				SetSortMethod(SORT_METHOD_LABEL);
			}
		}
		else
		{
			AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%F", "%D", "%L", ""));
			SetSortMethod(SORT_METHOD_LABEL);
		}
		SetViewAsControl(DEFAULT_VIEW_LIST);
		SetSortOrder(SORT_ORDER_ASC);
	}
	LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::SaveViewState()
{
	SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::AddOnlineShares()
{
	if (!g_advancedSettings.m_bVirtualShares) return;
	for (int i = 0; i < (int)g_settings.m_musicSources.size(); ++i)
	{
		CMediaSource share = g_settings.m_musicSources.at(i);
		if (share.strPath.Find("lastfm://") == 0)
			m_sources.push_back(share);
	}
}

VECSOURCES& CGUIViewStateWindowMusicNav::GetSources()
{
	m_sources.clear();
	CFileItemList items;
	CDirectory::GetDirectory("musicdb://", items);
	for (int i=0; i<items.Size(); ++i)
	{
		CFileItemPtr item=items[i];
		CMediaSource share;
		share.strName=item->GetLabel();
		share.strPath = item->GetPath();
		share.m_strThumbnailImage = item->GetIconImage();
		share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
		m_sources.push_back(share);
	}

	CMediaSource share;
	share.strName=g_localizeStrings.Get(136); // Playlists
	share.strPath = "special://musicplaylists/";
	share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicPlaylists.png");
	share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
	m_sources.push_back(share);

	AddOnlineShares();

	share.strName=g_localizeStrings.Get(137); // Search
	share.strPath = "musicsearch://";
	share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicSearch.png");
	share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
	m_sources.push_back(share);

	CVideoDatabase database;
	database.Open();
	if (database.HasContent(VIDEODB_CONTENT_MUSICVIDEOS))
	{
		share.strName = g_localizeStrings.Get(20389);
		share.strPath = "videodb://3/";
		share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicVideos.png");
		share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
		m_sources.push_back(share);
	}

	if (CPluginDirectory::HasPlugins("music") && g_advancedSettings.m_bVirtualShares)
	{
		share.strName = g_localizeStrings.Get(1038);
		share.strPath = "plugin://music/";
		share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicPlugins.png");
		share.m_ignore = true;
		m_sources.push_back(share);
	}

	return CGUIViewStateWindowMusic::GetSources();
}
*/

CGUIViewStateWindowMusicSongs::CGUIViewStateWindowMusicSongs(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
	if (items.IsVirtualDirectoryRoot())
	{
		AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS()); // Preformated
		AddSortMethod(SORT_METHOD_DRIVE_TYPE, 564, LABEL_MASKS()); // Preformated
		SetSortMethod(SORT_METHOD_LABEL);

		SetViewAsControl(DEFAULT_VIEW_LIST);

		SetSortOrder(SORT_ORDER_ASC);
	}
	/* TODO - Music playlists path not yet implemented
	else if (items.GetPath() == "special://musicplaylists/")
	{
		AddSortMethod(SORT_METHOD_LABEL_IGNORE_FOLDERS, 551, LABEL_MASKS("%F", "%D", "%L", ""));
		SetSortMethod(SORT_METHOD_LABEL_IGNORE_FOLDERS);
	}
	*/
	else
	{
		// TODO - g_guiSettings not yet implemented, using hardcoded defaults
		//CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
		//CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");
		CStdString strTrackLeft = "%L";
		CStdString strTrackRight = "%I";

		//if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
			AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", "")); // Userdefined, Userdefined | FolderName, empty
		//else
		//	AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));
		AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS(strTrackLeft, "%I", "%L", "%I")); // Userdefined, Size | FolderName, Size
		AddSortMethod(SORT_METHOD_BITRATE, 623, LABEL_MASKS(strTrackLeft, "%X", "%L", "%X")); // Userdefined, Bitrate | FolderName, Bitrate
		AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS(strTrackLeft, "%J", "%L", "%J")); // Userdefined, Date | FolderName, Date
		AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", "")); // Userdefined, Userdefined | FolderName, empty
		AddSortMethod(SORT_METHOD_LISTENERS, 20455, LABEL_MASKS(strTrackLeft, "%W", "%L", "%W"));

		SetSortMethod(SORT_METHOD_LABEL); // TODO - g_settings.m_viewStateMusicFiles.m_sortMethod
		SetViewAsControl(DEFAULT_VIEW_LIST); // TODO - g_settings.m_viewStateMusicFiles.m_viewMode
		SetSortOrder(SORT_ORDER_ASC); // TODO - g_settings.m_viewStateMusicFiles.m_sortOrder
	}
	LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

void CGUIViewStateWindowMusicSongs::SaveViewState()
{
	//SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES, &g_settings.m_viewStateMusicFiles); // TODO - ViewDatabase not yet implemented
}

VECSOURCES& CGUIViewStateWindowMusicSongs::GetSources()
{
	/* TODO - PluginDirectory not yet implemented
	if (CPluginDirectory::HasPlugins("music") && g_advancedSettings.m_bVirtualShares)
	{
		CMediaSource share;
		share.strName = g_localizeStrings.Get(1038);
		share.strPath = "plugin://music/";
		AddOrReplace(g_settings.m_musicSources,share);
	}
	*/
	return g_settings.m_musicSources;
}

/* TODO - CGUIViewStateWindowMusicPlaylist not yet implemented (requires PlaylistPlayer)
CGUIViewStateWindowMusicPlaylist::CGUIViewStateWindowMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
	CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.nowplayingtrackformat");
	if (strTrackLeft.IsEmpty())
		strTrackLeft = g_guiSettings.GetString("musicfiles.trackformat");
	CStdString strTrackRight=g_guiSettings.GetString("musicfiles.nowplayingtrackformatright");
	if (strTrackRight.IsEmpty())
		strTrackRight = g_guiSettings.GetString("musicfiles.trackformatright");

	AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));
	SetSortMethod(SORT_METHOD_NONE);
	SetViewAsControl(DEFAULT_VIEW_LIST);
	SetSortOrder(SORT_ORDER_NONE);

	LoadViewState(items.GetPath(), WINDOW_MUSIC_PLAYLIST);
}

void CGUIViewStateWindowMusicPlaylist::SaveViewState()
{
	SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_PLAYLIST);
}

int CGUIViewStateWindowMusicPlaylist::GetPlaylist()
{
	return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusicPlaylist::AutoPlayNextItem()
{
	return false;
}

bool CGUIViewStateWindowMusicPlaylist::HideParentDirItems()
{
	return true;
}

VECSOURCES& CGUIViewStateWindowMusicPlaylist::GetSources()
{
	m_sources.clear();
	CMediaSource share;
	share.strPath = "playlistmusic://";
	share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
	m_sources.push_back(share);

	return CGUIViewStateWindowMusic::GetSources();
}
*/

/* TODO - CGUIViewStateMusicShoutcast not yet implemented (requires Shoutcast)
CGUIViewStateMusicShoutcast::CGUIViewStateMusicShoutcast(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
	if( true || m_items.m_idepth > 1 )
	{
		AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%K", "%B kbps", "%K", ""));
		AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%K", "%A listeners", "%K", ""));
		AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS("%K", "%B kbps", "%K", ""));

		SetSortMethod(g_settings.m_viewStateMusicShoutcast.m_sortMethod);
		SetSortOrder(g_settings.m_viewStateMusicShoutcast.m_sortOrder);
	}
	else
	{
		AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%K", "", "%K", ""));
		SetSortMethod(SORT_METHOD_LABEL);
		SetSortOrder(SORT_ORDER_ASC);
	}

	SetViewAsControl(DEFAULT_VIEW_LIST);
	LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

bool CGUIViewStateMusicShoutcast::AutoPlayNextItem()
{
	return false;
}

void CGUIViewStateMusicShoutcast::SaveViewState()
{
	SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES, &g_settings.m_viewStateMusicShoutcast);
}
*/

/* TODO - CGUIViewStateMusicLastFM not yet implemented (requires LastFM)
CGUIViewStateMusicLastFM::CGUIViewStateMusicLastFM(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
	CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
	CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");

	AddSortMethod(SORT_METHOD_UNSORTED, 571, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));
	AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));
	AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS(strTrackLeft, "%I", "%L", "%I"));

	SetSortMethod(g_settings.m_viewStateMusicLastFM.m_sortMethod);
	SetSortOrder(g_settings.m_viewStateMusicLastFM.m_sortOrder);

	SetViewAsControl(DEFAULT_VIEW_LIST);
	LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

bool CGUIViewStateMusicLastFM::AutoPlayNextItem()
{
	return false;
}

void CGUIViewStateMusicLastFM::SaveViewState()
{
	SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES, &g_settings.m_viewStateMusicLastFM);
}
*/