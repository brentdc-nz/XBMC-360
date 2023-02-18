#ifndef XBTIMEZONE_H
#define XBTIMEZONE_H

#include <xtl.h>

typedef struct _DST_DATE
{
	BYTE Month;     // 1-12
	BYTE Week;      // 1-5
	BYTE DayOfWeek; // 0-6
	BYTE Hour;      // 0-24?
} DST_DATE;

typedef struct _MINI_TZI
{
	const char * Name;
	SHORT        Bias;
	const char * StandardName;
	DST_DATE     StandardDate;
	SHORT        StandardBias;
	const char * DaylightName;
	DST_DATE     DaylightDate;
	SHORT        DaylightBias;
} MINI_TZI;

extern const MINI_TZI g_TimeZoneInfo[];

class XBTimeZone
{
public:
	static const char * GetTimeZoneString(int index);
	static const char * GetTimeZoneName(int index);

	static int GetNumberOfTimeZones();
	static int GetTimeZoneIndex();
};

extern XBTimeZone g_timezone;

#endif //XBTIMEZONE_H