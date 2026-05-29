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
	PyObject* ControlSpin_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlSpin *self;

		self = (ControlSpin*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strTextureUp) string();
		new(&self->strTextureDown) string();
		new(&self->strTextureUpFocus) string();
		new(&self->strTextureDownFocus) string();

		self->color = 0xffffffff;
		self->pGUIControl = NULL;

		return (PyObject*)self;
	}

	void ControlSpin_Dealloc(ControlSpin* self)
	{
		self->strTextureUp.~string();
		self->strTextureDown.~string();
		self->strTextureUpFocus.~string();
		self->strTextureDownFocus.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(setTextures__doc__,
		"setTextures(up, down, upFocus, downFocus) -- Set's textures for this spin control.\n");
	PyObject* ControlSpin_SetTextures(ControlSpin *self, PyObject *args)
	{
		char *cUp = NULL, *cDown = NULL, *cUpFocus = NULL, *cDownFocus = NULL;
		if (!PyArg_ParseTuple(args, (char*)"ssss", &cUp, &cDown, &cUpFocus, &cDownFocus))
			return NULL;

		self->strTextureUp = cUp ? cUp : "";
		self->strTextureDown = cDown ? cDown : "";
		self->strTextureUpFocus = cUpFocus ? cUpFocus : "";
		self->strTextureDownFocus = cDownFocus ? cDownFocus : "";
		// TODO: Update GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(setColor__doc__,
		"setColor(color) -- Set's the spin control's color.\n");
	PyObject* ControlSpin_SetColor(ControlSpin *self, PyObject *args)
	{
		char *cColor = NULL;
		if (!PyArg_ParseTuple(args, (char*)"s", &cColor)) return NULL;
		if (cColor) self->color = (color_t)strtoul(cColor, NULL, 16);
		Py_RETURN_NONE;
	}

	PyMethodDef ControlSpin_methods[] = {
		{(char*)"setTextures", (PyCFunction)ControlSpin_SetTextures, METH_VARARGS, setTextures__doc__},
		{(char*)"setColor", (PyCFunction)ControlSpin_SetColor, METH_VARARGS, setColor__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlSpin__doc__,
		"ControlSpin class.\n"
		"\n"
		"- Not directly creatable from python. Use ControlList.getSpinControl().\n");

	PyTypeObject ControlSpin_Type;

	void initControlSpin_Type()
	{
		PyXBMCInitializeTypeObject(&ControlSpin_Type);

		ControlSpin_Type.tp_name = (char*)"xbmcgui.ControlSpin";
		ControlSpin_Type.tp_basicsize = sizeof(ControlSpin);
		ControlSpin_Type.tp_dealloc = (destructor)ControlSpin_Dealloc;
		ControlSpin_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlSpin_Type.tp_doc = controlSpin__doc__;
		ControlSpin_Type.tp_methods = ControlSpin_methods;
		ControlSpin_Type.tp_base = &Control_Type;
		ControlSpin_Type.tp_new = ControlSpin_New;
	}
}

#ifdef __cplusplus
}
#endif
