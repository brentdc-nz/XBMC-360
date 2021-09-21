#ifndef CDATETIME_H
#define CDATETIME_H

#include "utils\Thread.h"
#include <vector>

typedef __int64 int64;

class CTimeZone
{
public:
	CTimeZone(std::string strTimeZone, float iOffset, bool bHasDST, float iOffsetDST)
	{
		m_strTimeZone = strTimeZone;
		m_iUTCOffset = iOffset;
		m_bHasDST = bHasDST;
		m_iOffsetDST = iOffsetDST;
	}
	~CTimeZone() { }

	std::string GetString() { return m_strTimeZone; }
	float GetUTCOffset() { return m_iUTCOffset; }

private:
	std::string m_strTimeZone;
	float m_iUTCOffset;
	bool m_bHasDST;
	float m_iOffsetDST;
};

class CDateTime : public CThread
{
public:
	CDateTime();
	~CDateTime();

	void Initialize();
	void SetTimeZoneIndex(int iIndex);
	int GetTimeZoneIndex();
	const std::string GetTimeZoneString(int iIndex);
	int CDateTime::GetNumberOfTimeZones();
	void SetUTCUnixTime(int64 iUnixTime);
	void XBGetLocalTime(SYSTEMTIME* SystemTime);
	void Clear();
	CRITICAL_SECTION m_critSection;

private:
	virtual void Process();
	__int64 GetLocalUTCUnixTime();
	bool UnixToSystemTime(long long t, SYSTEMTIME *st);
	int CDateTime::DecimalHoursToMinutes(float fHours);

	DWORD m_dwTick;
	int64 m_iUnixTime; // EPOCH
	bool m_bTimeServerSycned;

	int m_iTimeZoneIndex;
	std::vector <CTimeZone*>m_vecTimeZones;
};

#endif //CDATETIME_H