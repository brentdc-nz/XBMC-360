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
	PyObject* ControlList_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		ControlList *self;
		static const char *keywords[] = {
			"x", "y", "width", "height",
			"font", "textColor", "buttonTexture", "buttonFocusTexture",
			"selectedColor", "imageWidth", "imageHeight", "itemTextXOffset",
			"itemTextYOffset", "itemHeight", "space", "alignmentY", NULL };

		char *cFont = NULL;
		char *cTextColor = NULL;
		char *cSelectedColor = NULL;
		char *cTextureButton = NULL;
		char *cTextureButtonFocus = NULL;

		self = (ControlList*)type->tp_alloc(type, 0);
		if (!self) return NULL;
		new(&self->vecItems) vector<ListItem*>();
		new(&self->strFont) string();
		new(&self->strTextureButton) string();
		new(&self->strTextureButtonFocus) string();

		self->pControlSpin = NULL;
		self->textColor = 0xe0f0f0f0;
		self->selectedColor = 0xffffffff;
		self->imageHeight = 10;
		self->imageWidth = 10;
		self->itemHeight = 27;
		self->space = 2;
		self->itemTextOffsetX = 0;
		self->itemTextOffsetY = 0;
		self->alignmentY = 4; // XBFONT_CENTER_Y

		if (!PyArg_ParseTupleAndKeywords(
			args, kwds, (char*)"iiii|sssssiiiiiiI", (char**)keywords,
			&self->dwPosX, &self->dwPosY, &self->dwWidth, &self->dwHeight,
			&cFont, &cTextColor, &cTextureButton, &cTextureButtonFocus,
			&cSelectedColor,
			&self->imageWidth, &self->imageHeight,
			&self->itemTextOffsetX, &self->itemTextOffsetY,
			&self->itemHeight, &self->space, &self->alignmentY))
		{
			Py_DECREF(self);
			return NULL;
		}

		if (cFont) self->strFont = cFont;
		else self->strFont = "font13";
		if (cTextColor) self->textColor = (color_t)strtoul(cTextColor, NULL, 16);
		if (cSelectedColor) self->selectedColor = (color_t)strtoul(cSelectedColor, NULL, 16);

		self->strTextureButton = cTextureButton ? cTextureButton :
			PyXBMCGetDefaultImage((char*)"listcontrol", (char*)"texturenofocus", (char*)"list-nofocus.png");
		self->strTextureButtonFocus = cTextureButtonFocus ? cTextureButtonFocus :
			PyXBMCGetDefaultImage((char*)"listcontrol", (char*)"texturefocus", (char*)"list-focus.png");

		// Create spin control (for scrolling)
		self->pControlSpin = (ControlSpin*)ControlSpin_Type.tp_alloc(&ControlSpin_Type, 0);
		if (self->pControlSpin)
		{
			new(&self->pControlSpin->strTextureUp) string();
			new(&self->pControlSpin->strTextureDown) string();
			new(&self->pControlSpin->strTextureUpFocus) string();
			new(&self->pControlSpin->strTextureDownFocus) string();
		}

		self->pGUIControl = NULL;
		return (PyObject*)self;
	}

	void ControlList_Dealloc(ControlList* self)
	{
		for (size_t i = 0; i < self->vecItems.size(); i++)
			Py_XDECREF(self->vecItems[i]);
		self->vecItems.~vector();
		self->strFont.~string();
		self->strTextureButton.~string();
		self->strTextureButtonFocus.~string();
		Py_XDECREF(self->pControlSpin);
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	PyDoc_STRVAR(addItem__doc__,
		"addItem(item) -- Add a new item to this list control.\n"
		"\n"
		"item               : string, unicode or ListItem - item to add.\n");
	PyObject* ControlList_AddItem(ControlList *self, PyObject *args)
	{
		PyObject *pObject;
		if (!PyArg_ParseTuple(args, (char*)"O", &pObject)) return NULL;

		ListItem* pListItem = NULL;

		if (ListItem_CheckExact(pObject))
		{
			pListItem = (ListItem*)pObject;
			Py_INCREF(pListItem);
		}
		else
		{
			string strText;
			if (!PyXBMCGetUnicodeString(strText, pObject, 1)) return NULL;
			pListItem = ListItem_FromString(strText);
			if (!pListItem) return NULL;
		}

		self->vecItems.push_back(pListItem);
		// TODO: Add to GUI control when available

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(addItems__doc__,
		"addItems(items) -- Adds a list of listitems or strings to this list control.\n");
	PyObject* ControlList_AddItems(ControlList *self, PyObject *args)
	{
		PyObject *pList = NULL;
		if (!PyArg_ParseTuple(args, (char*)"O", &pList)) return NULL;

		if (!PyList_Check(pList) && !PyTuple_Check(pList))
		{
			PyErr_SetString(PyExc_TypeError, "Object should be a list or tuple");
			return NULL;
		}

		Py_ssize_t iSize = PySequence_Size(pList);
		for (Py_ssize_t i = 0; i < iSize; i++)
		{
			PyObject* pItem = PySequence_GetItem(pList, i);
			if (!pItem) return NULL;

			ListItem* pListItem = NULL;
			if (ListItem_CheckExact(pItem))
			{
				pListItem = (ListItem*)pItem;
				Py_INCREF(pListItem);
			}
			else
			{
				string strText;
				if (!PyXBMCGetUnicodeString(strText, pItem, 1))
				{
					Py_DECREF(pItem);
					return NULL;
				}
				pListItem = ListItem_FromString(strText);
			}
			Py_DECREF(pItem);

			if (pListItem)
				self->vecItems.push_back(pListItem);
		}

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(selectItem__doc__,
		"selectItem(item) -- Select an item by index number.\n");
	PyObject* ControlList_SelectItem(ControlList *self, PyObject *args)
	{
		int iItem;
		if (!PyArg_ParseTuple(args, (char*)"i", &iItem)) return NULL;
		// TODO: Select item on GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(removeItem__doc__,
		"removeItem(index) -- Remove an item by index number.\n");
	PyObject* ControlList_RemoveItem(ControlList *self, PyObject *args)
	{
		int iIndex;
		if (!PyArg_ParseTuple(args, (char*)"i", &iIndex)) return NULL;

		if (iIndex < 0 || iIndex >= (int)self->vecItems.size())
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return NULL;
		}

		Py_XDECREF(self->vecItems[iIndex]);
		self->vecItems.erase(self->vecItems.begin() + iIndex);

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(reset_list__doc__,
		"reset() -- Clear all ListItems in this control list.\n");
	PyObject* ControlList_Reset(ControlList *self, PyObject *args)
	{
		for (size_t i = 0; i < self->vecItems.size(); i++)
			Py_XDECREF(self->vecItems[i]);
		self->vecItems.clear();
		// TODO: Clear GUI control when available
		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(getSpinControl__doc__,
		"getSpinControl() -- Returns the associated ControlSpin object.\n");
	PyObject* ControlList_GetSpinControl(ControlList *self)
	{
		Py_INCREF(self->pControlSpin);
		return (PyObject*)self->pControlSpin;
	}

	PyDoc_STRVAR(getSelectedPosition__doc__,
		"getSelectedPosition() -- Returns the position of the selected item as an integer.\n");
	PyObject* ControlList_GetSelectedPosition(ControlList *self)
	{
		// TODO: Get from GUI control when available
		return Py_BuildValue((char*)"i", -1);
	}

	PyDoc_STRVAR(getSelectedItem__doc__,
		"getSelectedItem() -- Returns the selected item as a ListItem object.\n");
	PyObject* ControlList_GetSelectedItem(ControlList *self)
	{
		// TODO: Get from GUI control when available
		if (self->vecItems.empty())
		{
			Py_RETURN_NONE;
		}
		Py_INCREF(self->vecItems[0]);
		return (PyObject*)self->vecItems[0];
	}

	PyDoc_STRVAR(getListItem__doc__,
		"getListItem(index) -- Returns a given ListItem in this List.\n");
	PyObject* ControlList_GetListItem(ControlList *self, PyObject *args)
	{
		int iIndex;
		if (!PyArg_ParseTuple(args, (char*)"i", &iIndex)) return NULL;

		if (iIndex < 0 || iIndex >= (int)self->vecItems.size())
		{
			PyErr_SetString(PyExc_IndexError, "index out of range");
			return NULL;
		}

		Py_INCREF(self->vecItems[iIndex]);
		return (PyObject*)self->vecItems[iIndex];
	}

	PyDoc_STRVAR(getItemHeight__doc__,
		"getItemHeight() -- Returns the control's current item height as an integer.\n");
	PyObject* ControlList_GetItemHeight(ControlList *self)
	{
		return Py_BuildValue((char*)"i", self->itemHeight);
	}

	PyDoc_STRVAR(getSpace__doc__,
		"getSpace() -- Returns the control's space between items as an integer.\n");
	PyObject* ControlList_GetSpace(ControlList *self)
	{
		return Py_BuildValue((char*)"i", self->space);
	}

	PyDoc_STRVAR(size_list__doc__,
		"size() -- Returns the total number of items in this list control as an integer.\n");
	PyObject* ControlList_Size(ControlList *self)
	{
		return Py_BuildValue((char*)"l", (long)self->vecItems.size());
	}

	PyMethodDef ControlList_methods[] = {
		{(char*)"addItem", (PyCFunction)ControlList_AddItem, METH_VARARGS, addItem__doc__},
		{(char*)"addItems", (PyCFunction)ControlList_AddItems, METH_VARARGS, addItems__doc__},
		{(char*)"selectItem", (PyCFunction)ControlList_SelectItem, METH_VARARGS, selectItem__doc__},
		{(char*)"removeItem", (PyCFunction)ControlList_RemoveItem, METH_VARARGS, removeItem__doc__},
		{(char*)"reset", (PyCFunction)ControlList_Reset, METH_VARARGS, reset_list__doc__},
		{(char*)"getSpinControl", (PyCFunction)ControlList_GetSpinControl, METH_NOARGS, getSpinControl__doc__},
		{(char*)"getSelectedPosition", (PyCFunction)ControlList_GetSelectedPosition, METH_NOARGS, getSelectedPosition__doc__},
		{(char*)"getSelectedItem", (PyCFunction)ControlList_GetSelectedItem, METH_NOARGS, getSelectedItem__doc__},
		{(char*)"getListItem", (PyCFunction)ControlList_GetListItem, METH_VARARGS, getListItem__doc__},
		{(char*)"getItemHeight", (PyCFunction)ControlList_GetItemHeight, METH_NOARGS, getItemHeight__doc__},
		{(char*)"getSpace", (PyCFunction)ControlList_GetSpace, METH_NOARGS, getSpace__doc__},
		{(char*)"size", (PyCFunction)ControlList_Size, METH_NOARGS, size_list__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(controlList__doc__,
		"ControlList class.\n"
		"\n"
		"ControlList(x, y, width, height[, font, textColor, buttonTexture, buttonFocusTexture,\n"
		"            selectedColor, imageWidth, imageHeight, itemTextXOffset, itemTextYOffset,\n"
		"            itemHeight, space, alignmentY])\n");

	PyTypeObject ControlList_Type;

	void initControlList_Type()
	{
		PyXBMCInitializeTypeObject(&ControlList_Type);

		ControlList_Type.tp_name = (char*)"xbmcgui.ControlList";
		ControlList_Type.tp_basicsize = sizeof(ControlList);
		ControlList_Type.tp_dealloc = (destructor)ControlList_Dealloc;
		ControlList_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		ControlList_Type.tp_doc = controlList__doc__;
		ControlList_Type.tp_methods = ControlList_methods;
		ControlList_Type.tp_base = &Control_Type;
		ControlList_Type.tp_new = ControlList_New;
	}
}

#ifdef __cplusplus
}
#endif
