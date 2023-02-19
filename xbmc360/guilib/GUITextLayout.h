#ifndef GUILIB_TEXTLAYOUT_H
#define GUILIB_TEXTLAYOUT_H

#include "utils\StdString.h"
#include "GUIFont.h"
#include <vector>

class CGUIFont;

typedef uint32_t character_t;
typedef uint32_t color_t;
typedef std::vector<character_t> vecText;
typedef std::vector<color_t> vecColors;

class CGUIString
{
public:
	typedef vecText::const_iterator iString;

	CGUIString(iString start, iString end, bool carriageReturn);

	CStdString GetAsString() const;

	vecText m_text;
	bool m_carriageReturn; // True if we have a carriage return here
};

class CGUITextLayout
{
public:
	CGUITextLayout(CGUIFont *font, bool wrap, float fHeight=0.0f, CGUIFont *borderFont = NULL);  // This may need changing - we may just use this class to replace CLabelInfo completely

	// Main function to render strings
	void Render(float x, float y, float angle, color_t color, color_t shadowColor, uint32_t alignment, float maxWidth, bool solid = false);
	void RenderScrolling(float x, float y, float angle, color_t color, color_t shadowColor, uint32_t alignment, float maxWidth, CScrollInfo &scrollInfo);

	// Returns the precalculated width and height of the text to be rendered (in constant time).
	// param width [out] width of text
	// param height [out] height of text
	void GetTextExtent(float &width, float &height) const;

	void SetText(const CStdStringW &text, float maxWidth = 0, bool forceLTRReadingOrder = false);
	bool Update(const CStdString &text, float maxWidth = 0, bool forceLTRReadingOrder = false);

	float GetTextWidth() const { return m_textWidth; };
	float GetTextWidth(const CStdStringW &text) const;
	unsigned int GetTextLength() const;

protected:
	void ParseText(const CStdStringW &text, vecText &parsedText);
	void LineBreakText(const vecText &text, std::vector<CGUIString> &lines);
	void WrapText(const vecText &text, float maxWidth);
	void BidiTransform(std::vector<CGUIString> &lines, bool forceLTRReadingOrder);
	CStdStringW BidiFlip(const CStdStringW &text, bool forceLTRReadingOrder);
	void CalcTextExtent();

	// Our text to render
	vecColors m_colors;
	std::vector<CGUIString> m_lines;


	// The layout and font details
	CGUIFont *m_font;        // Has style, colour info
	CGUIFont *m_borderFont;  // Oonly used for outlined text

	bool  m_wrap;            // Wrapping (true if justify is enabled!)
	float m_maxHeight;
	// The default color (may differ from the font objects defaults)
	color_t m_textColor;

	CStdString m_lastText;
	float m_textWidth;
	float m_textHeight;

private:
	inline bool IsSpace(character_t letter) const
	{
		return (letter & 0xffff) == L' ';
	};

	inline bool CanWrapAtLetter(character_t letter) const
	{
		character_t ch = letter & 0xffff;
		return ch == L' ' || (ch >=0x4e00 && ch <= 0x9fff);
	};

	static void AppendToUTF32(const CStdString &utf8, character_t colStyle, vecText &utf32);
	static void AppendToUTF32(const CStdStringW &utf16, character_t colStyle, vecText &utf32);

	static void ParseText(const CStdStringW &text, uint32_t defaultStyle, vecColors &colors, vecText &parsedText);

	static void utf8ToW(const CStdString &utf8, CStdStringW &utf16);

};

#endif //GUILIB_TEXTLAYOUT_H