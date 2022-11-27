#include "XBDateTime.h"

CDateTime::CDateTime()
{
	Reset();
}

const CDateTime& CDateTime::operator =(const FILETIME& right)
{
	m_time = right;
	SetValid(true);

	return *this;
}

void CDateTime::Reset()
{
	SetDateTime(1601, 1, 1, 0, 0, 0);
	SetValid(false);
}

void CDateTime::SetValid(bool yesNo)
{
	m_state=yesNo ? valid : invalid;
}

void CDateTime::SetDateTime(int year, int month, int day, int hour, int minute, int second)
{
	SYSTEMTIME st;
	ZeroMemory(&st, sizeof(SYSTEMTIME));

	st.wYear=year;
	st.wMonth=month;
	st.wDay=day;
	st.wHour=hour;
	st.wMinute=minute;
	st.wSecond=second;

	m_state = ToFileTime(st, m_time) ? valid : invalid;
}

bool CDateTime::ToFileTime(const SYSTEMTIME& time, FILETIME& fileTime) const
{
	return SystemTimeToFileTime(&time, &fileTime) == TRUE &&
		(fileTime.dwLowDateTime > 0 || fileTime.dwHighDateTime > 0);
}