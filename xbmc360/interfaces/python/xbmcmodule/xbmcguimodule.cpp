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

// xbmcgui Python module for XBMC-360 (stub)
// TODO: Port GUI window/control/dialog classes from xbmc4xbox

#include "Python.h"
#include "utils/log.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{

// =========================================================================
// xbmcgui.getCurrentWindowId() - TODO: stub
// =========================================================================
static PyObject* XBMCGUI_GetCurrentWindowId(PyObject* self, PyObject* args)
{
	// TODO: implement via g_windowManager.GetActiveWindow()
	return PyLong_FromLong(0);
}

// =========================================================================
// xbmcgui.getCurrentWindowDialogId() - TODO: stub
// =========================================================================
static PyObject* XBMCGUI_GetCurrentWindowDialogId(PyObject* self, PyObject* args)
{
	// TODO: implement
	return PyLong_FromLong(0);
}

// =========================================================================
// Method table
// =========================================================================
static PyMethodDef xbmcguiMethods[] = {
	{"getCurrentWindowId",       (PyCFunction)XBMCGUI_GetCurrentWindowId,       METH_VARARGS, "getCurrentWindowId() - Get current window ID."},
	{"getCurrentWindowDialogId", (PyCFunction)XBMCGUI_GetCurrentWindowDialogId, METH_VARARGS, "getCurrentWindowDialogId() - Get current dialog ID."},
	// TODO: Add Window, WindowDialog, WindowXML, WindowXMLDialog types
	// TODO: Add Control, ControlLabel, ControlButton, ControlList, etc.
	// TODO: Add Dialog, DialogProgress types
	// TODO: Add ListItem type
	{NULL, NULL, 0, NULL}
};

// =========================================================================
// Module definition (Python 3)
// =========================================================================
static struct PyModuleDef xbmcguiModuleDef = {
	PyModuleDef_HEAD_INIT,
	"xbmcgui",
	"XBMC GUI scripting module for Xbox 360 (stub)",
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

	PyObject* pModule = PyModule_Create(&xbmcguiModuleDef);
	if (!pModule)
		return NULL;

	// TODO: Add action ID constants
	// TODO: Add control alignment constants
	// TODO: Register Window, Dialog, Control types

	CLog::Log(LOGDEBUG, "Python: xbmcgui module loaded (stub)");
	return pModule;
}

#ifdef __cplusplus
}
#endif
