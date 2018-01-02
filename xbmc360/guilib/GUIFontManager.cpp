#include "GUIFontManager.h"
#include "tinyxml\tinyxml.h"
#include "..\utils\Log.h"

GUIFontManager g_fontManager;

GUIFontManager::GUIFontManager(void)
{
}

GUIFontManager::~GUIFontManager(void)
{
}

void GUIFontManager::LoadFonts(const CStdString& strFilename)
{
	TiXmlDocument xmlDoc;
	if ( !xmlDoc.LoadFile(strFilename.c_str()) )
	{
		CLog::Log(LOGDEBUG, "Could'nt load font xml: %s\n", strFilename.c_str()); 
		return ;
	}

	CStdString strFontFileName, strFontName;
	int iSize = 20;
	DWORD dwStyle = XUI_FONT_STYLE_NORMAL;

	TiXmlElement* pRootElement =xmlDoc.RootElement();

	string strValue=pRootElement->Value();
	if (strValue!=string("fonts")) return ;

	const TiXmlNode *pChild = pRootElement->FirstChild();
	while(pChild)
	{
		string strValue=pChild->Value();
		if(strValue=="font")
		{  
			const TiXmlNode *pNode = pChild->FirstChild("name");
			if (pNode)
			{
				strFontName=pNode->FirstChild()->Value();

				const TiXmlNode *pNode = pChild->FirstChild("filename");	
				if (pNode)
					strFontFileName=pNode->FirstChild()->Value();

				const TiXmlNode *pNode2 = pChild->FirstChild("size");
				if (pNode2)
				{
					iSize = atol(pNode2->FirstChild()->Value());
					if (iSize <= 0) iSize = 20;
				}

				pNode = pNode->FirstChild("style");
				if(pNode)
				{
					string style = pNode->FirstChild()->Value();
					if (style == "normal")
					dwStyle = XUI_FONT_STYLE_NORMAL;
					else if (style == "bold")
					dwStyle = XUI_FONT_STYLE_BOLD;
					else if (style == "italics")
					dwStyle = XUI_FONT_STYLE_ITALIC;
					else if (style == "bolditalics")
					dwStyle = XUI_FONT_STYLE_BOLD | XUI_FONT_STYLE_ITALIC;
				}
				Load(strFontName, strFontFileName, iSize, dwStyle);
			}
		}
		pChild=pChild->NextSibling();  
	}
}

CGUIFont* GUIFontManager::Load(const string& strFontName, const string& strFilename, int iSize, DWORD dwStyles)
{
	//check if font already exists
	CGUIFont* pFont = GetFont(strFontName);
	if (pFont) return pFont;
 
	CGUIFont* pNewFont = new CGUIFont();
	if (pNewFont->Load(strFontName, strFilename, iSize, dwStyles))
	{
		// font is loaded
		m_vecFonts.push_back(pNewFont);
		return pNewFont;
	}

	delete pNewFont;
	return NULL;
}

CGUIFont* GUIFontManager::GetFont(const string& strFontName)
{
	for (int i=0; i < (int)m_vecFonts.size(); ++i)
	{
		CGUIFont* pFont=m_vecFonts[i];
		if (pFont->GetFontName() == strFontName) return pFont;
	}
	return NULL;
}

void GUIFontManager::Clear()
{
	for (int i=0; i < (int)m_vecFonts.size(); ++i)
	{
		CGUIFont* pFont=m_vecFonts[i];
		delete pFont;
	}
	m_vecFonts.erase(m_vecFonts.begin(),m_vecFonts.end());
}