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

#include "winxml.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	PyObject* WindowXML_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "xmlFilename", "scriptPath",
			"defaultSkin", "defaultRes", NULL };
		char *cXMLFilename = NULL;
		char *cScriptPath = NULL;
		char *cDefaultSkin = NULL;
		char *cDefaultRes = NULL;

		if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"ss|ss",
			(char**)keywords, &cXMLFilename, &cScriptPath, &cDefaultSkin, &cDefaultRes))
			return NULL;

		// Use Window_New to allocate the base Window structure
		PyObject* emptyArgs = PyTuple_New(0);
		PyObject* self = Window_Type.tp_new(type, emptyArgs, NULL);
		Py_DECREF(emptyArgs);

		if (!self) return NULL;

		// TODO: Load XML skin file when CGUIWindow XML loading is available
		// Store XML filename and script path for later use

		return self;
	}

	// addItem for WindowXML
	PyDoc_STRVAR(addItem_xml__doc__,
		"addItem(item[, position]) -- Add a new item to this Window List.\n");
	PyObject* WindowXML_AddItem(Window *self, PyObject *args)
	{
		// TODO: Implement when CGUIWindowXML list support available
		Py_RETURN_NONE;
	}

	// removeItem for WindowXML
	PyDoc_STRVAR(removeItem_xml__doc__,
		"removeItem(position) -- Removes a specified item based on position, from the Window List.\n");
	PyObject* WindowXML_RemoveItem(Window *self, PyObject *args)
	{
		// TODO: Implement when CGUIWindowXML available
		Py_RETURN_NONE;
	}

	// getCurrentListPosition
	PyDoc_STRVAR(getCurrentListPosition__doc__,
		"getCurrentListPosition() -- Gets the current position in the Window List.\n");
	PyObject* WindowXML_GetCurrentListPosition(Window *self)
	{
		// TODO: Implement when CGUIWindowXML available
		return Py_BuildValue((char*)"i", -1);
	}

	// setCurrentListPosition
	PyDoc_STRVAR(setCurrentListPosition__doc__,
		"setCurrentListPosition(position) -- Set the current position in the Window List.\n");
	PyObject* WindowXML_SetCurrentListPosition(Window *self, PyObject *args)
	{
		// TODO: Implement when CGUIWindowXML available
		Py_RETURN_NONE;
	}

	// getListItem
	PyDoc_STRVAR(getListItem_xml__doc__,
		"getListItem(position) -- Returns a given ListItem in this Window List.\n");
	PyObject* WindowXML_GetListItem(Window *self, PyObject *args)
	{
		// TODO: Implement when CGUIWindowXML available
		PyErr_SetString(PyExc_RuntimeError, "WindowXML list not implemented");
		return NULL;
	}

	// getListSize
	PyDoc_STRVAR(getListSize__doc__,
		"getListSize() -- Returns the number of items in this Window List.\n");
	PyObject* WindowXML_GetListSize(Window *self)
	{
		// TODO: Implement when CGUIWindowXML available
		return Py_BuildValue((char*)"i", 0);
	}

	// clearList
	PyDoc_STRVAR(clearList__doc__,
		"clearList() -- Clear the Window List.\n");
	PyObject* WindowXML_ClearList(Window *self)
	{
		// TODO: Implement when CGUIWindowXML available
		Py_RETURN_NONE;
	}

	PyMethodDef WindowXML_methods[] = {
		{(char*)"addItem", (PyCFunction)WindowXML_AddItem, METH_VARARGS, addItem_xml__doc__},
		{(char*)"removeItem", (PyCFunction)WindowXML_RemoveItem, METH_VARARGS, removeItem_xml__doc__},
		{(char*)"getCurrentListPosition", (PyCFunction)WindowXML_GetCurrentListPosition, METH_NOARGS, getCurrentListPosition__doc__},
		{(char*)"setCurrentListPosition", (PyCFunction)WindowXML_SetCurrentListPosition, METH_VARARGS, setCurrentListPosition__doc__},
		{(char*)"getListItem", (PyCFunction)WindowXML_GetListItem, METH_VARARGS, getListItem_xml__doc__},
		{(char*)"getListSize", (PyCFunction)WindowXML_GetListSize, METH_NOARGS, getListSize__doc__},
		{(char*)"clearList", (PyCFunction)WindowXML_ClearList, METH_NOARGS, clearList__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(windowXML__doc__,
		"WindowXML class.\n"
		"\n"
		"WindowXML(xmlFilename, scriptPath[, defaultSkin, defaultRes])\n"
		"  -- Create a new WindowXML script.\n");

	PyTypeObject WindowXML_Type;

	void initWindowXML_Type()
	{
		PyXBMCInitializeTypeObject(&WindowXML_Type);

		WindowXML_Type.tp_name = (char*)"xbmcgui.WindowXML";
		WindowXML_Type.tp_basicsize = sizeof(Window);
		WindowXML_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		WindowXML_Type.tp_doc = windowXML__doc__;
		WindowXML_Type.tp_methods = WindowXML_methods;
		WindowXML_Type.tp_base = &Window_Type;
		WindowXML_Type.tp_new = WindowXML_New;
	}

	// WindowXMLDialog_Type and initWindowXMLDialog_Type() are in winxmldialog.cpp
}

#ifdef __cplusplus
}
#endif
