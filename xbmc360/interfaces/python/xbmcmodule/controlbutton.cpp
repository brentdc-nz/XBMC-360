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
	PyObject* ControlButton_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlButton *self;
		static const char *keywords[] = {
			"x", "y", "width", "height", "label",
			"focusTexture", "noFocusTexture",
			"textOffsetX", "textOffsetY", "alignment",
			"font", "textColor", "disabledColor", "angle",
			"shadowColor", "focusedColor", NULL };

		char *cFont = NULL;
		char *cTextColor = NULL;
		char *cDisabledColor = NULL;
		char *cShadowColor = NULL;
		char *cFocusedColor = NULL;
		char *cTextureFocus = NULL;
		char *cTextureNoFocus = NULL;
		PyObject *pObjectText = NULL;

		self = (ControlButton*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strFont) string();
		new(&self->strText) string();
		new(&self->strText2) string();
		new(&self->strTextureFocus) string();
		new(&self->strTextureNoFocus) string();

		self->textColor = 0xffffffff;
		self->disabledColor = 0x60ffffff;
		self->textOffsetX = 10;
		self->textOffsetY = 2;
		self->align = 0;
		self->iAngle = 0;
		self->shadowColor = 0;
		self->focusedColor = 0xffffffff;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"iiiiO|ssllIsssls", (char**)keywords,
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight,
			&pObjectText,
			&cTextureFocus, &cTextureNoFocus,
			&self->textOffsetX, &self->textOffsetY, &self->align,
			&cFont, &cTextColor, &cDisabledColor, &self->iAngle,
			&cShadowColor, &cFocusedColor))
		{
			Py_DECREF(self);
			return NULL;
		}

		if (cFont) self->strFont = cFont;
		else self->strFont = "font13";

		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);
		if (cDisabledColor) self->disabledColor = (color_t)strtoul(cDisabledColor, NULL, 16);
		if (cShadowColor) self->shadowColor = (int)strtoul(cShadowColor, NULL, 16);
		if (cFocusedColor) self->focusedColor = (int)strtoul(cFocusedColor, NULL, 16);

		self->strTextureFocus = cTextureFocus ? cTextureFocus :
			PyXBMCGetDefaultImage((char*)"button", (char*)"texturefocus", (char*)"button-focus.png");
		self->strTextureNoFocus = cTextureNoFocus ? cTextureNoFocus :
			PyXBMCGetDefaultImage((char*)"button", (char*)"texturenofocus", (char*)"button-nofocus.jpg");

		if (pObjectText)
		{
			string utf8String;
			if (PyXBMCGetUnicodeString(utf8String, pObjectText, 5))
				self->strText = utf8String;
		}

		self->pGUIControl = NULL;
		return (PyObject*)self;
	}

	void ControlButton_Dealloc(ControlButton* self)
	{
		self->strFont.~string();
		self->strText.~string();
		self->strText2.~string();
		self->strTextureFocus.~string();
		self->strTextureNoFocus.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(setLabel_button__doc__,
		"setLabel([label, font, textColor, disabledColor, shadowColor, focusedColor]) -- Set's this button's text attributes.\n");
	PyObject* ControlButton_SetLabel(ControlButton *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "label", "font", "textColor", "disabledColor", "shadowColor", "focusedColor", "label2", NULL };
		PyObject *pObjectText = NULL;
		PyObject *pObjectText2 = NULL;
		char *cFont = NULL;
		char *cTextColor = NULL;
		char *cDisabledColor = NULL;
		char *cShadowColor = NULL;
		char *cFocusedColor = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"|OsssssO", (char**)keywords,
			&pObjectText, &cFont, &cTextColor, &cDisabledColor, &cShadowColor, &cFocusedColor, &pObjectText2))
			return NULL;

		string utf8String;
		if (pObjectText && PyXBMCGetUnicodeString(utf8String, pObjectText, 1))
			self->strText = utf8String;
		if (pObjectText2 && PyXBMCGetUnicodeString(utf8String, pObjectText2, 7))
			self->strText2 = utf8String;
		if (cFont) self->strFont = cFont;
		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);
		if (cDisabledColor) self->disabledColor = (color_t)strtoul(cDisabledColor, NULL, 16);
		if (cShadowColor) self->shadowColor = (int)strtoul(cShadowColor, NULL, 16);
		if (cFocusedColor) self->focusedColor = (int)strtoul(cFocusedColor, NULL, 16);

		// TODO: Update GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setDisabledColor_button__doc__,
		"setDisabledColor(color) -- Set's this button's disabled color.\n");
	PyObject* ControlButton_SetDisabledColor(ControlButton *self, PyObject *args)
	{
		char *cColor = NULL;
		if (!PyArg_ParseTuple(args, (char*)"s", &cColor)) return NULL;
		if (cColor) self->disabledColor = (color_t)strtoul(cColor, NULL, 16);
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(getLabel_button__doc__,
		"getLabel() -- Returns the button's label as a unicode string.\n");
	PyObject* ControlButton_GetLabel(ControlButton *self)
	{
		return Py_BuildValue((char*)"s", self->strText.c_str());
	}

	PyDoc_STRVAR(getLabel2_button__doc__,
		"getLabel2() -- Returns the button's label2 as a unicode string.\n");
	PyObject* ControlButton_GetLabel2(ControlButton *self)
	{
		return Py_BuildValue((char*)"s", self->strText2.c_str());
	}

	PyMethodDef ControlButton_methods[] = {
		{(char*)"setLabel", (PyCFunction)ControlButton_SetLabel, METH_VARARGS|METH_KEYWORDS, setLabel_button__doc__},
		{(char*)"setDisabledColor", (PyCFunction)ControlButton_SetDisabledColor, METH_VARARGS, setDisabledColor_button__doc__},
		{(char*)"getLabel", (PyCFunction)ControlButton_GetLabel, METH_NOARGS, getLabel_button__doc__},
		{(char*)"getLabel2", (PyCFunction)ControlButton_GetLabel2, METH_NOARGS, getLabel2_button__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlButton__doc__,
		"ControlButton class.\n"
		"\n"
		"ControlButton(x, y, width, height, label[, ...])\n");

	PyTypeObject ControlButton_Type;

	void initControlButton_Type()
	{
		PyXBMCInitializeTypeObject(&ControlButton_Type);

		ControlButton_Type.tp_name = (char*)"xbmcgui.ControlButton";
		ControlButton_Type.tp_basicsize = sizeof(ControlButton);
		ControlButton_Type.tp_dealloc = (destructor)ControlButton_Dealloc;
		ControlButton_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlButton_Type.tp_doc = controlButton__doc__;
		ControlButton_Type.tp_methods = ControlButton_methods;
		ControlButton_Type.tp_base = &Control_Type;
		ControlButton_Type.tp_new = ControlButton_New;
	}
}

#ifdef __cplusplus
}
#endif
