#include "GUIFontManager.h"
#include "utils\Log.h"
#include "SkinInfo.h"
#include "XMLUtils.h"
#include "utils\StringUtils.h"
#include "URL.h"
#include "utils\URIUtils.h"
#include "filesystem\File.h"
#include "GUIControlFactory.h"

GUIFontManager g_fontManager;

GUIFontManager::GUIFontManager(void)
{
	m_skinResolution = INVALID;
	m_fontsetUnicode = false;
}

GUIFontManager::~GUIFontManager(void)
{
	Clear();
}

void GUIFontManager::Unload(const CStdString& strFontName)
{
	for (vector<CGUIFont*>::iterator iFont = m_vecFonts.begin(); iFont != m_vecFonts.end(); ++iFont)
	{
		if ((*iFont)->GetFontName() == strFontName)
		{
			delete (*iFont);
			m_vecFonts.erase(iFont);
			return;
		}
	}
}

void GUIFontManager::LoadFonts(const CStdString& strFontSet)
{
	TiXmlDocument xmlDoc;

	if (!OpenFontFile(xmlDoc))
		return;

	TiXmlElement* pRootElement = xmlDoc.RootElement();
	const TiXmlNode *pChild = pRootElement->FirstChild();

	// If there are no fontset's defined in the XML (old skin format) run in backward compatibility
	// and ignore the fontset request
	CStdString strValue = pChild->Value();
	if (strValue == "fontset")
	{
		CStdString foundTTF;
		while (pChild)
		{
			strValue = pChild->Value();
			if (strValue == "fontset")
			{
				const char* idAttr = ((TiXmlElement*) pChild)->Attribute("id");
				const char* unicodeAttr = ((TiXmlElement*) pChild)->Attribute("unicode");

				if (foundTTF.IsEmpty() && idAttr != NULL && unicodeAttr != NULL && stricmp(unicodeAttr, "true") == 0)
					foundTTF = idAttr;

				// Check if this is the fontset that we want
				if (idAttr != NULL && stricmp(strFontSet.c_str(), idAttr) == 0)
				{
					m_fontsetUnicode=false;

					// Check if this is the a ttf fontset
					if (unicodeAttr != NULL && stricmp(unicodeAttr, "true") == 0)
						m_fontsetUnicode=true;

					if (m_fontsetUnicode)
					{
						LoadFonts(pChild->FirstChild());
						break;
					}
				}
			}
			pChild = pChild->NextSibling();
		}

		// If no fontset was loaded
		if (pChild == NULL)
		{
			CLog::Log(LOGWARNING, "file doesnt have <fontset> with name '%s', defaulting to first fontset", strFontSet.c_str());

			if (!foundTTF.IsEmpty())
				LoadFonts(foundTTF);
		}
	}
	else
	{
		CLog::Log(LOGERROR, "file doesnt have <fontset> in <fonts>, but rather %s", strValue.c_str());
		return;
	}
}

CGUIFont* GUIFontManager::LoadTTF(const CStdString& strFontName, const CStdString& strFilename, color_t textColor, color_t shadowColor, const int iSize, const int iStyle, bool border, float lineSpacing, float aspect, RESOLUTION sourceRes, bool preserveAspect)
{
	float originalAspect = aspect;

	// Check if font already exists
	CGUIFont* pFont = GetFont(strFontName, false);
	if (pFont)
		return pFont;

	if (sourceRes == INVALID) // No source res specified, so assume the skin res
		sourceRes = m_skinResolution;

	float newSize = (float)iSize;
	RescaleFontSizeAndAspect(&newSize, &aspect, sourceRes, preserveAspect);

	CStdString strPath;
	if (!CURL::IsFullPath(strFilename))
	{
		strPath = URIUtils::AddFileToFolder(g_graphicsContext.GetMediaDir(), "fonts");
		strPath = URIUtils::AddFileToFolder(strPath, strFilename);
	}
	else
		strPath = strFilename;

	// Check if the file exists, otherwise try loading it from the global media dir
	if (!XFILE::CFile::Exists(strPath))
	{
		strPath = URIUtils::AddFileToFolder("D:\\media\\Fonts", URIUtils::GetFileName(strFilename));
	}

	// Check if we already have this font file loaded (font object could differ only by color or style)
	CStdString TTFfontName;
	TTFfontName.Format("%s_%f_%f%s", strFilename, newSize, aspect, border ? "_border" : "");
	CGUIFontTTF* pFontFile = GetFontFile(TTFfontName);
	
	if (!pFontFile)
	{
		pFontFile = new CGUIFontTTF(TTFfontName);
		bool bFontLoaded = pFontFile->Load(strPath, newSize, aspect, 1.0f, border);

		if (!bFontLoaded)
		{
			// Now try to load it from media\fonts
			if (strFilename[1] != ':')
			{
				strPath = "D:\\media\\Fonts\\";
				strPath += strFilename;
			}

			bFontLoaded = pFontFile->Load(strPath, newSize, aspect);
		}

		if (!bFontLoaded)
		{
			delete pFontFile;

			// Font could not be loaded - try Arial.ttf, which we distribute
			if (strFilename != "arial.ttf")
			{
				CLog::Log(LOGERROR, "Couldn't load font name: %s(%s), trying to substitute arial.ttf", strFontName.c_str(), strFilename.c_str());
				return LoadTTF(strFontName, "arial.ttf", textColor, shadowColor, iSize, iStyle, border, lineSpacing, originalAspect);
			}

			CLog::Log(LOGERROR, "Couldn't load font name:%s file:%s", strFontName.c_str(), strPath.c_str());

			return NULL;
		}
		m_vecFontFiles.push_back(pFontFile);
	}

	// Font file is loaded, create our CGUIFont
	CGUIFont *pNewFont = new CGUIFont(strFontName, iStyle, textColor, shadowColor, lineSpacing, pFontFile);
	m_vecFonts.push_back(pNewFont);

	// Store the original TTF font info in case we need to reload it in a different resolution
	OrigFontInfo fontInfo;
	fontInfo.size = iSize;
	fontInfo.aspect = originalAspect;
	fontInfo.fontFilePath = strPath;
	fontInfo.fileName = strFilename;
	fontInfo.sourceRes = sourceRes;
	fontInfo.preserveAspect = preserveAspect;
	fontInfo.border = border;
	m_vecFontInfo.push_back(fontInfo);

	return pNewFont;
}

CGUIFont* GUIFontManager::GetFont(const CStdString& strFontName, bool fallback /*= true*/)
{
	for (int i = 0; i < (int)m_vecFonts.size(); ++i)
	{
		CGUIFont* pFont = m_vecFonts[i];
		if (pFont->GetFontName() == strFontName)
			return pFont;
	}

	// Fall back to "font13" if we have none
	if (fallback && !strFontName.IsEmpty() && !strFontName.Equals("-") && !strFontName.Equals("font13"))
		return GetFont("font13");
	
	return NULL;
}

void GUIFontManager::RescaleFontSizeAndAspect(float *size, float *aspect, RESOLUTION sourceRes, bool preserveAspect) const
{
	// Set scaling resolution so that we can scale our font sizes correctly
	// as fonts aren't scaled at render time (due to aliasing) we must scale
	// the size of the fonts before they are drawn to bitmaps
	g_graphicsContext.SetScalingResolution(sourceRes, true);

	if (preserveAspect)
	{
		// Font always displayed in the aspect specified by the aspect parameter
		*aspect /= g_graphicsContext.GetPixelRatio(g_graphicsContext.GetVideoResolution());
	}
	else
	{
		// Font streched like the rest of the UI, aspect parameter being the original aspect

		// Adjust aspect ratio
		if (sourceRes == PAL_16x9 || sourceRes == PAL60_16x9 || sourceRes == NTSC_16x9 || sourceRes == HDTV_480p_16x9)
			*aspect *= 0.75f;

		*aspect *= g_graphicsContext.GetGUIScaleY() / g_graphicsContext.GetGUIScaleX();
	}

	*size /= g_graphicsContext.GetGUIScaleY();
}

void GUIFontManager::LoadFonts(const TiXmlNode* fontNode)
{
	while (fontNode)
	{
		CStdString strValue = fontNode->Value();
		if (strValue == "font")
		{
			const TiXmlNode *pNode = fontNode->FirstChild("name");
			if (pNode)
			{
				CStdString strFontName = pNode->FirstChild()->Value();
				color_t shadowColor = 0;
				color_t textColor = 0;
				CGUIControlFactory::GetColor(fontNode, "shadow", shadowColor);
				CGUIControlFactory::GetColor(fontNode, "color", textColor);

				const TiXmlNode *pNode = fontNode->FirstChild("filename");
				if (pNode)
				{
					CStdString strFontFileName = pNode->FirstChild()->Value();
					if (strFontFileName.Find(".ttf") >= 0)
					{
						int iSize = 20;
						int iStyle = FONT_STYLE_NORMAL;
						float aspect = 1.0f;
						float lineSpacing = 1.0f;

						XMLUtils::GetInt(fontNode, "size", iSize);
						if (iSize <= 0) iSize = 20;

						pNode = fontNode->FirstChild("style");
						if (pNode && pNode->FirstChild())
						{
							vector<string> styles = CStringUtils::Split(pNode->FirstChild()->ValueStr(), " ");
							for (vector<string>::iterator i = styles.begin(); i != styles.end(); ++i)
							{
								if (*i == "bold")
									iStyle |= FONT_STYLE_BOLD;
								else if (*i == "italics")
									iStyle |= FONT_STYLE_ITALICS;
								else if (*i == "bolditalics") // backward compatibility
									iStyle |= (FONT_STYLE_BOLD | FONT_STYLE_ITALICS);
								else if (*i == "uppercase")
									iStyle |= FONT_STYLE_UPPERCASE;
								else if (*i == "lowercase")
									iStyle |= FONT_STYLE_LOWERCASE;
							}
						}

						XMLUtils::GetFloat(fontNode, "linespacing", lineSpacing);
						XMLUtils::GetFloat(fontNode, "aspect", aspect);

						LoadTTF(strFontName, strFontFileName, textColor, shadowColor, iSize, iStyle, false, lineSpacing, aspect);
					}
				}
			}
		}
		fontNode = fontNode->NextSibling();
	}
}

bool GUIFontManager::OpenFontFile(TiXmlDocument& xmlDoc)
{
	// Get the file to load fonts from:
	CStdString strPath = g_SkinInfo.GetSkinPath("Fonts.xml", &m_skinResolution);
	CLog::Log(LOGINFO, "Loading fonts from %s", strPath.c_str());

	// First try our preferred file
	if ( !xmlDoc.LoadFile(strPath) )
	{
		CLog::Log(LOGERROR, "Couldn't load %s", strPath.c_str());
		return false;
	}
	
	TiXmlElement* pRootElement = xmlDoc.RootElement();

	CStdString strValue = pRootElement->Value();
	if (strValue != CStdString("fonts"))
	{
		CLog::Log(LOGERROR, "file %s doesnt start with <fonts>", strPath.c_str());
		return false;
	}

	return true;
}

CGUIFontTTF* GUIFontManager::GetFontFile(const CStdString& strFileName)
{
	for (int i = 0; i < (int)m_vecFontFiles.size(); ++i)
	{
		CGUIFontTTF* pFont = m_vecFontFiles[i];
		if (pFont->GetFileName() == strFileName)
			return pFont;
	}
	return NULL;
}

void GUIFontManager::Clear()
{
	for (int i = 0; i < (int)m_vecFonts.size(); ++i)
	{
		CGUIFont* pFont = m_vecFonts[i];
		delete pFont;
	}

	m_vecFonts.clear();
	m_vecFontFiles.clear();
	m_vecFontInfo.clear();
	m_fontsetUnicode=false;
}

void GUIFontManager::FreeFontFile(CGUIFontTTF *pFont)
{
	for (vector<CGUIFontTTF*>::iterator it = m_vecFontFiles.begin(); it != m_vecFontFiles.end(); ++it)
	{
		if (pFont == *it)
		{
			m_vecFontFiles.erase(it);
			delete pFont;
			return;
		}
	}
}

void GUIFontManager::ReloadTTFFonts(void)
{
	if (!m_vecFonts.size())
		return; // We haven't even loaded fonts in yet

	for (unsigned int i = 0; i < m_vecFonts.size(); i++)
	{
		CGUIFont* font = m_vecFonts[i];
		OrigFontInfo fontInfo = m_vecFontInfo[i];
		CGUIFontTTF* currentFontTTF = font->GetFont();

		float aspect = fontInfo.aspect;
		float newSize = (float)fontInfo.size;
		CStdString& strPath = fontInfo.fontFilePath;
		CStdString& strFilename = fontInfo.fileName;

		RescaleFontSizeAndAspect(&newSize, &aspect, fontInfo.sourceRes, fontInfo.preserveAspect);

		// Check if we already have this font file loaded (font object could differ only by color or style)
		CStdString TTFfontName;
		TTFfontName.Format("%s_%f_%f%s", strFilename, newSize, aspect, fontInfo.border ? "_border" : "");

		CGUIFontTTF* pFontFile = GetFontFile(TTFfontName);
		if (!pFontFile)
		{
			pFontFile = new CGUIFontTTF(TTFfontName);
			bool bFontLoaded = pFontFile->Load(strPath, newSize, aspect);
			pFontFile->CopyReferenceCountFrom(*currentFontTTF);

			if (!bFontLoaded)
			{
				delete pFontFile;
				// Font could not b loaded
				CLog::Log(LOGERROR, "Couldn't re-load font file:%s", strPath.c_str());
				return;
			}

			m_vecFontFiles.push_back(pFontFile);
		}

		font->SetFont(pFontFile);
	}
}