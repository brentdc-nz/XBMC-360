#include "DVDDemuxUtils.h"
#include "..\DVDClock.h"
#include "utils\Log.H"

extern "C"
{
#include "libavcodec/avcodec.h"
}

void CDVDDemuxUtils::FreeDemuxPacket(DemuxPacket* pPacket)
{
	if(pPacket)
	{
		try
		{
			if(pPacket->pData) _aligned_free(pPacket->pData);    
				delete pPacket;
		}
		catch(...)
		{
			CLog::Log(LOGERROR, "%s - Exception thrown while freeing packet", __FUNCTION__);
		}
	}
}

DemuxPacket* CDVDDemuxUtils::AllocateDemuxPacket(int iDataSize)
{
	DemuxPacket* pPacket = new DemuxPacket;
	if(!pPacket) return NULL;

	try
	{
		memset(pPacket, 0, sizeof(DemuxPacket));

		if(iDataSize > 0)
		{
			// Need to allocate a few bytes more.
			// From avcodec.h (ffmpeg)
			/**
			* Required number of additionally allocated bytes at the end of the input bitstream for decoding.
			* this is mainly needed because some optimized bitstream readers read 
			* 32 or 64 bit at once and could read over the end<br>
			* Note, if the first 23 bits of the additional bytes are not 0 then damaged
			* MPEG bitstreams could cause overread and segfault
			*/ 
			pPacket->pData =(BYTE*)_aligned_malloc(iDataSize + FF_INPUT_BUFFER_PADDING_SIZE, 16);    
			if(!pPacket->pData)
			{
				FreeDemuxPacket(pPacket);
				return NULL;
			}

			// Reset the last 8 bytes to 0;
			memset(pPacket->pData + iDataSize, 0, FF_INPUT_BUFFER_PADDING_SIZE);
		}

		// Setup defaults
		pPacket->dts       = DVD_NOPTS_VALUE;
		pPacket->pts       = DVD_NOPTS_VALUE;
		pPacket->iStreamId = -1;
	}
	catch(...)
	{
		CLog::Log(LOGERROR, "%s - Exception thrown", __FUNCTION__);
		FreeDemuxPacket(pPacket);
		pPacket = NULL;
	}  

	return pPacket;
}