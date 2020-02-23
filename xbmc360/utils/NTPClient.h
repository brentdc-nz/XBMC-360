#ifndef CNTPCLIENT_H
#define CNTPCLIENT_H

#include "Thread.h"
#include "StdString.h"

// Representation of an NTP timestamp
struct CNtpTimePacket
{
	DWORD m_dwInteger;
	DWORD m_dwFractional;
};

// Helper class to encapulate NTP time stamps
class CNtpTime
{
public:
	CNtpTime();
	CNtpTime(const CNtpTime& time);
	CNtpTime(CNtpTimePacket& packet);
	CNtpTime(const SYSTEMTIME& st);

	// General functions
	CNtpTime& operator=(const CNtpTime& time);
	double operator-(const CNtpTime& time) const;
	CNtpTime operator+(const double& timespan) const;
	operator SYSTEMTIME() const;
	operator CNtpTimePacket() const;
	operator unsigned __int64() const { return m_Time; };
	DWORD Seconds() const;
	DWORD Fraction() const;

	// Static functions
	static CNtpTime GetCurrentTime();
	static DWORD MsToNtpFraction(WORD wMilliSeconds);
	static WORD NtpFractionToMs(DWORD dwFraction);
	static double NtpFractionToSecond(DWORD dwFraction);

protected:
	// Internal static functions and data
	static long GetJulianDay(WORD Year, WORD Month, WORD Day);
	static void GetGregorianDate(long JD, WORD& Year, WORD& Month, WORD& Day);
	static DWORD m_MsToNTP[1000];

	// The actual data
	unsigned __int64 m_Time;
};

struct NtpServerResponse
{
	int m_nLeapIndicator; // 0: No warning
	// 1: Last minute in day has 61 seconds
	// 2: Last minute has 59 seconds
	// 3: Clock not synchronized

	int m_nStratum; // 0: Unspecified or unavailable
	// 1: Primary reference (e.g., radio clock)
	// 2-15: Secondary reference (via NTP or SNTP)
	// 16-255: Reserved
	
	CNtpTime m_OriginateTime;    // Time when the request was sent from the client to the SNTP server
	CNtpTime m_ReceiveTime;      // Time when the request was received by the server
	CNtpTime m_TransmitTime;     // Time when the server sent the request back to the client
	CNtpTime m_DestinationTime;  // Time when the reply was received by the client
	double m_RoundTripDelay;   // Round trip time in seconds
	double m_LocalClockOffset; // Local clock offset relative to the server
};

class CNTPClient : public CThread
{
public:
	CNTPClient();
	~CNTPClient();

	void SyncTime();
	bool UpdateNeeded();
	
private:
	virtual void Process();
	bool DoTimeSync(CStdString strHostName, NtpServerResponse& response, int nPort = 123);
	bool SetClientTime(const CNtpTime& NewTime);

	DWORD m_dwTimeout; // Next point in time we update
};

#endif //CNTPCLIENT_H