#ifndef CGUILIB_GUIFONT_H
#define CGUILIB_GUIFONT_H

#include "..\utils\StdString.h"
#include "..\utils\stdafx.h"
#include <xui.h>

class CGUIFont
{
public:
	CGUIFont(void);
	virtual ~CGUIFont(void);

	const CStdString& GetFontName() const;
	bool  Load(const CStdString& strFontName,const CStdString& strFilename, int iSize, DWORD dwStyles);
	bool DrawText( float fPosX, float fPosY, DWORD dwColor, const CStdString strText, DWORD dwFlags = XUI_FONT_STYLE_NORMAL/*, FLOAT fMaxPixelWidth*/ );

private:
	CStdString m_strFontName;
	CStdString m_strFontFile;
	float m_fSize;

	HXUIFONT m_Font;
};

#endif //CGUILIB_GUIFONT_H