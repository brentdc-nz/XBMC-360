#include "DVDDemuxFFmpeg.h"
#include "..\DVDClock.h"
#include "DVDDemuxUtils.h"

#include "..\DVDUtils\memutil.h"
#include "..\..\..\utils\thread.h"
#include "..\..\..\utils\Log.h"

// threashold for start values in AV_TIME_BASE units
#define PTS_START_THREASHOLD 100000

// class CDemuxStreamVideoFFmpeg
void CDemuxStreamVideoFFmpeg::GetStreamInfo(std::string& strInfo)
{
	if(!pPrivate) return;
	char temp[128];
	avcodec_string(temp, 128, ((AVStream*)pPrivate)->codec, 0);
	strInfo = temp;
}

// class CDemuxStreamVideoFFmpeg
void CDemuxStreamAudioFFmpeg::GetStreamInfo(std::string& strInfo)
{
	if(!pPrivate) return;
	char temp[128];
	avcodec_string(temp, 128, ((AVStream*)pPrivate)->codec, 0);
	strInfo = temp;
}

CDVDDemuxFFmpeg::CDVDDemuxFFmpeg()
{
	m_pFormatContext = NULL;
	InitializeCriticalSection(&m_critSection);
	for (int i = 0; i < MAX_STREAMS; i++) m_streams[i] = NULL;
	m_iCurrentPts = 0LL;
}

CDVDDemuxFFmpeg::~CDVDDemuxFFmpeg()
{
	Dispose();
	DeleteCriticalSection(&m_critSection);
}

void CDVDDemuxFFmpeg::Lock()
{
	EnterCriticalSection(&m_critSection);
}

void CDVDDemuxFFmpeg::Unlock()
{
	LeaveCriticalSection(&m_critSection);
}

bool CDVDDemuxFFmpeg::Open(/*CDVDInputStream* pInput*/string strFileParth)
{
	AVInputFormat* iformat = NULL;
	const char* strFile;
	m_iCurrentPts = 0LL;

	//if (!pInput) return false;

	// register codecs
	av_register_all(); // Move to codec classes??

	strFile = strFileParth.c_str();//m_pInput->GetFileName();

	if(av_open_input_file(&m_pFormatContext, strFile, NULL, 0, NULL)!=0)
	{
		CLog::Log(LOGNOTICE, "Can't open file for reading");
		return false;
	}

	// Retrieve stream information
	if(av_find_stream_info(m_pFormatContext)<0)
	{
		CLog::Log(LOGNOTICE, "Can't fetch info from file");
		return false;
	}

	// add the ffmpeg streams to our own stream array
	for (int i = 0; i < (int)m_pFormatContext->nb_streams; i++)
	{
		AddStream(i);
	}

	return true;
}

void CDVDDemuxFFmpeg::Dispose()
{
	if (m_pFormatContext) av_close_input_file/*avformat_close_input*/(m_pFormatContext);

	for (int i = 0; i < MAX_STREAMS; i++)
	{
		if (m_streams[i]) delete m_streams[i];
		m_streams[i] = NULL;
	}

	m_pFormatContext = NULL;
	//m_pInput = NULL;

	//ContextDeInit();
}

CDemuxStream* CDVDDemuxFFmpeg::GetStream(int iStreamId)
{
	if (iStreamId < 0 || iStreamId >= MAX_STREAMS) return NULL;
	return m_streams[iStreamId];
}

int CDVDDemuxFFmpeg::GetNrOfStreams()
{
	int i = 0;
	while (m_streams[i]) i++;
	return i;
}

CDVDDemux::DemuxPacket* CDVDDemuxFFmpeg::Read()
{
	AVPacket pkt;
	CDVDDemux::DemuxPacket* pPacket = NULL;
	Lock();

	if (m_pFormatContext)
	{
		if (av_read_frame(m_pFormatContext, &pkt) < 0)
		{
			// error reading from stream
			// XXX, just reset eof for now, and let the dvd player decide what todo
//			m_pFormatContext->pb.eof_reached = 0; //FIXME MARTY
			pPacket = NULL;
		}
		else
		{
			// pkt.pts is not the real pts, but a frame number.
			// to get our pts we need to multiply the frame delay with that number
			int num = m_pFormatContext->streams[pkt.stream_index]->time_base.num;
			int den = m_pFormatContext->streams[pkt.stream_index]->time_base.den;
        
			// XXX, in some cases ffmpeg returns a negative packet size
			if (pkt.size <= 0 || num == 0 || den == 0 || pkt.stream_index >= MAX_STREAMS)
			{
				CLog::Log(LOGERROR, "CDVDDemuxFFmpeg::Read() no valid packet");
			}
			else
			{
				pPacket = CDVDDemuxUtils::AllocateDemuxPacket(pkt.size);
				if (pPacket)
				{
					// copy contents into our own packet
					pPacket->iSize = pkt.size;
          
					// maybe we can avoid a memcpy here by detecting where pkt.destruct is pointing too?
					/*fast_*/memcpy(pPacket->pData, pkt.data, pPacket->iSize);
          
					if (pkt.pts == AV_NOPTS_VALUE) pPacket->pts = DVD_NOPTS_VALUE;
					else
					{
						pPacket->pts = (num * pkt.pts * AV_TIME_BASE) / den;
						if (m_pFormatContext->start_time != AV_NOPTS_VALUE)              
						{
							if( pPacket->pts > (unsigned __int64)m_pFormatContext->start_time )
								 pPacket->pts -= m_pFormatContext->start_time;
							else if( pPacket->pts + PTS_START_THREASHOLD > (unsigned __int64)m_pFormatContext->start_time )
								pPacket->pts = 0;
						}
            
						// convert to dvdplayer clock ticks
						pPacket->pts = (pPacket->pts * DVD_TIME_BASE) / AV_TIME_BASE;
					}
          
					if (pkt.dts == AV_NOPTS_VALUE) pPacket->dts = DVD_NOPTS_VALUE;
					else
					{
						pPacket->dts = (num * pkt.dts * AV_TIME_BASE) / den;
						if (m_pFormatContext->start_time !=  AV_NOPTS_VALUE)
						{
							if( pPacket->dts > (unsigned __int64)m_pFormatContext->start_time )
								pPacket->dts -= m_pFormatContext->start_time;
							else if( pPacket->dts + PTS_START_THREASHOLD > (unsigned __int64)m_pFormatContext->start_time )
								pPacket->dts = 0;
						}
            
						// convert to dvdplayer clock ticks
						pPacket->dts = (pPacket->dts * DVD_TIME_BASE) / AV_TIME_BASE;
            
						// used to guess streamlength
						if (pPacket->dts > m_iCurrentPts)
						{
							m_iCurrentPts = pPacket->dts;
						}
					}
          
					pPacket->iStreamId = pkt.stream_index; // XXX just for now
				}
			}
			av_free_packet(&pkt);
		}
	}
	Unlock();

	if (!pPacket) return NULL;

	//FIXME WIP MARTY V
 /* 
  // check streams, can we make this a bit more simple?
  if (pPacket && pPacket->iStreamId >= 0 && pPacket->iStreamId <= MAX_STREAMS)
  {
    if (!m_streams[pPacket->iStreamId] ||
        m_streams[pPacket->iStreamId]->pPrivate != m_pFormatContext->streams[pPacket->iStreamId] ||
        m_streams[pPacket->iStreamId]->codec != m_pFormatContext->streams[pPacket->iStreamId]->codec->codec_id)
    {
      // content has changed, or stream did not yet exist
      AddStream(pPacket->iStreamId);
    }
    // we already check for a valid m_streams[pPacket->iStreamId] above
    else if (m_streams[pPacket->iStreamId]->type == STREAM_AUDIO)
    {
      if (((CDemuxStreamAudio*)m_streams[pPacket->iStreamId])->iChannels != m_pFormatContext->streams[pPacket->iStreamId]->codec->channels ||
          ((CDemuxStreamAudio*)m_streams[pPacket->iStreamId])->iSampleRate != m_pFormatContext->streams[pPacket->iStreamId]->codec->sample_rate)
      {
        // content has changed
        AddStream(pPacket->iStreamId);
      }
    }
    else if (m_streams[pPacket->iStreamId]->type == STREAM_VIDEO)
    {
      if (((CDemuxStreamVideo*)m_streams[pPacket->iStreamId])->iWidth != m_pFormatContext->streams[pPacket->iStreamId]->codec->width ||
          ((CDemuxStreamVideo*)m_streams[pPacket->iStreamId])->iHeight != m_pFormatContext->streams[pPacket->iStreamId]->codec->height)
      {
        // content has changed
        AddStream(pPacket->iStreamId);
      }
    }
  }

  // hack for lpcm pts values, this is not how it should be done!
  // correct pts and dts values should be calculated in the demuxer (ffmpeg avformat)
  CDemuxStream* pDemuxStream = m_streams[pkt.stream_index];
  if (pDemuxStream->codec == CODEC_ID_LPCM_S24BE && pDemuxStream->type == STREAM_AUDIO)
  {
    CDemuxStreamAudioFFmpeg* p = (CDemuxStreamAudioFFmpeg*)pDemuxStream;
    if (pPacket->dts - p->previous_dts > (DVD_TIME_BASE / 50) &&
        pPacket->dts - p->previous_dts < DVD_TIME_BASE)
    {
      pPacket->pts = DVD_NOPTS_VALUE;
      pPacket->dts = DVD_NOPTS_VALUE;
    }
    else p->previous_dts = pPacket->dts;
  }        
*/
	return pPacket;
}

bool CDVDDemuxFFmpeg::Seek(int iTime)
{
	__int64 seek_pts = (__int64)iTime * (AV_TIME_BASE / 1000);
	if (m_pFormatContext->start_time != AV_NOPTS_VALUE && seek_pts < m_pFormatContext->start_time)
	{
		seek_pts += m_pFormatContext->start_time;
	}
  
	Lock();
	int ret = av_seek_frame(m_pFormatContext, -1, seek_pts, seek_pts < 0 ? AVSEEK_FLAG_BACKWARD : 0);
	m_iCurrentPts = 0LL;
	Unlock();
  
	return (ret >= 0);
}

void CDVDDemuxFFmpeg::AddStream(int iId)
{
	AVStream* pStream = m_pFormatContext->streams[iId];
	
	if (pStream)
	{
		if (m_streams[iId]) delete m_streams[iId];

		switch (pStream->codec->codec_type)
		{
			case AVMEDIA_TYPE_AUDIO:
			{
				m_streams[iId] = new CDemuxStreamAudioFFmpeg();
				m_streams[iId]->type = STREAM_AUDIO;
				((CDemuxStreamAudio*)m_streams[iId])->iChannels = pStream->codec->channels;
				((CDemuxStreamAudio*)m_streams[iId])->iSampleRate = pStream->codec->sample_rate;
				break;
			}
			case AVMEDIA_TYPE_VIDEO:
			{
				m_streams[iId] = new CDemuxStreamVideoFFmpeg();
				m_streams[iId]->type = STREAM_VIDEO;
				((CDemuxStreamVideo*)m_streams[iId])->iFpsRate = pStream->avg_frame_rate.num; //MARTY
				((CDemuxStreamVideo*)m_streams[iId])->iFpsScale = pStream->avg_frame_rate.den; //MARTY
//				((CDemuxStreamVideo*)m_streams[iId])->iFpsRate = pStream->codec->frame_rate;
//				((CDemuxStreamVideo*)m_streams[iId])->iFpsScale = pStream->codec->frame_rate_base;
				((CDemuxStreamVideo*)m_streams[iId])->iWidth = pStream->codec->width;
				((CDemuxStreamVideo*)m_streams[iId])->iHeight = pStream->codec->height;
				break;
			}
			case AVMEDIA_TYPE_DATA:
			{
				m_streams[iId] = new CDemuxStream();
				m_streams[iId]->type = STREAM_DATA;
				break;
			}
			default:
			{
				m_streams[iId] = new CDemuxStream();
				m_streams[iId]->type = STREAM_NONE;
				break;
			}
		}

		// generic stuff
		if (pStream->duration != AV_NOPTS_VALUE)
		{
			m_streams[iId]->iDuration = (int)((pStream->duration / AV_TIME_BASE) & 0xFFFFFFFF);
		}

		m_streams[iId]->codec = pStream->codec->codec_id;
		m_streams[iId]->iId = pStream->id;

		// we set this pointer to detect a stream changed inside ffmpeg
		// used to extract info too
		m_streams[iId]->pPrivate = pStream;
	}
}