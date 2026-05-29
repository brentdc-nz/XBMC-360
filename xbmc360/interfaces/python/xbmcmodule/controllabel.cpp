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
	PyObject* ControlLabel_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlLabel *self;
		static const char *keywords[] = {
			"x", "y", "width", "height", "label",
			"font", "textColor", "disabledColor", "alignment",
			"hasPath", "angle", NULL };

		char* cFont = NULL;
		char* cTextColor = NULL;
		char* cDisabledColor = NULL;
		PyObject* pObjectText = NULL;
		char bHasPath = false;

		self = (ControlLabel*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strFont) string();
		new(&self->strText) string();

		self->textColor = 0xffffffff;
		self->disabledColor = 0x60ffffff;
		self->align = 0;
		self->bHasPath = false;
		self->iAngle = 0;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"iiiiO|sslbbi", (char**)keywords,
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight,
			&pObjectText, &cFont, &cTextColor, &cDisabledColor,
			&self->align, &bHasPath, &self->iAngle))
		{
			Py_DECREF(self);
			return NULL;
		}

		self->bHasPath = (bHasPath != 0);
		if (cFont) self->strFont = cFont;
		else self->strFont = "font13";

		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);
		if (cDisabledColor) self->disabledColor = (color_t)strtoul(cDisabledColor, NULL, 16);

		if (pObjectText)
		{
			string utf8String;
			if (PyXBMCGetUnicodeString(utf8String, pObjectText, 5))
				self->strText = utf8String;
		}

		self->pGUIControl = NULL;
		return (PyObject*)self;
	}

	void ControlLabel_Dealloc(ControlLabel* self)
	{
		self->strFont.~string();
		self->strText.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(getLabel_label__doc__,
		"getLabel() -- Returns the text value for this label.\n");
	PyObject* ControlLabel_GetLabel(ControlLabel *self)
	{
		return Py_BuildValue((char*)"s", self->strText.c_str());
	}

	PyDoc_STRVAR(setLabel_label__doc__,
		"setLabel(label[, font, textColor, disabledColor]) -- Set's text for this label.\n");
	PyObject* ControlLabel_SetLabel(ControlLabel *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "label", "font", "textColor", "disabledColor", NULL };
		PyObject* pObjectText = NULL;
		char* cFont = NULL;
		char* cTextColor = NULL;
		char* cDisabledColor = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"O|sss", (char**)keywords,
			&pObjectText, &cFont, &cTextColor, &cDisabledColor))
			return NULL;

		string utf8String;
		if (pObjectText && PyXBMCGetUnicodeString(utf8String, pObjectText, 1))
			self->strText = utf8String;
		if (cFont) self->strFont = cFont;
		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);
		if (cDisabledColor) self->disabledColor = (color_t)strtoul(cDisabledColor, NULL, 16);

		// TODO: Update GUI control when available
		Py_RETURN_NONE;
	}

	PyMethodDef ControlLabel_methods[] = {
		{(char*)"getLabel", (PyCFunction)ControlLabel_GetLabel, METH_NOARGS, getLabel_label__doc__},
		{(char*)"setLabel", (PyCFunction)ControlLabel_SetLabel, METH_VARARGS|METH_KEYWORDS, setLabel_label__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlLabel__doc__,
		"ControlLabel class.\n"
		"\n"
		"ControlLabel(x, y, width, height, label[, font, textColor, disabledColor, alignment, hasPath, angle])\n");

	PyTypeObject ControlLabel_Type;

	void initControlLabel_Type()
	{
		PyXBMCInitializeTypeObject(&ControlLabel_Type);

		ControlLabel_Type.tp_name = (char*)"xbmcgui.ControlLabel";
		ControlLabel_Type.tp_basicsize = sizeof(ControlLabel);
		ControlLabel_Type.tp_dealloc = (destructor)ControlLabel_Dealloc;
		ControlLabel_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlLabel_Type.tp_doc = controlLabel__doc__;
		ControlLabel_Type.tp_methods = ControlLabel_methods;
		ControlLabel_Type.tp_base = &Control_Type;
		ControlLabel_Type.tp_new = ControlLabel_New;
	}
}

#ifdef __cplusplus
}
#endif
