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

#include "action.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	PyObject* Action_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		Action *self;

		self = (Action*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->strAction) string();

		self->id = -1;
		self->fAmount1 = 0.0f;
		self->fAmount2 = 0.0f;
		self->fRepeat = 0.0f;
		self->buttonCode = 0;
		self->strAction = "";

		return (PyObject*)self;
	}

	void Action_Dealloc(Action* self)
	{
		self->strAction.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyObject* Action_RichCompare(Action* obj1, PyObject* obj2, int method)
	{
		if (method == Py_EQ)
		{
			if (Action_Check(obj2))
			{
				Action* a2 = (Action*)obj2;
				if (obj1->id == a2->id &&
						obj1->buttonCode == a2->buttonCode &&
						obj1->fAmount1 == a2->fAmount1 &&
						obj1->fAmount2 == a2->fAmount2 &&
						obj1->fRepeat == a2->fRepeat &&
						obj1->strAction == a2->strAction)
				{
					Py_RETURN_TRUE;
				}
				else
				{
					Py_RETURN_FALSE;
				}
			}
			else
			{
				// for backwards compatibility in python scripts
				PyObject* o1 = PyLong_FromLong(obj1->id);
				PyObject* result = PyObject_RichCompare(o1, obj2, method);
				Py_DECREF(o1);
				return result;
			}
		}
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}

	// getId() Method
	PyDoc_STRVAR(getId__doc__,
		"getId() -- Returns the action's current id as a long or 0 if no action is mapped in the xml's.\n");

	PyObject* Action_GetId(Action* self, PyObject* args)
	{
		return Py_BuildValue((char*)"l", self->id);
	}

	// getButtonCode() Method
	PyDoc_STRVAR(getButtonCode__doc__,
		"getButtonCode() -- Returns the button code for this action.\n");

	PyObject* Action_GetButtonCode(Action* self, PyObject* args)
	{
		return Py_BuildValue((char*)"l", self->buttonCode);
	}

	PyDoc_STRVAR(getAmount1__doc__,
		"getAmount1() -- Returns the first amount of force applied to the thumbstick.\n");

	PyDoc_STRVAR(getAmount2__doc__,
		"getAmount2() -- Returns the second amount of force applied to the thumbstick.\n");

	PyObject* Action_GetAmount1(Action* self, PyObject* args)
	{
		return Py_BuildValue((char*)"f", self->fAmount1);
	}

	PyObject* Action_GetAmount2(Action* self, PyObject* args)
	{
		return Py_BuildValue((char*)"f", self->fAmount2);
	}

	PyMethodDef Action_methods[] = {
		{(char*)"getId", (PyCFunction)Action_GetId, METH_VARARGS, getId__doc__},
		{(char*)"getButtonCode", (PyCFunction)Action_GetButtonCode, METH_VARARGS, getButtonCode__doc__},
		{(char*)"getAmount1", (PyCFunction)Action_GetAmount1, METH_VARARGS, getAmount1__doc__},
		{(char*)"getAmount2", (PyCFunction)Action_GetAmount2, METH_VARARGS, getAmount2__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(action__doc__,
		"Action class.\n"
		"\n"
		"For backwards compatibility reasons the == operator is extended so that it\n"
		"can compare an action with other actions and action.GetID() with numbers\n"
		"  example: (action == ACTION_MOVE_LEFT)\n");

	PyTypeObject Action_Type;

	void initAction_Type()
	{
		PyXBMCInitializeTypeObject(&Action_Type);

		Action_Type.tp_name = (char*)"xbmcgui.Action";
		Action_Type.tp_basicsize = sizeof(Action);
		Action_Type.tp_dealloc = (destructor)Action_Dealloc;
		Action_Type.tp_richcompare = (richcmpfunc)Action_RichCompare;
		Action_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		Action_Type.tp_doc = action__doc__;
		Action_Type.tp_methods = Action_methods;
		Action_Type.tp_base = 0;
		Action_Type.tp_new = Action_New;
	}
}

#ifdef __cplusplus
}
#endif
