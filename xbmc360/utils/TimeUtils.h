#ifndef H_CTIMEUTILS
#define H_CTIMEUTILS

#include <stdint.h>
#include <time.h>

enum TIME_FORMAT { TIME_FORMAT_GUESS     =  0,
                   TIME_FORMAT_SS        =  1,
                   TIME_FORMAT_MM        =  2,
                   TIME_FORMAT_MM_SS     =  3,
                   TIME_FORMAT_HH        =  4,
                   TIME_FORMAT_HH_SS     =  5, // not particularly useful
                   TIME_FORMAT_HH_MM     =  6,
                   TIME_FORMAT_HH_MM_SS  =  7,
                   TIME_FORMAT_XX        =  8, // AM/PM
                   TIME_FORMAT_HH_MM_XX  = 14,
                   TIME_FORMAT_H         = 16,
                   TIME_FORMAT_H_MM_SS   = 19 };

int64_t CurrentHostCounter(void);
int64_t CurrentHostFrequency(void);

class CTimeUtils
{
public:
	static void UpdateFrameTime();      ///< update the frame time.  Not threadsafe
	static unsigned int GetFrameTime(); ///< returns the frame time in MS.  Not threadsafe
	static unsigned int GetTimeMS();

private:
	static unsigned int frameTime;
};

#endif //H_CTIMEUTILS

