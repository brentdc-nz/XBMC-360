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

#include "language.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	PyObject* Language_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
	{
		Language *self;

		self = (Language*)type->tp_alloc(type, 0);
		if (!self) return NULL;

		new(&self->strScriptPath) string();
		new(&self->strDefaultLanguage) string();

		static const char *keywords[] = { "scriptPath", "defaultLanguage", NULL };
		char *cScriptPath = NULL;
		char *cDefaultLanguage = NULL;

		if (!PyArg_ParseTupleAndKeywords(
			args,
			kwds,
			(char*)"s|s",
			(char**)keywords,
			&cScriptPath,
			&cDefaultLanguage
			))
		{
			Py_DECREF(self);
			return NULL;
		}

		self->strScriptPath = cScriptPath ? cScriptPath : "";
		self->strDefaultLanguage = cDefaultLanguage ? cDefaultLanguage : "English";

		// TODO: Load language strings from scriptPath/resources/language/<lang>/strings.xml
		// Requires CLocalizeStrings and URIUtils

		return (PyObject*)self;
	}

	void Language_Dealloc(Language* self)
	{
		self->strScriptPath.~string();
		self->strDefaultLanguage.~string();
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	// getLocalizedString() method
	PyObject* Language_GetLocalizedString(Language *self, PyObject *args, PyObject *kwds)
	{
		static const char *keywords[] = { "id", NULL };
		int id = -1;

		if (!PyArg_ParseTupleAndKeywords(
			args,
			kwds,
			(char*)"i",
			(char**)keywords,
			&id
			))
		{
			return NULL;
		}

		// TODO: implement via CLocalizeStrings - return localized string for id
		// For now return empty string
		return PyUnicode_FromString("");
	}

	PyMethodDef Language_methods[] = {
		{(char*)"getLocalizedString", (PyCFunction)Language_GetLocalizedString, METH_VARARGS|METH_KEYWORDS, (char*)"getLocalizedString(id) -- Returns a localized 'unicode string'."},
		{NULL, NULL, 0, NULL}
	};

	PyTypeObject Language_Type;

	void initLanguage_Type()
	{
		PyXBMCInitializeTypeObject(&Language_Type);

		Language_Type.tp_name = (char*)"xbmc.Language";
		Language_Type.tp_basicsize = sizeof(Language);
		Language_Type.tp_dealloc = (destructor)Language_Dealloc;
		Language_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		Language_Type.tp_doc = (char*)"Language(scriptPath, defaultLanguage) -- Creates a new Language class.";
		Language_Type.tp_methods = Language_methods;
		Language_Type.tp_base = 0;
		Language_Type.tp_new = Language_New;
	}
}

#ifdef __cplusplus
}
#endif
