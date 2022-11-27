#ifndef CDATETIME_H
#define CDATETIME_H

#include "utils\Stdafx.h"

//  DateTime class, which uses FILETIME as it's base
class CDateTime	
{
public:
	CDateTime();
	virtual ~CDateTime() {}

	const CDateTime& operator =(const FILETIME& right);

	void Reset();
	void SetValid(bool yesNo);
	void SetDateTime(int year, int month, int day, int hour, int minute, int second);

private:
	bool ToFileTime(const SYSTEMTIME& time, FILETIME& fileTime) const;

private:
	FILETIME m_time;

	typedef enum _STATE
	{
		invalid = 0,
		valid
	} STATE;

	STATE m_state;
};

#endif //CDATETIME_H