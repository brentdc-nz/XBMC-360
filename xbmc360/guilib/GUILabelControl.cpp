#include "include.h"
#include "GUILabelControl.h"
#include "utils\CharsetConverter.h"

using namespace std;

CGUILabelControl::CGUILabelControl(int parentID, int controlID, float posX, float posY, float width, float height, const CLabelInfo& labelInfo, bool wrapMultiLine, bool bHasPath)
	: CGUIControl(parentID, controlID, posX, posY, width, height)
	, m_label(posX, posY, width, height, labelInfo, wrapMultiLine ? CGUILabel::OVER_FLOW_WRAP : CGUILabel::OVER_FLOW_TRUNCATE)
{
	m_bHasPath = bHasPath;
	m_iCursorPos = 0;
	m_bShowCursor = false;
	m_dwCounter = 0;
	ControlType = GUICONTROL_LABEL;
	m_startHighlight = m_endHighlight = 0;
	m_minWidth = 0;

	if ((labelInfo.align & XBFONT_RIGHT) && m_width)
		m_posX -= m_width;
}

CGUILabelControl::~CGUILabelControl(void)
{
}

void CGUILabelControl::ShowCursor(bool bShow)
{
	m_bShowCursor = bShow;
}

void CGUILabelControl::SetCursorPos(int iPos)
{
	CStdString label = m_infoLabel.GetLabel(m_parentID);
	if (iPos > (int)label.length()) iPos = label.length();
	if (iPos < 0) iPos = 0;
	m_iCursorPos = iPos;
}

void CGUILabelControl::SetInfo(const CGUIInfoLabel &infoLabel)
{
	m_infoLabel = infoLabel;
}

void CGUILabelControl::UpdateColors()
{
	m_label.UpdateColors();
	CGUIControl::UpdateColors();
}

void CGUILabelControl::UpdateInfo(const CGUIListItem *item)
{
	CStdString label(m_infoLabel.GetLabel(m_parentID));

	if (m_bShowCursor)
	{
		// Cursor location assumes utf16 text, so deal with that (inefficient, but it's not as if it's a high-use area
		// virtual keyboard only)
		CStdStringW utf16;
		g_charsetConverter.utf8ToW(label, utf16);
		
		CStdStringW col;
		if ((++m_dwCounter % 50) > 25)
			col = L"|";
		else
			col = L"[COLOR 00FFFFFF]|[/COLOR]";

		utf16.Insert(m_iCursorPos, col);
		g_charsetConverter.wToUTF8(utf16, label);
	}
	else if (m_startHighlight || m_endHighlight)
	{
		// This is only used for times/dates, so working in ascii (utf8) is fine
		CStdString colorLabel;
		colorLabel.Format("[COLOR %x]%s[/COLOR]%s[COLOR %x]%s[/COLOR]", (color_t)m_label.GetLabelInfo().disabledColor, label.Left(m_startHighlight),
						label.Mid(m_startHighlight, m_endHighlight - m_startHighlight), (color_t)m_label.GetLabelInfo().disabledColor, label.Mid(m_endHighlight));
		label = colorLabel;
	}
	else if (m_bHasPath)
		label = ShortenPath(label);

	m_label.SetMaxRect(m_posX, m_posY, m_width, m_height);
	m_label.SetText(label);
}

void CGUILabelControl::Render()
{
	m_label.SetColor(IsDisabled() ? CGUILabel::COLOR_DISABLED : CGUILabel::COLOR_TEXT);
	m_label.SetMaxRect(m_posX, m_posY, m_width, m_height);
	m_label.Render();

	CGUIControl::Render();
}

bool CGUILabelControl::CanFocus() const
{
	return false;
}

void CGUILabelControl::SetLabel(const string &strLabel)
{
	m_infoLabel.SetLabel(strLabel, "", GetParentID());

	if (m_iCursorPos > (int)strLabel.size())
		m_iCursorPos = strLabel.size();
}

void CGUILabelControl::SetWidthControl(float minWidth, bool bScroll)
{
	m_minWidth = minWidth;
	m_label.SetScrolling(bScroll);
}

void CGUILabelControl::SetAlignment(uint32_t align)
{
	m_label.GetLabelInfo().align = align;
}

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

float CGUILabelControl::GetWidth() const
{
	if (m_minWidth && m_minWidth != m_width)
		return CLAMP(m_label.GetTextWidth(), m_minWidth, m_width);

	return m_width;
}

bool CGUILabelControl::OnMessage(CGUIMessage& message)
{
	if ( message.GetControlId() == GetID() )
	{
		if (message.GetMessage() == GUI_MSG_LABEL_SET)
		{
			SetLabel(message.GetLabel());
			return true;
		}
	}
	return CGUIControl::OnMessage(message);
}

CStdString CGUILabelControl::ShortenPath(const CStdString &path)
{
	if (m_width == 0 || path.IsEmpty())
		return path;

	char cDelim = '\0';
	size_t nPos;

	nPos = path.find_last_of( '\\' );
	if ( nPos != std::string::npos )
		cDelim = '\\';
	else
	{
		nPos = path.find_last_of( '/' );
		if ( nPos != std::string::npos )
			cDelim = '/';
	}

	if ( cDelim == '\0' )
		return path;

	CStdString workPath(path);

	// Remove trailing slashes
	if (workPath.size() > 3)
	{
		if (workPath.Right(3).Compare("://") != 0 && workPath.Right(2).Compare(":\\") != 0)
		{
			if (nPos == workPath.size() - 1)
			{
				workPath.erase(workPath.size() - 1);
				nPos = workPath.find_last_of( cDelim );
			}
		}
	}

	m_label.SetText(workPath);
	float textWidth = m_label.GetTextWidth();

	while ( textWidth > m_width )
	{
		size_t nGreaterDelim = workPath.find_last_of( cDelim, nPos );
		if (nGreaterDelim == std::string::npos)
			break;

		nPos = workPath.find_last_of( cDelim, nGreaterDelim - 1 );
		if ( nPos == std::string::npos )
			break;

		workPath.replace( nPos + 1, nGreaterDelim - nPos - 1, "..." );

		m_label.SetText(workPath);
		textWidth = m_label.GetTextWidth();
	}
	return workPath;
}

void CGUILabelControl::SetHighlight(unsigned int start, unsigned int end)
{
	m_startHighlight = start;
	m_endHighlight = end;
}

CStdString CGUILabelControl::GetDescription() const
{
	return m_infoLabel.GetLabel(m_parentID);
}