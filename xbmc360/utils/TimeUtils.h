#ifndef H_CTIMEUTILS
#define H_CTIMEUTILS

#include <stdint.h>
#include <time.h>

int64_t CurrentHostCounter(void);
int64_t CurrentHostFrequency(void);

class CTimeUtils
{
public:
	static void UpdateFrameTime(); // Update the frame time.  Not threadsafe
	static unsigned int GetFrameTime(); // Returns the frame time in MS.  Not threadsafe
	static unsigned int GetTimeMS();
	static DWORD timeGetTime(); // Missing from Xbox 360 XDK

private:
	static unsigned int frameTime;
};

#endif //H_CTIMEUTILS

