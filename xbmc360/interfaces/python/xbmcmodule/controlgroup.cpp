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
	PyObject* ControlGroup_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlGroup *self;

		self = (ControlGroup*)type->tp_alloc(type, 0);
		if (!self) return NULL;

		if (!PyArg_ParseTuple(args, (char*)"iiii",
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight))
		{
			Py_DECREF(self);
			return NULL;
		}

		self->pGUIControl = NULL;
		return (PyObject*)self;
	}

	void ControlGroup_Dealloc(ControlGroup* self)
	{
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyMethodDef ControlGroup_methods[] = {
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlGroup__doc__,
		"ControlGroup class.\n"
		"\n"
		"ControlGroup(x, y, width, height)\n");

	PyTypeObject ControlGroup_Type;

	void initControlGroup_Type()
	{
		PyXBMCInitializeTypeObject(&ControlGroup_Type);

		ControlGroup_Type.tp_name = (char*)"xbmcgui.ControlGroup";
		ControlGroup_Type.tp_basicsize = sizeof(ControlGroup);
		ControlGroup_Type.tp_dealloc = (destructor)ControlGroup_Dealloc;
		ControlGroup_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlGroup_Type.tp_doc = controlGroup__doc__;
		ControlGroup_Type.tp_methods = ControlGroup_methods;
		ControlGroup_Type.tp_base = &Control_Type;
		ControlGroup_Type.tp_new = ControlGroup_New;
	}
}

#ifdef __cplusplus
}
#endif
