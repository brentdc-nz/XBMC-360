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
	PyObject* ControlRadioButton_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlRadioButton *self;
		static const char *keywords[] = {
			"x", "y", "width", "height", "label",
			"focusTexture", "noFocusTexture",
			"textOffsetX", "textOffsetY", "alignment",
			"font", "textColor", "disabledColor", "angle",
			"shadowColor", "focusedColor",
			"radioFocusTexture", "radioNoFocusTexture", NULL };

		char *cFont = NULL;
		char *cTextColor = NULL;
		char *cDisabledColor = NULL;
		char *cShadowColor = NULL;
		char *cFocusedColor = NULL;
		char *cTextureFocus = NULL;
		char *cTextureNoFocus = NULL;
		char *cTextureRadioFocus = NULL;
		char *cTextureRadioNoFocus = NULL;
		PyObject *pObjectText = NULL;

		self = (ControlRadioButton*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strFont) string();
		new(&self->strText) string();
		new(&self->strTextureFocus) string();
		new(&self->strTextureNoFocus) string();
		new(&self->strTextureRadioFocus) string();
		new(&self->strTextureRadioNoFocus) string();

		self->textColor = 0xffffffff;
		self->disabledColor = 0x60ffffff;
		self->textOffsetX = 10;
		self->textOffsetY = 2;
		self->align = 0;
		self->iAngle = 0;
		self->shadowColor = 0;
		self->focusedColor = 0xffffffff;

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"iiiiO|ssllIsssls ss", (char**)keywords,
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight,
			&pObjectText,
			&cTextureFocus, &cTextureNoFocus,
			&self->textOffsetX, &self->textOffsetY, &self->align,
			&cFont, &cTextColor, &cDisabledColor, &self->iAngle,
			&cShadowColor, &cFocusedColor,
			&cTextureRadioFocus, &cTextureRadioNoFocus))
		{
			Py_DECREF(self);
			return NULL;
		}

		if (cFont) self->strFont = cFont;
		else self->strFont = "font13";

		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);
		if (cDisabledColor) self->disabledColor = (color_t)strtoul(cDisabledColor, NULL, 16);
		if (cShadowColor) self->shadowColor = (color_t)strtoul(cShadowColor, NULL, 16);
		if (cFocusedColor) self->focusedColor = (color_t)strtoul(cFocusedColor, NULL, 16);

		self->strTextureFocus = cTextureFocus ? cTextureFocus :
			PyXBMCGetDefaultImage((char*)"button", (char*)"texturefocus", (char*)"button-focus.png");
		self->strTextureNoFocus = cTextureNoFocus ? cTextureNoFocus :
			PyXBMCGetDefaultImage((char*)"button", (char*)"texturenofocus", (char*)"button-nofocus.jpg");
		self->strTextureRadioFocus = cTextureRadioFocus ? cTextureRadioFocus :
			PyXBMCGetDefaultImage((char*)"radiobutton", (char*)"texturefocus", (char*)"radiobutton-focus.png");
		self->strTextureRadioNoFocus = cTextureRadioNoFocus ? cTextureRadioNoFocus :
			PyXBMCGetDefaultImage((char*)"radiobutton", (char*)"texturenofocus", (char*)"radiobutton-nofocus.png");

		if (pObjectText)
		{
			string utf8String;
			if (PyXBMCGetUnicodeString(utf8String, pObjectText, 5))
				self->strText = utf8String;
		}

		self->pGUIControl = NULL;
		return (PyObject*)self;
	}

	void ControlRadioButton_Dealloc(ControlRadioButton* self)
	{
		self->strFont.~string();
		self->strText.~string();
		self->strTextureFocus.~string();
		self->strTextureNoFocus.~string();
		self->strTextureRadioFocus.~string();
		self->strTextureRadioNoFocus.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(setSelected_radio__doc__,
		"setSelected(selected) -- Sets the radio button's selected status.\n");
	PyObject* ControlRadioButton_SetSelected(ControlRadioButton *self, PyObject *args)
	{
		char bOnOff = false;
		if (!PyArg_ParseTuple(args, (char*)"b", &bOnOff)) return NULL;
		// TODO: Set on GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(isSelected_radio__doc__,
		"isSelected() -- Returns the radio button's selected status.\n");
	PyObject* ControlRadioButton_IsSelected(ControlRadioButton *self)
	{
		// TODO: Get from GUI control when available
		Py_RETURN_FALSE;
	}

	PyDoc_STRVAR(setLabel_radio__doc__,
		"setLabel(label[, font, textColor, disabledColor, shadowColor, focusedColor]) -- Set's the radio button text.\n");
	PyObject* ControlRadioButton_SetLabel(ControlRadioButton *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "label", "font", "textColor", "disabledColor", "shadowColor", "focusedColor", NULL };
		PyObject *pObjectText = NULL;
		char *cFont = NULL;
		char *cTextColor = NULL;
		char *cDisabledColor = NULL;
		char *cShadowColor = NULL;
		char *cFocusedColor = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"|Osssss", (char**)keywords,
			&pObjectText, &cFont, &cTextColor, &cDisabledColor, &cShadowColor, &cFocusedColor))
			return NULL;

		string utf8String;
		if (pObjectText && PyXBMCGetUnicodeString(utf8String, pObjectText, 1))
			self->strText = utf8String;
		if (cFont) self->strFont = cFont;
		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);
		if (cDisabledColor) self->disabledColor = (color_t)strtoul(cDisabledColor, NULL, 16);
		if (cShadowColor) self->shadowColor = (color_t)strtoul(cShadowColor, NULL, 16);
		if (cFocusedColor) self->focusedColor = (color_t)strtoul(cFocusedColor, NULL, 16);

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setRadioDimension_radio__doc__,
		"setRadioDimension([x, y, width, height]) -- Sets the radio button's radio texture's position and size.\n");
	PyObject* ControlRadioButton_SetRadioDimension(ControlRadioButton *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "x", "y", "width", "height", NULL };
		int x = 0, y = 0, width = 0, height = 0;
		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"|iiii", (char**)keywords,
			&x, &y, &width, &height))
			return NULL;
		// TODO: Set radio dimension on GUI control when available
		Py_RETURN_NONE;
	}

	PyMethodDef ControlRadioButton_methods[] = {
		{(char*)"setSelected", (PyCFunction)ControlRadioButton_SetSelected, METH_VARARGS, setSelected_radio__doc__},
		{(char*)"isSelected", (PyCFunction)ControlRadioButton_IsSelected, METH_NOARGS, isSelected_radio__doc__},
		{(char*)"setLabel", (PyCFunction)ControlRadioButton_SetLabel, METH_VARARGS|METH_KEYWORDS, setLabel_radio__doc__},
		{(char*)"setRadioDimension", (PyCFunction)ControlRadioButton_SetRadioDimension, METH_VARARGS|METH_KEYWORDS, setRadioDimension_radio__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlRadioButton__doc__,
		"ControlRadioButton class.\n"
		"\n"
		"ControlRadioButton(x, y, width, height, label[, ...])\n");

	PyTypeObject ControlRadioButton_Type;

	void initControlRadioButton_Type()
	{
		PyXBMCInitializeTypeObject(&ControlRadioButton_Type);

		ControlRadioButton_Type.tp_name = (char*)"xbmcgui.ControlRadioButton";
		ControlRadioButton_Type.tp_basicsize = sizeof(ControlRadioButton);
		ControlRadioButton_Type.tp_dealloc = (destructor)ControlRadioButton_Dealloc;
		ControlRadioButton_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlRadioButton_Type.tp_doc = controlRadioButton__doc__;
		ControlRadioButton_Type.tp_methods = ControlRadioButton_methods;
		ControlRadioButton_Type.tp_base = &Control_Type;
		ControlRadioButton_Type.tp_new = ControlRadioButton_New;
	}
}

#ifdef __cplusplus
}
#endif
