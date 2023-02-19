#include "GUIFontTTF.h"
#include "utils\MathUtils.h"
#include "utils\Log.h"
#include "GUIFontManager.h"

// Stuff for FreeType
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H

using namespace std;

#define ROUND(x) (float)(MathUtils::round_int(x))

#define ROUND_TO_PIXEL(x) (float)(MathUtils::round_int(x)) - 0.5f
#define TRUNC_TO_PIXEL(x) (float)(MathUtils::truncate_int(x)) - 0.5f

#define CHARS_PER_TEXTURE_LINE 20 // Number of characters to cache per texture line
#define CHAR_CHUNK    64          // 64 chars allocated at a time (1024 bytes)

int CGUIFontTTF::justification_word_weight = 6; // Weight of word spacing over letter spacing when justifying.
                                                // A larger number means more of the "dead space" is placed between
                                                // words rather than between letters.

unsigned int CGUIFontTTF::max_texture_size = 4096; // Max texture size - 4096 for xbox

class CFreeTypeLibrary
{
public:
	CFreeTypeLibrary()
	 {
		m_library = NULL;
	}

	virtual ~CFreeTypeLibrary()
	{
		if (m_library)
			FT_Done_FreeType(m_library);
	}

	FT_Face GetFont(const CStdString &filename, float size, float aspect)
	{
		// Don't have it yet - Create it
		if (!m_library)
			FT_Init_FreeType(&m_library);

		if (!m_library)
		{
			CLog::Log(LOGERROR, "Unable to initialize freetype library");
			return NULL;
		}

		FT_Face face;

		// Ok, now load the font face
		if (FT_New_Face( m_library, filename.c_str(), 0, &face ))
			return NULL;

		unsigned int ydpi = GetDPI();
		unsigned int xdpi = (unsigned int)ROUND(ydpi * aspect);

		// We set our screen res currently to 96dpi in both directions (windows default)
		// we cache our characters (for rendering speed) so it's probably
		// not a good idea to allow free scaling of fonts - rather, just
		// scaling to pixel ratio on screen perhaps?
		if (FT_Set_Char_Size( face, 0, (int)(size*64 + 0.5f), xdpi, ydpi ))
		{
			FT_Done_Face(face);
			return NULL;
		}

		return face;
	}

	FT_Stroker GetStroker()
	{
		if (!m_library)
			return NULL;

		FT_Stroker stroker;

		if (FT_Stroker_New(m_library, &stroker))
			return NULL;

		return stroker;
	};

	void ReleaseFont(FT_Face face)
	{
		assert(face);
		FT_Done_Face(face);
	};

	void ReleaseStroker(FT_Stroker stroker)
	{
		assert(stroker);
		FT_Stroker_Done(stroker);
	}

	unsigned int GetDPI() const
	{
		return 72; // Default DPI, matches what XPR fonts used to use for sizing
	};

private:
	FT_Library   m_library;
};

CFreeTypeLibrary g_freeTypeLibrary; // Our FreeType library

//-----------------------------------------------------------------------

namespace D3DFontShaders
{
// Vertex shader
const char* g_strVertexShader =
    " float4x4 matWVP : register(c0);              "
    "                                              "
    " struct VS_IN                                 "
    " {                                            "
    "     float4 ObjPos   : POSITION;              "
    "     float2 TexCoord : TEXCOORD;              "
    " };                                           "
    "                                              "
    " struct VS_OUT                                "
    " {                                            "
    "     float4 ProjPos  : POSITION;              "
    "     float2 TexCoord : TEXCOORD;              "
    " };                                           "
    "                                              "
    " VS_OUT main( VS_IN In )                      "
    " {                                            "
    "     VS_OUT Out;                              "
    "     Out.ProjPos = mul( matWVP, In.ObjPos );  "
    "     Out.TexCoord = In.TexCoord;              "
    "     return Out;                              "
    " }                                            ";

// Pixel shader
const char* g_strPixelShader =
    " struct PS_IN                                 "
    " {                                            "
    "     float2 TexCoord : TEXCOORD;              "
    " };                                           "
    "                                              "
    " float4 InputColor : register(c10);           "
	" sampler2D InputTexture : register(S0);       "
    "                                              "
    " float4 main(float2 uv : TEXCOORD) : COLOR    "
    " {                                            "
    "    float4 originalColor = tex2D( InputTexture, uv.xy);"
    "    return float4(InputColor[0], InputColor[1], InputColor[2], originalColor.a * InputColor[3]);"
    " }                                            ";
}

//-----------------------------------------------------------------------

CGUIFontTTF::CGUIFontTTF(const CStdString& strFileName)
{
	m_texture = NULL;
	m_char = NULL;
	m_maxChars = 0;
	m_nestedBeginCount = 0;
	m_face = NULL;
	m_stroker = NULL;
	memset(m_charquick, 0, sizeof(m_charquick));
	m_strFileName = strFileName;
	m_referenceCount = 0;

	m_numCharactersRendered = 0;
}

CGUIFontTTF::~CGUIFontTTF(void)
{
	Clear();
}

void CGUIFontTTF::Clear()
{
	if (m_texture)
	{
		g_graphicsContext.TLock();
		m_texture->Release();
		g_graphicsContext.TUnlock();
	}

	m_texture = NULL;
	
	if (m_pVertexDecl)
	{
		g_graphicsContext.TLock();
		m_pVertexDecl->Release();
		g_graphicsContext.TUnlock();
	}

	m_pVertexDecl = NULL;

	if (m_pVertexShader)
	{
		g_graphicsContext.TLock();
		m_pVertexShader->Release();
		g_graphicsContext.TUnlock();
	}

	m_pVertexShader = NULL;

	if (m_pPixelShader)
	{
		g_graphicsContext.TLock();	
		m_pPixelShader->Release();
		g_graphicsContext.TUnlock();
	}

	m_pPixelShader = NULL;

	if (m_char)
		delete[] m_char;
	
	memset(m_charquick, 0, sizeof(m_charquick));
	m_char = NULL;
	m_maxChars = 0;
	m_numChars = 0;
	m_posX = 0;
	m_posY = 0;
	m_nestedBeginCount = 0;

	if (m_face)
		g_freeTypeLibrary.ReleaseFont(m_face);
	
	m_face = NULL;
	
	if (m_stroker)
		g_freeTypeLibrary.ReleaseStroker(m_stroker);
	
	m_stroker = NULL;
}

bool CGUIFontTTF::Load(const CStdString& strFilename, float height, float aspect, float lineSpacing, bool border)
{
	// Create our character texture + font shader
	m_pD3DDevice = g_graphicsContext.Get3DDevice();

	// We now know that this object is unique - only the GUIFont objects are non-unique, so no need
	// for reference tracking these fonts
	m_face = g_freeTypeLibrary.GetFont(strFilename, height, aspect); //FIXME

	if (!m_face)
		return false;

	if (border)
	{
		m_stroker = g_freeTypeLibrary.GetStroker();

		FT_Pos strength = FT_MulFix( m_face->units_per_EM, m_face->size->metrics.y_scale) / 16;
		if (strength < 128) strength = 128;

		if (m_stroker)
			FT_Stroker_Set(m_stroker, strength, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}

	// Grab the maximum cell height and width
	unsigned int m_cellWidth = m_face->bbox.xMax - m_face->bbox.xMin;
	m_cellHeight = m_face->bbox.yMax - m_face->bbox.yMin;
	m_cellBaseLine = m_face->bbox.yMax;

	unsigned int ydpi = g_freeTypeLibrary.GetDPI();
	unsigned int xdpi = (unsigned int)ROUND(ydpi * aspect);

	m_cellWidth *= (unsigned int)(height * xdpi);
	m_cellWidth /= (72 * m_face->units_per_EM);

	m_cellHeight *= (unsigned int)(height * ydpi);
	m_cellHeight /= (72 * m_face->units_per_EM);

	m_cellBaseLine *= (unsigned int)(height * ydpi);
	m_cellBaseLine /= (72 * m_face->units_per_EM);

	// Increment for good measure to give space in our texture
	m_cellWidth++;
	m_cellHeight+=2;
	m_cellBaseLine++;

	CLog::Log(LOGDEBUG, "%s Scaled size of font %s (%f): width = %i, height = %i, lineheight = %li",
		__FUNCTION__, strFilename.c_str(), height, m_cellWidth, m_cellHeight, m_face->size->metrics.height / 64);

	m_height = height;

	if (m_texture)
		m_texture->Release();

	m_texture = NULL;

	if (m_char)
		delete[] m_char;

	m_char = NULL;

	m_maxChars = 0;
	m_numChars = 0;

	m_strFilename = strFilename;

	m_textureHeight = 0;
	m_textureWidth = ((m_cellHeight * CHARS_PER_TEXTURE_LINE) & ~63) + 64;
	if (m_textureWidth > max_texture_size) m_textureWidth = max_texture_size;

	// Set the posX and posY so that our texture will be created on first character write
	m_posX = m_textureWidth;
	m_posY = -(int)m_cellHeight;

	// Cache the ellipses width
	Character *ellipse = GetCharacter(L'.');
	if (ellipse) m_ellipsesWidth = ellipse->advance;

	//-------------------------------------------------------------------------------------------------

	// Compile vertex shader
	ID3DXBuffer* pVertexShaderCode;
   
	D3DXCompileShader( D3DFontShaders::g_strVertexShader,
                                    ( UINT )strlen( D3DFontShaders::g_strVertexShader ),
                                    NULL,
                                    NULL,
                                    "main",
                                    "vs_2_0",
                                    0,
                                    &pVertexShaderCode,
                                    NULL,
                                    NULL );

    // Create vertex shader
    m_pD3DDevice->CreateVertexShader( ( DWORD* )pVertexShaderCode->GetBufferPointer(),
                                      &m_pVertexShader );

	// Compile pixel shader
	ID3DXBuffer* pPixelShaderCode;

	D3DXCompileShader( D3DFontShaders::g_strPixelShader,
                            ( UINT )strlen( D3DFontShaders::g_strPixelShader ),
                            NULL,
                            NULL,
                            "main",
                            "ps_2_0",
                            0,
                            &pPixelShaderCode,
                            NULL,
                            NULL );

	// Create pixel shader
	m_pD3DDevice->CreatePixelShader( ( DWORD* )pPixelShaderCode->GetBufferPointer(),
                                     &m_pPixelShader );

    // Define the vertex elements and
    // Create a vertex declaration from the element descriptions
    static const D3DVERTEXELEMENT9 VertexElements[3] =
    {
        { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    m_pD3DDevice->CreateVertexDeclaration( VertexElements, &m_pVertexDecl );

	if(pVertexShaderCode)
		pVertexShaderCode->Release();

	if(pPixelShaderCode)
		pPixelShaderCode->Release();

	return true;
}

void CGUIFontTTF::Begin()
{
	if( !m_pD3DDevice || !m_pVertexShader || !m_pPixelShader)
	{
		CLog::Log(LOGERROR, "Direct3D rendering objects missing!");
		return;
	}

	g_graphicsContext.TLock();
	// Pass matrix into the vertex shader
	m_pD3DDevice->SetVertexShaderConstantF(0, (FLOAT*)&g_graphicsContext.GetFinalMatrix(), 4);
	g_graphicsContext.TUnlock();

	if (m_nestedBeginCount == 0)
	{
		// Just have to blit from our texture.
		g_graphicsContext.TLock();
		m_pD3DDevice->SetTexture( 0, m_texture );
		g_graphicsContext.TUnlock();

		g_graphicsContext.TLock();
		m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		g_graphicsContext.TUnlock();
		g_graphicsContext.TLock();
		m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		g_graphicsContext.TUnlock();

		g_graphicsContext.TLock();
		m_pD3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		g_graphicsContext.TUnlock();
	}

	// Keep track of the nested begin/end calls.
	m_nestedBeginCount++;
}

void CGUIFontTTF::End()
{
	if (m_nestedBeginCount == 0)
		return;

	if (--m_nestedBeginCount > 0)
		return;

	g_graphicsContext.TLock();
	m_pD3DDevice->SetTexture(0, NULL);
	g_graphicsContext.TUnlock();

	m_numCharactersRendered = 0;
}

void CGUIFontTTF::RenderCharacter(float posX, float posY, const Character *ch, D3DCOLOR dwColor, bool roundX)
{
	// Actual image width isn't same as the character width as that is
	// just baseline width and height should include the descent
	const float width = ch->right - ch->left;
	const float height = ch->bottom - ch->top;

	// posX and posY are relative to our origin, and the textcell is offset
	// from our (posX, posY).  Plus, these are unscaled quantities compared to the underlying GUI resolution
	CRect vertex((posX + ch->offsetX) * g_graphicsContext.GetGUIScaleX(),
				(posY + ch->offsetY) * g_graphicsContext.GetGUIScaleY(),
				(posX + ch->offsetX + width) * g_graphicsContext.GetGUIScaleX(),
				(posY + ch->offsetY + height) * g_graphicsContext.GetGUIScaleY());

	vertex += CPoint(m_originX, m_originY);
	CRect texture(ch->left, ch->top, ch->right, ch->bottom);
	g_graphicsContext.ClipRect(vertex, texture);

	// Transform our positions - note, no scaling due to GUI calibration/resolution occurs
	float x[4];

	x[0] = g_graphicsContext.ScaleFinalXCoord(vertex.x1, vertex.y1);
	x[1] = g_graphicsContext.ScaleFinalXCoord(vertex.x2, vertex.y1);
	x[2] = g_graphicsContext.ScaleFinalXCoord(vertex.x2, vertex.y2);
	x[3] = g_graphicsContext.ScaleFinalXCoord(vertex.x1, vertex.y2);

	if (roundX)
	{
		// We only round the "left" side of the character, and then use the direction of rounding to
		// move the "right" side of the character.  This ensures that a constant width is kept when rendering
		// the same letter at the same size at different places of the screen, avoiding the problem
		// of the "left" side rounding one way while the "right" side rounds the other way, thus getting
		// altering the width of thin characters substantially.  This only really works for positive
		// coordinates (due to the direction of truncation for negatives) but this is the only case that
		// really interests us anyway.
		float rx0 = ROUND_TO_PIXEL(x[0]);
		float rx3 = ROUND_TO_PIXEL(x[3]);

		x[1] = TRUNC_TO_PIXEL(x[1]);
		x[2] = TRUNC_TO_PIXEL(x[2]);

		if (rx0 > x[0])
			x[1] += 1;

		if (rx3 > x[3])
			x[2] += 1;

		x[0] = rx0;
		x[3] = rx3;
	}

	float y1 = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalYCoord(vertex.x1, vertex.y1));
	float z1 = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalZCoord(vertex.x1, vertex.y1));

	float y2 = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalYCoord(vertex.x2, vertex.y1));
	float z2 = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalZCoord(vertex.x2, vertex.y1));

	float y3 = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalYCoord(vertex.x2, vertex.y2));
	float z3 = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalZCoord(vertex.x2, vertex.y2));

	float y4 = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalYCoord(vertex.x1, vertex.y2));
	float z4 = ROUND_TO_PIXEL(g_graphicsContext.ScaleFinalZCoord(vertex.x1, vertex.y2));

	g_graphicsContext.TLock();

#pragma warning(push)
#pragma warning (disable:4244) // Not an issue here

	// Get the color and parse into the pixel shader
	float fInputColor[4] = { ((dwColor >> 16) & 0xFF) / 255.0, ((dwColor >> 8) & 0xFF) / 255.0, ((dwColor) & 0xFF) / 255.0, (dwColor >> 24 & 0xFF) / 255.0};
	m_pD3DDevice->SetPixelShaderConstantF(10, fInputColor, 4);

#pragma warning(pop)

	g_graphicsContext.TUnlock();

	m_numCharactersRendered++;

	// Tex coords converted to 0..1 range
	float tl = texture.x1 / m_textureWidth;
	float tr = texture.x2 / m_textureWidth;
	float tt = texture.y1 / m_textureHeight;
	float tb = texture.y2 / m_textureHeight;

	CUSTOMVERTEX Vertices[] =
	{
		{ x[0], y1, z1, tl, tt },
		{ x[1], y2, z2, tr, tt },
		{ x[2], y3, z3, tr, tb },
		{ x[3], y4, z4, tl, tb }
	};

	g_graphicsContext.TLock();
	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	g_graphicsContext.Get3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pD3DDevice->SetVertexDeclaration( m_pVertexDecl );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pD3DDevice->SetVertexShader( m_pVertexShader );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pD3DDevice->SetPixelShader( m_pPixelShader );
	g_graphicsContext.TUnlock();

	g_graphicsContext.TLock();
	m_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Vertices, sizeof(CUSTOMVERTEX));
	g_graphicsContext.TUnlock();
}

CGUIFontTTF::Character* CGUIFontTTF::GetCharacter(character_t chr)
{
	wchar_t letter = (wchar_t)(chr & 0xffff);
	character_t style = (chr & 0x3000000) >> 24;

	// Ignore linebreaks
	if (letter == L'\r')
		return NULL;

	// Quick access to ascii chars
	if (letter < 255)
	{
		character_t ch = (style << 8) | letter;
		if (m_charquick[ch])
			return m_charquick[ch];
	}

	// Letters are stored based on style and letter
	character_t ch = (style << 16) | letter;

	int low = 0;
	int high = m_numChars - 1;
	int mid;
	
	while (low <= high)
	{
		mid = (low + high) >> 1;
		if (ch > m_char[mid].letterAndStyle)
			low = mid + 1;
		else if (ch < m_char[mid].letterAndStyle)
			high = mid - 1;
		else
			return &m_char[mid];
	}

	// If we get to here, then low is where we should insert the new character

	// Increase the size of the buffer if we need it
	if (m_numChars >= m_maxChars)
	{
		// Need to increase the size of the buffer
		Character *newTable = new Character[m_maxChars + CHAR_CHUNK];
		if (m_char)
		{
			memcpy(newTable, m_char, low * sizeof(Character));
			memcpy(newTable + low + 1, m_char + low, (m_numChars - low) * sizeof(Character));
			delete[] m_char;
		}
		m_char = newTable;
		m_maxChars += CHAR_CHUNK;
	}
	else
	{
		// Just move the data along as necessary
		memmove(m_char + low + 1, m_char + low, (m_numChars - low) * sizeof(Character));
	}

	// Render the character to our texture
	// must End() as we can't render text to our texture during a Begin(), End() block
	unsigned int nestedBeginCount = m_nestedBeginCount;
	m_nestedBeginCount = 1;
	
	if (nestedBeginCount) End();
	
	if (!CacheCharacter(letter, style, m_char + low))
	{
		// Unable to cache character - try clearing them all out and starting over
		CLog::Log(LOGDEBUG, "GUIFontTTF::GetCharacter: Unable to cache character.  Clearing character cache of %i characters", m_numChars);
		ClearCharacterCache();
		low = 0;
		
		if (!CacheCharacter(letter, style, m_char + low))
		{
			CLog::Log(LOGERROR, "GUIFontTTF::GetCharacter: Unable to cache character (out of memory?)");
			if (nestedBeginCount) Begin();
			m_nestedBeginCount = nestedBeginCount;
			return NULL;
		}
	}
	if (nestedBeginCount) Begin();
	m_nestedBeginCount = nestedBeginCount;

	// Fixup quick access
	memset(m_charquick, 0, sizeof(m_charquick));
	for(int i=0;i<m_numChars;i++)
	{
		if ((m_char[i].letterAndStyle & 0xffff) < 255)
		{
			character_t ch = ((m_char[i].letterAndStyle & 0xffff0000) >> 8) | (m_char[i].letterAndStyle & 0xff);
			m_charquick[ch] = m_char+i;
		}
	}

	return m_char + low;
}

bool CGUIFontTTF::CacheCharacter(wchar_t letter, uint32_t style, Character *ch)
{
	int glyph_index = FT_Get_Char_Index( m_face, letter );

	FT_Glyph glyph = NULL;
	if (FT_Load_Glyph( m_face, glyph_index, FT_LOAD_TARGET_LIGHT ))
	{
		CLog::Log(LOGDEBUG, "%s Failed to load glyph %x", __FUNCTION__, letter);
		return false;
	}

	// Make bold if applicable
	if (style & FONT_STYLE_BOLD)
		EmboldenGlyph(m_face->glyph);

	// and italics if applicable
	if (style & FONT_STYLE_ITALICS)
		ObliqueGlyph(m_face->glyph);

	// Grab the glyph
	if (FT_Get_Glyph(m_face->glyph, &glyph))
	{
		CLog::Log(LOGDEBUG, "%s Failed to get glyph %x", __FUNCTION__, letter);
		return false;
	}

	if (m_stroker)
		FT_Glyph_StrokeBorder(&glyph, m_stroker, 0, 1);
	
	// Render the glyph
	if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, 1))
	{
		CLog::Log(LOGDEBUG, "%s Failed to render glyph %x to a bitmap", __FUNCTION__, letter);
		return false;
	}

	FT_BitmapGlyph bitGlyph = (FT_BitmapGlyph)glyph;
	FT_Bitmap bitmap = bitGlyph->bitmap;
	
	if (bitGlyph->left < 0)
		m_posX += -bitGlyph->left;

	// Check we have enough room for the character
	if (m_posX + bitGlyph->left + bitmap.width > (int)m_textureWidth)
	{
		// No space - Gotta drop to the next line (which means creating a new texture and copying it across)
		m_posX = 0;
		m_posY += m_cellHeight;
		
		if (bitGlyph->left < 0)
			m_posX += -bitGlyph->left;

		if(m_posY + m_cellHeight >= m_textureHeight)
		{
			// Create the new larger texture
			unsigned newHeight = m_posY + m_cellHeight;
			LPDIRECT3DTEXTURE9 newTexture;
			
			// Check for max height (can't be more than max_texture_size texels
			if (newHeight > max_texture_size)
			{
				CLog::Log(LOGDEBUG, "GUIFontTTF::CacheCharacter: New cache texture is too large (%u > %u pixels long)", newHeight, max_texture_size);
				FT_Done_Glyph(glyph);
				return false;
			}

			if (D3D_OK != D3DXCreateTexture(m_pD3DDevice, m_textureWidth, newHeight, 1, 0, D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED, &newTexture))
			{
				CLog::Log(LOGDEBUG, "GUIFontTTF::CacheCharacter: Error creating new cache texture for size %f", m_height);
				FT_Done_Glyph(glyph);
				CLog::Log(LOGDEBUG, "GUIFontTTF::CacheCharacter: Failed to allocate new texture of height %u", newHeight);
				return false;
			}

			// Correct texture sizes
			D3DSURFACE_DESC desc;
			newTexture->GetLevelDesc(0, &desc);
			m_textureHeight = desc.Height;
			m_textureWidth = desc.Width;

			// Clear texture, doesn't cost much
			D3DLOCKED_RECT rect;
			newTexture->LockRect(0, &rect, NULL, 0);
			memset(rect.pBits, 0, rect.Pitch * m_textureHeight);
			newTexture->UnlockRect(0);

			if (m_texture)
			{
				// Copy across from our current one using gpu
				LPDIRECT3DSURFACE9 pTarget, pSource;
				newTexture->GetSurfaceLevel(0, &pTarget);
				m_texture->GetSurfaceLevel(0, &pSource);
				
				g_graphicsContext.TLock();
				D3DXLoadSurfaceFromSurface( pTarget, NULL, NULL, pSource, NULL, NULL, D3DX_FILTER_NONE,	0);
				g_graphicsContext.TUnlock();

				g_graphicsContext.TLock();
				SAFE_RELEASE(pTarget);
				g_graphicsContext.TUnlock();

				g_graphicsContext.TLock();
				SAFE_RELEASE(pSource);
				g_graphicsContext.TUnlock();

				g_graphicsContext.TLock();
				SAFE_RELEASE(m_texture);
				g_graphicsContext.TUnlock();
			}
			m_texture = newTexture;
		}
	}

	if(m_texture == NULL)
	{
		CLog::Log(LOGDEBUG, "GUIFontTTF::CacheCharacter: no texture to cache character to");
		return false;
	}

	// Set the character in our table
	ch->letterAndStyle = (style << 16) | letter;
	ch->offsetX = (short)bitGlyph->left;
	ch->offsetY = (short)max((short)m_cellBaseLine - bitGlyph->top, 0);
	ch->left = (float)m_posX + ch->offsetX;
	ch->top = (float)m_posY + ch->offsetY;
	ch->right = ch->left + bitmap.width;
	ch->bottom = ch->top + bitmap.rows;
	ch->advance = ROUND( (float)m_face->glyph->advance.x / 64 );

	// We need only render if we actually have some pixels
	if (bitmap.width * bitmap.rows)
	{
		// Render this onto our normal texture using gpu
		LPDIRECT3DSURFACE9 target;
		m_texture->GetSurfaceLevel(0, &target);

		RECT sourcerect = { 0, 0, bitmap.width, bitmap.rows };
		RECT targetrect;
		targetrect.top = m_posY + ch->offsetY;
		targetrect.left = m_posX + bitGlyph->left;
		targetrect.bottom = targetrect.top + bitmap.rows;
		targetrect.right = targetrect.left + bitmap.width;

		g_graphicsContext.TLock();

		D3DXLoadSurfaceFromMemory( target, NULL, &targetrect,
			bitmap.buffer, D3DFMT_LIN_A8, bitmap.pitch, NULL, &sourcerect,
			false, 0, 0, D3DX_FILTER_NONE, 0x00000000);

		SAFE_RELEASE(target);

		g_graphicsContext.TUnlock();
	}

	m_posX += 1 + (unsigned short)max(ch->right - ch->left + ch->offsetX, ch->advance);
	m_numChars++;

	// Free the glyph
	FT_Done_Glyph(glyph);

	return true;
}

void CGUIFontTTF::ClearCharacterCache()
{
	if(m_texture)
		m_texture->Release();

	m_texture = NULL;
	
	if (m_char)
		delete[] m_char;

	m_char = new Character[CHAR_CHUNK];
	memset(m_charquick, 0, sizeof(m_charquick));
	m_numChars = 0;
	m_maxChars = CHAR_CHUNK;
	
	// Set the posX and posY so that our texture will be created on first character write.
	m_posX = m_textureWidth;
	m_posY = -(int)m_cellHeight;
	m_textureHeight = 0;
}

void CGUIFontTTF::DrawTextInternal(float x, float y, const vecColors &colors, const vecText &text, uint32_t alignment, float maxPixelWidth, bool scrolling)
{
	Begin();

	// Save the origin, which is scaled separately
	m_originX = x;
	m_originY = y;

	// Check if we will really need to truncate or justify the text
	if ( alignment & XBFONT_TRUNCATED )
	{
		if ( maxPixelWidth <= 0.0f || GetTextWidthInternal(text.begin(), text.end()) <= maxPixelWidth)
			alignment &= ~XBFONT_TRUNCATED;
	}
	else if ( alignment & XBFONT_JUSTIFIED )
	{
		if ( maxPixelWidth <= 0.0f )
			alignment &= ~XBFONT_JUSTIFIED;
	}

	// Calculate sizing information
	float startX = 0;
	float startY = (alignment & XBFONT_CENTER_Y) ? -0.5f*(m_cellHeight-2) : 0;  // vertical centering

	if ( alignment & (XBFONT_RIGHT | XBFONT_CENTER_X) )
	{
		// Get the extent of this line
		float w = GetTextWidthInternal( text.begin(), text.end() );

		if ( alignment & XBFONT_TRUNCATED && w > maxPixelWidth + 0.5f ) // + 0.5f due to rounding issues
			w = maxPixelWidth;

		if ( alignment & XBFONT_CENTER_X)
			w *= 0.5f;
		
		// Offset this line's starting position
		startX -= w;
	}

	float spacePerLetter = 0; // For justification effects

	if ( alignment & XBFONT_JUSTIFIED )
	{
		// Dirst compute the size of the text to render in both characters and pixels
		unsigned int lineChars = 0;
		float linePixels = 0;
		
		for (vecText::const_iterator pos = text.begin(); pos != text.end(); pos++)
		{
			Character *ch = GetCharacter(*pos);
			if (ch)
			{
				// Spaces have multiple times the justification spacing of normal letters
				lineChars += ((*pos & 0xffff) == L' ') ? justification_word_weight : 1;
				linePixels += ch->advance;
			}
		}
		if (lineChars > 1)
			spacePerLetter = (maxPixelWidth - linePixels) / (lineChars - 1);
	}
	float cursorX = 0; // Current position along the line

	for (vecText::const_iterator pos = text.begin(); pos != text.end(); pos++)
	{
		// If starting text on a new line, determine justification effects
		// Get the current letter in the CStdString
		color_t color = (*pos & 0xff0000) >> 16;
		
		if (color >= colors.size())
			color = 0;
	
		color = colors[color];

		// Grab the next character
		Character *ch = GetCharacter(*pos);
		if (!ch) continue;

		if ( alignment & XBFONT_TRUNCATED )
		{
			// Check if we will be exceeded the max allowed width
			if ( cursorX + ch->advance + 3 * m_ellipsesWidth > maxPixelWidth )
			{
				// Yup. Let's draw the ellipses, then bail
				// Perhaps we should really bail to the next line in this case??
				Character *period = GetCharacter(L'.');
				if (!period)
					break;

				for (int i = 0; i < 3; i++)
				{
					RenderCharacter(startX + cursorX, startY, period, color, !scrolling);
					cursorX += period->advance;
				}
				break;
			}
		}
		else if (maxPixelWidth > 0 && cursorX > maxPixelWidth)
			break; // Exceeded max allowed width - stop rendering

		RenderCharacter(startX + cursorX, startY, ch, color, !scrolling);
		
		if ( alignment & XBFONT_JUSTIFIED )
		{
			if ((*pos & 0xffff) == L' ')
				cursorX += ch->advance + spacePerLetter * justification_word_weight;
			else
				cursorX += ch->advance + spacePerLetter;
		}
		else
			cursorX += ch->advance;
	}
	End();
}

// This routine assumes a single line (i.e. it was called from GUITextLayout)
float CGUIFontTTF::GetTextWidthInternal(vecText::const_iterator start, vecText::const_iterator end)
{
	float width = 0;
	while (start != end)
	{
		Character *c = GetCharacter(*start++);
		if (c) width += c->advance;
	}
	return width;
}

float CGUIFontTTF::GetCharWidthInternal(character_t ch)
{
  Character *c = GetCharacter(ch);
  if (c) return c->advance;
  return 0;
}

float CGUIFontTTF::GetTextHeight(float lineSpacing, int numLines) const
{
	return (float)(numLines - 1) * GetLineHeight(lineSpacing) + (m_cellHeight - 2); // -2 as we increment this for space in our texture
}

float CGUIFontTTF::GetLineHeight(float lineSpacing) const
{
	if (m_face)
		return lineSpacing * m_face->size->metrics.height / 64.0f;

	return 0.0f;
}

void CGUIFontTTF::AddReference()
{
	m_referenceCount++;
}

void CGUIFontTTF::RemoveReference()
{
	// Delete this object when it's reference count hits zero
	m_referenceCount--;

	if (!m_referenceCount)
		g_fontManager.FreeFontFile(this);
}

// Oblique code - original taken from freetype2 (ftsynth.c)
void CGUIFontTTF::ObliqueGlyph(FT_GlyphSlot slot)
{
	// Only oblique outline glyphs
	if ( slot->format != FT_GLYPH_FORMAT_OUTLINE )
		return;

	/* We don't touch the advance width */

	// For italic, simply apply a shear transform, with an angle
	// of about 12 degrees.

	FT_Matrix    transform;
	transform.xx = 0x10000L;
	transform.yx = 0x00000L;

	transform.xy = 0x06000L;
	transform.yy = 0x10000L;

	FT_Outline_Transform( &slot->outline, &transform );
}

// Embolden code - Original taken from freetype2 (ftsynth.c)
void CGUIFontTTF::EmboldenGlyph(FT_GlyphSlot slot)
{
	if ( slot->format != FT_GLYPH_FORMAT_OUTLINE )
		return;

	// Some reasonable strength
	FT_Pos strength = FT_MulFix( m_face->units_per_EM,
						m_face->size->metrics.y_scale ) / 24;

	FT_BBox bbox_before, bbox_after;
	FT_Outline_Get_CBox( &slot->outline, &bbox_before );
	FT_Outline_Embolden( &slot->outline, strength );  // Ignore error
	FT_Outline_Get_CBox( &slot->outline, &bbox_after );

	FT_Pos dx = bbox_after.xMax - bbox_before.xMax;
	FT_Pos dy = bbox_after.yMax - bbox_before.yMax;

	if ( slot->advance.x )
		slot->advance.x += dx;

	if ( slot->advance.y )
		slot->advance.y += dy;

	slot->metrics.width        += dx;
	slot->metrics.height       += dy;
	slot->metrics.horiBearingY += dy;
	slot->metrics.horiAdvance  += dx;
	slot->metrics.vertBearingX -= dx / 2;
	slot->metrics.vertBearingY += dy;
	slot->metrics.vertAdvance  += dy;
}