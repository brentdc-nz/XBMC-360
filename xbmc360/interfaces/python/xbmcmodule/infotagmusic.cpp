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

#include "infotagmusic.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	PyObject* InfoTagMusic_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		InfoTagMusic *self;

		self = (InfoTagMusic*)type->tp_alloc(type, 0);
		if (!self) return NULL;

		new(&self->url) string();
		new(&self->title) string();
		new(&self->artist) string();
		new(&self->album) string();
		new(&self->albumArtist) string();
		new(&self->genre) string();
		new(&self->releaseDate) string();
		new(&self->lastPlayed) string();
		new(&self->comment) string();
		new(&self->lyrics) string();

		self->duration = 0;
		self->track = 0;
		self->disc = 0;
		self->listeners = 0;
		self->playCount = 0;
		self->rating = 0;

		return (PyObject*)self;
	}

	void InfoTagMusic_Dealloc(InfoTagMusic* self)
	{
		self->url.~string();
		self->title.~string();
		self->artist.~string();
		self->album.~string();
		self->albumArtist.~string();
		self->genre.~string();
		self->releaseDate.~string();
		self->lastPlayed.~string();
		self->comment.~string();
		self->lyrics.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(getURL__doc__,
		"getURL() -- returns url of song.\n");
	PyObject* InfoTagMusic_GetURL(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->url.c_str());
	}

	PyDoc_STRVAR(getTitle__doc__,
		"getTitle() -- returns the title of the song.\n");
	PyObject* InfoTagMusic_GetTitle(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->title.c_str());
	}

	PyDoc_STRVAR(getArtist__doc__,
		"getArtist() -- returns the artist of the song.\n");
	PyObject* InfoTagMusic_GetArtist(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->artist.c_str());
	}

	PyDoc_STRVAR(getAlbum__doc__,
		"getAlbum() -- returns the album of the song.\n");
	PyObject* InfoTagMusic_GetAlbum(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->album.c_str());
	}

	PyDoc_STRVAR(getAlbumArtist__doc__,
		"getAlbumArtist() -- returns the album artist of the song.\n");
	PyObject* InfoTagMusic_GetAlbumArtist(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->albumArtist.c_str());
	}

	PyDoc_STRVAR(getGenre__doc__,
		"getGenre() -- returns the genre of the song.\n");
	PyObject* InfoTagMusic_GetGenre(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->genre.c_str());
	}

	PyDoc_STRVAR(getDuration__doc__,
		"getDuration() -- returns the duration of the song.\n");
	PyObject* InfoTagMusic_GetDuration(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"i", self->duration);
	}

	PyDoc_STRVAR(getTrack__doc__,
		"getTrack() -- returns the track number.\n");
	PyObject* InfoTagMusic_GetTrack(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"i", self->track);
	}

	PyDoc_STRVAR(getDisc__doc__,
		"getDisc() -- returns the disc number.\n");
	PyObject* InfoTagMusic_GetDisc(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"i", self->disc);
	}

	PyDoc_STRVAR(getReleaseDate__doc__,
		"getReleaseDate() -- returns the release date as string from music info tag.\n");
	PyObject* InfoTagMusic_GetReleaseDate(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->releaseDate.c_str());
	}

	PyDoc_STRVAR(getListeners__doc__,
		"getListeners() -- returns the listeners as integer from music info tag.\n");
	PyObject* InfoTagMusic_GetListeners(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"i", self->listeners);
	}

	PyDoc_STRVAR(getPlayCount__doc__,
		"getPlayCount() -- returns the playcount as integer from music info tag.\n");
	PyObject* InfoTagMusic_GetPlayCount(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"i", self->playCount);
	}

	PyDoc_STRVAR(getLastPlayed__doc__,
		"getLastPlayed() -- returns the last played date as string from music info tag.\n");
	PyObject* InfoTagMusic_GetLastPlayed(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->lastPlayed.c_str());
	}

	PyDoc_STRVAR(getComment__doc__,
		"getComment() -- returns the comment as string from music info tag.\n");
	PyObject* InfoTagMusic_GetComment(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->comment.c_str());
	}

	PyDoc_STRVAR(getLyrics__doc__,
		"getLyrics() -- returns the lyrics as string from music info tag.\n");
	PyObject* InfoTagMusic_GetLyrics(InfoTagMusic *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->lyrics.c_str());
	}

	PyMethodDef InfoTagMusic_methods[] = {
		{(char*)"getURL", (PyCFunction)InfoTagMusic_GetURL, METH_VARARGS, getURL__doc__},
		{(char*)"getTitle", (PyCFunction)InfoTagMusic_GetTitle, METH_VARARGS, getTitle__doc__},
		{(char*)"getArtist", (PyCFunction)InfoTagMusic_GetArtist, METH_VARARGS, getArtist__doc__},
		{(char*)"getAlbum", (PyCFunction)InfoTagMusic_GetAlbum, METH_VARARGS, getAlbum__doc__},
		{(char*)"getAlbumArtist", (PyCFunction)InfoTagMusic_GetAlbumArtist, METH_VARARGS, getAlbumArtist__doc__},
		{(char*)"getGenre", (PyCFunction)InfoTagMusic_GetGenre, METH_VARARGS, getGenre__doc__},
		{(char*)"getDuration", (PyCFunction)InfoTagMusic_GetDuration, METH_VARARGS, getDuration__doc__},
		{(char*)"getTrack", (PyCFunction)InfoTagMusic_GetTrack, METH_VARARGS, getTrack__doc__},
		{(char*)"getDisc", (PyCFunction)InfoTagMusic_GetDisc, METH_VARARGS, getDisc__doc__},
		{(char*)"getReleaseDate", (PyCFunction)InfoTagMusic_GetReleaseDate, METH_VARARGS, getReleaseDate__doc__},
		{(char*)"getListeners", (PyCFunction)InfoTagMusic_GetListeners, METH_VARARGS, getListeners__doc__},
		{(char*)"getPlayCount", (PyCFunction)InfoTagMusic_GetPlayCount, METH_VARARGS, getPlayCount__doc__},
		{(char*)"getLastPlayed", (PyCFunction)InfoTagMusic_GetLastPlayed, METH_VARARGS, getLastPlayed__doc__},
		{(char*)"getComment", (PyCFunction)InfoTagMusic_GetComment, METH_VARARGS, getComment__doc__},
		{(char*)"getLyrics", (PyCFunction)InfoTagMusic_GetLyrics, METH_VARARGS, getLyrics__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(infoTagMusic__doc__,
		"InfoTagMusic class.\n"
		"\n"
		"To get music info tag from a listitem or the player.\n");

	PyTypeObject InfoTagMusic_Type;

	void initInfoTagMusic_Type()
	{
		PyXBMCInitializeTypeObject(&InfoTagMusic_Type);

		InfoTagMusic_Type.tp_name = (char*)"xbmc.InfoTagMusic";
		InfoTagMusic_Type.tp_basicsize = sizeof(InfoTagMusic);
		InfoTagMusic_Type.tp_dealloc = (destructor)InfoTagMusic_Dealloc;
		InfoTagMusic_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		InfoTagMusic_Type.tp_doc = infoTagMusic__doc__;
		InfoTagMusic_Type.tp_methods = InfoTagMusic_methods;
		InfoTagMusic_Type.tp_base = 0;
		InfoTagMusic_Type.tp_new = InfoTagMusic_New;
	}
}

#ifdef __cplusplus
}
#endif
