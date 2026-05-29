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

#include "keyboard.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	PyObject* Keyboard_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		Keyboard *self;
		char *cDefault = NULL;
		char *cHeading = NULL;
		char bHidden = false;

		if (!PyArg_ParseTuple(args, (char*)"|ssb", &cDefault, &cHeading, &bHidden))
			return NULL;

		self = (Keyboard*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strDefault) string();
		new(&self->strHeading) string();

		if (cDefault) self->strDefault = cDefault;
		if (cHeading) self->strHeading = cHeading;
		self->bHidden = (bHidden != 0);
		self->bConfirmed = false;

		return (PyObject*)self;
	}

	void Keyboard_Dealloc(Keyboard* self)
	{
		self->strDefault.~string();
		self->strHeading.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	// doModal method
	PyDoc_STRVAR(doModal__doc__,
		"doModal([autoclose]) -- Show keyboard and wait for user action.\n"
		"\n"
		"autoclose      : [opt] integer - milliseconds to autoclose dialog. (default=do not autoclose)\n");

	PyObject* Keyboard_DoModal(Keyboard *self, PyObject *args)
	{
		int autoClose = 0;
		if (!PyArg_ParseTuple(args, (char*)"|i", &autoClose))
			return NULL;

		// TODO: Implement keyboard dialog when CGUIDialogKeyboard is available
		// For now, accept the default text as input
		self->bConfirmed = true;

		Py_RETURN_NONE;
	}

	// setDefault method
	PyDoc_STRVAR(setDefault__doc__,
		"setDefault(default) -- Set the default text entry.\n"
		"\n"
		"default        : string - default text entry.\n");

	PyObject* Keyboard_SetDefault(Keyboard *self, PyObject *args)
	{
		char *cDefault = NULL;
		if (!PyArg_ParseTuple(args, (char*)"|s", &cDefault))
			return NULL;

		if (cDefault)
			self->strDefault = cDefault;

		Py_RETURN_NONE;
	}

	// setHiddenInput method
	PyDoc_STRVAR(setHiddenInput__doc__,
		"setHiddenInput(hidden) -- Allows hidden text entry.\n"
		"\n"
		"hidden        : boolean - True for hidden text entry.\n");

	PyObject* Keyboard_SetHiddenInput(Keyboard *self, PyObject *args)
	{
		char bHidden = false;
		if (!PyArg_ParseTuple(args, (char*)"|b", &bHidden))
			return NULL;

		self->bHidden = (bHidden != 0);

		Py_RETURN_NONE;
	}

	// setHeading method
	PyDoc_STRVAR(setHeading__doc__,
		"setHeading(heading) -- Set the keyboard heading.\n"
		"\n"
		"heading        : string - keyboard heading.\n");

	PyObject* Keyboard_SetHeading(Keyboard *self, PyObject *args)
	{
		char *cHeading = NULL;
		if (!PyArg_ParseTuple(args, (char*)"|s", &cHeading))
			return NULL;

		if (cHeading)
			self->strHeading = cHeading;

		Py_RETURN_NONE;
	}

	// getText method
	PyDoc_STRVAR(getText__doc__,
		"getText() -- Returns the user input as a string.\n");

	PyObject* Keyboard_GetText(Keyboard *self, PyObject *args)
	{
		return Py_BuildValue((char*)"s", self->strDefault.c_str());
	}

	// isConfirmed method
	PyDoc_STRVAR(isConfirmed__doc__,
		"isConfirmed() -- Returns False if the user cancelled the input.\n");

	PyObject* Keyboard_IsConfirmed(Keyboard *self, PyObject *args)
	{
		if (self->bConfirmed)
			Py_RETURN_TRUE;
		Py_RETURN_FALSE;
	}

	PyMethodDef Keyboard_methods[] = {
		{(char*)"doModal", (PyCFunction)Keyboard_DoModal, METH_VARARGS, doModal__doc__},
		{(char*)"setDefault", (PyCFunction)Keyboard_SetDefault, METH_VARARGS, setDefault__doc__},
		{(char*)"setHiddenInput", (PyCFunction)Keyboard_SetHiddenInput, METH_VARARGS, setHiddenInput__doc__},
		{(char*)"setHeading", (PyCFunction)Keyboard_SetHeading, METH_VARARGS, setHeading__doc__},
		{(char*)"getText", (PyCFunction)Keyboard_GetText, METH_VARARGS, getText__doc__},
		{(char*)"isConfirmed", (PyCFunction)Keyboard_IsConfirmed, METH_VARARGS, isConfirmed__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(keyboard__doc__,
		"Keyboard class.\n"
		"\n"
		"Keyboard([default, heading, hidden]) -- Creates a new Keyboard object with default text\n"
		"                                        heading and optional drop down hidden input flag.\n");

	PyTypeObject Keyboard_Type;

	void initKeyboard_Type()
	{
		PyXBMCInitializeTypeObject(&Keyboard_Type);

		Keyboard_Type.tp_name = (char*)"xbmc.Keyboard";
		Keyboard_Type.tp_basicsize = sizeof(Keyboard);
		Keyboard_Type.tp_dealloc = (destructor)Keyboard_Dealloc;
		Keyboard_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		Keyboard_Type.tp_doc = keyboard__doc__;
		Keyboard_Type.tp_methods = Keyboard_methods;
		Keyboard_Type.tp_base = 0;
		Keyboard_Type.tp_new = Keyboard_New;
	}
}

#ifdef __cplusplus
}
#endif
