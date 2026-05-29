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
	PyObject* ControlTextBox_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlTextBox *self;
		static const char *keywords[] = { "x", "y", "width", "height",
			"font", "textColor", NULL };

		char *cFont = NULL;
		char *cTextColor = NULL;

		self = (ControlTextBox*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strFont) string();

		self->textColor = 0xffffffff;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"iiii|ss", (char**)keywords,
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight,
			&cFont, &cTextColor))
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

	void ControlTextBox_Dealloc(ControlTextBox* self)
	{
		self->strFont.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(setText__doc__,
		"setText(text) -- Set's the text for this textbox.\n");
	PyObject* ControlTextBox_SetText(ControlTextBox *self, PyObject *args)
	{
		PyObject *pObjectText;
		if (!PyArg_ParseTuple(args, (char*)"O", &pObjectText)) return NULL;

		string utf8String;
		if (!PyXBMCGetUnicodeString(utf8String, pObjectText, 1)) return NULL;

		// TODO: Set text on GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(getText__doc__,
		"getText() -- Returns the text value for this textbox.\n");
	PyObject* ControlTextBox_GetText(ControlTextBox *self)
	{
		// TODO: Get text from GUI control when available
		return Py_BuildValue((char*)"s", "");
	}

	PyDoc_STRVAR(reset_textbox__doc__,
		"reset() -- Clear's this textbox.\n");
	PyObject* ControlTextBox_Reset(ControlTextBox *self, PyObject *args)
	{
		// TODO: Reset GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(scroll_textbox__doc__,
		"scroll(position) -- Scrolls to the given position.\n");
	PyObject* ControlTextBox_Scroll(ControlTextBox *self, PyObject *args)
	{
		int iPos;
		if (!PyArg_ParseTuple(args, (char*)"i", &iPos)) return NULL;
		// TODO: Scroll GUI control when available
		Py_RETURN_NONE;
	}

	PyMethodDef ControlTextBox_methods[] = {
		{(char*)"setText", (PyCFunction)ControlTextBox_SetText, METH_VARARGS, setText__doc__},
		{(char*)"getText", (PyCFunction)ControlTextBox_GetText, METH_NOARGS, getText__doc__},
		{(char*)"reset", (PyCFunction)ControlTextBox_Reset, METH_VARARGS, reset_textbox__doc__},
		{(char*)"scroll", (PyCFunction)ControlTextBox_Scroll, METH_VARARGS, scroll_textbox__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlTextBox__doc__,
		"ControlTextBox class.\n"
		"\n"
		"ControlTextBox(x, y, width, height[, font, textColor])\n");

	PyTypeObject ControlTextBox_Type;

	void initControlTextBox_Type()
	{
		PyXBMCInitializeTypeObject(&ControlTextBox_Type);

		ControlTextBox_Type.tp_name = (char*)"xbmcgui.ControlTextBox";
		ControlTextBox_Type.tp_basicsize = sizeof(ControlTextBox);
		ControlTextBox_Type.tp_dealloc = (destructor)ControlTextBox_Dealloc;
		ControlTextBox_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlTextBox_Type.tp_doc = controlTextBox__doc__;
		ControlTextBox_Type.tp_methods = ControlTextBox_methods;
		ControlTextBox_Type.tp_base = &Control_Type;
		ControlTextBox_Type.tp_new = ControlTextBox_New;
	}
}

#ifdef __cplusplus
}
#endif
