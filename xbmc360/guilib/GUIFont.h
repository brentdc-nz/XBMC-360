#ifndef CGUILIB_GUIFONT_H
#define CGUILIB_GUIFONT_H

#include "..\utils\StdString.h"
#include "..\utils\Stdafx.h"
#include <xui.h>

class CGUIFont
{
public:
	CGUIFont(void);
	virtual ~CGUIFont(void);

	const CStdString GetFontName();
	bool Load(const CStdString& strFontName,const CStdString& strFilename, int iSize, DWORD dwStyles);
	bool Reload(DWORD dwStyles);
	bool DrawText(float fPosX, float fPosY, DWORD dwColor, const CStdString strText, DWORD dwFlags = XUI_FONT_STYLE_NORMAL);
	bool DrawTextWidth(float fPosX, float fPosY, DWORD dwColor, const CStdString strText, float fMaxPixelWidth, DWORD dwFlags = XUI_FONT_STYLE_NORMAL);
	void GetTextExtent(const CStdString strText, float* pWidth, float* pHeight);
	void Release();
	
private:
	std::wstring m_wstrFontName;
	std::wstring m_wstrFontFile;
	float m_fSize;
	DWORD m_dwStyle;
	HXUIFONT m_Font;
};

#endif //CGUILIB_GUIFONT_H