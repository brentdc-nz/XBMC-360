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

#include "SkinInfo.h"

CSkinInfo g_SkinInfo; // global

CSkinInfo::CSkinInfo()
{
	m_strBaseDir = "";
}

CSkinInfo::~CSkinInfo()
{
}

void CSkinInfo::Load(const CStdString& strSkinDir, bool loadIncludes)
{
	m_strBaseDir = strSkinDir;

	// TODO!
}

bool CSkinInfo::ResolveConstant(const CStdString &constant, float &value) const
{
	return m_includes.ResolveConstant(constant, value);
}

CStdString CSkinInfo::GetSkinPath(const CStdString& strFile, RESOLUTION *res, const CStdString& strBaseDir /* = "" */) const
{
	CStdString strPathToUse = m_strBaseDir;

	if(!strBaseDir.IsEmpty())
		strPathToUse = strBaseDir;

	return strPathToUse + "\\" + strFile;
}