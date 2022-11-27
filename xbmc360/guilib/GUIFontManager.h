#ifndef GUILIB_FONTMANAGER_H
#define GUILIB_FONTMANAGER_H

#include <string>
#include <vector>
#include "GUIFont.h"
#include "GUIFontTTF.h"
#include "tinyxml\tinyxml.h"

// Forward
class CGUIFont;
class CGUIFontTTF;
class TiXmlDocument;
class TiXmlNode;

struct OrigFontInfo
{
   int size;
   float aspect;
   CStdString fontFilePath;
   CStdString fileName;
   RESOLUTION sourceRes;
   bool preserveAspect;
   bool border;
};

class GUIFontManager
{
public:
	GUIFontManager(void);
	virtual ~GUIFontManager(void);

	void Unload(const CStdString& strFontName);
	void LoadFonts(const CStdString& strFontSet);
	CGUIFont* LoadTTF(const CStdString& strFontName, const CStdString& strFilename, color_t textColor, color_t shadowColor, const int iSize, const int iStyle, bool border = false, float lineSpacing = 1.0f, float aspect = 1.0f, RESOLUTION res = INVALID, bool preserveAspect = false);
	CGUIFont* GetFont(const CStdString& strFontName, bool fallback = true);

	void Clear();
	void FreeFontFile(CGUIFontTTF *pFont);

	void ReloadTTFFonts(void);

protected:
	void RescaleFontSizeAndAspect(float *size, float *aspect, RESOLUTION sourceRes, bool preserveAspect) const;
	void LoadFonts(const TiXmlNode* fontNode);
	CGUIFontTTF* GetFontFile(const CStdString& strFontFile);
	bool OpenFontFile(TiXmlDocument& xmlDoc);

	std::vector<CGUIFont*> m_vecFonts;
	std::vector<CGUIFontTTF*> m_vecFontFiles;
	std::vector<OrigFontInfo> m_vecFontInfo;

	bool m_fontsetUnicode;
	RESOLUTION m_skinResolution;
};

extern GUIFontManager g_fontManager;

#endif //GUILIB_FONTMANAGER_H