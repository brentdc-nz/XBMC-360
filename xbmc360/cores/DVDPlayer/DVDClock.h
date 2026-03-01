#ifndef H_CDVDCLOCK
#define H_CDVDCLOCK

#include "utils\SharedSection.h"
#include "utils\CriticalSection.h"

#define DVD_TIME_BASE 1000000
#define DVD_NOPTS_VALUE    (-1LL<<52) // Should be possible to represent in both double and int64_t

#define DVD_TIME_TO_SEC(x)  ((int)((double)(x) / DVD_TIME_BASE))
#define DVD_TIME_TO_MSEC(x) ((int)((double)(x) * 1000 / DVD_TIME_BASE))
#define DVD_SEC_TO_TIME(x)  ((double)(x) * DVD_TIME_BASE)
#define DVD_MSEC_TO_TIME(x) ((double)(x) * DVD_TIME_BASE / 1000)

#define DVD_PLAYSPEED_PAUSE       0       // frame stepping
#define DVD_PLAYSPEED_NORMAL      1000

class CDVDClock
{
public:
	CDVDClock();
	~CDVDClock();

	double GetClock();
	double GetClock(double& absolute);

	void Discontinuity(double currentPts = 0LL);
 
	void Reset() { m_bReset = true; }
	void Pause();
	void Resume();
	void SetSpeed(int iSpeed);

	static double GetAbsoluteClock();
	static double GetFrequency() { return (double)m_systemFrequency.QuadPart ; }

protected:
	static void CheckSystemClock();
	static double SystemToAbsolute(LARGE_INTEGER system);
	double SystemToPlaying(LARGE_INTEGER system);

	CSharedSection m_critSection;
	LARGE_INTEGER m_systemUsed;  
	LARGE_INTEGER m_startClock;
	LARGE_INTEGER m_pauseClock;
	double m_iDisc;
	bool m_bReset;
  
	static LARGE_INTEGER m_systemFrequency;
	static CCriticalSection m_systemsection;
};

#endif //H_CDVDCLOCK