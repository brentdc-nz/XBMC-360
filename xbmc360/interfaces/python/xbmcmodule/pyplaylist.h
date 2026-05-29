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
#include <string>
#include <vector>

#define PlayList_Check(op) PyObject_TypeCheck(op, &PYXBMC::PlayList_Type)
#define PlayList_CheckExact(op) (Py_TYPE(op) == &PYXBMC::PlayList_Type)
#define PlayListItem_Check(op) PyObject_TypeCheck(op, &PYXBMC::PlayListItem_Type)
#define PlayListItem_CheckExact(op) (Py_TYPE(op) == &PYXBMC::PlayListItem_Type)

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	extern PyTypeObject PlayList_Type;
	extern PyTypeObject PlayListItem_Type;

	typedef struct {
		PyObject_HEAD
		int iPlayList;
		// TODO: Replace with actual CPlayList* when playlist subsystem is available
		std::vector<std::string> vecItems;
	} PlayList;

	typedef struct {
		PyObject_HEAD
		std::string strPath;
		std::string strDescription;
		long lDuration;
	} PlayListItem;

	void initPlayList_Type();
	void initPlayListItem_Type();
}

#ifdef __cplusplus
}
#endif
