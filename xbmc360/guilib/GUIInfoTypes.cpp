/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIInfoTypes.h"
#include "GUIInfoManager.h"
#include "..\utils\log.h"
#include "..\utils\StringUtils.h"

#include <string>

enum EINFOFORMAT { NONE = 0, FORMATINFO, FORMATESCINFO, FORMATVAR };

typedef struct
{
  const char *str;
  EINFOFORMAT  val;
} infoformat;

const static infoformat infoformatmap[] = {{ "$INFO[",    FORMATINFO },
                                           { "$ESCINFO[", FORMATESCINFO},
                                           { "$VAR[",     FORMATVAR}};

CGUIInfoBool::CGUIInfoBool(bool value)
{
	m_info = 0;
	m_value = value;
}

void CGUIInfoBool::Parse(const CStdString &info)
{
	m_info = g_infoManager.TranslateString(info);
	if (m_info == SYSTEM_ALWAYS_TRUE)
	{
		m_value = true;
		m_info = 0;
	}
	else if (m_info == SYSTEM_ALWAYS_FALSE)
	{
		m_value = false;
		m_info = 0;
	}
	else
		m_info = g_infoManager.GetBool(m_info);
}

CGUIInfoLabel::CGUIInfoLabel()
{
}

CGUIInfoLabel::CGUIInfoLabel(const CStdString &label, const CStdString &fallback, int context)
{
	SetLabel(label, fallback, context);
}

void CGUIInfoLabel::SetLabel(const CStdString &label, const CStdString &fallback, int context)
{
	m_fallback = fallback;
	Parse(label, context);
}

void CGUIInfoLabel::Parse(const CStdString &label, int context)
{
	m_info.clear();
	// Step 1: Replace all $LOCALIZE[number] with the real string
	CStdString work = label;//ReplaceLocalize(label); //MARTY FXIME
	// Step 2: Replace all $ADDON[id number] with the real string
//	work = ReplaceAddonStrings(work);
	// Step 3: Find all $INFO[info,prefix,postfix] blocks
	EINFOFORMAT format;
	do
	{
		format = NONE;
		int pos1 = work.size();
		int pos2;
		int len = 0;
		for (size_t i = 0; i < sizeof(infoformatmap) / sizeof(infoformat); i++)
		{
			pos2 = work.Find(infoformatmap[i].str);
			if (pos2 != (int)std::string::npos && pos2 < pos1)
			{
				pos1 = pos2;
				len = strlen(infoformatmap[i].str);
				format = infoformatmap[i].val;
			}
		}

		if (format != NONE)
		{
			if (pos1 > 0)
				m_info.push_back(CInfoPortion(0, work.Left(pos1), ""));

		pos2 = CStringUtils::FindEndBracket(work, '[', ']', pos1 + len);
		if (pos2 > pos1)
		{
			// decipher the block
			CStdString block = work.Mid(pos1 + len, pos2 - pos1 - len);
			CStdStringArray params;
			CStringUtils::SplitString(block, ",", params);
			int info;
			if (format == FORMATVAR)
			{
/*				info = g_infoManager.TranslateSkinVariableString(params[0], context);
				if (info == 0)
					info = g_infoManager.RegisterSkinVariableString(g_SkinInfo.CreateSkinVariable(params[0], context));
				if (info == 0) // skinner didn't define this conditional label!
					CLog::Log(LOGWARNING, "Label Formating: $VAR[%s] is not defined", params[0].c_str());
*/			}
			else
				info = g_infoManager.TranslateString(params[0]);
			CStdString prefix, postfix;
			if (params.size() > 1)
				prefix = params[1];
			if (params.size() > 2)
				postfix = params[2];
			m_info.push_back(CInfoPortion(info, prefix, postfix, format == FORMATESCINFO));
			// and delete it from our work string
			work = work.Mid(pos2 + 1);
		}
		else
		{
			CLog::Log(LOGERROR, "Error parsing label - missing ']' in \"%s\"", label.c_str());
			return;
		}
	}
  }
	while (format != NONE);

	if (!work.IsEmpty())
	m_info.push_back(CInfoPortion(0, work, ""));
}

CStdString CGUIInfoLabel::GetLabel(int contextWindow, bool preferImage) const
{
	CStdString label;
	for (unsigned int i = 0; i < m_info.size(); i++)
	{
		const CInfoPortion &portion = m_info[i];
		if (portion.m_info)
		{
			CStdString infoLabel;
//			if (preferImage) //FIXME MARTY
//				infoLabel = g_infoManager.GetImage(portion.m_info, contextWindow);
			if (infoLabel.IsEmpty())
				infoLabel = g_infoManager.GetLabel(portion.m_info, contextWindow);
			if (!infoLabel.IsEmpty())
				label += portion.GetLabel(infoLabel);
		}
		else
		{ 
			// no info, so just append the prefix
			label += portion.m_prefix;
		}
	}
	if (label.IsEmpty())  // empty label, use the fallback
		return m_fallback;
	return label;
}

bool CGUIInfoLabel::IsConstant() const
{
	return m_info.size() == 0 || (m_info.size() == 1 && m_info[0].m_info == 0);
}

CGUIInfoLabel::CInfoPortion::CInfoPortion(int info, const CStdString &prefix, const CStdString &postfix, bool escaped)
{
	m_info = info;
	m_prefix = prefix;
	m_postfix = postfix;
	m_escaped = escaped;
	// filter our prefix and postfix for comma's
	m_prefix.Replace("$COMMA", ",");
	m_postfix.Replace("$COMMA", ",");
	m_prefix.Replace("$LBRACKET", "["); m_prefix.Replace("$RBRACKET", "]");
	m_postfix.Replace("$LBRACKET", "["); m_postfix.Replace("$RBRACKET", "]");
}

CStdString CGUIInfoLabel::CInfoPortion::GetLabel(const CStdString &info) const
{
	CStdString label = m_prefix + info + m_postfix;
	if (m_escaped) // escape all quotes, then quote
	{
		label.Replace("\"", "\\\"");
		return "\"" + label + "\"";
	}
	return label;
}

