#ifndef CGUILIB_GUIFONTTTF_H
#define CGUILIB_GUIFONTTTF_H

#include "GraphicContext.h"
#include "utils\StdString.h"

// Forward definition
struct FT_FaceRec_;
struct FT_LibraryRec_;
struct FT_GlyphSlotRec_;
struct FT_StrokerRec_;

typedef struct FT_FaceRec_ *FT_Face;
typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_GlyphSlotRec_ *FT_GlyphSlot;
typedef struct FT_BitmapGlyphRec_ *FT_BitmapGlyph;
typedef struct FT_StrokerRec_ *FT_Stroker;

typedef uint32_t character_t;
typedef uint32_t color_t;
typedef std::vector<character_t> vecText;
typedef std::vector<color_t> vecColors;

class CGUIFontTTF
{
	struct Character
	{
		short offsetX, offsetY;
		float left, top, right, bottom;
		float advance;
		DWORD letterAndStyle;
	};

public:
	CGUIFontTTF(const CStdString& strFileName);
	virtual ~CGUIFontTTF(void);

	void Clear();

	bool Load(const CStdString& strFilename, float height = 20.0f, float aspect = 1.0f, float lineSpacing = 1.0f, bool border = false);

	void Begin();
	void End();

	const CStdString& GetFileName() const { return m_strFileName; };
	void CopyReferenceCountFrom(CGUIFontTTF& ttf) { m_referenceCount = ttf.m_referenceCount; }

//protected: //FIXME
	void AddReference();
	void RemoveReference();

	float GetTextWidthInternal(vecText::const_iterator start, vecText::const_iterator end);
	float GetCharWidthInternal(character_t ch);
	float GetTextHeight(float lineSpacing, int numLines) const;
	float GetLineHeight(float lineSpacing) const;

	void DrawTextInternal(float x, float y, const vecColors &colors, const vecText &text,
                            uint32_t alignment, float maxPixelWidth, bool scrolling);

	float m_height;
	CStdString m_strFilename;

	// Stuff for pre-rendering for speed
	inline Character *GetCharacter(character_t letter);
	bool CacheCharacter(wchar_t letter, uint32_t style, Character *ch);
	inline void RenderCharacter(float posX, float posY, const Character *ch, D3DCOLOR dwColor, bool roundX);
	void ClearCharacterCache();

	// Modifying glyphs
	void EmboldenGlyph(FT_GlyphSlot slot);
	void ObliqueGlyph(FT_GlyphSlot slot);

	LPDIRECT3DDEVICE9 m_pD3DDevice;
	LPDIRECT3DTEXTURE9 m_texture;      // Texture that holds our rendered characters (8bit alpha only)
	unsigned int m_textureWidth;       // Width of our texture
	unsigned int m_textureHeight;      // Heigth of our texture
	int m_posX;                        // Current position in the texture
	int m_posY;

	Character *m_char;                 // Our characters
	Character *m_charquick[256*4];     // ascii chars (4 styles) here	
	int m_maxChars;                    // Size of character array (can be incremented)
	int m_numChars;                    // The current number of cached characters

	float m_ellipsesWidth;             // This is used every character (width of '.')

	unsigned int m_cellBaseLine;
	unsigned int m_cellHeight;

	unsigned int m_nestedBeginCount;   // Speedups

	// Freetype stuff
	FT_Face    m_face;
	FT_Stroker m_stroker;

	float m_originX;
	float m_originY;

	static int justification_word_weight;
	static unsigned int max_texture_size;

	CStdString m_strFileName;

	DWORD m_numCharactersRendered;
	
	struct CUSTOMVERTEX
	{
		float fPosition[3];
		float fTexCoord[2];
	};

	IDirect3DVertexDeclaration9*	m_pVertexDecl; // Vertex format decl
	IDirect3DVertexShader9*			m_pVertexShader; // Vertex Shader
	IDirect3DPixelShader9*			m_pPixelShader;  // Pixel Shader

private:
	int m_referenceCount;
};


#endif //CGUILIB_GUIFONTTTF_H