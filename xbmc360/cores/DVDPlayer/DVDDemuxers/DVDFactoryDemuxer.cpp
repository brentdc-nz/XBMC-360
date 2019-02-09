#include "DVDFactoryDemuxer.h"
#include "DVDDemuxFFmpeg.h"

CDVDDemux* CDVDFactoryDemuxer::CreateDemuxer(CDVDInputStream* pInputStream)
{
	//if (pInputStream->IsStreamType(DVDSTREAM_TYPE_HTTP))
	{
		// TODO: Add more demuxers for other types
	}
  
	return new CDVDDemuxFFmpeg();
}