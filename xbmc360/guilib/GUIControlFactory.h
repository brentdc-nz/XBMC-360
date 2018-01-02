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

#ifndef H_CGUICONTROLFACTORY
#define H_CGUICONTROLFACTORY

#include "GUIControl.h"
#include "tinyxml\tinyxml.h"
#include "GUITexture.h"
#include "GUIInfoTypes.h"

#include <vector>

using namespace std;

class CGUIControlFactory
{
public:
	CGUIControlFactory(void);
	virtual ~CGUIControlFactory(void);
	CGUIControl* Create(int parentID, const FRECT &rect, TiXmlElement* pControlNode, bool insideContainer = false);
	static bool GetFloat(const TiXmlNode* pRootNode, const char* strTag, float& value);

   /*
   \param pRootNode XML node to read
   \param strTag tag within pRootNode to read
   \param value value to set, or maximum value if using auto
   \param min minimum value - set != 0 if auto is used.
   \return true if we found and read the tag.
   */
	static bool GetDimension(const TiXmlNode* pRootNode, const char* strTag, float &value, float &min);
	static bool GetInfoTexture(const TiXmlNode* pRootNode, const char* strTag, CTextureInfo &image, CGUIInfoLabel &info, int parentID);
	static bool GetTexture(const TiXmlNode* pRootNode, const char* strTag, CTextureInfo &image);
	static bool GetAlignment(const TiXmlNode* pRootNode, const char* strTag, DWORD& alignment);

	/*! \brief Create an info label from an XML element
	Processes XML elements of the form
    <xmltag fallback="fallback_value">info_value</xmltag>
	where info_value may use $INFO[], $LOCALIZE[], $NUMBER[] etc.
	If either the fallback_value or info_value are natural numbers they are interpreted
	as ids for lookup in strings.xml. The fallback attribute is optional.
	\param element XML element to process
	\param infoLabel returned infoLabel
	\return true if a valid info label was read, false otherwise
	*/
	static bool GetInfoLabelFromElement(const TiXmlElement *element, CGUIInfoLabel &infoLabel, int parentID);
	static void GetInfoLabel(const TiXmlNode *pControlNode, const CStdString &labelTag, CGUIInfoLabel &infoLabel, int parentID);
	static void GetInfoLabels(const TiXmlNode *pControlNode, const CStdString &labelTag, std::vector<CGUIInfoLabel> &infoLabels, int parentID);

	/*! \brief translate from control name to control type
	\param type name of the control
	\return type of control
	*/
	static CGUIControl::GUICONTROLTYPES TranslateControlType(const CStdString &type);

	static bool GetConditionalVisibility(const TiXmlNode* control, int &condition);

private:
	static CStdString GetType(const TiXmlElement *pControlNode);
	bool GetString(const TiXmlNode* pRootNode, const char* strTag, CStdString& strString);
	bool GetMultipleString(const TiXmlNode* pRootNode, const char* strTag, vector<CStdString>& vecStringValue);
};

#endif //H_CGUICONTROLFACTORY