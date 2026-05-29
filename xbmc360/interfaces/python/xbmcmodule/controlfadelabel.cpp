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
	PyObject* ControlFadeLabel_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlFadeLabel *self;
		static const char *keywords[] = { "x", "y", "width", "height",
			"font", "textColor", "alignment", NULL };

		char *cFont = NULL;
		char *cTextColor = NULL;

		self = (ControlFadeLabel*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strFont) string();
		new(&self->vecLabels) vector<string>();

		self->textColor = 0xffffffff;
		self->align = 0;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"iiii|ssl", (char**)keywords,
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight,
			&cFont, &cTextColor, &self->align))
		{
			Py_DECREF(self);
			return NULL;
		}

		if (cFont) self->strFont = cFont;
		else self->strFont = "font13";
		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);

		self->pGUIControl = NULL;
		return (PyObject*)self;
	}

	void ControlFadeLabel_Dealloc(ControlFadeLabel* self)
	{
		self->strFont.~string();
		self->vecLabels.~vector();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(addLabel__doc__,
		"addLabel(label) -- Add a label to this control for scrolling.\n");
	PyObject* ControlFadeLabel_AddLabel(ControlFadeLabel *self, PyObject *args)
	{
		PyObject *pObjectText;
		if (!PyArg_ParseTuple(args, (char*)"O", &pObjectText)) return NULL;

		string utf8String;
		if (!PyXBMCGetUnicodeString(utf8String, pObjectText, 1)) return NULL;

		self->vecLabels.push_back(utf8String);
		// TODO: Update GUI control when available

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(reset_fadelabel__doc__,
		"reset() -- Clear this fade label.\n");
	PyObject* ControlFadeLabel_Reset(ControlFadeLabel *self, PyObject *args)
	{
		self->vecLabels.clear();
		// TODO: Update GUI control when available
		Py_RETURN_NONE;
	}

	PyMethodDef ControlFadeLabel_methods[] = {
		{(char*)"addLabel", (PyCFunction)ControlFadeLabel_AddLabel, METH_VARARGS, addLabel__doc__},
		{(char*)"reset", (PyCFunction)ControlFadeLabel_Reset, METH_VARARGS, reset_fadelabel__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlFadeLabel__doc__,
		"ControlFadeLabel class.\n"
		"\n"
		"ControlFadeLabel(x, y, width, height[, font, textColor, alignment])\n");

	PyTypeObject ControlFadeLabel_Type;

	void initControlFadeLabel_Type()
	{
		PyXBMCInitializeTypeObject(&ControlFadeLabel_Type);

		ControlFadeLabel_Type.tp_name = (char*)"xbmcgui.ControlFadeLabel";
		ControlFadeLabel_Type.tp_basicsize = sizeof(ControlFadeLabel);
		ControlFadeLabel_Type.tp_dealloc = (destructor)ControlFadeLabel_Dealloc;
		ControlFadeLabel_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlFadeLabel_Type.tp_doc = controlFadeLabel__doc__;
		ControlFadeLabel_Type.tp_methods = ControlFadeLabel_methods;
		ControlFadeLabel_Type.tp_base = &Control_Type;
		ControlFadeLabel_Type.tp_new = ControlFadeLabel_New;
	}
}

#ifdef __cplusplus
}
#endif
