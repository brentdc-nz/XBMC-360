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

#ifndef H_CXMLUTILS
#define H_CXMLUTILS

#include "../utils/StdString.h"
//#include "../xbmc/xbox/PlatformDefs.h"

// forward
class TiXmlDocument;
class TiXmlNode;

class XMLUtils
{
public:
	static bool GetDWORD(const TiXmlNode* pRootNode, const char* strTag, DWORD& dwDWORDValue);
	static bool GetString(const TiXmlNode* pRootNode, const char* strTag, CStdString& strStringValue);
	static bool GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue);
	static bool GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue, const int min, const int max);
	static bool GetHex(const TiXmlNode* pRootNode, const char* strTag, DWORD& dwHexValue);
	static bool GetBoolean(const TiXmlNode* pRootNode, const char* strTag, bool& bBoolValue);
};

#endif // H_CXMLUTILS