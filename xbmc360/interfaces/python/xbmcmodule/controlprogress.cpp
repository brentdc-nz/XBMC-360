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

#include "control.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	PyObject* ControlProgress_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlProgress *self;
		static const char *keywords[] = { "x", "y", "width", "height",
			"texturebg", "textureleft", "texturemid", "textureright",
			"textureoverlay", NULL };

		char *cTextureBg = NULL;
		char *cTextureLeft = NULL;
		char *cTextureMid = NULL;
		char *cTextureRight = NULL;
		char *cTextureOverlay = NULL;

		self = (ControlProgress*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strTextureLeft) string();
		new(&self->strTextureMid) string();
		new(&self->strTextureRight) string();
		new(&self->strTextureBg) string();
		new(&self->strTextureOverlay) string();

		self->aspectRatio = 0;
		self->colorDiffuse = 0;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"iiii|sssss", (char**)keywords,
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight,
			&cTextureBg, &cTextureLeft, &cTextureMid, &cTextureRight, &cTextureOverlay))
		{
			Py_DECREF(self);
			return NULL;
		}

		self->strTextureBg = cTextureBg ? cTextureBg :
			PyXBMCGetDefaultImage((char*)"progress", (char*)"texturebg", (char*)"progress_back.png");
		self->strTextureLeft = cTextureLeft ? cTextureLeft :
			PyXBMCGetDefaultImage((char*)"progress", (char*)"lefttexture", (char*)"progress_left.png");
		self->strTextureMid = cTextureMid ? cTextureMid :
			PyXBMCGetDefaultImage((char*)"progress", (char*)"midtexture", (char*)"progress_mid.png");
		self->strTextureRight = cTextureRight ? cTextureRight :
			PyXBMCGetDefaultImage((char*)"progress", (char*)"righttexture", (char*)"progress_right.png");
		self->strTextureOverlay = cTextureOverlay ? cTextureOverlay :
			PyXBMCGetDefaultImage((char*)"progress", (char*)"overlaytexture", (char*)"progress_over.png");

		self->pGUIControl = NULL;
		return (PyObject*)self;
	}

	void ControlProgress_Dealloc(ControlProgress* self)
	{
		self->strTextureLeft.~string();
		self->strTextureMid.~string();
		self->strTextureRight.~string();
		self->strTextureBg.~string();
		self->strTextureOverlay.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(setPercent__doc__,
		"setPercent(percent) -- Sets the percentage of the progressbar to show.\n");
	PyObject* ControlProgress_SetPercent(ControlProgress *self, PyObject *args)
	{
		float fPercent = 0;
		if (!PyArg_ParseTuple(args, (char*)"f", &fPercent)) return NULL;
		// TODO: Set percent on GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(getPercent__doc__,
		"getPercent() -- Returns a float of the percent of the progress.\n");
	PyObject* ControlProgress_GetPercent(ControlProgress *self)
	{
		// TODO: Get percent from GUI control when available
		return Py_BuildValue((char*)"f", 0.0f);
	}

	PyMethodDef ControlProgress_methods[] = {
		{(char*)"setPercent", (PyCFunction)ControlProgress_SetPercent, METH_VARARGS, setPercent__doc__},
		{(char*)"getPercent", (PyCFunction)ControlProgress_GetPercent, METH_NOARGS, getPercent__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlProgress__doc__,
		"ControlProgress class.\n"
		"\n"
		"ControlProgress(x, y, width, height[, texturebg, textureleft, texturemid, textureright, textureoverlay])\n");

	PyTypeObject ControlProgress_Type;

	void initControlProgress_Type()
	{
		PyXBMCInitializeTypeObject(&ControlProgress_Type);

		ControlProgress_Type.tp_name = (char*)"xbmcgui.ControlProgress";
		ControlProgress_Type.tp_basicsize = sizeof(ControlProgress);
		ControlProgress_Type.tp_dealloc = (destructor)ControlProgress_Dealloc;
		ControlProgress_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlProgress_Type.tp_doc = controlProgress__doc__;
		ControlProgress_Type.tp_methods = ControlProgress_methods;
		ControlProgress_Type.tp_base = &Control_Type;
		ControlProgress_Type.tp_new = ControlProgress_New;
	}
}

#ifdef __cplusplus
}
#endif
