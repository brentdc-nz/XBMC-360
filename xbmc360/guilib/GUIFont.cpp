#include "GUIFont.h"
#include "GraphicContext.h"
#include "..\utils\StringUtils.h"

CGUIFont::CGUIFont(void)
{
	m_wstrFontName = L"";
	m_wstrFontFile = L"";
	m_dwStyle = 0;
	m_fSize = 0;
	m_Font = NULL;
}

CGUIFont::~CGUIFont(void)
{
}

const CStdString CGUIFont::GetFontName()
{
	// Convert back no normal string from wide
	string strFontName( m_wstrFontName.begin(), m_wstrFontName.end() );
	
	return strFontName;
}

bool CGUIFont::Load(const CStdString& strFontName,const CStdString& strFilename, int iSize, DWORD dwStyles)
{
	CStdString strFontPath = g_graphicsContext.GetMediaDir();

#if 1 // Nasty workaround for XUI to work..
	strFontPath.Replace("D:\\", "file://game:/");
#endif

	strFontPath += "fonts/" + strFilename;

	// Convert to wide for our wide members
	CStringUtils::StringtoWString(strFontName, m_wstrFontName);
	CStringUtils::StringtoWString(strFontPath, m_wstrFontFile);

	m_dwStyle = dwStyles;
	m_fSize = (float)iSize;

	TypefaceDescriptor typeface = {0};
	typeface.szTypeface = m_wstrFontName.c_str();
	typeface.szLocator = m_wstrFontFile.c_str();
	typeface.szReserved1 = NULL;
	XuiRegisterTypeface( &typeface, TRUE );

	XuiCreateFont( m_wstrFontName.c_str(), m_fSize, dwStyles, 0, &m_Font );

	return true;
}

bool CGUIFont::Reload(DWORD dwStyles)
{
	TypefaceDescriptor typeface = {0};
	typeface.szTypeface = m_wstrFontName.c_str();
	typeface.szLocator = m_wstrFontFile.c_str();
	typeface.szReserved1 = NULL;
	XuiRegisterTypeface( &typeface, TRUE );

	XuiCreateFont( m_wstrFontName.c_str(), m_fSize, dwStyles, 0, &m_Font );

	return true;
}

bool CGUIFont::DrawText( float fPosX, float fPosY, DWORD dwColor, const CStdString strText, DWORD dwFlags/* = XUI_FONT_STYLE_NORMAL*//*, FLOAT fMaxPixelWidth*/ )
{
	if ( !g_graphicsContext.IsFullScreenVideo() )
		g_graphicsContext.Lock();

	// Convert out text string to wide
	wstring wstrText;
	CStringUtils::StringtoWString(strText, wstrText);

	XuiRenderBegin( g_graphicsContext.GetXUIDevice(), D3DCOLOR_ARGB(255,0,0,0) );

	// If our font syle changed we need to recreate our font..
	// Why does XUI make this a pain to do ugh..
	if(m_dwStyle != dwFlags)
	{
		Release();
		Reload(dwFlags);
		m_dwStyle = dwFlags;
	}

    // Measure the text
	XUIRect clipRect( 0, 0, g_graphicsContext.GetWidth() - fPosX, g_graphicsContext.GetHeight() - fPosY );
	XuiMeasureText( m_Font, wstrText.c_str(), -1, dwFlags, 0, &clipRect );

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
	XuiDrawText( g_graphicsContext.GetXUIDevice(), wstrText.c_str(), dwFlags, 0, &clipRect );
	
	XuiRenderEnd( g_graphicsContext.GetXUIDevice() );
	XuiRenderPresent( g_graphicsContext.GetXUIDevice(), NULL, NULL, NULL );

	if ( !g_graphicsContext.IsFullScreenVideo() )
		g_graphicsContext.Unlock();

	return true;
}

void CGUIFont::Release()
{
	// Release the font
	XuiReleaseFont( m_Font );
	m_Font = NULL;
}