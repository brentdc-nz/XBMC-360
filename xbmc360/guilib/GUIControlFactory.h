#ifndef GUI_CONTROL_FACTORY_H
#define GIU_CONTROL_FACTORY_H

#include "GUIControl.h"

class CTextureInfo; // Forward
class CAspectRatio;
class CGUIInfoLabel;
class TiXmlNode;
class CGUIAction;

class CGUIControlFactory
{
public:
	CGUIControlFactory(void);
	virtual ~CGUIControlFactory(void);
	CGUIControl* Create(int parentID, const FRECT &rect, TiXmlElement* pControlNode, bool insideContainer = false);
	void ScaleElement(TiXmlElement *element, RESOLUTION fileRes, RESOLUTION destRes);
	static bool GetFloat(const TiXmlNode* pRootNode, const char* strTag, float& value);

	// Translate from control name to control type
	// param type name of the control
	// return type of control
	static CGUIControl::GUICONTROLTYPES TranslateControlType(const CStdString &type);

	// Translate from control type to control name
	// param type type of the control
	// return name of control
	static CStdString TranslateControlType(CGUIControl::GUICONTROLTYPES type);

	// Grab a dimension out of the XML
	/*  
	Supports plain reading of a number (or constant) and, in addition allows "auto" as the value
	for the dimension, whereby value is set to the max attribute (if it exists) and min is set the min
	attribute (if it exists) or 1.  Auto values are thus detected by min != 0.
	-param pRootNode XML node to read
	-param strTag tag within pRootNode to read
	-param value value to set, or maximum value if using auto
	-param min minimum value - set != 0 if auto is used.
	-return true if we found and read the tag.
	*/
	static bool GetDimension(const TiXmlNode* pRootNode, const char* strTag, float &value, float &min);
	static bool GetUnsigned(const TiXmlNode* pRootNode, const char* strTag, unsigned int& value);
	static bool GetAspectRatio(const TiXmlNode* pRootNode, const char* strTag, CAspectRatio &aspectRatio);
	static bool GetInfoTexture(const TiXmlNode* pRootNode, const char* strTag, CTextureInfo &image, CGUIInfoLabel &info, int parentID);
	static bool GetTexture(const TiXmlNode* pRootNode, const char* strTag, CTextureInfo &image);
	static bool GetAlignment(const TiXmlNode* pRootNode, const char* strTag, uint32_t& dwAlignment);
	static bool GetAlignmentY(const TiXmlNode* pRootNode, const char* strTag, uint32_t& dwAlignment);
	static bool GetAnimations(const TiXmlNode *control, const FRECT &rect, std::vector<CAnimation> &animation);

	// Create an info label from an XML element
	/*	
	Processes XML elements of the form
	<xmltag fallback="fallback_value">info_value</xmltag>
	where info_value may use $INFO[], $LOCALIZE[], $NUMBER[] etc.
	If either the fallback_value or info_value are natural numbers they are interpreted
	as ids for lookup in strings.xml. The fallback attribute is optional.
	-param element XML element to process
	-param infoLabel returned infoLabel
	-return true if a valid info label was read, false otherwise
	*/
	static bool GetInfoLabelFromElement(const TiXmlElement *element, CGUIInfoLabel &infoLabel, int parentID);
	static void GetInfoLabel(const TiXmlNode *pControlNode, const CStdString &labelTag, CGUIInfoLabel &infoLabel, int parentID);
	static void GetInfoLabels(const TiXmlNode *pControlNode, const CStdString &labelTag, std::vector<CGUIInfoLabel> &infoLabels, int parentID);
	static bool GetColor(const TiXmlNode* pRootNode, const char* strTag, color_t &value);
	static bool GetInfoColor(const TiXmlNode* pRootNode, const char* strTag, CGUIInfoColor &value, int parentID);
	static CStdString FilterLabel(const CStdString &label);
	static bool GetConditionalVisibility(const TiXmlNode* control, int &condition);
	static bool GetActions(const TiXmlNode* pRootNode, const char* strTag, CGUIAction& actions);
	static void GetRectFromString(const CStdString &string, FRECT &rect);
	static bool GetHitRect(const TiXmlNode* pRootNode, CRect &rect);

private:
	static CStdString GetType(const TiXmlElement *pControlNode);
	bool GetCondition(const TiXmlNode *control, const char *tag, int &condition);
	static bool GetConditionalVisibility(const TiXmlNode* control, int &condition, CGUIInfoBool &allowHiddenFocus);
	bool GetPath(const TiXmlNode* pRootNode, const char* strTag, CStdString& strStringPath);
	bool GetString(const TiXmlNode* pRootNode, const char* strTag, CStdString& strString);
	bool GetFloatRange(const TiXmlNode* pRootNode, const char* strTag, float& iMinValue, float& iMaxValue, float& iIntervalValue);
	bool GetIntRange(const TiXmlNode* pRootNode, const char* strTag, int& iMinValue, int& iMaxValue, int& iIntervalValue);
};

#endif //GIU_CONTROL_FACTORY_H