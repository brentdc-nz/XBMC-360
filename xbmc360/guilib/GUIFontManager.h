#ifndef GUILIB_FONTMANAGER_H
#define GUILIB_FONTMANAGER_H

#include <string>
#include <vector>
#include "GUIFont.h"

using namespace std;

class GUIFontManager
{
public:
	GUIFontManager(void);
	~GUIFontManager(void);

	void LoadFonts(const CStdString& strFilename);
	CGUIFont* Load(const string& strFontName, const string& strFilename, int iSize, DWORD dwStyles);
	CGUIFont* GetFont(const string& strFontName);
	void Clear();
private:
	vector<CGUIFont*> m_vecFonts;
};

extern GUIFontManager g_fontManager;

#endif //GUILIB_FONTMANAGER_H