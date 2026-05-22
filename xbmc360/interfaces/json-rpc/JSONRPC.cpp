/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "JSONRPC.h"
#include "utils\Log.h"
#include "utils\Variant.h"
#include "utils\JSONVariantParser.h"
#include "utils\SingleLock.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "PlayListPlayer.h"
#include "playlists\PlayList.h"
#include "FileItem.h"
#include "Settings.h"
#include "guilib\Key.h"
#include "video\VideoInfoTag.h"

using namespace JSONRPC;
using namespace PLAYLIST;

extern CApplication g_application;
extern CPlayListPlayer g_playlistPlayer;

//
// ===== Input Operations State (matches Eden CInputOperations) =====
//
// Virtual key codes matching Eden's XBMC_vkeys.h
#define XBMCVK_BACK    0x08
#define XBMCVK_RETURN  0x0D
#define XBMCVK_UP      0x80
#define XBMCVK_DOWN    0x81
#define XBMCVK_LEFT    0x82
#define XBMCVK_RIGHT   0x83

static CCriticalSection s_inputCritSection;
static uint32_t s_pendingKey = KEY_INVALID;

uint32_t CJSONRPC::GetInputKey()
{
	CSingleLock lock(s_inputCritSection);
	uint32_t currentKey = s_pendingKey;
	s_pendingKey = KEY_INVALID;
	return currentKey;
}

//
// ===== JSON-RPC Status Codes (matches Eden) =====
//
enum JSON_STATUS
{
	OK = 0,
	ACK = 1,
	InvalidRequest = -32600,
	MethodNotFound = -32601,
	InvalidParams = -32602,
	InternalError = -32603,
	ParseError = -32700,
	BadPermission = -32099,
	FailedToExecute = -32100
};

//
// ===== Response Builders (matches Eden CJSONRPC::BuildResponse) =====
//
static void BuildResponse(const CVariant &request, JSON_STATUS code, const CVariant &result, CVariant &response)
{
	response["jsonrpc"] = "2.0";
	response["id"] = request.isMember("id") ? request["id"] : CVariant();

	switch (code)
	{
		case OK:
			response["result"] = result;
			break;
		case ACK:
			response["result"] = CVariant("OK");
			break;
		case InvalidRequest:
			response["error"]["code"] = InvalidRequest;
			response["error"]["message"] = "Invalid request.";
			break;
		case InvalidParams:
			response["error"]["code"] = InvalidParams;
			response["error"]["message"] = "Invalid params.";
			break;
		case MethodNotFound:
			response["error"]["code"] = MethodNotFound;
			response["error"]["message"] = "Method not found.";
			break;
		case ParseError:
			response["error"]["code"] = ParseError;
			response["error"]["message"] = "Parse error.";
			break;
		case BadPermission:
			response["error"]["code"] = BadPermission;
			response["error"]["message"] = "Bad client permission.";
			break;
		case FailedToExecute:
			response["error"]["code"] = FailedToExecute;
			response["error"]["message"] = "Failed to execute method.";
			break;
		default:
			response["error"]["code"] = InternalError;
			response["error"]["message"] = "Internal error.";
			break;
	}
}

//
// ===== Player Operations (matches Eden CPlayerOperations) =====
//
static int GetActivePlayers()
{
	int players = 0;
	if (g_application.IsPlayingVideo())
		players |= 0x1; // Video
	if (g_application.IsPlayingAudio())
		players |= 0x2; // Audio
	return players;
}

static int GetPlaylist(int playerType)
{
	if (playerType == 0x1) return PLAYLIST_VIDEO;
	if (playerType == 0x2) return PLAYLIST_MUSIC;
	return -1;
}

static JSON_STATUS PlayerGetActivePlayers(const CVariant &params, CVariant &result)
{
	result = CVariant(CVariant::VariantTypeArray);
	int activePlayers = GetActivePlayers();

	if (activePlayers & 0x1)
	{
		CVariant video(CVariant::VariantTypeObject);
		video["playerid"] = PLAYLIST_VIDEO;
		video["type"] = "video";
		result.append(video);
	}
	if (activePlayers & 0x2)
	{
		CVariant audio(CVariant::VariantTypeObject);
		audio["playerid"] = PLAYLIST_MUSIC;
		audio["type"] = "audio";
		result.append(audio);
	}

	return OK;
}

static JSON_STATUS PlayerGetProperties(const CVariant &params, CVariant &result)
{
	result = CVariant(CVariant::VariantTypeObject);

	int playlist = g_playlistPlayer.GetCurrentPlaylist();

	for (unsigned int i = 0; i < params["properties"].size(); i++)
	{
		std::string prop = params["properties"][i].asString();

		if (prop == "type")
		{
			if (g_application.IsPlayingVideo())
				result["type"] = "video";
			else if (g_application.IsPlayingAudio())
				result["type"] = "audio";
			else
				result["type"] = "unknown";
		}
		else if (prop == "playlistid")
			result["playlistid"] = playlist;
		else if (prop == "speed")
			result["speed"] = g_application.IsPaused() ? 0 : g_application.GetPlaySpeed();
		else if (prop == "position")
		{
			result["position"] = g_playlistPlayer.GetCurrentSong();
		}
		else if (prop == "totaltime")
		{
			int total = (int)g_application.GetTotalTime();
			CVariant t(CVariant::VariantTypeObject);
			t["hours"] = total / 3600;
			t["minutes"] = (total % 3600) / 60;
			t["seconds"] = total % 60;
			t["milliseconds"] = 0;
			result["totaltime"] = t;
		}
		else if (prop == "time")
		{
			int current = (int)g_application.GetTime();
			CVariant t(CVariant::VariantTypeObject);
			t["hours"] = current / 3600;
			t["minutes"] = (current % 3600) / 60;
			t["seconds"] = current % 60;
			t["milliseconds"] = 0;
			result["time"] = t;
		}
		else if (prop == "percentage")
			result["percentage"] = (double)g_application.GetPercentage();
		else if (prop == "repeat")
			result["repeat"] = "off"; // Repeat state not easily accessible in XBMC-360
		else if (prop == "shuffled")
			result["shuffled"] = false; // Shuffle state not easily accessible in XBMC-360
		else if (prop == "canseek")
			result["canseek"] = (g_application.m_pPlayer != NULL);
		else if (prop == "canchangespeed")
			result["canchangespeed"] = true;
		else if (prop == "canshuffle")
			result["canshuffle"] = true;
		else if (prop == "canrepeat")
			result["canrepeat"] = true;
	}

	return OK;
}

static JSON_STATUS PlayerGetItem(const CVariant &params, CVariant &result)
{
	if (!g_application.IsPlaying())
		return FailedToExecute;

	CFileItem& currentItem = g_application.CurrentFileItem();

	CVariant item(CVariant::VariantTypeObject);
	item["label"] = currentItem.GetLabel().c_str();
	item["file"] = currentItem.GetPath().c_str();
	item["thumbnail"] = currentItem.GetThumbnailImage().c_str();

	if (currentItem.HasMusicInfoTag())
	{
		const MUSIC_INFO::CMusicInfoTag* tag = currentItem.GetMusicInfoTag();
		item["type"] = "song";
		item["title"] = tag->GetTitle().c_str();
		item["artist"] = tag->GetArtist().c_str();
		item["album"] = tag->GetAlbum().c_str();
		item["duration"] = tag->GetDuration();
		item["track"] = tag->GetTrackNumber();
		item["year"] = tag->GetYear();
		item["genre"] = tag->GetGenre().c_str();
	}
	else if (currentItem.HasVideoInfoTag())
	{
		const CVideoInfoTag* tag = currentItem.GetVideoInfoTag();
		item["type"] = "movie";
		item["title"] = tag->m_strTitle.c_str();
		item["year"] = tag->m_iYear;
	}
	else
	{
		item["type"] = "unknown";
		item["title"] = currentItem.GetLabel().c_str();
	}

	result["item"] = item;
	return OK;
}

static JSON_STATUS PlayerPlayPause(const CVariant &params, CVariant &result)
{
	if (!g_application.IsPlaying() && !g_application.IsPaused())
		return FailedToExecute;

	g_application.getApplicationMessenger().MediaPause();
	result["speed"] = g_application.IsPaused() ? 0 : g_application.GetPlaySpeed();
	return OK;
}

static JSON_STATUS PlayerStop(const CVariant &params, CVariant &result)
{
	if (!g_application.IsPlaying() && !g_application.IsPaused())
		return FailedToExecute;

	g_application.getApplicationMessenger().MediaStop();
	return ACK;
}

static JSON_STATUS PlayerGoNext(const CVariant &params, CVariant &result)
{
	g_application.getApplicationMessenger().PlayListPlayerNext();
	return ACK;
}

static JSON_STATUS PlayerGoPrevious(const CVariant &params, CVariant &result)
{
	g_application.getApplicationMessenger().PlayListPlayerPrevious();
	return ACK;
}

static JSON_STATUS PlayerGoTo(const CVariant &params, CVariant &result)
{
	int position = (int)params["position"].asInteger();
	g_playlistPlayer.Play(position);
	return ACK;
}

static JSON_STATUS PlayerSetSpeed(const CVariant &params, CVariant &result)
{
	if (params["speed"].isInteger())
	{
		int speed = (int)params["speed"].asInteger();
		if (speed == 0)
			g_application.getApplicationMessenger().MediaPause();
		else
			g_application.SetPlaySpeed(speed);
	}
	else if (params["speed"].isString())
	{
		std::string dir = params["speed"].asString();
		if (dir == "increment")
		{
			// If paused, unpause first
			if (g_application.IsPaused())
				g_application.getApplicationMessenger().MediaPause();
			int speed = g_application.GetPlaySpeed();
			if (speed < 1) speed = 1;
			else speed *= 2;
			if (speed > 32) speed = 32;
			g_application.SetPlaySpeed(speed);
		}
		else if (dir == "decrement")
		{
			int speed = g_application.GetPlaySpeed();
			if (speed > 1) speed /= 2;
			else speed = 1;
			g_application.SetPlaySpeed(speed);
		}
		else
			return InvalidParams;
	}

	result["speed"] = g_application.IsPaused() ? 0 : g_application.GetPlaySpeed();
	return OK;
}

//
// ===== Playlist Operations (matches Eden CPlaylistOperations) =====
//
static JSON_STATUS PlaylistGetPlaylists(const CVariant &params, CVariant &result)
{
	result = CVariant(CVariant::VariantTypeArray);

	CVariant audio(CVariant::VariantTypeObject);
	audio["playlistid"] = PLAYLIST_MUSIC;
	audio["type"] = "audio";
	result.append(audio);

	CVariant video(CVariant::VariantTypeObject);
	video["playlistid"] = PLAYLIST_VIDEO;
	video["type"] = "video";
	result.append(video);

	return OK;
}

static JSON_STATUS PlaylistGetItems(const CVariant &params, CVariant &result)
{
	int playlistid = (int)params["playlistid"].asInteger();
	const CPlayList& playlist = g_playlistPlayer.GetPlaylist(playlistid);

	CVariant items(CVariant::VariantTypeArray);
	for (int i = 0; i < playlist.size(); i++)
	{
		CVariant item(CVariant::VariantTypeObject);
		const CFileItemPtr fileItem = playlist[i];
		item["label"] = fileItem->GetLabel().c_str();

		if (fileItem->HasMusicInfoTag())
		{
			item["title"] = fileItem->GetMusicInfoTag()->GetTitle().c_str();
			item["artist"] = fileItem->GetMusicInfoTag()->GetArtist().c_str();
			item["album"] = fileItem->GetMusicInfoTag()->GetAlbum().c_str();
			item["duration"] = fileItem->GetMusicInfoTag()->GetDuration();
			item["thumbnail"] = fileItem->GetThumbnailImage().c_str();
		}
		else
		{
			item["title"] = fileItem->GetLabel().c_str();
			item["artist"] = "";
			item["album"] = "";
			item["duration"] = 0;
			item["thumbnail"] = "";
		}

		items.append(item);
	}

	result["items"] = items;
	CVariant limits(CVariant::VariantTypeObject);
	limits["start"] = 0;
	limits["end"] = playlist.size();
	limits["total"] = playlist.size();
	result["limits"] = limits;

	return OK;
}

//
// ===== Application Operations (matches Eden CApplicationOperations) =====
//
static JSON_STATUS ApplicationGetProperties(const CVariant &params, CVariant &result)
{
	result = CVariant(CVariant::VariantTypeObject);

	for (unsigned int i = 0; i < params["properties"].size(); i++)
	{
		std::string prop = params["properties"][i].asString();

		if (prop == "volume")
			result["volume"] = g_application.GetVolume();
		else if (prop == "muted")
			result["muted"] = g_settings.m_bMute;
		else if (prop == "name")
			result["name"] = "XBMC";
		else if (prop == "version")
		{
			CVariant ver(CVariant::VariantTypeObject);
			ver["major"] = 11;
			ver["minor"] = 0;
			ver["revision"] = "360";
			ver["tag"] = "stable";
			result["version"] = ver;
		}
	}

	return OK;
}

static JSON_STATUS ApplicationSetVolume(const CVariant &params, CVariant &result)
{
	int volume = (int)params["volume"].asInteger();
	g_application.SetVolume(volume);
	result = g_application.GetVolume();
	return OK;
}

static JSON_STATUS ApplicationSetMute(const CVariant &params, CVariant &result)
{
	if (params["mute"].isString())
	{
		std::string mute = params["mute"].asString();
		if (mute == "toggle")
			g_application.Mute();
	}
	else if (params["mute"].isBoolean())
	{
		bool wantMute = params["mute"].asBoolean();
		if (wantMute != g_settings.m_bMute)
			g_application.Mute();
	}

	result = g_settings.m_bMute;
	return OK;
}

//
// ===== System Operations (matches Eden CSystemOperations) =====
//
static JSON_STATUS SystemShutdown(const CVariant &params, CVariant &result)
{
	g_application.getApplicationMessenger().Shutdown();
	return ACK;
}

static JSON_STATUS SystemReboot(const CVariant &params, CVariant &result)
{
	g_application.getApplicationMessenger().Reboot();
	return ACK;
}

//
// ===== Input Operations (matches Eden CInputOperations) =====
//
static JSON_STATUS InputSendKey(uint32_t keyCode)
{
	if (keyCode == KEY_INVALID)
		return InternalError;

	CSingleLock lock(s_inputCritSection);
	s_pendingKey = keyCode | KEY_VKEY;
	return ACK;
}

static JSON_STATUS InputLeft(const CVariant &params, CVariant &result)
{
	return InputSendKey(XBMCVK_LEFT);
}

static JSON_STATUS InputRight(const CVariant &params, CVariant &result)
{
	return InputSendKey(XBMCVK_RIGHT);
}

static JSON_STATUS InputUp(const CVariant &params, CVariant &result)
{
	return InputSendKey(XBMCVK_UP);
}

static JSON_STATUS InputDown(const CVariant &params, CVariant &result)
{
	return InputSendKey(XBMCVK_DOWN);
}

static JSON_STATUS InputSelect(const CVariant &params, CVariant &result)
{
	return InputSendKey(XBMCVK_RETURN);
}

static JSON_STATUS InputBack(const CVariant &params, CVariant &result)
{
	return InputSendKey(XBMCVK_BACK);
}

static JSON_STATUS InputHome(const CVariant &params, CVariant &result)
{
	g_application.getApplicationMessenger().ExecBuiltIn("ActivateWindow(Home)");
	return ACK;
}

//
// ===== Library Stubs =====
//
static JSON_STATUS AudioLibraryGetAlbums(const CVariant &params, CVariant &result)
{
	// TODO: Port AudioLibrary.GetAlbums from Eden
	result["albums"] = CVariant(CVariant::VariantTypeArray);
	result["limits"]["start"] = 0;
	result["limits"]["end"] = 0;
	result["limits"]["total"] = 0;
	return OK;
}

static JSON_STATUS AudioLibraryGetSongs(const CVariant &params, CVariant &result)
{
	// TODO: Port AudioLibrary.GetSongs from Eden
	result["songs"] = CVariant(CVariant::VariantTypeArray);
	result["limits"]["start"] = 0;
	result["limits"]["end"] = 0;
	result["limits"]["total"] = 0;
	return OK;
}

static JSON_STATUS VideoLibraryGetMovies(const CVariant &params, CVariant &result)
{
	// TODO: Port VideoLibrary.GetMovies from Eden
	result["movies"] = CVariant(CVariant::VariantTypeArray);
	result["limits"]["start"] = 0;
	result["limits"]["end"] = 0;
	result["limits"]["total"] = 0;
	return OK;
}

static JSON_STATUS VideoLibraryGetTVShows(const CVariant &params, CVariant &result)
{
	// TODO: Port VideoLibrary.GetTVShows from Eden
	result["tvshows"] = CVariant(CVariant::VariantTypeArray);
	result["limits"]["start"] = 0;
	result["limits"]["end"] = 0;
	result["limits"]["total"] = 0;
	return OK;
}

static JSON_STATUS LibraryScan(const CVariant &params, CVariant &result)
{
	// TODO: Trigger library scan
	return ACK;
}

static JSON_STATUS LibraryClean(const CVariant &params, CVariant &result)
{
	// TODO: Trigger library clean
	return ACK;
}

//
// ===== Playlist Operations - Additional (matches Eden CPlaylistOperations) =====
//
static JSON_STATUS PlaylistGetProperties(const CVariant &params, CVariant &result)
{
	int playlistid = (int)params["playlistid"].asInteger();
	const CPlayList& playlist = g_playlistPlayer.GetPlaylist(playlistid);

	for (unsigned int i = 0; i < params["properties"].size(); i++)
	{
		std::string prop = params["properties"][i].asString();

		if (prop == "type")
		{
			if (playlistid == PLAYLIST_MUSIC)
				result["type"] = "audio";
			else
				result["type"] = "video";
		}
		else if (prop == "size")
			result["size"] = playlist.size();
	}

	return OK;
}

static JSON_STATUS PlaylistClear(const CVariant &params, CVariant &result)
{
	int playlistid = (int)params["playlistid"].asInteger();
	g_playlistPlayer.ClearPlaylist(playlistid);
	return ACK;
}

static JSON_STATUS PlaylistAdd(const CVariant &params, CVariant &result)
{
	int playlistid = (int)params["playlistid"].asInteger();
	CPlayList& playlist = g_playlistPlayer.GetPlaylist(playlistid);

	if (params["item"].isMember("file"))
	{
		CStdString file = params["item"]["file"].asString().c_str();
		CFileItemPtr item(new CFileItem(file, false));
		playlist.Add(item);
	}

	return ACK;
}

static JSON_STATUS PlaylistInsert(const CVariant &params, CVariant &result)
{
	int playlistid = (int)params["playlistid"].asInteger();
	int position = (int)params["position"].asInteger();
	CPlayList& playlist = g_playlistPlayer.GetPlaylist(playlistid);

	if (params["item"].isMember("file"))
	{
		CStdString file = params["item"]["file"].asString().c_str();
		CFileItemPtr item(new CFileItem(file, false));
//		playlist.Add(item, position, 0); //TODO
	}

	return ACK;
}

static JSON_STATUS PlaylistRemove(const CVariant &params, CVariant &result)
{
	int playlistid = (int)params["playlistid"].asInteger();
	int position = (int)params["position"].asInteger();

	// Don't remove the currently playing item (matches Eden)
	if (g_playlistPlayer.GetCurrentPlaylist() == playlistid && g_playlistPlayer.GetCurrentSong() == position)
		return InvalidParams;

	CPlayList& playlist = g_playlistPlayer.GetPlaylist(playlistid);
	playlist.Remove(position);
	return ACK;
}

//
// ===== Player Operations - Additional (matches Eden CPlayerOperations) =====
//
static JSON_STATUS PlayerOpen(const CVariant &params, CVariant &result)
{
	if (params["item"].isMember("playlistid"))
	{
		int playlistid = (int)params["item"]["playlistid"].asInteger();
		int position = 0;
		if (params["item"].isMember("position"))
			position = (int)params["item"]["position"].asInteger();

		g_playlistPlayer.SetCurrentPlaylist(playlistid);
		g_playlistPlayer.Play(position);
	}
	else if (params["item"].isMember("file"))
	{
		CStdString file = params["item"]["file"].asString().c_str();
		CFileItem item(file, false);
		g_application.getApplicationMessenger().MediaPlay(item);
	}

	return ACK;
}

static JSON_STATUS PlayerSeek(const CVariant &params, CVariant &result)
{
	if (!g_application.IsPlaying())
		return FailedToExecute;

	if (params["value"].isObject())
	{
		// Time-based seek: {hours, minutes, seconds, milliseconds}
		double seekTime = ((params["value"]["hours"].asInteger() * 60) +
			params["value"]["minutes"].asInteger()) * 60 +
			params["value"]["seconds"].asInteger() +
			((double)params["value"]["milliseconds"].asInteger() / 1000.0);
		g_application.SeekTime(seekTime);
	}
	else if (params["value"].isDouble() || params["value"].isInteger())
	{
		// Percentage-based seek
		float percentage = (float)params["value"].asDouble();
		double totalTime = g_application.GetTotalTime();
		if (totalTime > 0)
			g_application.SeekTime(totalTime * percentage / 100.0);
	}
	else if (params["value"].isString())
	{
		// Step-based seek
		std::string step = params["value"].asString();
		if (step == "smallforward")
			g_application.getApplicationMessenger().ExecBuiltIn("playercontrol(smallskipforward)");
		else if (step == "smallbackward")
			g_application.getApplicationMessenger().ExecBuiltIn("playercontrol(smallskipbackward)");
		else if (step == "bigforward")
			g_application.getApplicationMessenger().ExecBuiltIn("playercontrol(bigskipforward)");
		else if (step == "bigbackward")
			g_application.getApplicationMessenger().ExecBuiltIn("playercontrol(bigskipbackward)");
		else
			return InvalidParams;
	}
	else
		return InvalidParams;

	result["percentage"] = (double)g_application.GetPercentage();
	int current = (int)g_application.GetTime();
	CVariant t(CVariant::VariantTypeObject);
	t["hours"] = current / 3600;
	t["minutes"] = (current % 3600) / 60;
	t["seconds"] = current % 60;
	t["milliseconds"] = 0;
	result["time"] = t;
	int total = (int)g_application.GetTotalTime();
	CVariant tt(CVariant::VariantTypeObject);
	tt["hours"] = total / 3600;
	tt["minutes"] = (total % 3600) / 60;
	tt["seconds"] = total % 60;
	tt["milliseconds"] = 0;
	result["totaltime"] = tt;
	return OK;
}

//
// ===== JSONRPC Intrinsics =====
//
static JSON_STATUS JSONRPCPing(const CVariant &params, CVariant &result)
{
	CVariant pong("pong");
	result.swap(pong);
	return OK;
}

//
// ===== Method Dispatch (matches Eden CJSONRPC::HandleMethodCall) =====
//
typedef JSON_STATUS (*MethodHandler)(const CVariant &params, CVariant &result);

struct MethodEntry
{
	const char *name;
	MethodHandler handler;
};

static const MethodEntry s_methods[] =
{
	// JSONRPC
	{ "jsonrpc.ping",               JSONRPCPing },

	// Player
	{ "player.getactiveplayers",    PlayerGetActivePlayers },
	{ "player.getproperties",       PlayerGetProperties },
	{ "player.getitem",             PlayerGetItem },
	{ "player.playpause",           PlayerPlayPause },
	{ "player.stop",                PlayerStop },
	{ "player.gonext",              PlayerGoNext },
	{ "player.goprevious",          PlayerGoPrevious },
	{ "player.goto",                PlayerGoTo },
	{ "player.setspeed",            PlayerSetSpeed },
	{ "player.open",                PlayerOpen },
	{ "player.seek",                PlayerSeek },

	// Playlist
	{ "playlist.getplaylists",      PlaylistGetPlaylists },
	{ "playlist.getproperties",     PlaylistGetProperties },
	{ "playlist.getitems",          PlaylistGetItems },
	{ "playlist.clear",             PlaylistClear },
	{ "playlist.add",               PlaylistAdd },
	{ "playlist.insert",            PlaylistInsert },
	{ "playlist.remove",            PlaylistRemove },

	// Application
	{ "application.getproperties",  ApplicationGetProperties },
	{ "application.setvolume",      ApplicationSetVolume },
	{ "application.setmute",        ApplicationSetMute },

	// System
	{ "system.shutdown",            SystemShutdown },
	{ "system.reboot",              SystemReboot },

	// Input
	{ "input.up",                   InputUp },
	{ "input.down",                 InputDown },
	{ "input.left",                 InputLeft },
	{ "input.right",                InputRight },
	{ "input.select",               InputSelect },
	{ "input.back",                 InputBack },
	{ "input.home",                 InputHome },

	// Audio Library
	{ "audiolibrary.getalbums",     AudioLibraryGetAlbums },
	{ "audiolibrary.getsongs",      AudioLibraryGetSongs },
	{ "audiolibrary.scan",          LibraryScan },
	{ "audiolibrary.clean",         LibraryClean },

	// Video Library
	{ "videolibrary.getmovies",     VideoLibraryGetMovies },
	{ "videolibrary.gettvshows",    VideoLibraryGetTVShows },
	{ "videolibrary.scan",          LibraryScan },
	{ "videolibrary.clean",         LibraryClean },

	{ NULL, NULL }
};

static MethodHandler FindMethod(const std::string &method)
{
	std::string lower = method;
	for (size_t i = 0; i < lower.size(); i++)
		lower[i] = (char)tolower((unsigned char)lower[i]);

	for (const MethodEntry *entry = s_methods; entry->name != NULL; entry++)
	{
		if (lower == entry->name)
			return entry->handler;
	}
	return NULL;
}

//
// ===== Public Interface =====
//
void CJSONRPC::Initialize()
{
	CLog::Log(LOGINFO, "JSONRPC: Initialized");
}

CStdString CJSONRPC::MethodCall(const CStdString &inputString, ITransportLayer *transport, IClient *client)
{
	CVariant inputroot, outputroot, result;

	CLog::Log(LOGDEBUG, "JSONRPC: Incoming request: %s", inputString.substr(0, 200).c_str());

	inputroot = CJSONVariantParser::Parse(inputString.c_str(), inputString.length());

	if (inputroot.isNull())
	{
		CLog::Log(LOGERROR, "JSONRPC: Failed to parse request");
		BuildResponse(inputroot, ParseError, CVariant(), outputroot);
		return CJSONVariantWriter::Write(outputroot, true).c_str();
	}

	// Validate JSON-RPC 2.0 structure
	if (!inputroot.isObject() || !inputroot.isMember("method") || !inputroot["method"].isString())
	{
		BuildResponse(inputroot, InvalidRequest, CVariant(), outputroot);
		return CJSONVariantWriter::Write(outputroot, true).c_str();
	}

	std::string methodName = inputroot["method"].asString();
	CVariant params = inputroot.isMember("params") ? inputroot["params"] : CVariant(CVariant::VariantTypeObject);

	CLog::Log(LOGDEBUG, "JSONRPC: Calling %s", methodName.c_str());

	MethodHandler handler = FindMethod(methodName);
	if (handler)
	{
		JSON_STATUS status = handler(params, result);
		BuildResponse(inputroot, status, result, outputroot);
	}
	else
	{
		CLog::Log(LOGWARNING, "JSONRPC: Method not found: %s", methodName.c_str());
		BuildResponse(inputroot, MethodNotFound, CVariant(), outputroot);
	}

	return CJSONVariantWriter::Write(outputroot, true).c_str();
}
