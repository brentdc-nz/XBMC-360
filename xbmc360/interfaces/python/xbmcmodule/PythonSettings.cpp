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

#include "PythonSettings.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	PyObject* Settings_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		Settings *self;

		self = (Settings*)type->tp_alloc(type, 0);
		if (!self) return NULL;

		new(&self->strScriptPath) string();
		new(&self->settings) map<string, string>();

		static const char *keywords[] = { "path", NULL };
		char *cScriptPath = NULL;

		if (!PyArg_ParseTupleAndKeywords(
			args,
			kwds,
			(char*)"s",
			(char**)keywords,
			&cScriptPath
			))
		{
			Py_DECREF(self);
			return NULL;
		}

		self->strScriptPath = cScriptPath ? cScriptPath : "";

		// TODO: Load settings from scriptPath/resources/settings.xml
		// Requires CScriptSettings

		return (PyObject*)self;
	}

	void Settings_Dealloc(Settings* self)
	{
		self->strScriptPath.~string();
		self->settings.~map();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	// getSetting(id) method
	PyObject* Settings_GetSetting(Settings *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "id", NULL };
		char *id = NULL;

		if (!PyArg_ParseTupleAndKeywords(
			args,
			kwds,
			(char*)"s",
			(char**)keywords,
			&id
			))
		{
			return NULL;
		}

		// TODO: implement via CScriptSettings::Get()
		map<string, string>::iterator it = self->settings.find(id);
		if (it != self->settings.end())
			return PyUnicode_FromString(it->second.c_str());

		return PyUnicode_FromString("");
	}

	// setSetting(id, value) method
	PyObject* Settings_SetSetting(Settings *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "id", "value", NULL };
		char *id = NULL;
		char *value = NULL;

		if (!PyArg_ParseTupleAndKeywords(
			args,
			kwds,
			(char*)"ss",
			(char**)keywords,
			&id,
			&value
			))
		{
			return NULL;
		}

		if (!id || !value)
		{
			PyErr_SetString(PyExc_ValueError, "Invalid id or value!");
			return NULL;
		}

		// TODO: implement via CScriptSettings::Set() and Save()
		self->settings[id] = value;

		Py_RETURN_NONE;
	}

	// openSettings() method
	PyObject* Settings_OpenSettings(Settings *self, PyObject *args, PyObject *kwds)
	{
		// TODO: implement via CGUIDialogPluginSettings::ShowAndGetInput()
		// For now return False (user didn't change settings)
		Py_RETURN_FALSE;
	}

	PyMethodDef Settings_methods[] = {
		{(char*)"getSetting", (PyCFunction)Settings_GetSetting, METH_VARARGS|METH_KEYWORDS, (char*)"getSetting(id) -- Returns the value of a setting as a string."},
		{(char*)"setSetting", (PyCFunction)Settings_SetSetting, METH_VARARGS|METH_KEYWORDS, (char*)"setSetting(id, value) -- Sets a script setting."},
		{(char*)"openSettings", (PyCFunction)Settings_OpenSettings, METH_VARARGS|METH_KEYWORDS, (char*)"openSettings() -- Opens this scripts settings dialog."},
		{NULL, NULL, 0, NULL}
	};

	PyTypeObject Settings_Type;

	void initSettings_Type()
	{
		PyXBMCInitializeTypeObject(&Settings_Type);

		Settings_Type.tp_name = (char*)"xbmc.Settings";
		Settings_Type.tp_basicsize = sizeof(Settings);
		Settings_Type.tp_dealloc = (destructor)Settings_Dealloc;
		Settings_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		Settings_Type.tp_doc = (char*)"Settings(path) -- Creates a new Settings class.";
		Settings_Type.tp_methods = Settings_methods;
		Settings_Type.tp_base = 0;
		Settings_Type.tp_new = Settings_New;
	}
}

#ifdef __cplusplus
}
#endif
