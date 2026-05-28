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

// xbmcplugin Python module for XBMC-360 (stub)
// TODO: Port plugin content directory functions from xbmc4xbox

#include "Python.h"
#include "utils/log.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{

// =========================================================================
// xbmcplugin.addDirectoryItem(handle, url, listitem, isFolder, totalItems)
// TODO: stub
// =========================================================================
static PyObject* XBMCPLUGIN_AddDirectoryItem(PyObject* self, PyObject* args, PyObject* kwds)
{
	// TODO: implement plugin content listing
	CLog::Log(LOGDEBUG, "Python: xbmcplugin.addDirectoryItem() - stub");
	Py_RETURN_TRUE;
}

// =========================================================================
// xbmcplugin.endOfDirectory(handle, succeeded, updateListing, cacheToDisc)
// TODO: stub
// =========================================================================
static PyObject* XBMCPLUGIN_EndOfDirectory(PyObject* self, PyObject* args, PyObject* kwds)
{
	// TODO: implement
	CLog::Log(LOGDEBUG, "Python: xbmcplugin.endOfDirectory() - stub");
	Py_RETURN_NONE;
}

// =========================================================================
// xbmcplugin.setResolvedUrl(handle, succeeded, listitem) - TODO: stub
// =========================================================================
static PyObject* XBMCPLUGIN_SetResolvedUrl(PyObject* self, PyObject* args)
{
	// TODO: implement
	CLog::Log(LOGDEBUG, "Python: xbmcplugin.setResolvedUrl() - stub");
	Py_RETURN_NONE;
}

// =========================================================================
// xbmcplugin.addSortMethod(handle, sortMethod) - TODO: stub
// =========================================================================
static PyObject* XBMCPLUGIN_AddSortMethod(PyObject* self, PyObject* args)
{
	// TODO: implement
	Py_RETURN_NONE;
}

// =========================================================================
// xbmcplugin.setContent(handle, content) - TODO: stub
// =========================================================================
static PyObject* XBMCPLUGIN_SetContent(PyObject* self, PyObject* args)
{
	// TODO: implement
	Py_RETURN_NONE;
}

// =========================================================================
// xbmcplugin.setPluginCategory(handle, category) - TODO: stub
// =========================================================================
static PyObject* XBMCPLUGIN_SetPluginCategory(PyObject* self, PyObject* args)
{
	// TODO: implement
	Py_RETURN_NONE;
}

// =========================================================================
// Method table
// =========================================================================
static PyMethodDef xbmcpluginMethods[] = {
	{"addDirectoryItem",  (PyCFunction)XBMCPLUGIN_AddDirectoryItem, METH_VARARGS|METH_KEYWORDS, "addDirectoryItem(handle, url, listitem, isFolder, totalItems) - Add directory item."},
	{"endOfDirectory",    (PyCFunction)XBMCPLUGIN_EndOfDirectory,   METH_VARARGS|METH_KEYWORDS, "endOfDirectory(handle, succeeded, updateListing, cacheToDisc) - Signal end of directory."},
	{"setResolvedUrl",    (PyCFunction)XBMCPLUGIN_SetResolvedUrl,   METH_VARARGS, "setResolvedUrl(handle, succeeded, listitem) - Set resolved URL."},
	{"addSortMethod",     (PyCFunction)XBMCPLUGIN_AddSortMethod,    METH_VARARGS, "addSortMethod(handle, sortMethod) - Add sort method."},
	{"setContent",        (PyCFunction)XBMCPLUGIN_SetContent,       METH_VARARGS, "setContent(handle, content) - Set content type."},
	{"setPluginCategory", (PyCFunction)XBMCPLUGIN_SetPluginCategory,METH_VARARGS, "setPluginCategory(handle, category) - Set plugin category."},
	{NULL, NULL, 0, NULL}
};

// =========================================================================
// Module definition (Python 3)
// =========================================================================
static struct PyModuleDef xbmcpluginModuleDef = {
	PyModuleDef_HEAD_INIT,
	"xbmcplugin",
	"XBMC plugin scripting module for Xbox 360 (stub)",
	-1,
	xbmcpluginMethods,
	NULL, NULL, NULL, NULL
};

} // namespace PYXBMC

// =========================================================================
// Module init
// =========================================================================
PyMODINIT_FUNC PyInit_xbmcplugin(void)
{
	using namespace PYXBMC;

	PyObject* pModule = PyModule_Create(&xbmcpluginModuleDef);
	if (!pModule)
		return NULL;

	// TODO: Add sort method constants
	// SORT_METHOD_NONE, SORT_METHOD_LABEL, SORT_METHOD_DATE, etc.

	CLog::Log(LOGDEBUG, "Python: xbmcplugin module loaded (stub)");
	return pModule;
}

#ifdef __cplusplus
}
#endif
