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

#include "window.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	// Auto-incrementing window id for python windows
	static int iWindowCount = 13000;

	PyObject* Window_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		Window *self;
		int iExistingWindowId = -1;

		if (!PyArg_ParseTuple(args, (char*)"|i", &iExistingWindowId))
			return NULL;

		self = (Window*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->vecControls) vector<Control*>();

		self->bModal = false;
		self->bIsPythonWindow = true;
		self->iCurrentControlId = 3000;
		self->iOldWindowId = 0;

		if (iExistingWindowId == -1)
		{
			self->iWindowId = iWindowCount++;
		}
		else
		{
			self->iWindowId = iExistingWindowId;
			self->bIsPythonWindow = false;
		}

		// TODO: Create actual CGUIWindow when GUI subsystem available

		return (PyObject*)self;
	}

	void Window_Dealloc(Window* self)
	{
		// Release references to controls
		for (size_t i = 0; i < self->vecControls.size(); i++)
			Py_XDECREF(self->vecControls[i]);
		self->vecControls.~vector();

		// TODO: Destroy CGUIWindow when GUI subsystem available

		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	// show()
	PyDoc_STRVAR(show__doc__,
		"show() -- Show this window.\n"
		"\n"
		"Shows this window by activating it, calling close() after it wil activate the\n"
		"current window again.\n");
	PyObject* Window_Show(Window *self, PyObject *args)
	{
		// TODO: Activate window when WindowManager available
		Py_RETURN_NONE;
	}

	// close()
	PyDoc_STRVAR(close__doc__,
		"close() -- Closes this window.\n"
		"\n"
		"Closes this window by activating the old window.\n");
	PyObject* Window_Close(Window *self, PyObject *args)
	{
		// TODO: Close/deactivate window when WindowManager available
		self->bModal = false;
		Py_RETURN_NONE;
	}

	// doModal()
	PyDoc_STRVAR(doModal__doc__,
		"doModal() -- Display this window until close() is called.\n");
	PyObject* Window_DoModal(Window *self, PyObject *args)
	{
		// TODO: Implement modal loop when GUI subsystem available
		self->bModal = true;

		Py_BEGIN_ALLOW_THREADS
		// Would block here until close() is called
		// For now, just return immediately
		Py_END_ALLOW_THREADS

		self->bModal = false;
		Py_RETURN_NONE;
	}

	// addControl()
	PyDoc_STRVAR(addControl__doc__,
		"addControl(Control) -- Add a Control to this window.\n");
	PyObject* Window_AddControl(Window *self, PyObject *args)
	{
		Control* pControl;
		if (!PyArg_ParseTuple(args, (char*)"O!", &Control_Type, &pControl))
			return NULL;

		pControl->iControlId = self->iCurrentControlId++;
		pControl->iParentId = self->iWindowId;

		Py_INCREF(pControl);
		self->vecControls.push_back(pControl);

		// TODO: Create and add GUI control when available

		Py_RETURN_NONE;
	}

	// removeControl()
	PyDoc_STRVAR(removeControl__doc__,
		"removeControl(Control) -- Removes the control from this window.\n");
	PyObject* Window_RemoveControl(Window *self, PyObject *args)
	{
		Control* pControl;
		if (!PyArg_ParseTuple(args, (char*)"O!", &Control_Type, &pControl))
			return NULL;

		for (auto it = self->vecControls.begin(); it != self->vecControls.end(); ++it)
		{
			if (*it == pControl)
			{
				self->vecControls.erase(it);
				Py_DECREF(pControl);
				// TODO: Remove from GUI window when available
				Py_RETURN_NONE;
			}
		}

		PyErr_SetString(PyExc_RuntimeError, "Control does not exist in window");
		return NULL;
	}

	// getControl()
	PyDoc_STRVAR(getControl__doc__,
		"getControl(controlId) -- Gets the control from this window.\n");
	PyObject* Window_GetControl(Window *self, PyObject *args)
	{
		int iControlId;
		if (!PyArg_ParseTuple(args, (char*)"i", &iControlId))
			return NULL;

		for (size_t i = 0; i < self->vecControls.size(); i++)
		{
			if (self->vecControls[i]->iControlId == iControlId)
			{
				Py_INCREF(self->vecControls[i]);
				return (PyObject*)self->vecControls[i];
			}
		}

		// TODO: Look up control from CGUIWindow when available
		PyErr_SetString(PyExc_RuntimeError, "Control not found");
		return NULL;
	}

	// setFocus()
	PyDoc_STRVAR(setFocus__doc__,
		"setFocus(Control) -- Give the supplied control focus.\n");
	PyObject* Window_SetFocus(Window *self, PyObject *args)
	{
		Control* pControl;
		if (!PyArg_ParseTuple(args, (char*)"O!", &Control_Type, &pControl))
			return NULL;
		// TODO: Set focus on GUI control when available
		Py_RETURN_NONE;
	}

	// getFocus()
	PyDoc_STRVAR(getFocus__doc__,
		"getFocus() -- Returns the control which is focused.\n");
	PyObject* Window_GetFocus(Window *self, PyObject *args)
	{
		// TODO: Get focused control from GUI window when available
		PyErr_SetString(PyExc_RuntimeError, "No control has focus");
		return NULL;
	}

	// setFocusId()
	PyDoc_STRVAR(setFocusId__doc__,
		"setFocusId(ControlId) -- Sets focus to a control based on it's id.\n");
	PyObject* Window_SetFocusId(Window *self, PyObject *args)
	{
		int iControlId;
		if (!PyArg_ParseTuple(args, (char*)"i", &iControlId))
			return NULL;
		// TODO: Set focus on GUI when available
		Py_RETURN_NONE;
	}

	// getFocusId()
	PyDoc_STRVAR(getFocusId__doc__,
		"getFocusId() -- Returns the id of the control which is focused.\n");
	PyObject* Window_GetFocusId(Window *self, PyObject *args)
	{
		// TODO: Get from GUI window when available
		return Py_BuildValue((char*)"i", -1);
	}

	// setProperty()
	PyDoc_STRVAR(setProperty_win__doc__,
		"setProperty(key, value) -- Sets a window property, similar to an infolabel.\n");
	PyObject* Window_SetProperty(Window *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "key", "value", NULL };
		char *cKey = NULL;
		PyObject *pValue = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"sO",
			(char**)keywords, &cKey, &pValue))
			return NULL;

		// TODO: Set property on GUI window when available
		Py_RETURN_NONE;
	}

	// getProperty()
	PyDoc_STRVAR(getProperty_win__doc__,
		"getProperty(key) -- Returns a window property as a string, similar to an infolabel.\n");
	PyObject* Window_GetProperty(Window *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "key", NULL };
		char *cKey = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"s",
			(char**)keywords, &cKey))
			return NULL;

		// TODO: Get from GUI window when available
		return Py_BuildValue((char*)"s", "");
	}

	// clearProperties()
	PyDoc_STRVAR(clearProperties__doc__,
		"clearProperties() -- Clears all window properties.\n");
	PyObject* Window_ClearProperties(Window *self, PyObject *args)
	{
		// TODO: Clear properties on GUI window when available
		Py_RETURN_NONE;
	}

	// onAction callback placeholder
	PyDoc_STRVAR(onAction__doc__,
		"onAction(self, Action) -- onAction method.\n"
		"\n"
		"This method will receive all actions that the main program will send\n"
		"to this window.\n");
	PyObject* Window_OnAction(Window *self, PyObject *args)
	{
		Py_RETURN_NONE;
	}

	// onClick callback placeholder
	PyDoc_STRVAR(onClick__doc__,
		"onClick(self, controlId) -- onClick method.\n");
	PyObject* Window_OnClick(Window *self, PyObject *args)
	{
		Py_RETURN_NONE;
	}

	// onFocus callback placeholder
	PyDoc_STRVAR(onFocus__doc__,
		"onFocus(self, controlId) -- onFocus method.\n");
	PyObject* Window_OnFocus(Window *self, PyObject *args)
	{
		Py_RETURN_NONE;
	}

	// onInit callback placeholder
	PyDoc_STRVAR(onInit__doc__,
		"onInit(self) -- onInit method.\n");
	PyObject* Window_OnInit(Window *self, PyObject *args)
	{
		Py_RETURN_NONE;
	}

	PyMethodDef Window_methods[] = {
		{(char*)"show", (PyCFunction)Window_Show, METH_VARARGS, show__doc__},
		{(char*)"close", (PyCFunction)Window_Close, METH_VARARGS, close__doc__},
		{(char*)"doModal", (PyCFunction)Window_DoModal, METH_VARARGS, doModal__doc__},
		{(char*)"addControl", (PyCFunction)Window_AddControl, METH_VARARGS, addControl__doc__},
		{(char*)"removeControl", (PyCFunction)Window_RemoveControl, METH_VARARGS, removeControl__doc__},
		{(char*)"getControl", (PyCFunction)Window_GetControl, METH_VARARGS, getControl__doc__},
		{(char*)"setFocus", (PyCFunction)Window_SetFocus, METH_VARARGS, setFocus__doc__},
		{(char*)"getFocus", (PyCFunction)Window_GetFocus, METH_VARARGS, getFocus__doc__},
		{(char*)"setFocusId", (PyCFunction)Window_SetFocusId, METH_VARARGS, setFocusId__doc__},
		{(char*)"getFocusId", (PyCFunction)Window_GetFocusId, METH_VARARGS, getFocusId__doc__},
		{(char*)"setProperty", (PyCFunction)Window_SetProperty, METH_VARARGS|METH_KEYWORDS, setProperty_win__doc__},
		{(char*)"getProperty", (PyCFunction)Window_GetProperty, METH_VARARGS|METH_KEYWORDS, getProperty_win__doc__},
		{(char*)"clearProperties", (PyCFunction)Window_ClearProperties, METH_VARARGS, clearProperties__doc__},
		{(char*)"onAction", (PyCFunction)Window_OnAction, METH_VARARGS, onAction__doc__},
		{(char*)"onClick", (PyCFunction)Window_OnClick, METH_VARARGS, onClick__doc__},
		{(char*)"onFocus", (PyCFunction)Window_OnFocus, METH_VARARGS, onFocus__doc__},
		{(char*)"onInit", (PyCFunction)Window_OnInit, METH_VARARGS, onInit__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(window__doc__,
		"Window class.\n"
		"\n"
		"Window([existingWindowId]) -- Create a new Window to draw on.\n"
		"                              Specify an id to use an existing window.\n");

	PyTypeObject Window_Type;

	void initWindow_Type()
	{
		PyXBMCInitializeTypeObject(&Window_Type);

		Window_Type.tp_name = (char*)"xbmcgui.Window";
		Window_Type.tp_basicsize = sizeof(Window);
		Window_Type.tp_dealloc = (destructor)Window_Dealloc;
		Window_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		Window_Type.tp_doc = window__doc__;
		Window_Type.tp_methods = Window_methods;
		Window_Type.tp_base = 0;
		Window_Type.tp_new = Window_New;
	}

	// WindowDialog - just a Window subclass
	PyDoc_STRVAR(windowDialog__doc__,
		"WindowDialog class.\n"
		"\n"
		"WindowDialog() -- Create a new WindowDialog.\n"
		"                  A WindowDialog is rendered on top of the current window.\n");

	PyTypeObject WindowDialog_Type;

	void initWindowDialog_Type()
	{
		PyXBMCInitializeTypeObject(&WindowDialog_Type);

		WindowDialog_Type.tp_name = (char*)"xbmcgui.WindowDialog";
		WindowDialog_Type.tp_basicsize = sizeof(Window);
		WindowDialog_Type.tp_dealloc = (destructor)Window_Dealloc;
		WindowDialog_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		WindowDialog_Type.tp_doc = windowDialog__doc__;
		WindowDialog_Type.tp_methods = Window_methods;
		WindowDialog_Type.tp_base = &Window_Type;
		WindowDialog_Type.tp_new = Window_New;
	}
}

#ifdef __cplusplus
}
#endif
