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

#include "pyplaylist.h"
#include "pyutil.h"
#include "listitem.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{

/* PlayListItem Functions */

	PyObject* PlayListItem_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		PlayListItem *self;

		self = (PlayListItem*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strPath) string();
		new(&self->strDescription) string();

		self->lDuration = 0;

		return (PyObject*)self;
	}

	void PlayListItem_Dealloc(PlayListItem* self)
	{
		self->strPath.~string();
		self->strDescription.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(getDescription__doc__,
		"getdescription() -- Returns the description of this PlayListItem.\n");

	PyObject* PlayListItem_GetDescription(PlayListItem *self, PyObject *key)
	{
		return Py_BuildValue((char*)"s", self->strDescription.c_str());
	}

	PyDoc_STRVAR(getDuration__doc__,
		"getduration() -- Returns the duration of this PlayListItem.\n");

	PyObject* PlayListItem_GetDuration(PlayListItem *self, PyObject *key)
	{
		return Py_BuildValue((char*)"l", self->lDuration);
	}

	PyDoc_STRVAR(getFilename__doc__,
		"getfilename() -- Returns the filename of this PlayListItem.\n");

	PyObject* PlayListItem_GetFileName(PlayListItem *self, PyObject *key)
	{
		return Py_BuildValue((char*)"s", self->strPath.c_str());
	}

/* PlayList Functions */

	PyObject* PlayList_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		int iNr;
		PlayList *self;
		if (!PyArg_ParseTuple(args, (char*)"i", &iNr)) return NULL;

		self = (PlayList*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->vecItems) vector<string>();

		// Validate playlist type (0=music, 1=video)
		if (iNr != 0 && iNr != 1)
		{
			PyErr_SetString(PyExc_ValueError, "PlayList does not exist");
			Py_DECREF(self);
			return NULL;
		}

		self->iPlayList = iNr;

		return (PyObject*)self;
	}

	void PlayList_Dealloc(PlayList* self)
	{
		self->vecItems.~vector();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(add__doc__,
		"add(url[, listitem, index]) -- Adds a new file to the playlist.\n"
		"\n"
		"url            : string or unicode - filename or url to add.\n"
		"listitem       : [opt] listitem - used with setInfo() to set different infolabels.\n"
		"index          : [opt] integer - position to add playlist item. (default=end)\n");

	PyObject* PlayList_Add(PlayList *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "url", "listitem", "index", NULL };

		PyObject *pObjectUrl = NULL;
		PyObject *pObjectListItem = NULL;
		int iPos = -1;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"O|Oi", (char**)keywords,
			&pObjectUrl, &pObjectListItem, &iPos))
		{
			return NULL;
		}

		string strUrl;
		if (!PyXBMCGetUnicodeString(strUrl, pObjectUrl)) return NULL;

		// TODO: Use actual PlayList subsystem when available
		if (iPos < 0 || iPos >= (int)self->vecItems.size())
			self->vecItems.push_back(strUrl);
		else
			self->vecItems.insert(self->vecItems.begin() + iPos, strUrl);

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(load__doc__,
		"load(filename) -- Load a playlist.\n"
		"\n"
		"clear current playlist and copy items from the file to this Playlist\n"
		"filename can be like .pls or .m3u ...\n"
		"returns False if unable to load playlist, True otherwise.\n");

	PyObject* PlayList_Load(PlayList *self, PyObject *args)
	{
		char* cFileName = NULL;
		if (!PyArg_ParseTuple(args, (char*)"s", &cFileName)) return NULL;

		// TODO: Implement playlist file loading when PlayListFactory is available
		PyErr_SetString(PyExc_NotImplementedError, "PlayList.load() not yet implemented");
		return NULL;
	}

	PyDoc_STRVAR(remove__doc__,
		"remove(filename) -- remove an item with this filename from the playlist.\n");

	PyObject* PlayList_Remove(PlayList *self, PyObject *args)
	{
		char *cFileName = NULL;
		if (!PyArg_ParseTuple(args, (char*)"s", &cFileName)) return NULL;

		for (auto it = self->vecItems.begin(); it != self->vecItems.end(); ++it)
		{
			if (*it == cFileName)
			{
				self->vecItems.erase(it);
				break;
			}
		}

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(clear__doc__,
		"clear() -- clear all items in the playlist.\n");

	PyObject* PlayList_Clear(PlayList *self, PyObject *args)
	{
		self->vecItems.clear();
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(shuffle__doc__,
		"shuffle() -- shuffle the playlist.\n");

	PyObject* PlayList_Shuffle(PlayList *self, PyObject *args)
	{
		// TODO: Implement shuffle
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(unshuffle__doc__,
		"unshuffle() -- unshuffle the playlist.\n");

	PyObject* PlayList_UnShuffle(PlayList *self, PyObject *args)
	{
		// TODO: Implement unshuffle
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(size__doc__,
		"size() -- returns the total number of PlayListItems in this playlist.\n");

	PyObject* PlayList_Size(PlayList *self, PyObject *key)
	{
		return Py_BuildValue((char*)"i", (int)self->vecItems.size());
	}

	Py_ssize_t PlayList_Length(PyObject *self)
	{
		return (Py_ssize_t)((PlayList*)self)->vecItems.size();
	}

	PyObject* PlayList_GetItem(PyObject *self, PyObject *pAttr)
	{
		long pos = -1;
		int iPlayListSize = (int)((PlayList*)self)->vecItems.size();

		if (PyLong_Check(pAttr))
		{
			pos = PyLong_AsLong(pAttr);
			if (pos == -1 && PyErr_Occurred()) return NULL;
			if (pos < 0) pos += iPlayListSize;
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "playlist indices must be integers");
			return NULL;
		}

		if (pos < 0 || pos >= iPlayListSize)
		{
			PyErr_SetString(PyExc_IndexError, "playlist index out of range");
			return NULL;
		}

		PlayListItem* item = (PlayListItem*)PlayListItem_Type.tp_alloc(&PlayListItem_Type, 0);
		if (!item) return NULL;
		new(&item->strPath) string();
		new(&item->strDescription) string();

		item->strPath = ((PlayList*)self)->vecItems[pos];
		item->strDescription = item->strPath;
		item->lDuration = 0;

		return (PyObject*)item;
	}

	PyDoc_STRVAR(getposition__doc__,
		"getposition() -- returns the position of the current song in this playlist.\n");

	PyObject* PlayList_GetPosition(PlayList *self, PyObject *key)
	{
		// TODO: Return actual position from playlist player
		return Py_BuildValue((char*)"i", 0);
	}

	PyMethodDef PlayListItem_methods[] = {
		{(char*)"getdescription", (PyCFunction)PlayListItem_GetDescription, METH_VARARGS, getDescription__doc__},
		{(char*)"getduration", (PyCFunction)PlayListItem_GetDuration, METH_VARARGS, getDuration__doc__},
		{(char*)"getfilename", (PyCFunction)PlayListItem_GetFileName, METH_VARARGS, getFilename__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyMappingMethods Playlist_as_mapping = {
		PlayList_Length,     /* mp_length */
		PlayList_GetItem,   /* mp_subscript */
		0,                  /* mp_ass_subscript */
	};

	PyMethodDef PlayList_methods[] = {
		{(char*)"add", (PyCFunction)PlayList_Add, METH_VARARGS|METH_KEYWORDS, add__doc__},
		{(char*)"load", (PyCFunction)PlayList_Load, METH_VARARGS, load__doc__},
		{(char*)"remove", (PyCFunction)PlayList_Remove, METH_VARARGS, remove__doc__},
		{(char*)"clear", (PyCFunction)PlayList_Clear, METH_VARARGS, clear__doc__},
		{(char*)"size", (PyCFunction)PlayList_Size, METH_VARARGS, size__doc__},
		{(char*)"shuffle", (PyCFunction)PlayList_Shuffle, METH_VARARGS, shuffle__doc__},
		{(char*)"unshuffle", (PyCFunction)PlayList_UnShuffle, METH_VARARGS, unshuffle__doc__},
		{(char*)"getposition", (PyCFunction)PlayList_GetPosition, METH_VARARGS, getposition__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(playlistItem__doc__,
		"PlayListItem class.\n"
		"\n"
		"PlayListItem() -- Creates a new PlaylistItem which can be added to a PlayList.\n");

	PyDoc_STRVAR(playlist__doc__,
		"PlayList class.\n"
		"\n"
		"PlayList(int playlist) -- retrieve a reference from a valid xbmc playlist\n"
		"\n"
		"int playlist can be one of the next values:\n"
		"\n"
		"  0 : xbmc.PLAYLIST_MUSIC\n"
		"  1 : xbmc.PLAYLIST_VIDEO\n"
		"\n"
		"Use PlayList[int position] or __getitem__(int position) to get a PlayListItem.\n");

	PyTypeObject PlayListItem_Type;

	void initPlayListItem_Type()
	{
		PyXBMCInitializeTypeObject(&PlayListItem_Type);

		PlayListItem_Type.tp_name = (char*)"xbmc.PlayListItem";
		PlayListItem_Type.tp_basicsize = sizeof(PlayListItem);
		PlayListItem_Type.tp_dealloc = (destructor)PlayListItem_Dealloc;
		PlayListItem_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		PlayListItem_Type.tp_doc = playlistItem__doc__;
		PlayListItem_Type.tp_methods = PlayListItem_methods;
		PlayListItem_Type.tp_base = 0;
		PlayListItem_Type.tp_new = PlayListItem_New;
	}

	PyTypeObject PlayList_Type;

	void initPlayList_Type()
	{
		PyXBMCInitializeTypeObject(&PlayList_Type);

		PlayList_Type.tp_name = (char*)"xbmc.PlayList";
		PlayList_Type.tp_basicsize = sizeof(PlayList);
		PlayList_Type.tp_dealloc = (destructor)PlayList_Dealloc;
		PlayList_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		PlayList_Type.tp_doc = playlist__doc__;
		PlayList_Type.tp_methods = PlayList_methods;
		PlayList_Type.tp_as_mapping = &Playlist_as_mapping;
		PlayList_Type.tp_base = 0;
		PlayList_Type.tp_new = PlayList_New;
	}
}

#ifdef __cplusplus
}
#endif
