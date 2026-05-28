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

#include "Application.h"
#include "ApplicationMessenger.h"
#include "PlayListPlayer.h"
#include "player.h"
#include "FileItem.h"
#include "utils/log.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{

// =========================================================================
// Player.__new__
// =========================================================================
PyObject* Player_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Player *self;
	int playerCore = EPC_NONE;

	self = (Player*)type->tp_alloc(type, 0);
	if (!self) return NULL;

	if (!PyArg_ParseTuple(args, (char*)"|i", &playerCore)) return NULL;

	self->iPlayList = PLAYLIST_MUSIC;
	self->pPlayer = new CPythonPlayer();
	self->pPlayer->SetCallback((PyObject*)self);
	self->playerCore = EPC_NONE;

	if (playerCore == EPC_DVDPLAYER ||
	    playerCore == EPC_PAPLAYER)
	{
		self->playerCore = (EPLAYERCORES)playerCore;
	}

	return (PyObject*)self;
}

// =========================================================================
// Player.__del__
// =========================================================================
void Player_Dealloc(Player* self)
{
	if (self->pPlayer) delete self->pPlayer;
	self->pPlayer = NULL;
	Py_TYPE(self)->tp_free((PyObject*)self);
}

// =========================================================================
// Player.play([item, listitem, windowed])
// =========================================================================
PyDoc_STRVAR(play__doc__,
	"play([item, listitem, windowed]) -- Play this item.\n"
	"\n"
	"item           : [opt] string - filename, url or playlist.\n"
	"listitem       : [opt] listitem - used with setInfo() to set different infolabels.\n"
	"windowed       : [opt] bool - true=play video windowed, false=play users preference.(default)\n"
	"\n"
	"*Note, If item is not given then the Player will try to play the current item\n"
	"       in the current playlist.\n"
	"\n"
	"example:\n"
	"  - xbmc.Player().play('/path/to/file.mp3')\n");

PyObject* Player_Play(Player *self, PyObject *args, PyObject *kwds)
{
	PyObject *pObject = NULL;
	PyObject *pObjectListItem = NULL;
	char bWindowed = false;
	static const char *keywords[] = { "item", "listitem", "windowed", NULL };

	if (!PyArg_ParseTupleAndKeywords(
		args,
		kwds,
		(char*)"|OOb",
		(char**)keywords,
		&pObject,
		&pObjectListItem,
		&bWindowed))
	{
		return NULL;
	}

	if (pObject == NULL)
	{
		// play current file in playlist
		if (g_playlistPlayer.GetCurrentPlaylist() != self->iPlayList)
		{
			g_playlistPlayer.SetCurrentPlaylist(self->iPlayList);
		}
		g_application.getApplicationMessenger().PlayListPlayerPlay(
			g_playlistPlayer.GetCurrentSong());
	}
	else if (PyUnicode_Check(pObject))
	{
		// Play a URL/path string
		PyObject* pyUtf8 = PyUnicode_AsUTF8String(pObject);
		if (!pyUtf8) return NULL;
		const char* url = PyBytes_AsString(pyUtf8);

		CFileItem item(url, false);
		g_application.getApplicationMessenger().MediaPlay(item.GetPath());
		Py_DECREF(pyUtf8);
	}
	else if (PyBytes_Check(pObject))
	{
		const char* url = PyBytes_AsString(pObject);
		CFileItem item(url, false);
		g_application.getApplicationMessenger().MediaPlay(item.GetPath());
	}

	Py_RETURN_NONE;
}

// =========================================================================
// Player.stop()
// =========================================================================
PyDoc_STRVAR(stop__doc__,
	"stop() -- Stop playing.");

PyObject* Player_Stop(PyObject *self, PyObject *args)
{
	g_application.getApplicationMessenger().MediaStop();
	Py_RETURN_NONE;
}

// =========================================================================
// Player.pause()
// =========================================================================
PyDoc_STRVAR(pause__doc__,
	"pause() -- Pause playing.");

PyObject* Player_Pause(PyObject *self, PyObject *args)
{
	g_application.getApplicationMessenger().MediaPause();
	Py_RETURN_NONE;
}

// =========================================================================
// Player.playnext()
// =========================================================================
PyDoc_STRVAR(playnext__doc__,
	"playnext() -- Play next item in playlist.");

PyObject* Player_PlayNext(Player *self, PyObject *args)
{
	g_application.getApplicationMessenger().PlayListPlayerNext();
	Py_RETURN_NONE;
}

// =========================================================================
// Player.playprevious()
// =========================================================================
PyDoc_STRVAR(playprevious__doc__,
	"playprevious() -- Play previous item in playlist.");

PyObject* Player_PlayPrevious(Player *self, PyObject *args)
{
	g_application.getApplicationMessenger().PlayListPlayerPrevious();
	Py_RETURN_NONE;
}

// =========================================================================
// Player.playselected(iItem)
// =========================================================================
PyDoc_STRVAR(playselected__doc__,
	"playselected(iItem) -- Play a certain item from the current playlist.");

PyObject* Player_PlaySelected(Player *self, PyObject *args)
{
	int iItem;
	if (!PyArg_ParseTuple(args, (char*)"i", &iItem)) return NULL;

	if (g_playlistPlayer.GetCurrentPlaylist() != self->iPlayList)
	{
		g_playlistPlayer.SetCurrentPlaylist(self->iPlayList);
	}
	g_playlistPlayer.SetCurrentSong(iItem);

	g_application.getApplicationMessenger().PlayListPlayerPlay(iItem);

	Py_RETURN_NONE;
}

// =========================================================================
// Callback stubs (overridable from Python)
// =========================================================================
PyDoc_STRVAR(onPlayBackStarted__doc__,
	"onPlayBackStarted() -- onPlayBackStarted method.\n"
	"\n"
	"Will be called when xbmc starts playing a file");

PyObject* Player_OnPlayBackStarted(PyObject *self, PyObject *args)
{
	Py_RETURN_NONE;
}

PyDoc_STRVAR(onPlayBackEnded__doc__,
	"onPlayBackEnded() -- onPlayBackEnded method.\n"
	"\n"
	"Will be called when xbmc stops playing a file");

PyObject* Player_OnPlayBackEnded(PyObject *self, PyObject *args)
{
	Py_RETURN_NONE;
}

PyDoc_STRVAR(onPlayBackStopped__doc__,
	"onPlayBackStopped() -- onPlayBackStopped method.\n"
	"\n"
	"Will be called when user stops xbmc playing a file");

PyObject* Player_OnPlayBackStopped(PyObject *self, PyObject *args)
{
	Py_RETURN_NONE;
}

PyDoc_STRVAR(onPlayBackPaused__doc__,
	"onPlayBackPaused() -- onPlayBackPaused method.\n"
	"\n"
	"Will be called when user pauses a playing file");

PyObject* Player_OnPlayBackPaused(PyObject *self, PyObject *args)
{
	Py_RETURN_NONE;
}

PyDoc_STRVAR(onPlayBackResumed__doc__,
	"onPlayBackResumed() -- onPlayBackResumed method.\n"
	"\n"
	"Will be called when user resumes a paused file");

PyObject* Player_OnPlayBackResumed(PyObject *self, PyObject *args)
{
	Py_RETURN_NONE;
}

// =========================================================================
// Player.isPlaying()
// =========================================================================
PyDoc_STRVAR(isPlaying__doc__,
	"isPlaying() -- returns True if xbmc is playing a file.");

PyObject* Player_IsPlaying(PyObject *self, PyObject *args)
{
	return Py_BuildValue((char*)"b", g_application.IsPlaying());
}

// =========================================================================
// Player.isPlayingAudio()
// =========================================================================
PyDoc_STRVAR(isPlayingAudio__doc__,
	"isPlayingAudio() -- returns True if xbmc is playing an audio file.");

PyObject* Player_IsPlayingAudio(PyObject *self, PyObject *args)
{
	return Py_BuildValue((char*)"b", g_application.IsPlayingAudio());
}

// =========================================================================
// Player.isPlayingVideo()
// =========================================================================
PyDoc_STRVAR(isPlayingVideo__doc__,
	"isPlayingVideo() -- returns True if xbmc is playing a video.");

PyObject* Player_IsPlayingVideo(PyObject *self, PyObject *args)
{
	return Py_BuildValue((char*)"b", g_application.IsPlayingVideo());
}

// =========================================================================
// Player.getPlayingFile()
// =========================================================================
PyDoc_STRVAR(getPlayingFile__doc__,
	"getPlayingFile() -- returns the current playing file as a string.\n"
	"\n"
	"Throws: Exception, if player is not playing a file.\n");

PyObject* Player_GetPlayingFile(PyObject *self, PyObject *args)
{
	if (!g_application.IsPlaying())
	{
		PyErr_SetString(PyExc_Exception, "XBMC is not playing any file");
		return NULL;
	}
	return Py_BuildValue((char*)"s", g_application.CurrentFile().c_str());
}

// =========================================================================
// Player.getTotalTime()
// =========================================================================
PyDoc_STRVAR(getTotalTime__doc__,
	"getTotalTime() -- Returns the total time of the current playing media in\n"
	"                  seconds.  This is only accurate to the full second.\n"
	"\n"
	"Throws: Exception, if player is not playing a file.\n");

PyObject* Player_GetTotalTime(PyObject *self)
{
	if (!g_application.IsPlaying())
	{
		PyErr_SetString(PyExc_Exception, "XBMC is not playing any media file");
		return NULL;
	}
	return PyFloat_FromDouble(g_application.GetTotalTime());
}

// =========================================================================
// Player.getTime()
// =========================================================================
PyDoc_STRVAR(getTime__doc__,
	"getTime() -- Returns the current time of the current playing media as fractional seconds.\n"
	"\n"
	"Throws: Exception, if player is not playing a file.\n");

PyObject* Player_GetTime(PyObject *self)
{
	if (!g_application.IsPlaying())
	{
		PyErr_SetString(PyExc_Exception, "XBMC is not playing any media file");
		return NULL;
	}
	double dTime = g_application.GetTime();
	return Py_BuildValue((char*)"d", dTime);
}

// =========================================================================
// Player.seekTime(time)
// =========================================================================
PyDoc_STRVAR(seekTime__doc__,
	"seekTime(time) -- Seeks the specified amount of time as fractional seconds.\n"
	"              The time specified is relative to the beginning of the\n"
	"              currently playing media file.\n"
	"\n"
	"Throws: Exception, if player is not playing a file.\n");

PyObject* Player_SeekTime(PyObject *self, PyObject *args)
{
	double pTime = 0.0;

	if (!g_application.IsPlaying())
	{
		PyErr_SetString(PyExc_Exception, "XBMC is not playing any media file");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, (char*)"d", &pTime)) return NULL;

	g_application.SeekTime(pTime);

	Py_RETURN_NONE;
}

// =========================================================================
// Method table
// =========================================================================
PyMethodDef Player_methods[] = {
	{(char*)"play",              (PyCFunction)Player_Play,              METH_VARARGS|METH_KEYWORDS, play__doc__},
	{(char*)"stop",              (PyCFunction)Player_Stop,              METH_VARARGS, stop__doc__},
	{(char*)"pause",             (PyCFunction)Player_Pause,             METH_VARARGS, pause__doc__},
	{(char*)"playnext",          (PyCFunction)Player_PlayNext,          METH_VARARGS, playnext__doc__},
	{(char*)"playprevious",      (PyCFunction)Player_PlayPrevious,      METH_VARARGS, playprevious__doc__},
	{(char*)"playselected",      (PyCFunction)Player_PlaySelected,      METH_VARARGS, playselected__doc__},
	{(char*)"onPlayBackStarted", (PyCFunction)Player_OnPlayBackStarted, METH_VARARGS, onPlayBackStarted__doc__},
	{(char*)"onPlayBackEnded",   (PyCFunction)Player_OnPlayBackEnded,   METH_VARARGS, onPlayBackEnded__doc__},
	{(char*)"onPlayBackStopped", (PyCFunction)Player_OnPlayBackStopped, METH_VARARGS, onPlayBackStopped__doc__},
	{(char*)"onPlayBackPaused",  (PyCFunction)Player_OnPlayBackPaused,  METH_VARARGS, onPlayBackPaused__doc__},
	{(char*)"onPlayBackResumed", (PyCFunction)Player_OnPlayBackResumed, METH_VARARGS, onPlayBackResumed__doc__},
	{(char*)"isPlaying",         (PyCFunction)Player_IsPlaying,         METH_VARARGS, isPlaying__doc__},
	{(char*)"isPlayingAudio",    (PyCFunction)Player_IsPlayingAudio,    METH_VARARGS, isPlayingAudio__doc__},
	{(char*)"isPlayingVideo",    (PyCFunction)Player_IsPlayingVideo,    METH_VARARGS, isPlayingVideo__doc__},
	{(char*)"getPlayingFile",    (PyCFunction)Player_GetPlayingFile,    METH_VARARGS, getPlayingFile__doc__},
	{(char*)"getTotalTime",      (PyCFunction)Player_GetTotalTime,      METH_NOARGS,  getTotalTime__doc__},
	{(char*)"getTime",           (PyCFunction)Player_GetTime,           METH_NOARGS,  getTime__doc__},
	{(char*)"seekTime",          (PyCFunction)Player_SeekTime,          METH_VARARGS, seekTime__doc__},
	{NULL, NULL, 0, NULL}
};

// =========================================================================
// Type definition
// =========================================================================
PyDoc_STRVAR(player__doc__,
	"Player class.\n"
	"\n"
	"Player([core]) -- Creates a new Player with as default the xbmc music playlist.\n"
	"\n"
	"core     : (optional) Use a specified playcore instead of letting xbmc decide.\n"
	"         : - xbmc.PLAYER_CORE_AUTO\n"
	"         : - xbmc.PLAYER_CORE_DVDPLAYER\n"
	"         : - xbmc.PLAYER_CORE_PAPLAYER\n");

PyTypeObject Player_Type;

void initPlayer_Type()
{
	memset(&Player_Type, 0, sizeof(PyTypeObject));
	PyVarObject ob_base = {PyObject_HEAD_INIT(NULL) 0};
	*(PyVarObject*)&Player_Type = ob_base;

	Player_Type.tp_name = (char*)"xbmc.Player";
	Player_Type.tp_basicsize = sizeof(Player);
	Player_Type.tp_dealloc = (destructor)Player_Dealloc;
	Player_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
	Player_Type.tp_doc = player__doc__;
	Player_Type.tp_methods = Player_methods;
	Player_Type.tp_base = 0;
	Player_Type.tp_new = Player_New;
}

} // namespace PYXBMC

#ifdef __cplusplus
}
#endif
