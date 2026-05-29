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
	PyObject* ControlImage_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlImage *self;
		static const char *keywords[] = { "x", "y", "width", "height", "filename",
			"aspectRatio", "colorDiffuse", NULL };

		char *cFileName = NULL;
		char *cColorDiffuse = NULL;

		self = (ControlImage*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strFileName) string();

		self->aspectRatio = 0;
		self->colorDiffuse = 0;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"iiiis|is", (char**)keywords,
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight,
			&cFileName, &self->aspectRatio, &cColorDiffuse))
		{
			Py_DECREF(self);
			return NULL;
		}

		self->strFileName = cFileName ? cFileName : "";
		if (cColorDiffuse) self->colorDiffuse = (color_t)strtoul(cColorDiffuse, NULL, 16);

		self->pGUIControl = NULL;
		return (PyObject*)self;
	}

	void ControlImage_Dealloc(ControlImage* self)
	{
		self->strFileName.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(setImage__doc__,
		"setImage(filename) -- Changes the image.\n");
	PyObject* ControlImage_SetImage(ControlImage *self, PyObject *args)
	{
		char *cFileName = NULL;
		if (!PyArg_ParseTuple(args, (char*)"s", &cFileName)) return NULL;

		self->strFileName = cFileName ? cFileName : "";
		// TODO: Update GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setColorDiffuse__doc__,
		"setColorDiffuse(color) -- Changes the images color diffuse.\n");
	PyObject* ControlImage_SetColorDiffuse(ControlImage *self, PyObject *args)
	{
		char *cColor = NULL;
		if (!PyArg_ParseTuple(args, (char*)"s", &cColor)) return NULL;
		if (cColor) self->colorDiffuse = (color_t)strtoul(cColor, NULL, 16);
		// TODO: Update GUI control when available
		Py_RETURN_NONE;
	}

	PyMethodDef ControlImage_methods[] = {
		{(char*)"setImage", (PyCFunction)ControlImage_SetImage, METH_VARARGS, setImage__doc__},
		{(char*)"setColorDiffuse", (PyCFunction)ControlImage_SetColorDiffuse, METH_VARARGS, setColorDiffuse__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlImage__doc__,
		"ControlImage class.\n"
		"\n"
		"ControlImage(x, y, width, height, filename[, aspectRatio, colorDiffuse])\n");

	PyTypeObject ControlImage_Type;

	void initControlImage_Type()
	{
		PyXBMCInitializeTypeObject(&ControlImage_Type);

		ControlImage_Type.tp_name = (char*)"xbmcgui.ControlImage";
		ControlImage_Type.tp_basicsize = sizeof(ControlImage);
		ControlImage_Type.tp_dealloc = (destructor)ControlImage_Dealloc;
		ControlImage_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlImage_Type.tp_doc = controlImage__doc__;
		ControlImage_Type.tp_methods = ControlImage_methods;
		ControlImage_Type.tp_base = &Control_Type;
		ControlImage_Type.tp_new = ControlImage_New;
	}
}

#ifdef __cplusplus
}
#endif
