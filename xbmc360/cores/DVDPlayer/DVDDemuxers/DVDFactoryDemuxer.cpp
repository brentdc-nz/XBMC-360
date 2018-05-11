#include "DVDFactoryDemuxer.h"
#include "DVDDemuxFFmpeg.h"

CDVDDemux* CDVDFactoryDemuxer::CreateDemuxer(CDVDInputStream* pInputStream)
{
	CDVDDemux* pDemuxer = new CDVDDemuxFFmpeg;
	
	if(pDemuxer->Open(pInputStream))
	return pDemuxer;

	return NULL;
}