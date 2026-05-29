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
#include "control.h"
#include "action.h"
#include <vector>

#define Window_Check(op) PyObject_TypeCheck(op, &PYXBMC::Window_Type)
#define Window_CheckExact(op) (Py_TYPE(op) == &PYXBMC::Window_Type)

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	typedef struct {
		PyObject_HEAD
		int iWindowId;
		int iOldWindowId;
		int iCurrentControlId;
		bool bModal;
		bool bIsPythonWindow;
		std::vector<Control*> vecControls;
		// TODO: Add CGUIWindow* pWindow when GUI subsystem available
	} Window;

	extern PyTypeObject Window_Type;
	extern PyTypeObject WindowDialog_Type;

	void initWindow_Type();
	void initWindowDialog_Type();
}

#ifdef __cplusplus
}
#endif
