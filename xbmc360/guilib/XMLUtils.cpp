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

//#include "include.h"
#include "XMLUtils.h"
//#include "URL.h"
//#include "FileSystem/SpecialProtocol.h"
//#include "utils/StringUtils.h"

#include "tinyxml\tinyxml.h"

bool XMLUtils::GetString(const TiXmlNode* pRootNode, const char* strTag, CStdString& strStringValue)
{
	const TiXmlElement* pElement = pRootNode->FirstChildElement(strTag );
	if (!pElement) return false;
		const char* encoded = pElement->Attribute("urlencoded");
	const TiXmlNode* pNode = pElement->FirstChild();
	if (pNode != NULL)
	{
		strStringValue = pNode->Value();
//		if (encoded && stricmp(encoded,"yes") == 0) FIXME - MARTY
//			CURL::Decode(strStringValue);
		return true;
	}
	strStringValue.Empty();
	return false;
}

bool XMLUtils::GetDWORD(const TiXmlNode* pRootNode, const char* strTag, DWORD& dwDWORDValue)
{
	const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
	if (!pNode || !pNode->FirstChild()) return false;
	dwDWORDValue = atol(pNode->FirstChild()->Value());
	return true;
}

bool XMLUtils::GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue)
{
	const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
	if (!pNode || !pNode->FirstChild()) return false;
		iIntValue = atoi(pNode->FirstChild()->Value());
  
	return true;
}

bool XMLUtils::GetInt(const TiXmlNode* pRootNode, const char* strTag, int &value, const int min, const int max)
{
	if (GetInt(pRootNode, strTag, value))
	{
		if (value < min) value = min;
		if (value > max) value = max;
		return true;
	}
	return false;
}

bool XMLUtils::GetHex(const TiXmlNode* pRootNode, const char* strTag, DWORD& dwHexValue)
{
	const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
	if (!pNode || !pNode->FirstChild()) return false;
	sscanf(pNode->FirstChild()->Value(), "%x", &dwHexValue );
	return true;
}

bool XMLUtils::GetBoolean(const TiXmlNode* pRootNode, const char* strTag, bool& bBoolValue)
{
	const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
	if (!pNode || !pNode->FirstChild()) return false;
	CStdString strEnabled = pNode->FirstChild()->Value();
	strEnabled.ToLower();
	if (strEnabled == "off" || strEnabled == "no" || strEnabled == "disabled" || strEnabled == "false" || strEnabled == "0" )
		bBoolValue = false;
	else
	{
		bBoolValue = true;
		if (strEnabled != "on" && strEnabled != "yes" && strEnabled != "enabled" && strEnabled != "true")
			return false; // invalid bool switch - it's probably some other string.
	}
	return true;
}