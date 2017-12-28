#ifndef CGUIINFOMANAGER_H
#define CGUIINFOMANAGER_H
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
#include "..\utils\StdString.h"
#include <list>

#define OPERATOR_NOT  3
#define OPERATOR_AND  2
#define OPERATOR_OR   1

#define PLAYER_HAS_MEDIA              1

#define PLAYER_SHOWCODEC			30

#define SYSTEM_TIME                 110
#define SYSTEM_DATE                 111
#define SYSTEM_ALWAYS_TRUE          125   // useful for <visible fade="10" start="hidden">true</visible>, to fade in a control
#define SYSTEM_ALWAYS_FALSE         126   // used for <visible fade="10">false</visible>, to fade out a control (ie not particularly useful!)

// the multiple information vector
#define MULTI_INFO_START              40000
#define MULTI_INFO_END                99999
#define COMBINED_VALUES_START        100000

#define CONTROL_HAS_FOCUS           30000

// structure to hold multiple integer data
// for storage referenced from a single integer
class GUIInfo
{
public:
	GUIInfo(int info, uint32_t data1 = 0, int data2 = 0, uint32_t flag = 0)
	{
		m_info = info;
		m_data1 = data1;
		m_data2 = data2;
		if (flag)
			SetInfoFlag(flag);
	}
	bool operator ==(const GUIInfo &right) const
	{
		return (m_info == right.m_info && m_data1 == right.m_data1 && m_data2 == right.m_data2);
	};
	uint32_t GetInfoFlag() const;
	uint32_t GetData1() const;
	int GetData2() const;
	int m_info;
private:
	void SetInfoFlag(uint32_t flag);
	uint32_t m_data1;
	int m_data2;
};

class CGUIInfoManager
{
public:
	CGUIInfoManager(void);
	virtual ~CGUIInfoManager(void);
	
	int TranslateString(const CStdString &strCondition);
	int TranslateSingleString(const CStdString &strCondition);

	CStdString GetLabel(int info, int contextWindow = 0);
	bool GetBool(int condition1);
	void UpdateFPS();

	void SetShowCodec(bool showcodec) { m_playerShowCodec = showcodec; };
	void ToggleShowCodec() { m_playerShowCodec = !m_playerShowCodec; };

	inline float GetFPS() const { return m_fps; };

protected:
	int AddMultiInfo(const GUIInfo &info);
	bool GetMultiInfoBool(const GUIInfo &info, int contextWindow = 0);

	CStdString GetTime(bool bSeconds = false);
	CStdString GetDate(bool bNumbersOnly = false);

private:

	//Fullscreen OSD Stuff
	bool m_playerShowCodec;

	// FPS counters
	float m_fps;
	unsigned int m_frameCounter;
	unsigned int m_lastFPSTime;

	class CCombinedValue
	{
	public:
		CStdString m_info;    // the text expression
		int m_id;             // the id used to identify this expression
		std::list<int> m_postfix;  // the postfix binary expression
		CCombinedValue& operator=(const CCombinedValue& mSrc);
	};
	
	std::vector<CCombinedValue> m_CombinedValues;

	int GetOperator(const char ch);
	int TranslateBooleanExpression(const CStdString &expression);
	bool EvaluateBooleanExpression(const CCombinedValue &expression, bool &result, int contextWindow);

	// Array of multiple information mapped to a single integer lookup
	std::vector<GUIInfo> m_multiInfo;
};

extern CGUIInfoManager g_infoManager;

#endif //CGUIINFOMANAGER_H
