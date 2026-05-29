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
#include "listitem.h"
#include <string>
#include <vector>

// color_t is used for color storage (ARGB)
typedef unsigned long color_t;

// Control common fields macro - matches xbmc4xbox structure
#define PyObject_HEAD_XBMC_CONTROL  \
		PyObject_HEAD                   \
		int iControlId;                 \
		int iParentId;                  \
		int dwPosX;                     \
		int dwPosY;                     \
		int dwWidth;                    \
		int dwHeight;                   \
		int iControlUp;                 \
		int iControlDown;               \
		int iControlLeft;               \
		int iControlRight;              \
		void* pGUIControl;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
	} Control;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		color_t color;
		std::string strTextureUp;
		std::string strTextureDown;
		std::string strTextureUpFocus;
		std::string strTextureDownFocus;
	} ControlSpin;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		std::string strFont;
		std::string strText;
		color_t textColor;
		color_t disabledColor;
		uint32_t align;
		bool bHasPath;
		int iAngle;
	} ControlLabel;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		std::string strFont;
		color_t textColor;
		std::vector<std::string> vecLabels;
		uint32_t align;
	} ControlFadeLabel;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		std::string strFont;
		color_t textColor;
	} ControlTextBox;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		std::string strFileName;
		int aspectRatio;
		color_t colorDiffuse;
	} ControlImage;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		std::string strTextureLeft;
		std::string strTextureMid;
		std::string strTextureRight;
		std::string strTextureBg;
		std::string strTextureOverlay;
		int aspectRatio;
		color_t colorDiffuse;
	} ControlProgress;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		std::string strFont;
		std::string strText;
		std::string strText2;
		std::string strTextureFocus;
		std::string strTextureNoFocus;
		color_t textColor;
		color_t disabledColor;
		int textOffsetX;
		int textOffsetY;
		color_t align;
		int iAngle;
		int shadowColor;
		int focusedColor;
	} ControlButton;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		std::string strFont;
		std::string strText;
		std::string strTextureFocus;
		std::string strTextureNoFocus;
		color_t textColor;
		color_t disabledColor;
		int checkWidth;
		int checkHeight;
		uint32_t align;
	} ControlCheckMark;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		std::vector<PYXBMC::ListItem*> vecItems;
		std::string strFont;
		ControlSpin* pControlSpin;

		color_t textColor;
		color_t selectedColor;
		std::string strTextureButton;
		std::string strTextureButtonFocus;

		int imageHeight;
		int imageWidth;
		int itemHeight;
		int space;

		int itemTextOffsetX;
		int itemTextOffsetY;
		uint32_t alignmentY;
	} ControlList;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
	} ControlGroup;

	typedef struct {
		PyObject_HEAD_XBMC_CONTROL
		std::string strFont;
		std::string strText;
		std::string strTextureFocus;
		std::string strTextureNoFocus;
		std::string strTextureRadioFocus;
		std::string strTextureRadioNoFocus;
		color_t textColor;
		color_t disabledColor;
		int textOffsetX;
		int textOffsetY;
		uint32_t align;
		int iAngle;
		color_t shadowColor;
		color_t focusedColor;
	} ControlRadioButton;

	extern void Control_Dealloc(Control* self);
	extern PyMethodDef Control_methods[];

	extern PyTypeObject Control_Type;
	extern PyTypeObject ControlSpin_Type;
	extern PyTypeObject ControlLabel_Type;
	extern PyTypeObject ControlFadeLabel_Type;
	extern PyTypeObject ControlTextBox_Type;
	extern PyTypeObject ControlImage_Type;
	extern PyTypeObject ControlGroup_Type;
	extern PyTypeObject ControlButton_Type;
	extern PyTypeObject ControlCheckMark_Type;
	extern PyTypeObject ControlList_Type;
	extern PyTypeObject ControlProgress_Type;
	extern PyTypeObject ControlRadioButton_Type;

	void initControl_Type();
	void initControlSpin_Type();
	void initControlLabel_Type();
	void initControlFadeLabel_Type();
	void initControlTextBox_Type();
	void initControlButton_Type();
	void initControlCheckMark_Type();
	void initControlList_Type();
	void initControlImage_Type();
	void initControlGroup_Type();
	void initControlProgress_Type();
	void initControlRadioButton_Type();
}

#ifdef __cplusplus
}
#endif
