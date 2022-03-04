#ifndef H_CDVDCODECMISC
#define H_CDVDCODECMISC

#include <string>
#include <vector>

extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __GNUC__
#pragma warning(disable:4244)
#endif
#include "libavcodec/avcodec.h"
}

// 0x100000 Is the video starting range
// 0x200000 Is the audio starting range

// Special options that can be passed to a codec
class CDVDCodecOption
{
public:
	CDVDCodecOption(std::string name, std::string value) { m_name = name; m_value = value; }
	std::string m_name;
	std::string m_value;
};

typedef std::vector<CDVDCodecOption> CDVDCodecOptions;

#endif //H_CDVDCODECMISC