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
#include "Python.h"
#include "pyutil.h"
#include "GUIPythonWindowXMLDialog.h"
#include "utils/log.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace PYXBMC
{
	PyObject* WindowXMLDialog_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		WindowXMLDialog *self;

		self = (WindowXMLDialog*)type->tp_alloc(type, 0);
		if (!self) return NULL;

		new(&self->sXMLFileName) string();
		new(&self->sFallBackPath) string();
		new(&self->vecControls) vector<Control*>();

		self->iWindowId = -1;
		self->bModal = true;
		self->pWindow = NULL;

		PyObject* pyOXMLname = NULL;
		PyObject* pyOname = NULL;
		PyObject* pyDName = NULL;
		PyObject* pyRes = NULL;

		string strXMLname, strFallbackPath;
		string strDefault = "Default";
		string resolution = "720p";

		if (!PyArg_ParseTuple(args, (char*)"OO|OO", &pyOXMLname, &pyOname, &pyDName, &pyRes))
			return NULL;

		PyXBMCGetUnicodeString(strXMLname, pyOXMLname);
		PyXBMCGetUnicodeString(strFallbackPath, pyOname);
		if (pyDName) PyXBMCGetUnicodeString(strDefault, pyDName);
		if (pyRes) PyXBMCGetUnicodeString(resolution, pyRes);

		// TODO: Resolve skin XML path via g_SkinInfo
		// TODO: Check XFILE::CFile::Exists for the XML
		// TODO: Fall back to script's resources/skins/<default>/<res> folder
		self->sXMLFileName = strXMLname;
		self->sFallBackPath = strFallbackPath;
		self->bUsingXML = true;

		// TODO: Create CGUIPythonWindowXMLDialog window instance
		// TODO: Window_CreateNewWindow((Window*)self, true);
		CLog::Log(LOGDEBUG, "Python: WindowXMLDialog created (stub) - XML: %s", strXMLname.c_str());

		return (PyObject*)self;
	}

	PyMethodDef windowXMLDialog_methods[] = {
		{NULL, NULL, 0, NULL}
	};

	PyTypeObject WindowXMLDialog_Type;

	void initWindowXMLDialog_Type()
	{
		PyXBMCInitializeTypeObject(&WindowXMLDialog_Type);

		WindowXMLDialog_Type.tp_name = (char*)"xbmcgui.WindowXMLDialog";
		WindowXMLDialog_Type.tp_basicsize = sizeof(WindowXMLDialog);
		WindowXMLDialog_Type.tp_dealloc = (destructor)Window_Dealloc;
		WindowXMLDialog_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		WindowXMLDialog_Type.tp_doc = (char*)"WindowXMLDialog(xmlFilename, scriptPath[, defaultSkin, defaultRes]) -- Create a new WindowXMLDialog.";
		WindowXMLDialog_Type.tp_methods = windowXMLDialog_methods;
		WindowXMLDialog_Type.tp_base = &WindowXML_Type;
		WindowXMLDialog_Type.tp_new = WindowXMLDialog_New;
	}
}

#ifdef __cplusplus
}
#endif
