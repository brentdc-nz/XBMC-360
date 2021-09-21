#include "DateTime.h"
#include "utils\log.h"
#include <time.h>
#include <math.h>

// This class manages the time and date for the XBMC-360 application 
// this is due to NtSetSystemTime() being blocked on retail machines
// it returns "NTSTATUS = STATUS_ACCESS_DENIED". Have tried to patch
// it out with various different homebrew tools without any luck

#pragma warning(disable:4244)

CDateTime::CDateTime()
{
	m_iUnixTime = 0;
	m_dwTick = 0;
	m_bTimeServerSycned = false;

	// UTC time zone by default when not set
	m_iTimeZoneIndex = -1;

	InitializeCriticalSection(&m_critSection);
}

CDateTime::~CDateTime()
{
}

void CDateTime::Initialize()
{
	// Load the time zones
	// TODO: Add DST times as well and make it a GUI option
	m_vecTimeZones.push_back(new CTimeZone("(UTC-12:00) International Date Line West",-12.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-11:00) Coordinated Universal Time-11",-11.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-10:00) Aleutian Islands",-10.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-10:00) Hawaii",-10.0,false,0));	
	m_vecTimeZones.push_back(new CTimeZone("(UTC-09:30) Marquesas Islands",-9.5,false,0));	
	m_vecTimeZones.push_back(new CTimeZone("(UTC-09:00) Alaska",-9.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-09:00) Coordinated Universal Time-09",-9.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-08:00) Baja California",-8.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-08:00) Coordinated Universal Time-08",-8.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-08:00) Pacific Time (US & Canada)",-8.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-07:00) Arizona",-7.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-07:00) Chihuahua, La Paz, Mazatlan",-7.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-07:00) Mountain Time (US & Canada)",-7.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-07:00) Yukon",-7.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-06:00) Central America",-6.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-06:00) Central Time (US & Canada)",-6.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-06:00) Easter Island",-6.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-06:00) Guadalajara, Mexico City, Monterrey",-6.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-06:00) Saskatchewan",-6.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-05:00) Bogota, Lima, Quito, Rio Branco",-5.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-05:00) Chetumal",-5.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-05:00) Eastern Time (US & Canada)",-5.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-05:00) Haiti",-5.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-05:00) Havana",-5.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-05:00) Indiana (East)",-5.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-05:00) Turks and Caicos",-5.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-04:00) Asuncion",-4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-04:00) Atlantic Time (Canada)",-4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-04:00) Caracas",-4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-04:00) Cuiaba",-4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-04:00) Georgetown, La Paz, Manaus, San Juan",-4.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-04:00) Santiago",-4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:30) Newfoundland",-3.5,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:00) Araguaina",-3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:00) Brasilia",-3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:00) Cayenne, Fortaleza",-3.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:00) City of Buenos Aires",-3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:00) Greenland",-3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:00) Montevideo",-3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:00) Punta Arenas",-3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:00) Saint Pierre and Miquelon",-3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-03:00) Salvador",-3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-02:00) Coordinated Universal Time-02",-2.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-02:00) Mid-Atlantic - Old",-2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-01:00) Azores",-1.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC-01:00) Cabo Verde Is",-1.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC) Coordinated Universal Time",0.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+00:00) Dublin, Edinburgh, Lisbon, London",0.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+00:00) Monrovia, Reykjavik",0.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+00:00) Sao Tome",0.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+01:00) Casablanca",0.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna",1.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague",1.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+01:00) Brussels, Copenhagen, Madrid, Paris",1.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+01:00) Sarajevo, Skopje, Warsaw, Zagreb",1.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+01:00) West Central Africa",1.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Amman",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Athens, Bucharest",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Beirut",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Cairo",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Chisinau",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Damascus",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Gaza, Hebron",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Harare, Pretoria",2.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Jerusalem",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Juba",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Kaliningrad",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Khartoum",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Tripoli",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+02:00) Windhoek",2.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+03:00) Baghdad",3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+03:00) Istanbul",3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+03:00) Kuwait, Riyadh",3.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+03:00) Minsk",3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+03:00) Moscow, St. Petersburg",3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+03:00) Nairobi",3.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+03:00) Volgograd",3.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+03:30) Tehran",3.5,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+04:00) Abu Dhabi, Muscat",4.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+04:00) Astrakhan, Ulyanovsk",4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+04:00) Baku",4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+04:00) Izhevsk, Samara",4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+04:00) Port Louis",4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+04:00) Saratov",4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+04:00) Tbilisi",4.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+04:00) Yerevan",4.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+04:30) Kabul",4.5,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+05:00) Ashgabat, Tashkent",5.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+05:00) Ekaterinburg",5.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+05:00) Islamabad, Karachi",5.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+05:00) Qyzylorda",5.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+05:30) Chennai, Kolkata, Mumbai, New Delhi",5.5,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+05:30) Sri Jayawardenepura",5.5,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+05:45) Kathmandu",5.75,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+06:00) Astana",6.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+06:00) Dhaka",6.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+06:00) Omsk",6.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+06:30) Yangon (Rangoon)",6.5,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+07:00) Bangkok, Hanoi, Jakarta",7.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+07:00) Barnaul, Gorno-Altaysk",7.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+07:00) Hovd",7.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+07:00) Krasnoyarsk",7.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+07:00) Novosibirsk",7.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+07:00) Tomsk",7.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+08:00) Beijing, Chongqing, Hong Kong, Urumqi",8.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+08:00) Irkutsk",8.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+08:00) Kuala Lumpur, Singapore",8.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+08:00) Perth",8.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+08:00) Taipei",8.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+08:00) Ulaanbaatar",8.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+08:45) Eucla",8.75,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+09:00) Chita",9.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+09:00) Osaka, Sapporo, Tokyo",9.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+09:00) Pyongyang",9.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+09:00) Seoul",9.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+09:00) Yakutsk",9.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+09:30) Adelaide",9.5,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+09:30) Darwin",9.5,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+10:00) Brisbane",10.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+10:00) Canberra, Melbourne, Sydney",10.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+10:00) Guam, Port Moresby",10.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+10:00) Hobart",10.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+10:00) Vladivostok",10.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+10:30) Lord Howe Island",10.5,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+11:00) Bougainville Island",11.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+11:00) Chokurdakh",11.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+11:00) Magadan",11.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+11:00) Norfolk Island",11.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+11:00) Sakhalin",11.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+11:00) Solomon Is., New Caledonia",11.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+12:00) Anadyr, Petropavlovsk-Kamchatsky",12.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+12:00) Auckland, Wellington",12.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+12:00) Coordinated Universal Time+12",12.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+12:00) Fiji",12.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+12:45) Chatham Islands",12.75,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+13:00) Coordinated Universal Time+13",13.0,false,0));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+13:00) Nuku'alofa",13.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+13:00) Samoa",13.0,true,1));
	m_vecTimeZones.push_back(new CTimeZone("(UTC+14:00) Kiritimati Island",14.0,false,0));
}

void CDateTime::Process() // Keep track using this thread
{
	while(!CThread::m_bStop)
	{
		if((m_dwTick < GetTickCount()) && (m_bTimeServerSycned))
		{
			m_iUnixTime++;
			m_dwTick = GetTickCount() + 1*1000;
		}
		else
			Sleep(10);
	}
}

void CDateTime::SetTimeZoneIndex(int iIndex)
{
	if (iIndex != -1 && iIndex < (int)m_vecTimeZones.size())
		m_iTimeZoneIndex = iIndex;
}

int CDateTime::GetTimeZoneIndex()
{
	return m_iTimeZoneIndex;
}

int CDateTime::GetNumberOfTimeZones()
{
	return m_vecTimeZones.size();
}

const std::string CDateTime::GetTimeZoneString(int iIndex)
{
	if (iIndex != -1 && iIndex < (int)m_vecTimeZones.size())
		return m_vecTimeZones[iIndex]->GetString();
	else
		return "N/A";
}

void CDateTime::SetUTCUnixTime(__int64 iUnixTime)
{
	// Start the time keeping thread as we
	// are now keeping track of our own time
	if(!m_bTimeServerSycned)
	{
		CLog::Log(LOGNOTICE, "CDateTime: Starting time keeping thread.");	
 		CThread::Create(false/*, THREAD_MINSTACKSIZE*/);
	}

	EnterCriticalSection(&m_critSection);

	m_iUnixTime = iUnixTime;

	LeaveCriticalSection(&m_critSection);

	if(!m_bTimeServerSycned)
		m_bTimeServerSycned = true;
}

void CDateTime::XBGetLocalTime(SYSTEMTIME* SystemTime)
{
	__int64 iEpochTime = 0;

	if(m_bTimeServerSycned)
		iEpochTime = m_iUnixTime; // If we have a NTP sync use our own time
	else
		iEpochTime = GetLocalUTCUnixTime(); // If not just use the system time

	// Now adjust for correct time zone
	if (m_iTimeZoneIndex != -1 && m_iTimeZoneIndex < (int)m_vecTimeZones.size())
		iEpochTime += DecimalHoursToMinutes(m_vecTimeZones[m_iTimeZoneIndex]->GetUTCOffset());

	// Convert to correct type
	SYSTEMTIME systm;
	UnixToSystemTime(iEpochTime, &systm);

	memcpy(SystemTime, &systm, sizeof(SYSTEMTIME));
}

void CDateTime::Clear()
{
	// Stop the time keeping thread
	CLog::Log(LOGNOTICE, "CDateTime: Sopping the time keeping thread.");
	CThread::StopThread();

	for (int i=0; i < (int)m_vecTimeZones.size(); ++i)
	{
		CTimeZone* pTimeZone = m_vecTimeZones[i];
		delete  pTimeZone;
	}

	m_vecTimeZones.erase(m_vecTimeZones.begin(), m_vecTimeZones.end());
}

__int64 CDateTime::GetLocalUTCUnixTime()
{
   // Get the number of seconds since January 1, 1970 12:00am UTC
   const __int64 UNIX_TIME_START = 0x019DB1DED53E8000; // January 1, 1970 (start of Unix epoch) in "ticks"
   const __int64 TICKS_PER_SECOND = 10000000; // A tick is 100ns

   FILETIME ft;
   GetSystemTimeAsFileTime(&ft); // Returns ticks in UTC

   // Copy the low and high parts of FILETIME into a LARGE_INTEGER
   // This is so we can access the full 64-bits as an Int64 without causing an alignment fault
   LARGE_INTEGER li;
   li.LowPart  = ft.dwLowDateTime;
   li.HighPart = ft.dwHighDateTime;
 
   // Convert ticks since 1/1/1970 into seconds
   return (li.QuadPart - UNIX_TIME_START) / TICKS_PER_SECOND;
}

// 2000-03-01 (mod 400 year, immediately after Feb 29
#define LEAPOCH (946684800LL + 86400*(31+29))
#define DAYS_PER_400Y (365*400 + 97)
#define DAYS_PER_100Y (365*100 + 24)
#define DAYS_PER_4Y   (365*4   + 1)

bool CDateTime::UnixToSystemTime(long long t, SYSTEMTIME *st)
{
	long long days, secs;
	int remdays, remsecs, remyears;
	int qc_cycles, c_cycles, q_cycles;
	int years, months;
	int wday, yday, leap;
	static const char days_in_month[] = {31,30,31,30,31,31,30,31,30,31,31,29};

	// Reject time_t values whose year would overflow int
	if (t < INT_MIN * 31622400LL || t > INT_MAX * 31622400LL)
		return false;

	secs = t - LEAPOCH;
	days = secs / 86400;
	remsecs = secs % 86400;
	if(remsecs < 0)
	{
		remsecs += 86400;
		days--;
	}

	wday = (3+days)%7;
	if(wday < 0) wday += 7;

	qc_cycles = days / DAYS_PER_400Y;
	remdays = days % DAYS_PER_400Y;
	
	if(remdays < 0)
	{
		remdays += DAYS_PER_400Y;
		qc_cycles--;
	}

	c_cycles = remdays / DAYS_PER_100Y;
	if (c_cycles == 4) c_cycles--;
	remdays -= c_cycles * DAYS_PER_100Y;

	q_cycles = remdays / DAYS_PER_4Y;
	if (q_cycles == 25) q_cycles--;
	remdays -= q_cycles * DAYS_PER_4Y;

	remyears = remdays / 365;
	if (remyears == 4) remyears--;
	remdays -= remyears * 365;

	leap = !remyears && (q_cycles || !c_cycles);
	yday = remdays + 31 + 28 + leap;
	if (yday >= 365+leap) yday -= 365+leap;

	years = remyears + 4*q_cycles + 100*c_cycles + 400*qc_cycles;

	for (months=0; days_in_month[months] <= remdays; months++)
		remdays -= days_in_month[months];

	if (years+100 > INT_MAX || years+100 < INT_MIN)
		return false;

	tm tmp;

	tmp.tm_year = years + 100;
	tmp.tm_mon = months + 2;
	
	if (tmp.tm_mon >= 12)
	{
		tmp.tm_mon -=12;
		tmp.tm_year++;
	}
	
	tmp.tm_mday = remdays + 1;
	tmp.tm_wday = wday;
	tmp.tm_yday = yday;

	tmp.tm_hour = remsecs / 3600;
	tmp.tm_min = remsecs / 60 % 60;
	tmp.tm_sec = remsecs % 60;

	// Now put into the SYSTEMTIME struct
	st->wYear =  1900 + tmp.tm_year;
	st->wMonth =  1 + tmp.tm_mon;
	st->wDayOfWeek = tmp.tm_wday;
	st->wDay = tmp.tm_mday;
	st->wHour = tmp.tm_hour;
	st->wMinute = tmp.tm_min;
	st->wSecond = tmp.tm_sec;
	st->wMilliseconds = 0;

	return true;
}

int CDateTime::DecimalHoursToMinutes(float fHours)
{
	float fMin = fHours * 60;
	return (fMin * 60);
}