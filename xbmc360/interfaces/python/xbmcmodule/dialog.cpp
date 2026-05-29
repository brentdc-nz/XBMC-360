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

#include "dialog.h"
#include "pyutil.h"
#include "guilib\dialogs\GUIDialogOK.h"
#include "guilib\dialogs\GUIDialogYesNo.h"
#include "guilib\dialogs\GUIDialogSelect.h"
#include "guilib\dialogs\GUIDialogProgress.h"
#include "guilib\dialogs\GUIDialogFileBrowser.h"
#include "guilib\dialogs\GUIDialogNumeric.h"
#include "guilib\GUIWindowManager.h"
#include "ApplicationMessenger.h"
#include "Settings.h"
#include "MediaManager.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	typedef struct {
		PyObject_HEAD
	} Dialog;

	typedef struct {
		PyObject_HEAD
		bool bCanceled;
		float fPercent;
	} DialogProgress;

	// Dialog methods
	PyDoc_STRVAR(ok__doc__,
		"ok(heading, line1[, line2, line3]) -- Show a dialog 'OK'.\n");
	PyObject* Dialog_OK(Dialog *self, PyObject *args)
	{
		const int window = WINDOW_DIALOG_OK;
		PyObject* unicodeLine[4];
		for (int i = 0; i < 4; i++) unicodeLine[i] = NULL;

		CGUIDialogOK* pDialog = (CGUIDialogOK*)g_windowManager.GetWindow(window);
		if (PyXBMCWindowIsNull(pDialog)) return NULL;

		// get lines, last 2 lines are optional.
		string utf8Line[4];
		if (!PyArg_ParseTuple(args, (char*)"OO|OO", &unicodeLine[0], &unicodeLine[1], &unicodeLine[2], &unicodeLine[3]))
			return NULL;

		for (int i = 0; i < 4; i++)
		{
			if (unicodeLine[i] && !PyXBMCGetUnicodeString(utf8Line[i], unicodeLine[i], i+1))
				return NULL;
		}
		pDialog->SetHeading(utf8Line[0]);
		pDialog->SetLine(0, utf8Line[1]);
		pDialog->SetLine(1, utf8Line[2]);
		pDialog->SetLine(2, utf8Line[3]);

		// send message and wait for user input
		PyXBMCWaitForThreadMessage(TMSG_DIALOG_DOMODAL, window, g_windowManager.GetActiveWindow());

		return Py_BuildValue((char*)"b", pDialog->IsConfirmed());
	}

	PyDoc_STRVAR(yesno__doc__,
		"yesno(heading, line1[, line2, line3, nolabel, yeslabel]) -- Show a dialog 'YES/NO'.\n"
		"\n"
		"Returns True if 'Yes' was pressed, else False.\n");
	PyObject* Dialog_YesNo(Dialog *self, PyObject *args)
	{
		const int window = WINDOW_DIALOG_YES_NO;
		PyObject* unicodeLine[6];
		for (int i = 0; i < 6; i++) unicodeLine[i] = NULL;
		CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(window);
		if (PyXBMCWindowIsNull(pDialog)) return NULL;

		// get lines, last 4 lines are optional.
		string utf8Line[6];
		if (!PyArg_ParseTuple(args, (char*)"OO|OOOO", &unicodeLine[0], &unicodeLine[1], &unicodeLine[2], &unicodeLine[3], &unicodeLine[4], &unicodeLine[5]))
			return NULL;

		for (int i = 0; i < 6; ++i)
		{
			if (unicodeLine[i] && !PyXBMCGetUnicodeString(utf8Line[i], unicodeLine[i], i+1))
				return NULL;
		}
		pDialog->SetHeading(utf8Line[0]);
		pDialog->SetLine(0, utf8Line[1]);
		pDialog->SetLine(1, utf8Line[2]);
		pDialog->SetLine(2, utf8Line[3]);
		if (utf8Line[4] != "")
			pDialog->SetChoice(0, utf8Line[4]);
		if (utf8Line[5] != "")
			pDialog->SetChoice(1, utf8Line[5]);

		// send message and wait for user input
		PyXBMCWaitForThreadMessage(TMSG_DIALOG_DOMODAL, window, g_windowManager.GetActiveWindow());

		return Py_BuildValue((char*)"b", pDialog->IsConfirmed());
	}

	PyDoc_STRVAR(select__doc__,
		"select(heading, list[, autoclose]) -- Show a select dialog.\n"
		"\n"
		"Returns the position of the highlighted item as an integer.\n");
	PyObject* Dialog_Select(Dialog *self, PyObject *args)
	{
		const int window = WINDOW_DIALOG_SELECT;
		PyObject *heading = NULL;
		PyObject *list = NULL;
		int autoClose = 0;

		if (!PyArg_ParseTuple(args, (char*)"OO|i", &heading, &list, &autoClose))
			return NULL;
		if (!PyList_Check(list)) return NULL;

		CGUIDialogSelect* pDialog = (CGUIDialogSelect*)g_windowManager.GetWindow(window);
		if (PyXBMCWindowIsNull(pDialog)) return NULL;

		pDialog->Reset();
		CStdString utf8Heading;
		if (heading && PyXBMCGetUnicodeString(utf8Heading, heading, 1))
			pDialog->SetHeading(utf8Heading);

		PyObject *listLine = NULL;
		for (int i = 0; i < PyList_Size(list); i++)
		{
			listLine = PyList_GetItem(list, i);
			CStdString utf8Line;
			if (listLine && PyXBMCGetUnicodeString(utf8Line, listLine, i))
				pDialog->Add(utf8Line);
		}
		if (autoClose > 0)
			pDialog->SetAutoClose(autoClose);

		// send message and wait for user input
		PyXBMCWaitForThreadMessage(TMSG_DIALOG_DOMODAL, window, g_windowManager.GetActiveWindow());

		return Py_BuildValue((char*)"i", pDialog->GetSelectedLabel());
	}

	PyDoc_STRVAR(browse__doc__,
		"browse(type, heading, shares[, mask, useThumbs, treatAsFolder, default]) -- Show a 'Browse' dialog.\n"
		"\n"
		"Returns the selected file/folder as string.\n");
	PyObject* Dialog_Browse(Dialog *self, PyObject *args)
	{
		int browsetype = 0;
		char useThumbs = false;
		char useFileDirectories = false;
		CStdString value;
		PyObject* unicodeLine[3];
		string utf8Line[3];
		char *cDefault = NULL;
		for (int i = 0; i < 3; i++) unicodeLine[i] = NULL;
		if (!PyArg_ParseTuple(args, (char*)"iOO|Obbs", &browsetype, &unicodeLine[0], &unicodeLine[1], &unicodeLine[2], &useThumbs, &useFileDirectories, &cDefault))
			return NULL;
		for (int i = 0; i < 3; i++)
		{
			if (unicodeLine[i] && !PyXBMCGetUnicodeString(utf8Line[i], unicodeLine[i], i+1))
				return NULL;
		}
		VECSOURCES *shares = NULL;
		shares = g_settings.GetSourcesFromType(utf8Line[1].c_str());
		if (!shares) return NULL;
		VECSOURCES localShares;
		localShares = *shares;
		g_mediaManager.GetLocalDrives(localShares);

		if (useFileDirectories && !utf8Line[2].size() == 0)
			utf8Line[2] += "|.rar|.zip";

		value = cDefault ? cDefault : "";
		if (browsetype == 1)
			CGUIDialogFileBrowser::ShowAndGetFile(localShares, utf8Line[2].c_str(), utf8Line[0].c_str(), value, 0 != useThumbs, 0 != useFileDirectories);
		else if (browsetype == 2)
			CGUIDialogFileBrowser::ShowAndGetImage(localShares, utf8Line[0].c_str(), value);
		else
			CGUIDialogFileBrowser::ShowAndGetDirectory(localShares, utf8Line[0].c_str(), value, browsetype != 0);
		return Py_BuildValue((char*)"s", value.c_str());
	}

	PyDoc_STRVAR(numeric__doc__,
		"numeric(type, heading[, default]) -- Show a 'Numeric' dialog.\n"
		"\n"
		"Returns the entered data as a string.\n");
	PyObject* Dialog_Numeric(Dialog *self, PyObject *args)
	{
		int inputtype = 0;
		CStdString value;
		PyObject *heading = NULL;
		char *cDefault = NULL;
		SYSTEMTIME timedate;
		GetLocalTime(&timedate);
		if (!PyArg_ParseTuple(args, (char*)"iO|s", &inputtype, &heading, &cDefault))
			return NULL;

		CStdString utf8Heading;
		if (heading && PyXBMCGetUnicodeString(utf8Heading, heading, 1))
		{
			if (inputtype == 1)
			{
				if (cDefault && strlen(cDefault) == 10)
				{
					CStdString sDefault = cDefault;
					timedate.wDay = atoi(sDefault.Left(2));
					timedate.wMonth = atoi(sDefault.Mid(3, 4));
					timedate.wYear = atoi(sDefault.Right(4));
				}
				if (CGUIDialogNumeric::ShowAndGetDate(timedate, utf8Heading))
					value.Format("%2d/%2d/%4d", timedate.wDay, timedate.wMonth, timedate.wYear);
				else
					Py_RETURN_NONE;
			}
			else if (inputtype == 2)
			{
				if (cDefault && strlen(cDefault) == 5)
				{
					CStdString sDefault = cDefault;
					timedate.wHour = atoi(sDefault.Left(2));
					timedate.wMinute = atoi(sDefault.Right(2));
				}
				if (CGUIDialogNumeric::ShowAndGetTime(timedate, utf8Heading))
					value.Format("%2d:%02d", timedate.wHour, timedate.wMinute);
				else
					Py_RETURN_NONE;
			}
			else if (inputtype == 3)
			{
				value = cDefault ? cDefault : "";
				if (!CGUIDialogNumeric::ShowAndGetIPAddress(value, utf8Heading))
					Py_RETURN_NONE;
			}
			else
			{
				value = cDefault ? cDefault : "";
				if (!CGUIDialogNumeric::ShowAndGetNumber(value, utf8Heading))
					Py_RETURN_NONE;
			}
		}
		return Py_BuildValue((char*)"s", value.c_str());
	}

	PyMethodDef Dialog_methods[] = {
		{(char*)"ok", (PyCFunction)Dialog_OK, METH_VARARGS, ok__doc__},
		{(char*)"yesno", (PyCFunction)Dialog_YesNo, METH_VARARGS, yesno__doc__},
		{(char*)"select", (PyCFunction)Dialog_Select, METH_VARARGS, select__doc__},
		{(char*)"browse", (PyCFunction)Dialog_Browse, METH_VARARGS, browse__doc__},
		{(char*)"numeric", (PyCFunction)Dialog_Numeric, METH_VARARGS, numeric__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(dialog__doc__,
		"Dialog class.\n"
		"\n"
		"Dialog() -- Creates a new Dialog.\n");

	PyTypeObject Dialog_Type;

	void initDialog_Type()
	{
		PyXBMCInitializeTypeObject(&Dialog_Type);

		Dialog_Type.tp_name = (char*)"xbmcgui.Dialog";
		Dialog_Type.tp_basicsize = sizeof(Dialog);
		Dialog_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		Dialog_Type.tp_doc = dialog__doc__;
		Dialog_Type.tp_methods = Dialog_methods;
		Dialog_Type.tp_base = 0;
		Dialog_Type.tp_new = PyType_GenericNew;
	}

	// DialogProgress methods
	PyDoc_STRVAR(create__doc__,
		"create(heading[, line1, line2, line3]) -- Create and show a progress dialog.\n");
	PyObject* DialogProgress_Create(DialogProgress *self, PyObject *args)
	{
		PyObject* unicodeLine[4];
		for (int i = 0; i < 4; i++) unicodeLine[i] = NULL;

		// get lines, last 3 lines are optional.
		if (!PyArg_ParseTuple(args, (char*)"O|OOO", &unicodeLine[0], &unicodeLine[1], &unicodeLine[2], &unicodeLine[3]))
			return NULL;

		string utf8Line[4];
		for (int i = 0; i < 4; i++)
		{
			if (unicodeLine[i] && !PyXBMCGetUnicodeString(utf8Line[i], unicodeLine[i], i+1))
				return NULL;
		}

		CGUIDialogProgress* pDialog = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
		if (PyXBMCWindowIsNull(pDialog)) return NULL;

		pDialog->SetHeading(utf8Line[0]);

		for (int i = 1; i < 4; i++)
			pDialog->SetLine(i - 1, utf8Line[i]);

		pDialog->StartModal(true);

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(update_progress__doc__,
		"update(percent[, line1, line2, line3]) -- Update's the progress dialog.\n");
	PyObject* DialogProgress_Update(DialogProgress *self, PyObject *args)
	{
		int percentage = 0;
		PyObject *unicodeLine[3];
		for (int i = 0; i < 3; i++) unicodeLine[i] = NULL;
		if (!PyArg_ParseTuple(args, (char*)"i|OOO", &percentage, &unicodeLine[0], &unicodeLine[1], &unicodeLine[2]))
			return NULL;

		string utf8Line[3];
		for (int i = 0; i < 3; i++)
		{
			if (unicodeLine[i] && !PyXBMCGetUnicodeString(utf8Line[i], unicodeLine[i], i+2))
				return NULL;
		}

		CGUIDialogProgress* pDialog = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
		if (PyXBMCWindowIsNull(pDialog)) return NULL;

		if (percentage >= 0 && percentage <= 100)
		{
			pDialog->SetPercentage(percentage);
			pDialog->ShowProgressBar(true);
		}
		else
		{
			pDialog->ShowProgressBar(false);
		}
		for (int i = 0; i < 3; i++)
		{
			if (unicodeLine[i])
				pDialog->SetLine(i, utf8Line[i]);
		}

		Py_RETURN_NONE;
	}

	PyDoc_STRVAR(iscanceled__doc__,
		"iscanceled() -- Returns True if the user pressed cancel.\n");
	PyObject* DialogProgress_IsCanceled(DialogProgress *self, PyObject *args)
	{
		CGUIDialogProgress* pDialog = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
		if (PyXBMCWindowIsNull(pDialog)) return NULL;

		return Py_BuildValue((char*)"b", pDialog->IsCanceled());
	}

	PyDoc_STRVAR(close_progress__doc__,
		"close() -- Close the progress dialog.\n");
	PyObject* DialogProgress_Close(DialogProgress *self, PyObject *args)
	{
		CGUIDialogProgress* pDialog = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
		if (PyXBMCWindowIsNull(pDialog)) return NULL;

		pDialog->Close();

		Py_RETURN_NONE;
	}

	PyMethodDef DialogProgress_methods[] = {
		{(char*)"create", (PyCFunction)DialogProgress_Create, METH_VARARGS, create__doc__},
		{(char*)"update", (PyCFunction)DialogProgress_Update, METH_VARARGS, update_progress__doc__},
		{(char*)"iscanceled", (PyCFunction)DialogProgress_IsCanceled, METH_VARARGS, iscanceled__doc__},
		{(char*)"close", (PyCFunction)DialogProgress_Close, METH_VARARGS, close_progress__doc__},
		{NULL, NULL, 0, NULL}
	};

	PyDoc_STRVAR(dialogProgress__doc__,
		"DialogProgress class.\n"
		"\n"
		"DialogProgress() -- Creates a new progress dialog.\n");

	PyTypeObject DialogProgress_Type;

	void initDialogProgress_Type()
	{
		PyXBMCInitializeTypeObject(&DialogProgress_Type);

		DialogProgress_Type.tp_name = (char*)"xbmcgui.DialogProgress";
		DialogProgress_Type.tp_basicsize = sizeof(DialogProgress);
		DialogProgress_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
		DialogProgress_Type.tp_doc = dialogProgress__doc__;
		DialogProgress_Type.tp_methods = DialogProgress_methods;
		DialogProgress_Type.tp_base = 0;
		DialogProgress_Type.tp_new = PyType_GenericNew;
	}
}

#ifdef __cplusplus
}
#endif