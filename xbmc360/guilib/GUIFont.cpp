#include "GUIFont.h"
#include "GraphicContext.h"
#include "..\utils\StringUtils.h"

CGUIFont::CGUIFont(void)
{
	m_strFontName = "";
	m_strFontFile = "";
	m_fSize = 0;
	m_Font = NULL;
}

CGUIFont::~CGUIFont(void)
{
}

const CStdString& CGUIFont::GetFontName() const
{
	return m_strFontName;
}

bool CGUIFont::Load(const CStdString& strFontName,const CStdString& strFilename, int iSize, DWORD dwStyles)
{
	m_strFontName = strFontName;
	m_strFontFile = strFilename;
	m_fSize = (float)iSize;

	return true;
}

bool CGUIFont::DrawText( float fPosX, float fPosY, DWORD dwColor, const CStdString strText, DWORD dwFlags/* = XUI_FONT_STYLE_NORMAL*//*, FLOAT fMaxPixelWidth*/ )
{
	//==================================================================
	// FIXME - WTF!!! 
	// Why does CStringUtils::String2WString() Not work for this!!!! 
	// Yet it works when put through the below three bloks of same code.		
	//==================================================================

	char* strTempText1 = (char*)(void*)m_strFontFile.c_str();
	int num1 = lstrlenA(strTempText1) + 1;
	int len1 = MultiByteToWideChar(CP_ACP, 0, strTempText1, num1, 0, 0);
	wchar_t* buf1 = new wchar_t[len1];
	MultiByteToWideChar(CP_ACP, 0, strTempText1, num1, buf1, len1);
	std::wstring r1(buf1);
	delete[] buf1; 
	LPCWSTR resultstrFontFile = r1.c_str();

	char* strTempText2 = (char*)(void*)m_strFontName.c_str();
	int num2 = lstrlenA(strTempText2) + 1;
	int len2 = MultiByteToWideChar(CP_ACP, 0, strTempText2, num2, 0, 0);
	wchar_t* buf2 = new wchar_t[len2];
	MultiByteToWideChar(CP_ACP, 0, strTempText2, num2, buf2, len2);
	std::wstring r2(buf2);
	delete[] buf2; 
	LPCWSTR resultstrFontName = r2.c_str();

	char* strTempText3 = (char*)(void*)strText.c_str();
	int num3 = lstrlenA(strTempText3) + 1;
	int len3 = MultiByteToWideChar(CP_ACP, 0, strTempText3, num3, 0, 0);
	wchar_t* buf3 = new wchar_t[len3];
	MultiByteToWideChar(CP_ACP, 0, strTempText3, num3, buf3, len3);
	std::wstring r3(buf3);
	delete[] buf3; 
	LPCWSTR resultstrText = r3.c_str();

	//==================================================================

	// With XUI don't create the font and store it in vector to render, 
	// just run the below each frame, seems odd but it works...

	if ( !g_graphicsContext.IsFullScreenVideo() )
		g_graphicsContext.Lock();

	XuiRenderBegin( g_graphicsContext.GetXUIDevice(), D3DCOLOR_ARGB(255,0,0,0) );

	TypefaceDescriptor typeface = {0};
	typeface.szTypeface = resultstrFontName;
	typeface.szLocator = resultstrFontFile;
	typeface.szReserved1 = NULL;
	XuiRegisterTypeface( &typeface, TRUE );

	XuiCreateFont( resultstrFontName, m_fSize, dwFlags, 0, &m_Font );

    // Measure the text
	XUIRect clipRect( 0, 0, g_graphicsContext.GetWidth() - fPosX, g_graphicsContext.GetHeight() - fPosY );
    XuiMeasureText( m_Font, resultstrText, -1, dwFlags, 0, &clipRect );

	if(dwFlags & XUI_FONT_STYLE_RIGHT_ALIGN) //HACK: Using XUI to do this should be easy, but it's a pain in the butt...
	{
		clipRect.right = g_graphicsContext.GetWidth() - fPosX + clipRect.GetWidth();
		fPosX = fPosX - clipRect.GetWidth();
	}

	// Set the text position in the device context
	D3DXMATRIX matXForm;
	D3DXMatrixIdentity( &matXForm );
	matXForm._41 = fPosX;
	matXForm._42 = fPosY;
	XuiRenderSetTransform( g_graphicsContext.GetXUIDevice(), &matXForm );

	// Select the font and color into the device context
    XuiSelectFont( g_graphicsContext.GetXUIDevice(), m_Font );
    XuiSetColorFactor( g_graphicsContext.GetXUIDevice(), ( DWORD )dwColor );

    // Set the view
    D3DXMATRIX matView;
    D3DXMatrixIdentity( &matView );
    XuiRenderSetViewTransform( g_graphicsContext.GetXUIDevice(), &matView );

    // Draw the text
    XuiDrawText( g_graphicsContext.GetXUIDevice(), resultstrText, dwFlags, 0, &clipRect );
	
	// release the font
	XuiReleaseFont( m_Font );

	XuiRenderEnd( g_graphicsContext.GetXUIDevice() );
	XuiRenderPresent( g_graphicsContext.GetXUIDevice(), NULL, NULL, NULL );

	if ( !g_graphicsContext.IsFullScreenVideo() )
		g_graphicsContext.Unlock();

	return true;
}