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

#include "Python.h"
#include "utils/log.h"
#include "pyutil.h"
#include "action.h"
#include "control.h"
#include "listitem.h"
#include "window.h"
#include "dialog.h"
#include "winxml.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{

// =========================================================================
// xbmcgui.getCurrentWindowId()
// =========================================================================
static PyObject* XBMCGUI_GetCurrentWindowId(PyObject* self, PyObject* args)
{
	// TODO: implement via g_windowManager.GetActiveWindow()
	return PyLong_FromLong(0);
}

// =========================================================================
// xbmcgui.getCurrentWindowDialogId()
// =========================================================================
static PyObject* XBMCGUI_GetCurrentWindowDialogId(PyObject* self, PyObject* args)
{
	// TODO: implement
	return PyLong_FromLong(0);
}

// =========================================================================
// xbmcgui.lock() / unlock()
// =========================================================================
static PyObject* XBMCGUI_Lock(PyObject* self, PyObject* args)
{
	PyXBMCGUILock();
	Py_RETURN_NONE;
}

static PyObject* XBMCGUI_Unlock(PyObject* self, PyObject* args)
{
	PyXBMCGUIUnlock();
	Py_RETURN_NONE;
}

// =========================================================================
// Method table
// =========================================================================
static PyMethodDef xbmcguiMethods[] = {
		{(char*)"lock", (PyCFunction)XBMCGUI_Lock, METH_VARARGS, (char*)"lock() - Lock the gui until xbmcgui.unlock() is called."},
		{(char*)"unlock", (PyCFunction)XBMCGUI_Unlock, METH_VARARGS, (char*)"unlock() - Unlock the gui from a lock() call."},
		{(char*)"getCurrentWindowId", (PyCFunction)XBMCGUI_GetCurrentWindowId, METH_VARARGS, (char*)"getCurrentWindowId() - Get current window ID."},
		{(char*)"getCurrentWindowDialogId", (PyCFunction)XBMCGUI_GetCurrentWindowDialogId, METH_VARARGS, (char*)"getCurrentWindowDialogId() - Get current dialog ID."},
		{NULL, NULL, 0, NULL}
};

// =========================================================================
// Module definition (Python 3)
// =========================================================================
static struct PyModuleDef xbmcguiModuleDef = {
	PyModuleDef_HEAD_INIT,
	"xbmcgui",
	"XBMC GUI scripting module for Xbox 360",
	-1,
	xbmcguiMethods,
	NULL, NULL, NULL, NULL
};

} // namespace PYXBMC

// =========================================================================
// Module init
// =========================================================================
PyMODINIT_FUNC PyInit_xbmcgui(void)
{
	using namespace PYXBMC;

	// Initialize all types
	initAction_Type();
	initControl_Type();
	initControlSpin_Type();
	initControlLabel_Type();
	initControlFadeLabel_Type();
	initControlTextBox_Type();
	initControlButton_Type();
	initControlCheckMark_Type();
	initControlImage_Type();
	initControlGroup_Type();
	initControlList_Type();
	initControlProgress_Type();
	initControlRadioButton_Type();
	initListItem_Type();
	initWindow_Type();
	initWindowDialog_Type();
	initWindowXML_Type();
	initWindowXMLDialog_Type();
	initDialog_Type();
	initDialogProgress_Type();

	// Ready all types
	if (PyType_Ready(&Action_Type) < 0) return NULL;
	if (PyType_Ready(&Control_Type) < 0) return NULL;
	if (PyType_Ready(&ControlSpin_Type) < 0) return NULL;
	if (PyType_Ready(&ControlLabel_Type) < 0) return NULL;
	if (PyType_Ready(&ControlFadeLabel_Type) < 0) return NULL;
	if (PyType_Ready(&ControlTextBox_Type) < 0) return NULL;
	if (PyType_Ready(&ControlButton_Type) < 0) return NULL;
	if (PyType_Ready(&ControlCheckMark_Type) < 0) return NULL;
	if (PyType_Ready(&ControlImage_Type) < 0) return NULL;
	if (PyType_Ready(&ControlGroup_Type) < 0) return NULL;
	if (PyType_Ready(&ControlList_Type) < 0) return NULL;
	if (PyType_Ready(&ControlProgress_Type) < 0) return NULL;
	if (PyType_Ready(&ControlRadioButton_Type) < 0) return NULL;
	if (PyType_Ready(&ListItem_Type) < 0) return NULL;
	if (PyType_Ready(&Window_Type) < 0) return NULL;
	if (PyType_Ready(&WindowDialog_Type) < 0) return NULL;
	if (PyType_Ready(&WindowXML_Type) < 0) return NULL;
	if (PyType_Ready(&WindowXMLDialog_Type) < 0) return NULL;
	if (PyType_Ready(&Dialog_Type) < 0) return NULL;
	if (PyType_Ready(&DialogProgress_Type) < 0) return NULL;

	PyObject* pModule = PyModule_Create(&xbmcguiModuleDef);
	if (!pModule)
		return NULL;

	// Register types on module
	Py_INCREF(&Action_Type);
	PyModule_AddObject(pModule, "Action", (PyObject*)&Action_Type);
	Py_INCREF(&Control_Type);
	PyModule_AddObject(pModule, "Control", (PyObject*)&Control_Type);
	Py_INCREF(&ControlSpin_Type);
	PyModule_AddObject(pModule, "ControlSpin", (PyObject*)&ControlSpin_Type);
	Py_INCREF(&ControlLabel_Type);
	PyModule_AddObject(pModule, "ControlLabel", (PyObject*)&ControlLabel_Type);
	Py_INCREF(&ControlFadeLabel_Type);
	PyModule_AddObject(pModule, "ControlFadeLabel", (PyObject*)&ControlFadeLabel_Type);
	Py_INCREF(&ControlTextBox_Type);
	PyModule_AddObject(pModule, "ControlTextBox", (PyObject*)&ControlTextBox_Type);
	Py_INCREF(&ControlButton_Type);
	PyModule_AddObject(pModule, "ControlButton", (PyObject*)&ControlButton_Type);
	Py_INCREF(&ControlCheckMark_Type);
	PyModule_AddObject(pModule, "ControlCheckMark", (PyObject*)&ControlCheckMark_Type);
	Py_INCREF(&ControlImage_Type);
	PyModule_AddObject(pModule, "ControlImage", (PyObject*)&ControlImage_Type);
	Py_INCREF(&ControlGroup_Type);
	PyModule_AddObject(pModule, "ControlGroup", (PyObject*)&ControlGroup_Type);
	Py_INCREF(&ControlList_Type);
	PyModule_AddObject(pModule, "ControlList", (PyObject*)&ControlList_Type);
	Py_INCREF(&ControlProgress_Type);
	PyModule_AddObject(pModule, "ControlProgress", (PyObject*)&ControlProgress_Type);
	Py_INCREF(&ControlRadioButton_Type);
	PyModule_AddObject(pModule, "ControlRadioButton", (PyObject*)&ControlRadioButton_Type);
	Py_INCREF(&ListItem_Type);
	PyModule_AddObject(pModule, "ListItem", (PyObject*)&ListItem_Type);
	Py_INCREF(&Window_Type);
	PyModule_AddObject(pModule, "Window", (PyObject*)&Window_Type);
	Py_INCREF(&WindowDialog_Type);
	PyModule_AddObject(pModule, "WindowDialog", (PyObject*)&WindowDialog_Type);
	Py_INCREF(&WindowXML_Type);
	PyModule_AddObject(pModule, "WindowXML", (PyObject*)&WindowXML_Type);
	Py_INCREF(&WindowXMLDialog_Type);
	PyModule_AddObject(pModule, "WindowXMLDialog", (PyObject*)&WindowXMLDialog_Type);
	Py_INCREF(&Dialog_Type);
	PyModule_AddObject(pModule, "Dialog", (PyObject*)&Dialog_Type);
	Py_INCREF(&DialogProgress_Type);
	PyModule_AddObject(pModule, "DialogProgress", (PyObject*)&DialogProgress_Type);

	// Action ID constants
	PyModule_AddIntConstant(pModule, "ACTION_NONE", 0);
	PyModule_AddIntConstant(pModule, "ACTION_MOVE_LEFT", 1);
	PyModule_AddIntConstant(pModule, "ACTION_MOVE_RIGHT", 2);
	PyModule_AddIntConstant(pModule, "ACTION_MOVE_UP", 3);
	PyModule_AddIntConstant(pModule, "ACTION_MOVE_DOWN", 4);
	PyModule_AddIntConstant(pModule, "ACTION_SELECT_ITEM", 7);
	PyModule_AddIntConstant(pModule, "ACTION_PREVIOUS_MENU", 10);
	PyModule_AddIntConstant(pModule, "ACTION_NAV_BACK", 92);
	PyModule_AddIntConstant(pModule, "ACTION_PARENT_DIR", 9);

	// Control alignment constants
	PyModule_AddIntConstant(pModule, "ALIGN_LEFT", 0);
	PyModule_AddIntConstant(pModule, "ALIGN_RIGHT", 1);
	PyModule_AddIntConstant(pModule, "ALIGN_CENTER_X", 2);
	PyModule_AddIntConstant(pModule, "ALIGN_CENTER_Y", 4);
	PyModule_AddIntConstant(pModule, "ALIGN_TRUNCATED", 8);

	CLog::Log(LOGDEBUG, "Python: xbmcgui module loaded");
	return pModule;
}

#ifdef __cplusplus
}
#endif
