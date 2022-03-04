#ifndef H_CDVDDEMUXUTILS
#define H_CDVDDEMUXUTILS

#include "DVDDemux.h"

class CDVDDemuxUtils
{
public:
	static void FreeDemuxPacket(DemuxPacket* pPacket);
	static DemuxPacket* AllocateDemuxPacket(int iDataSize = 0);
};


#endif //H_CDVDDEMUXUTILS