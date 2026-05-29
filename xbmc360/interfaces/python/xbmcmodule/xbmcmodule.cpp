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

// xbmc Python module for XBMC-360
// Provides core scripting functions: log, sleep, translatePath, etc.
// Functions not yet ported are stubbed with TODO markers.

#include "Python.h"
#include "utils/log.h"
#include "player.h"
#include "pyutil.h"
#include "keyboard.h"
#include "pyplaylist.h"
#include "infotagmusic.h"
#include "infotagvideo.h"
#include "language.h"
#include "PythonSettings.h"
#include "PlayListPlayer.h"
#include "cores/PlayerCoreFactory.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{

// =========================================================================
// xbmc.log(msg, level=LOGNOTICE)
// =========================================================================
static PyObject* XBMC_Log(PyObject* self, PyObject* args, PyObject* kwds)
{
	static const char* keywords[] = { "msg", "level", NULL };
	char* s_line = NULL;
	int iLevel = LOGNOTICE;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|i",
		const_cast<char**>(keywords), &s_line, &iLevel))
		return NULL;

	if (iLevel < LOGDEBUG || iLevel > LOGNONE)
		iLevel = LOGNOTICE;

	CLog::Log(iLevel, "%s", s_line);

	Py_RETURN_NONE;
}

// =========================================================================
// xbmc.sleep(time) - sleep for 'time' milliseconds
// =========================================================================
static PyObject* XBMC_Sleep(PyObject* self, PyObject* args)
{
	int iTime = 0;
	if (!PyArg_ParseTuple(args, "i", &iTime))
		return NULL;

	// Release GIL during sleep so other threads can run
	Py_BEGIN_ALLOW_THREADS
	Sleep(iTime);
	Py_END_ALLOW_THREADS

	Py_RETURN_NONE;
}

// =========================================================================
// xbmc.executescript(script) - TODO: execute a python script
// =========================================================================
static PyObject* XBMC_ExecuteScript(PyObject* self, PyObject* args)
{
	char* cLine = NULL;
	if (!PyArg_ParseTuple(args, "s", &cLine))
		return NULL;

	// TODO: implement via ApplicationMessenger
	CLog::Log(LOGWARNING, "Python: xbmc.executescript() not yet implemented");
	Py_RETURN_NONE;
}

// =========================================================================
// xbmc.executebuiltin(function) - TODO: execute a built-in function
// =========================================================================
static PyObject* XBMC_ExecuteBuiltIn(PyObject* self, PyObject* args)
{
	char* cLine = NULL;
	if (!PyArg_ParseTuple(args, "s", &cLine))
		return NULL;

	// TODO: implement via ApplicationMessenger / Builtins
	CLog::Log(LOGWARNING, "Python: xbmc.executebuiltin('%s') not yet implemented", cLine);
	Py_RETURN_NONE;
}

// =========================================================================
// xbmc.getLocalizedString(id) - TODO: get localized string
// =========================================================================
static PyObject* XBMC_GetLocalizedString(PyObject* self, PyObject* args)
{
	int iId = 0;
	if (!PyArg_ParseTuple(args, "i", &iId))
		return NULL;

	// TODO: implement via g_localizeStrings.Get(iId)
	CLog::Log(LOGDEBUG, "Python: xbmc.getLocalizedString(%d) - stub", iId);
	return PyUnicode_FromString("");
}

// =========================================================================
// xbmc.getSkinDir() - TODO: return current skin directory name
// =========================================================================
static PyObject* XBMC_GetSkinDir(PyObject* self, PyObject* args)
{
	// TODO: implement via g_guiSettings.GetString("lookandfeel.skin")
	return PyUnicode_FromString("Project Mayhem III");
}

// =========================================================================
// xbmc.getLanguage() - TODO: return current language
// =========================================================================
static PyObject* XBMC_GetLanguage(PyObject* self, PyObject* args)
{
	// TODO: implement via g_guiSettings
	return PyUnicode_FromString("English");
}

// =========================================================================
// xbmc.getIPAddress() - TODO: return IP address
// =========================================================================
static PyObject* XBMC_GetIPAddress(PyObject* self, PyObject* args)
{
	// TODO: implement via network manager
	return PyUnicode_FromString("0.0.0.0");
}

// =========================================================================
// xbmc.getFreeMem() - TODO: return free memory in MB
// =========================================================================
static PyObject* XBMC_GetFreeMem(PyObject* self, PyObject* args)
{
	// TODO: implement via GlobalMemoryStatus or Xbox 360 equivalent
	return PyLong_FromLong(0);
}

// =========================================================================
// xbmc.getInfoLabel(infotag) - TODO: return info label value
// =========================================================================
static PyObject* XBMC_GetInfoLabel(PyObject* self, PyObject* args)
{
	char* cLine = NULL;
	if (!PyArg_ParseTuple(args, "s", &cLine))
		return NULL;

	// TODO: implement via g_infoManager.GetLabel()
	CLog::Log(LOGDEBUG, "Python: xbmc.getInfoLabel('%s') - stub", cLine);
	return PyUnicode_FromString("");
}

// =========================================================================
// xbmc.getInfoImage(infotag) - TODO: return info image path
// =========================================================================
static PyObject* XBMC_GetInfoImage(PyObject* self, PyObject* args)
{
	char* cLine = NULL;
	if (!PyArg_ParseTuple(args, "s", &cLine))
		return NULL;

	// TODO: implement via g_infoManager.GetImage()
	return PyUnicode_FromString("");
}

// =========================================================================
// xbmc.getCondVisibility(condition) - TODO: evaluate visibility condition
// =========================================================================
static PyObject* XBMC_GetCondVisibility(PyObject* self, PyObject* args)
{
	char* cLine = NULL;
	if (!PyArg_ParseTuple(args, "s", &cLine))
		return NULL;

	// TODO: implement via g_infoManager.GetBool()
	Py_RETURN_FALSE;
}

// =========================================================================
// xbmc.translatePath(path) - TODO: translate special:// paths
// =========================================================================
static PyObject* XBMC_TranslatePath(PyObject* self, PyObject* args)
{
	char* cLine = NULL;
	if (!PyArg_ParseTuple(args, "s", &cLine))
		return NULL;

	// TODO: implement via CSpecialProtocol::TranslatePath
	// For now just return the input path unchanged
	return PyUnicode_FromString(cLine);
}

// =========================================================================
// xbmc.validatePath(path) - TODO: validate/normalize path
// =========================================================================
static PyObject* XBMC_ValidatePath(PyObject* self, PyObject* args)
{
	char* cLine = NULL;
	if (!PyArg_ParseTuple(args, "s", &cLine))
		return NULL;

	// TODO: implement path validation
	return PyUnicode_FromString(cLine);
}

// =========================================================================
// xbmc.getGlobalIdleTime() - TODO: return idle time in seconds
// =========================================================================
static PyObject* XBMC_GetGlobalIdleTime(PyObject* self, PyObject* args)
{
	// TODO: implement
	return PyLong_FromLong(0);
}

// =========================================================================
// xbmc.playSFX(filename) - TODO: play sound effect
// =========================================================================
static PyObject* XBMC_PlaySFX(PyObject* self, PyObject* args)
{
	char* cFile = NULL;
	if (!PyArg_ParseTuple(args, "s", &cFile))
		return NULL;

	// TODO: implement via CGUIAudioManager
	CLog::Log(LOGDEBUG, "Python: xbmc.playSFX('%s') - stub", cFile);
	Py_RETURN_NONE;
}

// =========================================================================
// xbmc.enableNavSounds(enable) - TODO: enable/disable nav sounds
// =========================================================================
static PyObject* XBMC_EnableNavSounds(PyObject* self, PyObject* args)
{
	int iEnable = 0;
	if (!PyArg_ParseTuple(args, "i", &iEnable))
		return NULL;

	// TODO: implement
	Py_RETURN_NONE;
}

// =========================================================================
// xbmc.getRegion(id) - TODO: return region info
// =========================================================================
static PyObject* XBMC_GetRegion(PyObject* self, PyObject* args, PyObject* kwds)
{
	static const char* keywords[] = { "id", NULL };
	char* cId = NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s",
		const_cast<char**>(keywords), &cId))
		return NULL;

	// TODO: implement via CLangInfo
	return PyUnicode_FromString("");
}

// =========================================================================
// xbmc.shutdown() - TODO: shutdown console
// =========================================================================
static PyObject* XBMC_Shutdown(PyObject* self, PyObject* args)
{
	// TODO: implement via ApplicationMessenger
	CLog::Log(LOGWARNING, "Python: xbmc.shutdown() not yet implemented");
	Py_RETURN_NONE;
}

// =========================================================================
// xbmc.restart() - TODO: restart console
// =========================================================================
static PyObject* XBMC_Restart(PyObject* self, PyObject* args)
{
	// TODO: implement via ApplicationMessenger
	CLog::Log(LOGWARNING, "Python: xbmc.restart() not yet implemented");
	Py_RETURN_NONE;
}

// =========================================================================
// Method table
// =========================================================================
static PyMethodDef xbmcMethods[] = {
	{"log",               (PyCFunction)XBMC_Log,               METH_VARARGS|METH_KEYWORDS, "log(msg, level) - Write to XBMC log."},
	{"sleep",             (PyCFunction)XBMC_Sleep,             METH_VARARGS, "sleep(time) - Sleep for time milliseconds."},
	{"executescript",     (PyCFunction)XBMC_ExecuteScript,     METH_VARARGS, "executescript(script) - Execute a Python script."},
	{"executebuiltin",    (PyCFunction)XBMC_ExecuteBuiltIn,    METH_VARARGS, "executebuiltin(function) - Execute a built-in function."},
	{"getLocalizedString",(PyCFunction)XBMC_GetLocalizedString,METH_VARARGS, "getLocalizedString(id) - Get localized string."},
	{"getSkinDir",        (PyCFunction)XBMC_GetSkinDir,        METH_VARARGS, "getSkinDir() - Get current skin directory."},
	{"getLanguage",       (PyCFunction)XBMC_GetLanguage,       METH_VARARGS, "getLanguage() - Get current language."},
	{"getIPAddress",      (PyCFunction)XBMC_GetIPAddress,      METH_VARARGS, "getIPAddress() - Get IP address."},
	{"getFreeMem",        (PyCFunction)XBMC_GetFreeMem,        METH_VARARGS, "getFreeMem() - Get free memory in MB."},
	{"getInfoLabel",      (PyCFunction)XBMC_GetInfoLabel,      METH_VARARGS, "getInfoLabel(infotag) - Get info label value."},
	{"getInfoImage",      (PyCFunction)XBMC_GetInfoImage,      METH_VARARGS, "getInfoImage(infotag) - Get info image path."},
	{"getCondVisibility", (PyCFunction)XBMC_GetCondVisibility, METH_VARARGS, "getCondVisibility(condition) - Evaluate visibility condition."},
	{"translatePath",     (PyCFunction)XBMC_TranslatePath,     METH_VARARGS, "translatePath(path) - Translate special:// paths."},
	{"validatePath",      (PyCFunction)XBMC_ValidatePath,      METH_VARARGS, "validatePath(path) - Validate/normalize path."},
	{"getGlobalIdleTime", (PyCFunction)XBMC_GetGlobalIdleTime, METH_VARARGS, "getGlobalIdleTime() - Get idle time in seconds."},
	{"playSFX",           (PyCFunction)XBMC_PlaySFX,           METH_VARARGS, "playSFX(filename) - Play sound effect."},
	{"enableNavSounds",   (PyCFunction)XBMC_EnableNavSounds,   METH_VARARGS, "enableNavSounds(enable) - Enable/disable nav sounds."},
	{"getRegion",         (PyCFunction)XBMC_GetRegion,         METH_VARARGS|METH_KEYWORDS, "getRegion(id) - Get region info."},
	{"shutdown",          (PyCFunction)XBMC_Shutdown,          METH_VARARGS, "shutdown() - Shutdown console."},
	{"restart",           (PyCFunction)XBMC_Restart,           METH_VARARGS, "restart() - Restart console."},
	{NULL, NULL, 0, NULL}
};

// =========================================================================
// Module definition (Python 3 style)
// =========================================================================
static struct PyModuleDef xbmcModuleDef = {
	PyModuleDef_HEAD_INIT,
	"xbmc",           // module name
	"XBMC core scripting module for Xbox 360",  // docstring
	-1,               // module state size (-1 = global state)
	xbmcMethods,
	NULL, NULL, NULL, NULL
};

} // namespace PYXBMC

// =========================================================================
// Module init function - called by Python when "import xbmc" is used
// =========================================================================
PyMODINIT_FUNC PyInit_xbmc(void)
{
	using namespace PYXBMC;

	PyObject* pModule = PyModule_Create(&xbmcModuleDef);
	if (!pModule)
		return NULL;

	// Log level constants
	PyModule_AddIntConstant(pModule, "LOGDEBUG",   LOGDEBUG);
	PyModule_AddIntConstant(pModule, "LOGINFO",    LOGINFO);
	PyModule_AddIntConstant(pModule, "LOGNOTICE",  LOGNOTICE);
	PyModule_AddIntConstant(pModule, "LOGWARNING", LOGWARNING);
	PyModule_AddIntConstant(pModule, "LOGERROR",   LOGERROR);
	PyModule_AddIntConstant(pModule, "LOGSEVERE",  LOGSEVERE);
	PyModule_AddIntConstant(pModule, "LOGFATAL",   LOGFATAL);
	PyModule_AddIntConstant(pModule, "LOGNONE",    LOGNONE);

	// Playlist constants
	PyModule_AddIntConstant(pModule, "PLAYLIST_MUSIC", PLAYLIST_MUSIC);
	PyModule_AddIntConstant(pModule, "PLAYLIST_VIDEO", PLAYLIST_VIDEO);

	// Player type
	initPlayer_Type();
	if (PyType_Ready(&Player_Type) < 0)
	{
		Py_DECREF(pModule);
		return NULL;
	}
	Py_INCREF(&Player_Type);
	PyModule_AddObject(pModule, "Player", (PyObject*)&Player_Type);

	// Keyboard type
	initKeyboard_Type();
	if (PyType_Ready(&Keyboard_Type) < 0)
	{
		Py_DECREF(pModule);
		return NULL;
	}
	Py_INCREF(&Keyboard_Type);
	PyModule_AddObject(pModule, "Keyboard", (PyObject*)&Keyboard_Type);

	// PlayList types
	initPlayList_Type();
	initPlayListItem_Type();
	if (PyType_Ready(&PlayList_Type) < 0 || PyType_Ready(&PlayListItem_Type) < 0)
	{
		Py_DECREF(pModule);
		return NULL;
	}
	Py_INCREF(&PlayList_Type);
	PyModule_AddObject(pModule, "PlayList", (PyObject*)&PlayList_Type);
	Py_INCREF(&PlayListItem_Type);
	PyModule_AddObject(pModule, "PlayListItem", (PyObject*)&PlayListItem_Type);

	// InfoTag types
	initInfoTagMusic_Type();
	initInfoTagVideo_Type();
	if (PyType_Ready(&InfoTagMusic_Type) < 0 || PyType_Ready(&InfoTagVideo_Type) < 0)
	{
		Py_DECREF(pModule);
		return NULL;
	}
	Py_INCREF(&InfoTagMusic_Type);
	PyModule_AddObject(pModule, "InfoTagMusic", (PyObject*)&InfoTagMusic_Type);
	Py_INCREF(&InfoTagVideo_Type);
	PyModule_AddObject(pModule, "InfoTagVideo", (PyObject*)&InfoTagVideo_Type);

	// Language type
	initLanguage_Type();
	if (PyType_Ready(&Language_Type) < 0)
	{
		Py_DECREF(pModule);
		return NULL;
	}
	Py_INCREF(&Language_Type);
	PyModule_AddObject(pModule, "Language", (PyObject*)&Language_Type);

	// Settings type
	initSettings_Type();
	if (PyType_Ready(&Settings_Type) < 0)
	{
		Py_DECREF(pModule);
		return NULL;
	}
	Py_INCREF(&Settings_Type);
	PyModule_AddObject(pModule, "Settings", (PyObject*)&Settings_Type);

	return pModule;
}

#ifdef __cplusplus
}
#endif
