#include "GUITextLayout.h"
#include "GUIColorManager.h"
#include "utils\CharsetConverter.h"
#include "GraphicContext.h"

using namespace std;

#define WORK_AROUND_NEEDED_FOR_LINE_BREAKS

CGUIString::CGUIString(iString start, iString end, bool carriageReturn)
{
	m_text.assign(start, end);
	m_carriageReturn = carriageReturn;
}

CStdString CGUIString::GetAsString() const
{
	CStdString text;
	for (unsigned int i = 0; i < m_text.size(); i++)
		text += (char)(m_text[i] & 0xff);
	
	return text;
}

CGUITextLayout::CGUITextLayout(CGUIFont *font, bool wrap, float fHeight, CGUIFont *borderFont)
{
	m_font = font;
	m_borderFont = borderFont;
	m_textColor = 0;
	m_wrap = wrap;
	m_maxHeight = fHeight;
	m_textWidth = 0;
	m_textHeight = 0;
}

void CGUITextLayout::Render(float x, float y, float angle, color_t color, color_t shadowColor, uint32_t alignment, float maxWidth, bool solid)
{
	if (!m_font)
		return;

	// Set the main text color
	if (m_colors.size())
		m_colors[0] = color;

	// Render the text at the required location, angle, and size
	if (angle)
	{
		static const float degrees_to_radians = 0.01745329252f;
		g_graphicsContext.AddTransform(TransformMatrix::CreateZRotation(angle * degrees_to_radians, x, y, g_graphicsContext.GetScalingPixelRatio()));
	}
	
	// Center our text vertically
	if (alignment & XBFONT_CENTER_Y)
	{
		y -= m_font->GetTextHeight(m_lines.size()) * 0.5f;;
		alignment &= ~XBFONT_CENTER_Y;
	}
	
	m_font->Begin();
	
	for (vector<CGUIString>::iterator i = m_lines.begin(); i != m_lines.end(); i++)
	{
		const CGUIString &string = *i;
		uint32_t align = alignment;
		
		if (align & XBFONT_JUSTIFIED && string.m_carriageReturn)
			align &= ~XBFONT_JUSTIFIED;
		
		if (solid)
			m_font->DrawText(x, y, m_colors[0], shadowColor, string.m_text, align, maxWidth);
		else
			m_font->DrawText(x, y, m_colors, shadowColor, string.m_text, align, maxWidth);
		
		y += m_font->GetLineHeight();
	}
	
	m_font->End();

	if (angle)
		g_graphicsContext.RemoveTransform();
}

void CGUITextLayout::RenderScrolling(float x, float y, float angle, color_t color, color_t shadowColor, uint32_t alignment, float maxWidth, CScrollInfo &scrollInfo)
{
	if (!m_font)
		return;

	// Set the main text color
	if (m_colors.size())
		m_colors[0] = color;

	// Render the text at the required location, angle, and size
	if (angle)
	{
		static const float degrees_to_radians = 0.01745329252f;
		g_graphicsContext.AddTransform(TransformMatrix::CreateZRotation(angle * degrees_to_radians, x, y, g_graphicsContext.GetScalingPixelRatio()));
	}
	
	// Center our text vertically
	if (alignment & XBFONT_CENTER_Y)
	{
		y -= m_font->GetTextHeight(m_lines.size()) * 0.5f;;
		alignment &= ~XBFONT_CENTER_Y;
	}
	
	m_font->Begin();
	// NOTE: This workaround is needed as otherwise multi-line text that scrolls
	//       will scroll in proportion to the number of lines. Ideally we should
	//       do the DrawScrollingText calculation here. This probably won't make
	//       any difference to the smoothness of scrolling though which will be
	//       jumpy with this sort of thing.  It's not exactly a well used situation
	//       though, so this hack is probably OK.
	float speed = scrollInfo.pixelSpeed;
	
	for (vector<CGUIString>::iterator i = m_lines.begin(); i != m_lines.end(); i++)
	{
		const CGUIString &string = *i;
		m_font->DrawScrollingText(x, y, m_colors, shadowColor, string.m_text, alignment, maxWidth, scrollInfo);
		y += m_font->GetLineHeight();
		scrollInfo.pixelSpeed = 0;
	}
	
	scrollInfo.pixelSpeed = speed;
	m_font->End();
	
	if (angle)
		g_graphicsContext.RemoveTransform();
}

void CGUITextLayout::GetTextExtent(float &width, float &height) const
{
	width = m_textWidth;
	height = m_textHeight;
}

bool CGUITextLayout::Update(const CStdString &text, float maxWidth, bool forceLTRReadingOrder /*= false*/)
{
	if (text == m_lastText)
		return false;

	// Convert to utf16
	CStdStringW utf16;
	utf8ToW(text, utf16);

	// Update
	SetText(utf16, maxWidth, forceLTRReadingOrder);

	// and set our parameters to indicate no further update is required
	m_lastText = text;
	return true;
}

void CGUITextLayout::SetText(const CStdStringW &text, float maxWidth, bool forceLTRReadingOrder /*= false*/)	
{
	vecText parsedText;

	// Empty out our previous string
	m_lines.clear();
	m_colors.clear();
	m_colors.push_back(m_textColor);

	// Parse the text into our string objects
	ParseText(text, parsedText);

	// Add \n to the end of the string
	parsedText.push_back(L'\n');

	// If we need to wrap the text, then do so
	if (m_wrap && maxWidth > 0)
		WrapText(parsedText, maxWidth);
	else
		LineBreakText(parsedText, m_lines);

	// Remove any trailing blank lines
	while (!m_lines.empty() && m_lines.back().m_text.empty())
		m_lines.pop_back();

	BidiTransform(m_lines, forceLTRReadingOrder);

	// and cache the width and height for later reading
	CalcTextExtent();
}

// BidiTransform is used to handle RTL text flipping in the string
void CGUITextLayout::BidiTransform(vector<CGUIString> &lines, bool forceLTRReadingOrder)
{
	for (unsigned int i=0; i<lines.size(); i++)
	{
		CGUIString &line = lines[i];

		// Reserve enough space in the flipped text
		vecText flippedText;
		flippedText.reserve(line.m_text.size());

		character_t sectionStyle = 0xffff0000; // Impossible to achieve
		CStdStringW sectionText;
		
		for (vecText::iterator it = line.m_text.begin(); it != line.m_text.end(); ++it)
		{
			character_t style = *it & 0xffff0000;
			if (style != sectionStyle)
			{
				if (!sectionText.IsEmpty())
				{
					// Style has changed, bidi flip text
					CStdStringW sectionFlipped = BidiFlip(sectionText, forceLTRReadingOrder);
					for (unsigned int j = 0; j < sectionFlipped.size(); j++)
						flippedText.push_back(sectionStyle | sectionFlipped[j]);
				}
				sectionStyle = style;
				sectionText.clear();
			}
			sectionText.push_back( (wchar_t)(*it & 0xffff) );
		}

		// Handle the last section
		if (!sectionText.IsEmpty())
		{
			CStdStringW sectionFlipped = BidiFlip(sectionText, forceLTRReadingOrder);
			for (unsigned int j = 0; j < sectionFlipped.size(); j++)
				flippedText.push_back(sectionStyle | sectionFlipped[j]);
		}

		// Replace the original line with the proccessed one
		lines[i] = CGUIString(flippedText.begin(), flippedText.end(), line.m_carriageReturn);
	}
}

CStdStringW CGUITextLayout::BidiFlip(const CStdStringW &text, bool forceLTRReadingOrder) //FIXME !!
{
  CStdStringA utf8text;
  CStdStringW visualText;

  // Convert to utf8, and back to utf16 with bidi flipping
  g_charsetConverter.wToUTF8(text, utf8text);
  g_charsetConverter.utf8ToW(utf8text, visualText, true, forceLTRReadingOrder);

  return visualText;
}

void CGUITextLayout::CalcTextExtent()
{
	m_textWidth = 0;
	m_textHeight = 0;
	
	if (!m_font) return;

	for (vector<CGUIString>::iterator i = m_lines.begin(); i != m_lines.end(); i++)
	{
		const CGUIString &string = *i;
		float w = m_font->GetTextWidth(string.m_text);
		
		if (w > m_textWidth)
			m_textWidth = w;
	}
	m_textHeight = m_font->GetTextHeight(m_lines.size());
}

float CGUITextLayout::GetTextWidth(const CStdStringW &text) const
{
	// NOTE: Assumes a single line of text
	if (!m_font) return 0;
	vecText utf32;
	AppendToUTF32(text, (m_font->GetStyle() & 3) << 24, utf32);
	
	return m_font->GetTextWidth(utf32);
}

unsigned int CGUITextLayout::GetTextLength() const
{
	unsigned int length = 0;
	for (vector<CGUIString>::const_iterator i = m_lines.begin(); i != m_lines.end(); i++)
		length += i->m_text.size();
	
	return length;
}

void CGUITextLayout::LineBreakText(const vecText &text, vector<CGUIString> &lines)
{
	int nMaxLines = (m_maxHeight > 0 && m_font && m_font->GetLineHeight() > 0)?(int)ceilf(m_maxHeight / m_font->GetLineHeight()):-1;
	vecText::const_iterator lineStart = text.begin();
	vecText::const_iterator pos = text.begin();
	
	while (pos != text.end() && (nMaxLines <= 0 || lines.size() < (size_t)nMaxLines))
	{
		// Get the current letter in the string
		character_t letter = *pos;

		// Handle the newline character
		if ((letter & 0xffff) == L'\n' )
		{
			// Push back everything up till now
			CGUIString string(lineStart, pos, true);
			lines.push_back(string);
			lineStart = pos + 1;
		}
		pos++;
	}
}

void CGUITextLayout::WrapText(const vecText &text, float maxWidth)
{
	if (!m_font)
		return;

	int nMaxLines = (m_maxHeight > 0 && m_font->GetLineHeight() > 0)?(int)ceilf(m_maxHeight / m_font->GetLineHeight()):-1;

	m_lines.clear();

	vector<CGUIString> lines;
	LineBreakText(text, lines);

	for (unsigned int i = 0; i < lines.size(); i++)
	{
		const CGUIString &line = lines[i];
		vecText::const_iterator lastSpace = line.m_text.begin();
		vecText::const_iterator pos = line.m_text.begin();
		unsigned int lastSpaceInLine = 0;
		vecText curLine;
		
		while (pos != line.m_text.end() && (nMaxLines <= 0 || m_lines.size() < (size_t)nMaxLines))
		{
			// Get the current letter in the string
			character_t letter = *pos;
			
			// Check for a space
			if (CanWrapAtLetter(letter))
			{
				float width = m_font->GetTextWidth(curLine);
				if (width > maxWidth)
				{
					if (lastSpace != line.m_text.begin() && lastSpaceInLine > 0)
					{
						CGUIString string(curLine.begin(), curLine.begin() + lastSpaceInLine, false);
						m_lines.push_back(string);
						
						if (IsSpace(letter))
							lastSpace++;  // Ignore the space
						
						pos = lastSpace;
						curLine.clear();
						lastSpaceInLine = 0;
						lastSpace = line.m_text.begin();
						continue;
					}
				}
				
				// Only add spaces if we're not empty
				if (!IsSpace(letter) || curLine.size())
				{
					lastSpace = pos;
					lastSpaceInLine = curLine.size();
					curLine.push_back(letter);
				}
			}
			else
				curLine.push_back(letter);
			
			pos++;
		}
		
		// Now add whatever we have left to the string
		float width = m_font->GetTextWidth(curLine);
		if (width > maxWidth)
		{
			// Too long - put up to the last space on if we can + remove it from what's left.
			if (lastSpace != line.m_text.begin() && lastSpaceInLine > 0)
			{
				CGUIString string(curLine.begin(), curLine.begin() + lastSpaceInLine, false);
				m_lines.push_back(string);
				curLine.erase(curLine.begin(), curLine.begin() + lastSpaceInLine);
				
				while (curLine.size() && IsSpace(curLine.at(0)))
					curLine.erase(curLine.begin());
			}
		}
		CGUIString string(curLine.begin(), curLine.end(), true);
		m_lines.push_back(string);
	}
}

void CGUITextLayout::ParseText(const CStdStringW &text, vecText &parsedText)
{
	if (!m_font)
		return;
	
	ParseText(text, m_font->GetStyle(), m_colors, parsedText);
}

void CGUITextLayout::AppendToUTF32(const CStdStringW &utf16, character_t colStyle, vecText &utf32)
{
	// NOTE: Assumes a single line of text
	utf32.reserve(utf32.size() + utf16.size());

	for (unsigned int i = 0; i < utf16.size(); i++)
		utf32.push_back(utf16[i] | colStyle);
}

void CGUITextLayout::AppendToUTF32(const CStdString &utf8, character_t colStyle, vecText &utf32)
{
	CStdStringW utf16;
	utf8ToW(utf8, utf16);
	AppendToUTF32(utf16, colStyle, utf32);
}

void CGUITextLayout::utf8ToW(const CStdString &utf8, CStdStringW &utf16)
{
#ifdef WORK_AROUND_NEEDED_FOR_LINE_BREAKS
	// NOTE: This appears to strip \n characters from text.  This may be a consequence of incorrect
	//       expression of the \n in utf8 (we just use character code 10) or it might be something
	//       more sinister.  For now, we use the workaround below.
	
	CStdStringArray multiLines;
	CStringUtils::SplitString(utf8, "\n", multiLines);
	
	for (unsigned int i = 0; i < multiLines.size(); i++)
	{
		CStdStringW line;

		// No need to bidiflip here - it's done in BidiTransform above
		g_charsetConverter.utf8ToW(multiLines[i], line, false);
		utf16 += line;
		
		if (i < multiLines.size() - 1)
			utf16.push_back(L'\n');
	}
#else
	// No need to bidiflip here - It's done in BidiTransform above
	g_charsetConverter.utf8ToW(utf8, utf16, false);
#endif
}

void CGUITextLayout::ParseText(const CStdStringW &text, uint32_t defaultStyle, vecColors &colors, vecText &parsedText)
{
	// run through the string, searching for:
	// [B] or [/B] -> toggle bold on and off
	// [I] or [/I] -> toggle italics on and off
	// [COLOR ffab007f] or [/COLOR] -> toggle color on and off
	// [CAPS <option>] or [/CAPS] -> toggle capatilization on and off

	uint32_t currentStyle = defaultStyle; // start with the default font's style
	color_t currentColor = 0;

	stack<color_t> colorStack;
	colorStack.push(0);

	// These aren't independent, but that's probably not too much of an issue
	// eg [UPPERCASE]Glah[LOWERCASE]FReD[/LOWERCASE]Georeg[/UPPERCASE] will work (lower case >> upper case)
	// but [LOWERCASE]Glah[UPPERCASE]FReD[/UPPERCASE]Georeg[/LOWERCASE] won't

	int startPos = 0;
	size_t pos = text.Find(L'[');
	
	while (pos != CStdString::npos && pos + 1 < text.size())
	{
		uint32_t newStyle = 0;
		color_t newColor = currentColor;
		bool newLine = false;
		
		// Have a [ - check if it's an ON or OFF switch
		bool on(true);
		int endPos = pos++; // Finish of string
		
		if (text[pos] == L'/')
		{
			on = false;
			pos++;
		}
		
		// Check for each type
		if (text.Mid(pos,2) == L"B]")
		{
			// Bold - Finish the current text block and assign the bold state
			pos += 2;
			if ((on && text.Find(L"[/B]",pos) >= 0) ||          // Check for a matching end point
				(!on && (currentStyle & FONT_STYLE_BOLD)))      // or matching start point
				newStyle = FONT_STYLE_BOLD;
		}
		else if (text.Mid(pos,2) == L"I]")
		{
			// Italics
			pos += 2;
			if ((on && text.Find(L"[/I]",pos) >= 0) ||          // Check for a matching end point
				(!on && (currentStyle & FONT_STYLE_ITALICS)))   // or matching start point
				newStyle = FONT_STYLE_ITALICS;
		}
		else if (text.Mid(pos,10) == L"UPPERCASE]")
		{
			pos += 10;
			if ((on && text.Find(L"[/UPPERCASE]",pos) >= 0) ||  // Check for a matching end point
				(!on && (currentStyle & FONT_STYLE_UPPERCASE))) // or matching start point
				newStyle = FONT_STYLE_UPPERCASE;
		}
		else if (text.Mid(pos,10) == L"LOWERCASE]")
		{
			pos += 10;
			if ((on && text.Find(L"[/LOWERCASE]",pos) >= 0) ||  // Check for a matching end point
				(!on && (currentStyle & FONT_STYLE_LOWERCASE))) // or matching start point
					newStyle = FONT_STYLE_LOWERCASE;
		}
		else if (text.Mid(pos,3) == L"CR]" && on)
		{
			newLine = true;
			pos += 3;
		}
		else if (text.Mid(pos,5) == L"COLOR")
		{
			// Color
			size_t finish = text.Find(L']', pos + 5);
			if (on && finish != CStdString::npos && (size_t)text.Find(L"[/COLOR]",finish) != CStdString::npos)
			{
				// Create new color
				newColor = colors.size();
				colors.push_back(g_colorManager.GetColor(text.Mid(pos + 5, finish - pos - 5)));
				colorStack.push(newColor);
			}
			else if (!on && finish == pos + 5 && colorStack.size() > 1)
			{
				// Revert to previous color
				colorStack.pop();
				newColor = colorStack.top();
			}
			if (finish != CStdString::npos)
				pos = finish + 1;
		}

		if (newStyle || newColor != currentColor || newLine)
		{
			// We have a new style or a new color, so format up the previous segment
			CStdStringW subText = text.Mid(startPos, endPos - startPos);
			
			if (currentStyle & FONT_STYLE_UPPERCASE)
					subText.ToUpper();
			
			if (currentStyle & FONT_STYLE_LOWERCASE)
				subText.ToLower();
			
			AppendToUTF32(subText, ((currentStyle & 3) << 24) | (currentColor << 16), parsedText);
			
			if (newLine)
				parsedText.push_back(L'\n');

			// and switch to the new style
			startPos = pos;
			currentColor = newColor;
			
			if (on)
				currentStyle |= newStyle;
			else
				currentStyle &= ~newStyle;
		}
		pos = text.Find(L'[',pos);
	}
	
	// Now grab the remainder of the string
	CStdStringW subText = text.Mid(startPos, text.GetLength() - startPos);
	
	if (currentStyle & FONT_STYLE_UPPERCASE)
		subText.ToUpper();
	
	if (currentStyle & FONT_STYLE_LOWERCASE)
		subText.ToLower();
	
	AppendToUTF32(subText, ((currentStyle & 3) << 24) | (currentColor << 16), parsedText);
}