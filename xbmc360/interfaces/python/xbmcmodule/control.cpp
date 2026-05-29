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
	// Base Control dealloc
	void Control_Dealloc(Control* self)
	{
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	// getId
	PyDoc_STRVAR(getId__doc__,
		"getId() -- Returns the control's current id as an integer.\n");
	PyObject* Control_GetId(Control *self, PyObject *args)
	{
		return Py_BuildValue((char*)"i", self->iControlId);
	}

	// getPosition
	PyDoc_STRVAR(getPosition__doc__,
		"getPosition() -- Returns the control's current position as a x,y integer tuple.\n");
	PyObject* Control_GetPosition(Control *self, PyObject *args)
	{
		return Py_BuildValue((char*)"(ii)", self->dwPosX, self->dwPosY);
	}

	// getWidth
	PyDoc_STRVAR(getWidth__doc__,
		"getWidth() -- Returns the control's current width as an integer.\n");
	PyObject* Control_GetWidth(Control *self, PyObject *args)
	{
		return Py_BuildValue((char*)"i", self->dwWidth);
	}

	// getHeight
	PyDoc_STRVAR(getHeight__doc__,
		"getHeight() -- Returns the control's current height as an integer.\n");
	PyObject* Control_GetHeight(Control *self, PyObject *args)
	{
		return Py_BuildValue((char*)"i", self->dwHeight);
	}

	// setEnabled
	PyDoc_STRVAR(setEnabled__doc__,
		"setEnabled(enabled) -- Set's the control's enabled/disabled state.\n"
		"\n"
		"enabled        : bool - True=enabled / False=disabled.\n");
	PyObject* Control_SetEnabled(Control *self, PyObject *args)
	{
		char enabled;
		if (!PyArg_ParseTuple(args, (char*)"b", &enabled)) return NULL;
		// TODO: Set enabled state on GUI control when available
		Py_RETURN_NONE;
	}

	// setVisible
	PyDoc_STRVAR(setVisible__doc__,
		"setVisible(visible) -- Set's the control's visible/hidden state.\n"
		"\n"
		"visible        : bool - True=visible / False=hidden.\n");
	PyObject* Control_SetVisible(Control *self, PyObject *args)
	{
		char visible;
		if (!PyArg_ParseTuple(args, (char*)"b", &visible)) return NULL;
		// TODO: Set visible state on GUI control when available
		Py_RETURN_NONE;
	}

	// setPosition
	PyDoc_STRVAR(setPosition__doc__,
		"setPosition(x, y) -- Set's the control's position.\n"
		"\n"
		"x              : integer - x coordinate of control.\n"
		"y              : integer - y coordinate of control.\n");
	PyObject* Control_SetPosition(Control *self, PyObject *args)
	{
		if (!PyArg_ParseTuple(args, (char*)"ii", &self->dwPosX, &self->dwPosY)) return NULL;
		// TODO: Update GUI control position when available
		Py_RETURN_NONE;
	}

	// setWidth
	PyDoc_STRVAR(setWidth__doc__,
		"setWidth(width) -- Set's the control's width.\n"
		"\n"
		"width          : integer - width of control.\n");
	PyObject* Control_SetWidth(Control *self, PyObject *args)
	{
		if (!PyArg_ParseTuple(args, (char*)"i", &self->dwWidth)) return NULL;
		// TODO: Update GUI control width when available
		Py_RETURN_NONE;
	}

	// setHeight
	PyDoc_STRVAR(setHeight__doc__,
		"setHeight(height) -- Set's the control's height.\n"
		"\n"
		"height         : integer - height of control.\n");
	PyObject* Control_SetHeight(Control *self, PyObject *args)
	{
		if (!PyArg_ParseTuple(args, (char*)"i", &self->dwHeight)) return NULL;
		// TODO: Update GUI control height when available
		Py_RETURN_NONE;
	}

	// setNavigation
	PyDoc_STRVAR(setNavigation__doc__,
		"setNavigation(up, down, left, right) -- Set's the control's navigation.\n"
		"\n"
		"up             : control object - control to navigate to on up.\n"
		"down           : control object - control to navigate to on down.\n"
		"left           : control object - control to navigate to on left.\n"
		"right          : control object - control to navigate to on right.\n");
	PyObject* Control_SetNavigation(Control *self, PyObject *args)
	{
		Control *pUp, *pDown, *pLeft, *pRight;
		if (!PyArg_ParseTuple(args, (char*)"O!O!O!O!",
			&Control_Type, &pUp,
			&Control_Type, &pDown,
			&Control_Type, &pLeft,
			&Control_Type, &pRight)) return NULL;

		self->iControlUp = pUp->iControlId;
		self->iControlDown = pDown->iControlId;
		self->iControlLeft = pLeft->iControlId;
		self->iControlRight = pRight->iControlId;

		Py_RETURN_NONE;
	}

	// controlUp
	PyDoc_STRVAR(controlUp__doc__,
		"controlUp(control) -- Set's the control that is navigated to on up.\n");
	PyObject* Control_ControlUp(Control *self, PyObject *args)
	{
		Control *pControl;
		if (!PyArg_ParseTuple(args, (char*)"O!", &Control_Type, &pControl)) return NULL;
		self->iControlUp = pControl->iControlId;
		Py_RETURN_NONE;
	}

	// controlDown
	PyDoc_STRVAR(controlDown__doc__,
		"controlDown(control) -- Set's the control that is navigated to on down.\n");
	PyObject* Control_ControlDown(Control *self, PyObject *args)
	{
		Control *pControl;
		if (!PyArg_ParseTuple(args, (char*)"O!", &Control_Type, &pControl)) return NULL;
		self->iControlDown = pControl->iControlId;
		Py_RETURN_NONE;
	}

	// controlLeft
	PyDoc_STRVAR(controlLeft__doc__,
		"controlLeft(control) -- Set's the control that is navigated to on left.\n");
	PyObject* Control_ControlLeft(Control *self, PyObject *args)
	{
		Control *pControl;
		if (!PyArg_ParseTuple(args, (char*)"O!", &Control_Type, &pControl)) return NULL;
		self->iControlLeft = pControl->iControlId;
		Py_RETURN_NONE;
	}

	// controlRight
	PyDoc_STRVAR(controlRight__doc__,
		"controlRight(control) -- Set's the control that is navigated to on right.\n");
	PyObject* Control_ControlRight(Control *self, PyObject *args)
	{
		Control *pControl;
		if (!PyArg_ParseTuple(args, (char*)"O!", &Control_Type, &pControl)) return NULL;
		self->iControlRight = pControl->iControlId;
		Py_RETURN_NONE;
	}

	PyMethodDef Control_methods[] = {
		{(char*)"getId", (PyCFunction)Control_GetId, METH_VARARGS, getId__doc__},
		{(char*)"getPosition", (PyCFunction)Control_GetPosition, METH_VARARGS, getPosition__doc__},
		{(char*)"getWidth", (PyCFunction)Control_GetWidth, METH_VARARGS, getWidth__doc__},
		{(char*)"getHeight", (PyCFunction)Control_GetHeight, METH_VARARGS, getHeight__doc__},
		{(char*)"setEnabled", (PyCFunction)Control_SetEnabled, METH_VARARGS, setEnabled__doc__},
		{(char*)"setVisible", (PyCFunction)Control_SetVisible, METH_VARARGS, setVisible__doc__},
		{(char*)"setPosition", (PyCFunction)Control_SetPosition, METH_VARARGS, setPosition__doc__},
		{(char*)"setWidth", (PyCFunction)Control_SetWidth, METH_VARARGS, setWidth__doc__},
		{(char*)"setHeight", (PyCFunction)Control_SetHeight, METH_VARARGS, setHeight__doc__},
		{(char*)"setNavigation", (PyCFunction)Control_SetNavigation, METH_VARARGS, setNavigation__doc__},
		{(char*)"controlUp", (PyCFunction)Control_ControlUp, METH_VARARGS, controlUp__doc__},
		{(char*)"controlDown", (PyCFunction)Control_ControlDown, METH_VARARGS, controlDown__doc__},
		{(char*)"controlLeft", (PyCFunction)Control_ControlLeft, METH_VARARGS, controlLeft__doc__},
		{(char*)"controlRight", (PyCFunction)Control_ControlRight, METH_VARARGS, controlRight__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(control__doc__,
		"Control class.\n"
		"\n"
		"Base class for all controls.\n");

	PyTypeObject Control_Type;

	void initControl_Type()
	{
		PyXBMCInitializeTypeObject(&Control_Type);

		Control_Type.tp_name = (char*)"xbmcgui.Control";
		Control_Type.tp_basicsize = sizeof(Control);
		Control_Type.tp_dealloc = (destructor)Control_Dealloc;
		Control_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		Control_Type.tp_doc = control__doc__;
		Control_Type.tp_methods = Control_methods;
		Control_Type.tp_base = 0;
		Control_Type.tp_new = PyType_GenericNew;
	}
}

#ifdef __cplusplus
}
#endif
