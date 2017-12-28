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

#ifndef H_CGUIINFOTYPES
#define H_CGUIINFOTYPES

#include "..\utils\StdString.h"

class CGUIInfoBool
{
public:
	CGUIInfoBool(bool value = false);
	operator bool() const { return m_value; };

	void Parse(const CStdString &label);

private:
	int m_info;
	bool m_value;
};


class CGUIInfoLabel
{
public:
	CGUIInfoLabel();
	CGUIInfoLabel(const CStdString &label, const CStdString &fallback = "", int context = 0);
	void SetLabel(const CStdString &label, const CStdString &fallback, int context = 0);


	CStdString GetLabel(int contextWindow, bool preferImage = false) const;
	bool IsConstant() const;

private:
	void Parse(const CStdString &label, int context);
	
	class CInfoPortion
	{
	public:
		CInfoPortion(int info, const CStdString &prefix, const CStdString &postfix, bool escaped = false);
		CStdString GetLabel(const CStdString &info) const;
		int m_info;
		CStdString m_prefix;
		CStdString m_postfix;
	private:
		bool m_escaped;
  };


	CStdString m_fallback;	
	std::vector<CInfoPortion> m_info;
};

#endif //H_CGUIINFOTYPES