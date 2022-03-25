#include "GUIControlFactory.h"
#include "XMLUtils.h"
#include "SkinInfo.h"
#include "LocalizeStrings.h"
#include "..\utils\StringUtils.h"
#include "GUIImage.h"
#include "GUILabelControl.h"
#include "GUIFontManager.h"
#include "GUIVideoControl.h"
#include "GUIButtonControl.h"
#include "GUIInfoManager.h"
#include "GUISpinControl.h"
#include "GUISpinControlEx.h"
#include "GUIThumbnailPanel.h"
#include "GUIScrollBarControl.h"
#include "GUIListControl.h"
#include "GUIProgressControl.h"

typedef struct
{
	const char* name;
	CGUIControl::GUICONTROLTYPES type;
} ControlMapping;

static const ControlMapping controls[] =
	{{"button",            CGUIControl::GUICONTROL_BUTTON},
	{"checkmark",         CGUIControl::GUICONTROL_CHECKMARK},
	{"fadelabel",         CGUIControl::GUICONTROL_FADELABEL},
	{"image",             CGUIControl::GUICONTROL_IMAGE},
	{"largeimage",        CGUIControl::GUICONTROL_IMAGE},
	{"image",             CGUIControl::GUICONTROL_BORDEREDIMAGE},
	{"label",             CGUIControl::GUICONTROL_LABEL},
	{"label",             CGUIControl::GUICONTROL_LISTLABEL},
	{"group",             CGUIControl::GUICONTROL_GROUP},
	{"group",             CGUIControl::GUICONTROL_LISTGROUP},
	{"progress",          CGUIControl::GUICONTROL_PROGRESS},
	{"radiobutton",       CGUIControl::GUICONTROL_RADIO},
	{"rss",               CGUIControl::GUICONTROL_RSS},
	{"selectbutton",      CGUIControl::GUICONTROL_SELECTBUTTON},
	{"slider",            CGUIControl::GUICONTROL_SLIDER},
	{"sliderex",          CGUIControl::GUICONTROL_SETTINGS_SLIDER},
	{"spincontrol",       CGUIControl::GUICONTROL_SPIN},
	{"spincontrolex",     CGUIControl::GUICONTROL_SPINEX},
	{"textbox",           CGUIControl::GUICONTROL_TEXTBOX},
	{"togglebutton",      CGUIControl::GUICONTROL_TOGGLEBUTTON},
	{"videowindow",       CGUIControl::GUICONTROL_VIDEO},
	{"mover",             CGUIControl::GUICONTROL_MOVER},
	{"resize",            CGUIControl::GUICONTROL_RESIZE},
	{"buttonscroller",    CGUIControl::GUICONTROL_BUTTONBAR},
	{"edit",              CGUIControl::GUICONTROL_EDIT},
	{"visualisation",     CGUIControl::GUICONTROL_VISUALISATION},
	{"karvisualisation",  CGUIControl::GUICONTROL_VISUALISATION},
	{"multiimage",        CGUIControl::GUICONTROL_MULTI_IMAGE},
	{"grouplist",         CGUIControl::GUICONTROL_GROUPLIST},
	{"scrollbar",         CGUIControl::GUICONTROL_SCROLLBAR},
	{"multiselect",       CGUIControl::GUICONTROL_MULTISELECT},
	{"list",              CGUIControl::GUICONTROL_LIST},
	{"wraplist",          CGUIControl::GUICONTAINER_WRAPLIST},
	{"fixedlist",         CGUIControl::GUICONTAINER_FIXEDLIST},
	{"thumbnailpanel",    CGUIControl::GUICONTAINER_THUMBNAILPANEL}};

CGUIControlFactory::CGUIControlFactory(void)
{}

CGUIControlFactory::~CGUIControlFactory(void)
{}

CGUIControl* CGUIControlFactory::Create(int parentID, const FRECT &rect, TiXmlElement* pControlNode, bool insideContainer)
{
	// Resolve any <include> tag's in this control
//	g_SkinInfo.ResolveIncludes(pControlNode);

	// get the control type
	CStdString strType = GetType(pControlNode);
	CGUIControl::GUICONTROLTYPES type = TranslateControlType(strType);

	// Resolve again with strType set so that <default> tags are added
//	g_SkinInfo.ResolveIncludes(pControlNode, strType);

	float posX = 0, posY = 0;
	float width = 0, height = 0;
	float minWidth = 0;

	DWORD dwID, left = 0, right = 0, up = 0, down = 0;

	int pageControl = 0;
	int defaultControl = 0;
	bool  defaultAlways = false;
	CStdString strTmp;
	int singleInfo = 0;
	CStdString strLabel;
	int iUrlSet = 0;
	int iSpace = 2;

	float spinWidth = 16;
	float spinHeight = 16;

	float thumbHeight = 120;
	float thumbWidth = 120;
	float thumbTexHeight = 90;
	float thumbTexWidth = 90;

	float spinPosX = 0, spinPosY = 0;
	float checkWidth = 0, checkHeight = 0;
	CStdString strSubType;
	int iType = SPIN_CONTROL_TYPE_TEXT;
	int iMin = 0;
	int iMax = 100;
	int iInterval = 1;
	float fMin = 0.0f;
	float fMax = 1.0f;
	float fInterval = 0.1f;
	float rMin = 0.0f;
	float rMax = 100.0f;
	CTextureInfo textureBackground, textureMid;
	CTextureInfo /*textureNib, textureNibFocus,*/ textureBar, textureBarFocus;
	CTextureInfo textureUp, textureDown;
	CTextureInfo textureUpFocus, textureDownFocus;
	CTextureInfo texture, borderTexture;
	CGUIInfoLabel textureFile;
	CTextureInfo textureFocus, textureNoFocus;
	FRECT borderSize = { 0, 0, 0, 0};

	float sliderWidth = 150, sliderHeight = 16;

	bool bHasPath = false;
	vector<CStdString> clickActions;
	CStdString strTitle = "";
	CStdString strRSSTags = "";

	DWORD dwBuddyControlID = 0;
	int iNumSlots = 7;
	float buttonGap = 5;
	int iDefaultSlot = 2;
	int iMovementRange = 0;
	bool bHorizontal = false;
	int iAlpha = 0;
	bool bWrapAround = true;
	bool bSmoothScrolling = true;

	int iVisibleCondition = 0;
	int enableCondition = 0;

	bool bScrollLabel = false;
	bool bPulse = true;
	unsigned int timePerImage = 0;
	unsigned int fadeTime = 0;
	unsigned int timeToPauseAtEnd = 0;
	bool randomized = false;
	bool loop = true;
	bool wrapMultiLine = false;
	bool showOnePage = true;
	bool scrollOut = true;
	int preloadItems = 0;

	CLabelInfo labelInfo;

	float radioWidth = 0;
	float radioHeight = 0;
	float radioPosX = 0;
	float radioPosY = 0;

	CStdString altLabel;
	CStdString strLabel2;

	int focusPosition = 0;
	int scrollTime = 200;
	bool useControlCoords = false;
	bool renderFocusedLast = false;

	bool hasCamera = false;
	bool resetOnLabelChange = true;
	bool bPassword = false;

	//
	// Read control properties from XML
	//

	XMLUtils::GetDWORD(pControlNode, "id", dwID);
	// TODO: Perhaps we should check here whether id is valid for focusable controls
	// such as buttons etc.  For labels/fadelabels/images it does not matter

	GetFloat(pControlNode, "posx", posX);
	GetFloat(pControlNode, "posy", posY);

	GetDimension(pControlNode, "width", width, minWidth);
	GetFloat(pControlNode, "height", height);

	if(!XMLUtils::GetDWORD(pControlNode, "onup" , up))
		up = dwID - 1;

	if(!XMLUtils::GetDWORD(pControlNode, "ondown" , down))
		down = dwID + 1;

	if(!XMLUtils::GetDWORD(pControlNode, "onleft" , left ))
		left = dwID;

	if(!XMLUtils::GetDWORD(pControlNode, "onright", right))
		right = dwID;

	GetConditionalVisibility(pControlNode, iVisibleCondition/*, allowHiddenFocus*/);

	XMLUtils::GetHex(pControlNode, "textcolor", labelInfo.dwTextColor);
	XMLUtils::GetHex(pControlNode, "selectedcolor", labelInfo.dwSelectedTextColor);
	XMLUtils::GetHex(pControlNode, "disabledcolor", labelInfo.dwDisabledTextColor);

	GetFloat(pControlNode, "textoffsetx", labelInfo.offsetX);
	GetFloat(pControlNode, "textoffsety", labelInfo.offsetY);

	CStdString strFont;
	if(XMLUtils::GetString(pControlNode, "font", strFont))
		labelInfo.font = g_fontManager.GetFont(strFont);

	GetAlignment(pControlNode, "align", labelInfo.dwAlign);

	DWORD alignY = 0;
	if(GetAlignmentY(pControlNode, "aligny", alignY))
		labelInfo.dwAlign |= alignY;

	GetMultipleString(pControlNode, "onclick", clickActions);

	CStdString infoString;
	if(XMLUtils::GetString(pControlNode, "info", infoString))
		singleInfo = g_infoManager.TranslateString(infoString);

	GetTexture(pControlNode, "texturefocus", textureFocus);
	GetTexture(pControlNode, "texturenofocus", textureNoFocus);
	GetTexture(pControlNode, "textureup", textureUp);
	GetTexture(pControlNode, "texturedown", textureDown);
	GetTexture(pControlNode, "textureupfocus", textureUpFocus);
	GetTexture(pControlNode, "texturedownfocus", textureDownFocus);

	GetFloat(pControlNode, "spinwidth", spinWidth);
	GetFloat(pControlNode, "spinheight", spinHeight);

	GetFloat(pControlNode, "thumbheight", thumbHeight);
	GetFloat(pControlNode, "thumbwidth", thumbWidth);
	GetFloat(pControlNode, "thumbtextureheight", thumbTexHeight);
	GetFloat(pControlNode, "thumbtexturewidth", thumbTexWidth);

	// Scroll bar textures
	GetTexture(pControlNode, "texturesliderbackground", textureBackground);
	GetTexture(pControlNode, "texturesliderbar", textureBar);
	GetTexture(pControlNode, "texturesliderbarfocus", textureBarFocus);
 
	// Progress bar textures
	GetTexture(pControlNode, "texturebg", textureBackground);
	GetTexture(pControlNode, "midtexture", textureMid);

	if(XMLUtils::GetString(pControlNode, "subtype", strSubType))
	{
		strSubType.ToLower();

		if(strSubType == "int")
			iType = SPIN_CONTROL_TYPE_INT;
		else if(strSubType == "float")
			iType = SPIN_CONTROL_TYPE_FLOAT;
		else
			iType = SPIN_CONTROL_TYPE_TEXT;
	}

	// The <texture> tag can be overridden by the <info> tag
	GetInfoTexture(pControlNode, "texture", texture, textureFile, parentID);

	// Fade label can have a whole bunch, but most just have one
	vector<CGUIInfoLabel> infoLabels;
	GetInfoLabels(pControlNode, "label", infoLabels, parentID);

	GetString(pControlNode, "label", strLabel);

	XMLUtils::GetInt(pControlNode,"spacebetweenitems", iSpace);

	CStdString borderStr;
/*	if(XMLUtils::GetString(pControlNode, "bordersize", borderStr))
		GetRectFromString(borderStr, borderSize);
*/
	//
	// Instantiate a new control using the properties gathered above
	//

	CGUIControl *control = NULL;

	if(type == CGUIControl::GUICONTROL_LABEL)
	{
		const CGUIInfoLabel &content = (infoLabels.size()) ? infoLabels[0] : CGUIInfoLabel("");
		if(insideContainer)
		{ 
			// Inside lists we use CGUIListLabel
//			control = new CGUIListLabel(parentID, id, posX, posY, width, height, labelInfo, content, bScrollLabel);
		}
		else
		{
			control = new CGUILabelControl(
			parentID, dwID, posX, posY, width, height,
			labelInfo, wrapMultiLine, bHasPath);

			((CGUILabelControl *)control)->SetInfo(content);
//			((CGUILabelControl *)control)->SetWidthControl(minWidth, bScrollLabel);
		}
	}
	else if(type == CGUIControl::GUICONTROL_VIDEO)
	{
		control = new CGUIVideoControl(parentID, dwID, posX, posY, width, height);
	}
	else if(type == CGUIControl::GUICONTROL_BUTTON)
	{
		control = new CGUIButtonControl(
			parentID, dwID, posX, posY, width, height,
			textureFocus, textureNoFocus,
			labelInfo);

		((CGUIButtonControl *)control)->SetLabel(strLabel);
//		((CGUIButtonControl *)control)->SetLabel2(strLabel2);
		((CGUIButtonControl *)control)->SetClickActions(clickActions);
//		((CGUIButtonControl *)control)->SetFocusActions(focusActions);
		((CGUIButtonControl *)control)->SetNavigation(up, down, left, right);
//		((CGUIButtonControl *)control)->SetUnFocusActions(unfocusActions);
	}
	else if(type == CGUIControl::GUICONTROL_SCROLLBAR)
	{
		control = new CGUIScrollBar(parentID, dwID, posX, posY, width, height,
									textureBackground, textureBar, textureBarFocus);
									//,textureNib, textureNibFocus, orientation, showOnePage);

		((CGUIScrollBar*)control)->SetNavigation(up, down, left, right);
	}
	else if(type == CGUIControl::GUICONTROL_PROGRESS)
	{
		control = new CGUIProgressControl(
			parentID, dwID, posX, posY, width, height,
			textureBackground, textureMid, rMin, rMax);
			
		((CGUIProgressControl *)control)->SetInfo(singleInfo);
	}
	else if(type == CGUIControl::GUICONTROL_IMAGE)
	{
		if (strType == "largeimage")
			texture.useLarge = true;

		// use a bordered texture if we have <bordersize> or <bordertexture> specified.
		if (borderTexture.filename.IsEmpty() && borderStr.IsEmpty())
			control = new CGUIImage(parentID, dwID, posX, posY, width, height, texture);
//		else
//			control = new CGUIBorderedImage(parentID, id, posX, posY, width, height, texture, borderTexture, borderSize);

		((CGUIImage *)control)->SetInfo(textureFile);
//		((CGUIImage *)control)->SetAspectRatio(aspect);
//		((CGUIImage *)control)->SetCrossFade(fadeTime);
	}
	else if(type == CGUIControl::GUICONTROL_LIST)
	{
		control = new CGUIListControl(
			parentID, dwID, posX, posY, width, height,
			spinWidth, spinHeight,
//			strUp, strDown,
//			strUpFocus, strDownFocus,
//			spinInfo, iSpinPosX, iSpinPosY,
			labelInfo, //labelInfo2,
			textureNoFocus, textureFocus);

		((CGUIListControl *)control)->SetNavigation(up, down, left, right);
//		((CGUIListControl *)control)->SetColourDiffuse(dwColorDiffuse);
//		((CGUIListControl *)control)->SetScrollySuffix(strSuffix);
///		((CGUIListControl *)control)->SetVisibleCondition(iVisibleCondition, allowHiddenFocus);
//		((CGUIListControl *)control)->SetAnimations(animations);
//		((CGUIListControl *)control)->SetImageDimensions(dwitemWidth, dwitemHeight);
//		((CGUIListControl *)control)->SetItemHeight(iTextureHeight);
		((CGUIListControl *)control)->SetSpace(iSpace);
///		((CGUIListControl *)control)->SetPulseOnSelect(bPulse);
	}
	else if(type == CGUIControl::GUICONTAINER_THUMBNAILPANEL)
	{
		control = new CGUIThumbnailPanel(parentID, dwID, posX, posY, width, height, thumbWidth, 
										 thumbHeight, thumbTexWidth, thumbTexHeight, 
										 textureFocus, textureNoFocus, labelInfo);

		((CGUIThumbnailPanel*)control)->SetNavigation(up, down, left, right);
	}
	else if(type == CGUIControl::GUICONTROL_SPINEX)
	{
		control = new CGUISpinControlEx(
		parentID, dwID, posX, posY, width, height, spinWidth, spinHeight,
		labelInfo, textureFocus, textureNoFocus, textureUp, textureDown, textureUpFocus,
		textureDownFocus, labelInfo, iType);

		((CGUISpinControlEx*)control)->SetNavigation(up, down, left, right);
//		((CGUISpinControlEx*)control)->SetSpinPosition(spinPosX);
		((CGUISpinControlEx*)control)->SetText(strLabel);
//		((CGUISpinControlEx*)control)->SetReverse(bReverse);
	}

	//
	// Things that apply to all controls
	//

	if(control)
	{
		control->SetVisibleCondition(iVisibleCondition/*, allowHiddenFocus*/);
	}

	// Finsihed
	return control;
}

bool CGUIControlFactory::GetFloat(const TiXmlNode* pRootNode, const char* strTag, float& value)
{
	const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
	
	if(!pNode || !pNode->FirstChild()) return false;

	return g_SkinInfo.ResolveConstant(pNode->FirstChild()->Value(), value);
}

bool CGUIControlFactory::GetDimension(const TiXmlNode *pRootNode, const char* strTag, float &value, float &min)
{
	const TiXmlElement* pNode = pRootNode->FirstChildElement(strTag);
	
	if(!pNode || !pNode->FirstChild()) return false;
	
	if(0 == strnicmp("auto", pNode->FirstChild()->Value(), 4))
	{
		// Auto-width - At least min must be set
		g_SkinInfo.ResolveConstant(pNode->Attribute("max"), value);
		g_SkinInfo.ResolveConstant(pNode->Attribute("min"), min);
		
		if(!min) min = 1;
			return true;
	}
	return g_SkinInfo.ResolveConstant(pNode->FirstChild()->Value(), value);
}

bool CGUIControlFactory::GetInfoTexture(const TiXmlNode* pRootNode, const char* strTag, CTextureInfo &image, CGUIInfoLabel &info, int parentID)
{
	GetTexture(pRootNode, strTag, image);
	image.filename = "";
	GetInfoLabel(pRootNode, strTag, info, parentID);
	
	return true;
}

bool CGUIControlFactory::GetTexture(const TiXmlNode* pRootNode, const char* strTag, CTextureInfo &image)
{
	const TiXmlElement* pNode = pRootNode->FirstChildElement(strTag);

	if(!pNode) return false;
		const char *border = pNode->Attribute("border");

//	if(border)
//		GetRectFromString(border, image.border);

	image.orientation = 0;
	const char *flipX = pNode->Attribute("flipx");

	if(flipX && strcmpi(flipX, "true") == 0) image.orientation = 1;
		const char *flipY = pNode->Attribute("flipy");

	if(flipY && strcmpi(flipY, "true") == 0) image.orientation = 3 - image.orientation;  // Either 3 or 2
		image.diffuse = pNode->Attribute("diffuse");

	const char *background = pNode->Attribute("background");

	if(background && strnicmp(background, "true", 4) == 0)
		image.useLarge = true;

	image.filename = (pNode->FirstChild() && pNode->FirstChild()->ValueStr() != "-") ? pNode->FirstChild()->Value() : "";

	return true;
}

bool CGUIControlFactory::GetAlignment(const TiXmlNode* pRootNode, const char* strTag, DWORD& alignment)
{
	const TiXmlNode* pNode = pRootNode->FirstChild(strTag);
	if(!pNode || !pNode->FirstChild()) return false;

	CStdString strAlign = pNode->FirstChild()->Value();

	if(strAlign == "right" )alignment = XUI_FONT_STYLE_RIGHT_ALIGN;
	else if(strAlign == "center") alignment = XUI_FONT_STYLE_CENTER_ALIGN;
//	else if(strAlign == "justify") alignment = XBFONT_JUSTIFIED; // TODO
	else alignment = NULL;

	return true;
}

bool CGUIControlFactory::GetAlignmentY(const TiXmlNode* pRootNode, const char* strTag, DWORD& alignment)
{
	const TiXmlNode* pNode = pRootNode->FirstChild(strTag );

	if(!pNode || !pNode->FirstChild())
		return false;

	CStdString strAlign = pNode->FirstChild()->Value();
	alignment = 0;

	if(strAlign == "center")
		alignment = XUI_FONT_STYLE_VERTICAL_CENTER;

	return true;
}

bool CGUIControlFactory::GetInfoLabelFromElement(const TiXmlElement *element, CGUIInfoLabel &infoLabel, int parentID)
{
	if(!element || !element->FirstChild())
		return false;

	CStdString label = element->FirstChild()->Value();

	if(label.IsEmpty() || label == "-")
		return false;

	CStdString fallback = element->Attribute("fallback");

	if(CStringUtils::IsNaturalNumber(label))
		label = g_localizeStrings.Get(atoi(label));

	infoLabel.SetLabel(label, fallback, parentID);

	return true;
}

void CGUIControlFactory::GetInfoLabel(const TiXmlNode *pControlNode, const CStdString &labelTag, CGUIInfoLabel &infoLabel, int parentID)
{
	vector<CGUIInfoLabel> labels;
	GetInfoLabels(pControlNode, labelTag, labels, parentID);

	if(labels.size())
		infoLabel = labels[0];
}

void CGUIControlFactory::GetInfoLabels(const TiXmlNode *pControlNode, const CStdString &labelTag, vector<CGUIInfoLabel> &infoLabels, int parentID)
{
	// We can have the following infolabels:
	// 1.  <number>1234</number> -> direct number
	// 2.  <label>number</label> -> lookup in localizestrings
	// 3.  <label fallback="blah">$LOCALIZE(blah) $INFO(blah)</label> -> infolabel with given fallback
	// 4.  <info>ListItem.Album</info> (uses <label> as fallback)
	int labelNumber = 0;

	if(XMLUtils::GetInt(pControlNode, "number", labelNumber))
	{
		CStdString label;
		label.Format("%i", labelNumber);
		infoLabels.push_back(CGUIInfoLabel(label));
		return; // done
	}

	const TiXmlElement *labelNode = pControlNode->FirstChildElement(labelTag);

	while(labelNode)
	{
		CGUIInfoLabel label;
		if (GetInfoLabelFromElement(labelNode, label, parentID))
			infoLabels.push_back(label);
		labelNode = labelNode->NextSiblingElement(labelTag);
	}

	const TiXmlNode *infoNode = pControlNode->FirstChild("info");
	
	if(infoNode)
	{ 
		// <info> nodes override <label>'s (backward compatibility)
		CStdString fallback;

		if(infoLabels.size())
			fallback = infoLabels[0].GetLabel(0);

		infoLabels.clear();
		
		while(infoNode)
		{
			if(infoNode->FirstChild())
			{
				CStdString info;
				info.Format("$INFO[%s]", infoNode->FirstChild()->Value());
				infoLabels.push_back(CGUIInfoLabel(info, fallback, parentID));
			}
			infoNode = infoNode->NextSibling("info");
		}
	}
}

CGUIControl::GUICONTROLTYPES CGUIControlFactory::TranslateControlType(const CStdString &type)
{
	for(unsigned int i = 0; i < sizeof(controls) / sizeof(controls[0]); ++i)
	{
		if(0 == type.CompareNoCase(controls[i].name))
			return controls[i].type;
	}

	return CGUIControl::GUICONTROL_UNKNOWN;
}

bool CGUIControlFactory::GetConditionalVisibility(const TiXmlNode* control, int &condition)
{
	const TiXmlElement* node = control->FirstChildElement("visible");
	if(!node) return false;

	vector<CStdString> conditions;
	while(node)
	{
		// Add to our condition string
		if (!node->NoChildren())
		  conditions.push_back(node->FirstChild()->Value());

		node = node->NextSiblingElement("visible");
	}

	if(!conditions.size())
		return false;

	if(conditions.size() == 1)
		condition = g_infoManager.TranslateString(conditions[0]);
	else
	{
		// Multiple conditions should be anded together
		CStdString conditionString = "[";
		for(unsigned int i = 0; i < conditions.size() - 1; i++)
			conditionString += conditions[i] + "] + [";

		conditionString += conditions[conditions.size() - 1] + "]";
		condition = g_infoManager.TranslateString(conditionString);
	}
	return (condition != 0);
}


CStdString CGUIControlFactory::GetType(const TiXmlElement *pControlNode)
{
	CStdString type;
	const char *szType = pControlNode->Attribute("type");

	if(szType)
		type = szType;
	else // Backward compatibility - Not desired
		XMLUtils::GetString(pControlNode, "type", type);

	return type;
}

bool CGUIControlFactory::GetString(const TiXmlNode* pRootNode, const char *strTag, CStdString &text)
{
	if(!XMLUtils::GetString(pRootNode, strTag, text))
		return false;

	if(text == "-")
		text.Empty();

//	if(StringUtils::IsNaturalNumber(text)) // FIXME
//		text = g_localizeStrings.Get(atoi(text.c_str()));
//	else
//		g_charsetConverter.unknownToUTF8(text);

	return true;
}

bool CGUIControlFactory::GetMultipleString(const TiXmlNode* pRootNode, const char* strTag, vector<CStdString>& vecStringValue)
{
	const TiXmlNode* pNode = pRootNode->FirstChild( strTag );
	if(!pNode) return false;

	vecStringValue.clear();
	bool bFound = false;
	
	while(pNode)
	{
		const TiXmlNode *pChild = pNode->FirstChild();
		if(pChild != NULL)
		{
			vecStringValue.push_back(pChild->Value());
			bFound = true;
		}
		pNode = pNode->NextSibling(strTag);
	}
	return bFound;
}