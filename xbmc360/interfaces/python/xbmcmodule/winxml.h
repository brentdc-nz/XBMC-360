#pragma once

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
#include "window.h"
#include "control.h"
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	// WindowXML extends Window with XML skin file info
	typedef struct {
		PyObject_HEAD
		int iWindowId;
		int iOldWindowId;
		int iCurrentControlId;
		bool bModal;
		bool bIsPythonWindow;
		bool bUsingXML;
		std::vector<Control*> vecControls;
		std::string sXMLFileName;
		std::string sFallBackPath;
		void* pWindow; // TODO: CGUIPythonWindowXML* when available
	} WindowXML;

	// WindowXMLDialog is same struct as WindowXML
	typedef WindowXML WindowXMLDialog;

	extern PyTypeObject WindowXML_Type;
	extern PyTypeObject WindowXMLDialog_Type;

	void initWindowXML_Type();
	void initWindowXMLDialog_Type();
	void Window_Dealloc(Window* self);
}

#ifdef __cplusplus
}
#endif
