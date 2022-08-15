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

#ifndef H_CGUISKININFO
#define H_CGUISKININFO

#include "GraphicContext.h" // needed for the RESOLUTION members
#include "GUIIncludes.h"    // needed for the GUIInclude member

#include "..\utils\StdString.h"

#define CREDIT_LINE_LENGTH 50

class CSkinInfo
{
public:
	CSkinInfo();
	~CSkinInfo();

	void Load(const CStdString& skinDir, bool loadIncludes = true);
	bool ResolveConstant(const CStdString &constant, float &value) const;

	// Brief Get the full path to the specified file in the skin
	// We search for XML files in the skin folder that best matches the current resolution
	CStdString GetSkinPath(const CStdString& file, RESOLUTION *res = NULL, const CStdString& baseDir = "") const;

protected:
	CStdString m_strBaseDir;
	CGUIIncludes m_includes;
};

extern CSkinInfo g_SkinInfo;

#endif //H_CGUISKININFO