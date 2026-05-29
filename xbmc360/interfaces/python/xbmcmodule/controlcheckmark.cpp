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
	PyObject* ControlCheckMark_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlCheckMark *self;
		static const char *keywords[] = {
			"x", "y", "width", "height", "label",
			"focusTexture", "noFocusTexture",
			"checkWidth", "checkHeight", "alignment",
			"font", "textColor", "disabledColor", NULL };

		char *cFont = NULL;
		char *cTextColor = NULL;
		char *cDisabledColor = NULL;
		char *cTextureFocus = NULL;
		char *cTextureNoFocus = NULL;
		PyObject *pObjectText = NULL;

		self = (ControlCheckMark*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strFont) string();
		new(&self->strText) string();
		new(&self->strTextureFocus) string();
		new(&self->strTextureNoFocus) string();

		self->textColor = 0xffffffff;
		self->disabledColor = 0x60ffffff;
		self->checkWidth = 30;
		self->checkHeight = 30;
		self->align = 0;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"iiiiO|ssiiIsss", (char**)keywords,
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight,
			&pObjectText,
			&cTextureFocus, &cTextureNoFocus,
			&self->checkWidth, &self->checkHeight, &self->align,
			&cFont, &cTextColor, &cDisabledColor))
		{
			Py_DECREF(self);
			return NULL;
		}

		if (cFont) self->strFont = cFont;
		else self->strFont = "font13";

		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);
		if (cDisabledColor) self->disabledColor = (color_t)strtoul(cDisabledColor, NULL, 16);

		self->strTextureFocus = cTextureFocus ? cTextureFocus :
			PyXBMCGetDefaultImage((char*)"checkmark", (char*)"texturefocus", (char*)"check-box.png");
		self->strTextureNoFocus = cTextureNoFocus ? cTextureNoFocus :
			PyXBMCGetDefaultImage((char*)"checkmark", (char*)"texturenofocus", (char*)"check-boxNF.png");

		if (pObjectText)
		{
			string utf8String;
			if (PyXBMCGetUnicodeString(utf8String, pObjectText, 5))
				self->strText = utf8String;
		}

		self->pGUIControl = NULL;
		return (PyObject*)self;
	}

	void ControlCheckMark_Dealloc(ControlCheckMark* self)
	{
		self->strFont.~string();
		self->strText.~string();
		self->strTextureFocus.~string();
		self->strTextureNoFocus.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(getSelected__doc__,
		"getSelected() -- Returns the selected status for this checkmark as a bool.\n");
	PyObject* ControlCheckMark_GetSelected(ControlCheckMark *self)
	{
		// TODO: Get from actual GUI control when available
		Py_RETURN_FALSE;
	}

	PyDoc_STRVAR(setSelected__doc__,
		"setSelected(isOn) -- Sets this checkmark status to on or off.\n");
	PyObject* ControlCheckMark_SetSelected(ControlCheckMark *self, PyObject *args)
	{
		char bOnOff = false;
		if (!PyArg_ParseTuple(args, (char*)"b", &bOnOff)) return NULL;
		// TODO: Set on actual GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setLabel__checkmark_doc__,
		"setLabel(label[, font, textColor, disabledColor]) -- Set's this checkmark's text attributes.\n");
	PyObject* ControlCheckMark_SetLabel(ControlCheckMark *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "label", "font", "textColor", "disabledColor", NULL };
		PyObject *pObjectText = NULL;
		char *cFont = NULL;
		char *cTextColor = NULL;
		char *cDisabledColor = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"|Osss", (char**)keywords,
			&pObjectText, &cFont, &cTextColor, &cDisabledColor))
			return NULL;

		string utf8String;
		if (pObjectText && PyXBMCGetUnicodeString(utf8String, pObjectText, 1))
			self->strText = utf8String;
		if (cFont) self->strFont = cFont;
		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);
		if (cDisabledColor) self->disabledColor = (color_t)strtoul(cDisabledColor, NULL, 16);

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setDisabledColor__checkmark_doc__,
		"setDisabledColor(color) -- Set's this checkmark's disabled color.\n");
	PyObject* ControlCheckMark_SetDisabledColor(ControlCheckMark *self, PyObject *args)
	{
		char *cColor = NULL;
		if (!PyArg_ParseTuple(args, (char*)"s", &cColor)) return NULL;
		if (cColor) self->disabledColor = (color_t)strtoul(cColor, NULL, 16);
		Py_RETURN_NONE;
	}

	PyMethodDef ControlCheckMark_methods[] = {
		{(char*)"getSelected", (PyCFunction)ControlCheckMark_GetSelected, METH_NOARGS, getSelected__doc__},
		{(char*)"setSelected", (PyCFunction)ControlCheckMark_SetSelected, METH_VARARGS, setSelected__doc__},
		{(char*)"setLabel", (PyCFunction)ControlCheckMark_SetLabel, METH_VARARGS|METH_KEYWORDS, setLabel__checkmark_doc__},
		{(char*)"setDisabledColor", (PyCFunction)ControlCheckMark_SetDisabledColor, METH_VARARGS, setDisabledColor__checkmark_doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlCheckMark__doc__,
		"ControlCheckMark class.\n"
		"\n"
		"ControlCheckMark(x, y, width, height, label[, ...])\n");

	PyTypeObject ControlCheckMark_Type;

	void initControlCheckMark_Type()
	{
		PyXBMCInitializeTypeObject(&ControlCheckMark_Type);

		ControlCheckMark_Type.tp_name = (char*)"xbmcgui.ControlCheckMark";
		ControlCheckMark_Type.tp_basicsize = sizeof(ControlCheckMark);
		ControlCheckMark_Type.tp_dealloc = (destructor)ControlCheckMark_Dealloc;
		ControlCheckMark_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlCheckMark_Type.tp_doc = controlCheckMark__doc__;
		ControlCheckMark_Type.tp_methods = ControlCheckMark_methods;
		ControlCheckMark_Type.tp_base = &Control_Type;
		ControlCheckMark_Type.tp_new = ControlCheckMark_New;
	}
}

#ifdef __cplusplus
}
#endif
