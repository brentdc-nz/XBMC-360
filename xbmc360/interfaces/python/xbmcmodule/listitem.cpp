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

#include "listitem.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	PyObject* ListItem_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ListItem *self;
		static const char *keywords[] = { "label", "label2",
			"iconImage", "thumbnailImage", "path", NULL };

		PyObject* label = NULL;
		PyObject* label2 = NULL;
		PyObject* iconImage = NULL;
		PyObject* thumbnailImage = NULL;
		PyObject* path = NULL;

		self = (ListItem*)type->tp_alloc(type, 0);
		if (!self) return NULL;

		new(&self->strLabel) string();
		new(&self->strLabel2) string();
		new(&self->strIconImage) string();
		new(&self->strThumbnailImage) string();
		new(&self->strPath) string();
		new(&self->infoLabels) map<string,string>();
		new(&self->properties) map<string,string>();
		self->bSelected = false;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"|OOOOO", (char**)keywords,
			&label, &label2, &iconImage, &thumbnailImage, &path))
		{
			Py_DECREF(self);
			return NULL;
		}

		string utf8String;
		if (label && PyXBMCGetUnicodeString(utf8String, label, 1))
			self->strLabel = utf8String;
		if (label2 && PyXBMCGetUnicodeString(utf8String, label2, 1))
			self->strLabel2 = utf8String;
		if (iconImage && PyXBMCGetUnicodeString(utf8String, iconImage, 1))
			self->strIconImage = utf8String;
		if (thumbnailImage && PyXBMCGetUnicodeString(utf8String, thumbnailImage, 1))
			self->strThumbnailImage = utf8String;
		if (path && PyXBMCGetUnicodeString(utf8String, path, 1))
			self->strPath = utf8String;

		return (PyObject*)self;
	}

	ListItem* ListItem_FromString(string strLabel)
	{
		ListItem* self = (ListItem*)ListItem_Type.tp_alloc(&ListItem_Type, 0);
		if (!self) return NULL;

		new(&self->strLabel) string(strLabel);
		new(&self->strLabel2) string();
		new(&self->strIconImage) string();
		new(&self->strThumbnailImage) string();
		new(&self->strPath) string();
		new(&self->infoLabels) map<string,string>();
		new(&self->properties) map<string,string>();
		self->bSelected = false;

		return self;
	}

	void ListItem_Dealloc(ListItem* self)
	{
		self->strLabel.~string();
		self->strLabel2.~string();
		self->strIconImage.~string();
		self->strThumbnailImage.~string();
		self->strPath.~string();
		self->infoLabels.~map();
		self->properties.~map();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(getLabel__doc__,
		"getLabel() -- Returns the listitem label.\n");
	PyObject* ListItem_GetLabel(ListItem *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->strLabel.c_str());
	}

	PyDoc_STRVAR(getLabel2__doc__,
		"getLabel2() -- Returns the listitem's second label.\n");
	PyObject* ListItem_GetLabel2(ListItem *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->strLabel2.c_str());
	}

	PyDoc_STRVAR(setLabel__doc__,
		"setLabel(label) -- Sets the listitem's label.\n"
		"\n"
		"label          : string or unicode - text string.\n");
	PyObject* ListItem_SetLabel(ListItem *self, PyObject *args)
	{
		PyObject* unicodeLine = NULL;
		if (!PyArg_ParseTuple(args, (char*)"O", &unicodeLine)) return NULL;

		string utf8Line;
		if (unicodeLine && !PyXBMCGetUnicodeString(utf8Line, unicodeLine, 1))
			return NULL;

		self->strLabel = utf8Line;
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setLabel2__doc__,
		"setLabel2(label2) -- Sets the listitem's second label.\n"
		"\n"
		"label2         : string or unicode - text string.\n");
	PyObject* ListItem_SetLabel2(ListItem *self, PyObject *args)
	{
		PyObject* unicodeLine = NULL;
		if (!PyArg_ParseTuple(args, (char*)"O", &unicodeLine)) return NULL;

		string utf8Line;
		if (unicodeLine && !PyXBMCGetUnicodeString(utf8Line, unicodeLine, 1))
			return NULL;

		self->strLabel2 = utf8Line;
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setIconImage__doc__,
		"setIconImage(icon) -- Sets the listitem's icon image.\n"
		"\n"
		"icon            : string - image filename.\n");
	PyObject* ListItem_SetIconImage(ListItem *self, PyObject *args)
	{
		char *cLine = NULL;
		if (!PyArg_ParseTuple(args, (char*)"s", &cLine)) return NULL;
		self->strIconImage = cLine ? cLine : "";
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setThumbnailImage__doc__,
		"setThumbnailImage(thumb) -- Sets the listitem's thumbnail image.\n"
		"\n"
		"thumb           : string - image filename.\n");
	PyObject* ListItem_SetThumbnailImage(ListItem *self, PyObject *args)
	{
		char *cLine = NULL;
		if (!PyArg_ParseTuple(args, (char*)"s", &cLine)) return NULL;
		self->strThumbnailImage = cLine ? cLine : "";
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(select__doc__,
		"select(selected) -- Sets the listitem's selected status.\n"
		"\n"
		"selected        : bool - True=selected/False=not selected\n");
	PyObject* ListItem_Select(ListItem *self, PyObject *args)
	{
		char bOnOff = false;
		if (!PyArg_ParseTuple(args, (char*)"b", &bOnOff)) return NULL;
		self->bSelected = (bOnOff != 0);
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(isSelected__doc__,
		"isSelected() -- Returns the listitem's selected status.\n");
	PyObject* ListItem_IsSelected(ListItem *self, PyObject *args)
	{
		return Py_BuildValue((char*)"b", self->bSelected);
	}

	PyDoc_STRVAR(setInfo__doc__,
		"setInfo(type, infoLabels) -- Sets the listitem's infoLabels.\n"
		"\n"
		"type              : string - type of media(video/music/pictures).\n"
		"infoLabels        : dictionary - pairs of { label: value }.\n");
	PyObject* ListItem_SetInfo(ListItem *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "type", "infoLabels", NULL };
		char *cType = NULL;
		PyObject *pInfoLabels = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"sO",
			(char**)keywords, &cType, &pInfoLabels))
		{
			return NULL;
		}

		if (!PyDict_Check(pInfoLabels))
		{
			PyErr_SetString(PyExc_TypeError, "infoLabels object should be of type Dict");
			return NULL;
		}

		PyObject *key, *value;
		Py_ssize_t pos = 0;

		while (PyDict_Next(pInfoLabels, &pos, &key, &value))
		{
			string strKey, strValue;
			if (PyXBMCGetUnicodeString(strKey, key) && PyXBMCGetUnicodeString(strValue, value))
			{
				self->infoLabels[strKey] = strValue;
			}
		}

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setProperty__doc__,
		"setProperty(key, value) -- Sets a listitem property, similar to an infolabel.\n"
		"\n"
		"key            : string - property name.\n"
		"value          : string or unicode - value of property.\n");
	PyObject* ListItem_SetProperty(ListItem *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "key", "value", NULL };
		char *cKey = NULL;
		PyObject *pValue = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"sO",
			(char**)keywords, &cKey, &pValue))
		{
			return NULL;
		}

		if (!cKey) { Py_RETURN_NONE; }

		string strValue;
		if (pValue && !PyXBMCGetUnicodeString(strValue, pValue, 1))
			return NULL;

		string lowerKey(cKey);
		self->properties[lowerKey] = strValue;

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(getProperty__doc__,
		"getProperty(key) -- Returns a listitem property as a string, similar to an infolabel.\n"
		"\n"
		"key            : string - property name.\n");
	PyObject* ListItem_GetProperty(ListItem *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "key", NULL };
		char *cKey = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"s",
			(char**)keywords, &cKey))
		{
			return NULL;
		}

		string lowerKey(cKey ? cKey : "");
		auto it = self->properties.find(lowerKey);
		if (it != self->properties.end())
			return Py_BuildValue((char*)"s", it->second.c_str());

		return Py_BuildValue((char*)"s", "");
	}

	PyDoc_STRVAR(setPath__doc__,
		"setPath(path) -- Sets the listitem's path.\n"
		"\n"
		"path           : string or unicode - path, activated when item is clicked.\n");
	PyObject* ListItem_SetPath(ListItem *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "path", NULL };
		PyObject *pPath = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"O",
			(char**)keywords, &pPath))
		{
			return NULL;
		}

		string strPath;
		if (!PyXBMCGetUnicodeString(strPath, pPath, 1))
			return NULL;

		self->strPath = strPath;
		Py_RETURN_NONE;
	}

	PyMethodDef ListItem_methods[] = {
		{(char*)"getLabel", (PyCFunction)ListItem_GetLabel, METH_VARARGS, getLabel__doc__},
		{(char*)"getLabel2", (PyCFunction)ListItem_GetLabel2, METH_VARARGS, getLabel2__doc__},
		{(char*)"setLabel", (PyCFunction)ListItem_SetLabel, METH_VARARGS, setLabel__doc__},
		{(char*)"setLabel2", (PyCFunction)ListItem_SetLabel2, METH_VARARGS, setLabel2__doc__},
		{(char*)"setIconImage", (PyCFunction)ListItem_SetIconImage, METH_VARARGS, setIconImage__doc__},
		{(char*)"setThumbnailImage", (PyCFunction)ListItem_SetThumbnailImage, METH_VARARGS, setThumbnailImage__doc__},
		{(char*)"select", (PyCFunction)ListItem_Select, METH_VARARGS, select__doc__},
		{(char*)"isSelected", (PyCFunction)ListItem_IsSelected, METH_VARARGS, isSelected__doc__},
		{(char*)"setInfo", (PyCFunction)ListItem_SetInfo, METH_VARARGS|METH_KEYWORDS, setInfo__doc__},
		{(char*)"setProperty", (PyCFunction)ListItem_SetProperty, METH_VARARGS|METH_KEYWORDS, setProperty__doc__},
		{(char*)"getProperty", (PyCFunction)ListItem_GetProperty, METH_VARARGS|METH_KEYWORDS, getProperty__doc__},
		{(char*)"setPath", (PyCFunction)ListItem_SetPath, METH_VARARGS|METH_KEYWORDS, setPath__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(listItem__doc__,
		"ListItem class.\n"
		"\n"
		"ListItem([label, label2, iconImage, thumbnailImage, path]) -- Creates a new ListItem.\n"
		"\n"
		"label          : [opt] string or unicode\n"
		"label2         : [opt] string or unicode - label2 text\n"
		"iconImage      : [opt] string - icon filename\n"
		"thumbnailImage : [opt] string - thumbnail filename\n"
		"path           : [opt] string or unicode\n");

	PyTypeObject ListItem_Type;

	void initListItem_Type()
	{
		PyXBMCInitializeTypeObject(&ListItem_Type);

		ListItem_Type.tp_name = (char*)"xbmcgui.ListItem";
		ListItem_Type.tp_basicsize = sizeof(ListItem);
		ListItem_Type.tp_dealloc = (destructor)ListItem_Dealloc;
		ListItem_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ListItem_Type.tp_doc = listItem__doc__;
		ListItem_Type.tp_methods = ListItem_methods;
		ListItem_Type.tp_base = 0;
		ListItem_Type.tp_new = ListItem_New;
	}
}

#ifdef __cplusplus
}
#endif
