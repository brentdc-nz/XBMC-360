#include "DVDDemuxFFmpeg.h"
#include "..\DVDClock.h" // For DVD_TIME_BASE
#include "Utils\Log.h"
#include "DVDDemuxUtils.h"
#include "utils\Win32Exception.h"

//FFMPEG
extern "C" 
{
#include "stdint.h"
#include "libavutil\avutil.h"
#include "libavcodec\avcodec.h"
}

extern "C" void ff_read_frame_flush(AVFormatContext *s);

#ifdef _MSC_VER
static __declspec(thread) CDVDDemuxFFmpeg* g_demuxer = 0;
#else
static TLS g_tls;
#define g_demuxer (*((CDVDDemuxFFmpeg**)g_tls.Get()))
#endif

typedef int64_t offset_t;

void CDemuxStreamAudioFFmpeg::GetStreamInfo(std::string& strInfo)
{
	if(!m_stream) return;
	char temp[128];
	/*m_parent->m_dllAvCodec.*/avcodec_string(temp, 128, m_stream->codec, 0);
	strInfo = temp;
}

void CDemuxStreamAudioFFmpeg::GetStreamName(std::string& strInfo)
{
	if(!m_stream) return;
	if(!m_description.empty())
		strInfo = m_description;
	else
		CDemuxStream::GetStreamName(strInfo);
}

void CDemuxStreamSubtitleFFmpeg::GetStreamName(std::string& strInfo)
{
	if(!m_stream) return;
	if(!m_description.empty())
		strInfo = m_description;
	else
		CDemuxStream::GetStreamName(strInfo);
}

void CDemuxStreamVideoFFmpeg::GetStreamInfo(std::string& strInfo) // DVDTODO
{
	if(!m_stream) return;
	char temp[128];
	/*m_parent->m_dllAvCodec.*/avcodec_string(temp, 128, m_stream->codec, 0);
	strInfo = temp;
}

void CDemuxStreamSubtitleFFmpeg::GetStreamInfo(std::string& strInfo)
{
	if(!m_stream) return;
	char temp[128];
	/*m_parent->m_dllAvCodec.*/avcodec_string(temp, 128, m_stream->codec, 0);
	strInfo = temp;
}

static int interrupt_cb(void)
{
	if(g_demuxer && g_demuxer->Aborted())
		return 1;

	return 0;
}

static offset_t dvd_file_seek(void *h, offset_t pos, int whence)
{
	if(interrupt_cb())
		return -1;

	CDVDInputStream* pInputStream = (CDVDInputStream*)h;

	if(whence == AVSEEK_SIZE)
		return pInputStream->GetLength();
	else
		return pInputStream->Seek(pos, whence & ~AVSEEK_FORCE);
}

static int dvd_file_read(void *h, uint8_t* buf, int size)
{
	if(interrupt_cb())
		return -1;

	CDVDInputStream* pInputStream = (CDVDInputStream*)h;

	return pInputStream->Read(buf, size);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

CDVDDemuxFFmpeg::CDVDDemuxFFmpeg() : CDVDDemux()
{
	m_pFormatContext = NULL;
	m_pInput = NULL;
	m_ioContext = NULL;
	InitializeCriticalSection(&m_critSection);
	for (int i = 0; i < MAX_STREAMS; i++) m_streams[i] = NULL;
	m_iCurrentPts = DVD_NOPTS_VALUE;
}

CDVDDemuxFFmpeg::~CDVDDemuxFFmpeg()
{
}

void CDVDDemuxFFmpeg::Dispose()
{
	g_demuxer = this;

	if(m_pFormatContext)
	{
		if(m_ioContext)
		{
			if(m_pFormatContext->pb && m_pFormatContext->pb != m_ioContext)
			{
				CLog::Log(LOGWARNING, "CDVDDemuxFFmpeg::Dispose - demuxer changed our byte context behind our back, possible memleak");
				m_ioContext = m_pFormatContext->pb;
			}

			/*m_dllAvFormat.*/av_close_input_stream(m_pFormatContext);

			if(m_ioContext->buffer)
				/*m_dllAvUtil.*/av_free(m_ioContext->buffer);

			/*m_dllAvUtil.*/av_free(m_ioContext);
		}
		else
			/*m_dllAvFormat.*/av_close_input_file(m_pFormatContext);
	}

	m_ioContext = NULL;
	m_pFormatContext = NULL;
	m_speed = DVD_PLAYSPEED_NORMAL;

	for(int i = 0; i < MAX_STREAMS; i++)
	{
		if(m_streams[i])
		{
			if(m_streams[i]->ExtraData)
				delete[] (BYTE*)(m_streams[i]->ExtraData);

			delete m_streams[i];
		}
		m_streams[i] = NULL;
	}
	m_pInput = NULL;

#if 0//ndef_HANDLINKED
	m_dllAvFormat.Unload();
	m_dllAvCodec.Unload();
	m_dllAvUtil.Unload();
#endif
}

void CDVDDemuxFFmpeg::Reset()
{
	CDVDInputStream* pInputStream = m_pInput;
	Dispose();
	Open(pInputStream);
}

void CDVDDemuxFFmpeg::Abort()
{
	m_timeout = 1;
}

void CDVDDemuxFFmpeg::Flush()
{
	g_demuxer = this;

	// Naughty usage of an internal ffmpeg function
	if(m_pFormatContext)
	{
//		/*m_dllAvFormat.*/av_read_frame_flush(m_pFormatContext);
		/*m_dllAvFormat.*/ff_read_frame_flush(m_pFormatContext);
	}

	m_iCurrentPts = DVD_NOPTS_VALUE;
}

bool CDVDDemuxFFmpeg::Open(CDVDInputStream* pInput)
{
	AVInputFormat* iformat = NULL;
	std::string strFile;
	m_iCurrentPts = DVD_NOPTS_VALUE;
	m_speed = DVD_PLAYSPEED_NORMAL;
	g_demuxer = this;

	if(!pInput) return false;

#if 0 //ndef _HARDLINKED
	if (!m_dllAvUtil.Load() || !m_dllAvCodec.Load() || !m_dllAvFormat.Load())
	{
		CLog::Log(LOGERROR,"CDVDDemuxFFmpeg::Open - failed to load ffmpeg libraries");
		return false;
	}
#endif

	// Register codecs
	/*m_dllAvFormat.*/av_register_all();
	/*m_dllAvFormat.*/url_set_interrupt_cb(interrupt_cb);

	m_pInput = pInput;
	strFile = m_pInput->GetFileName();

	bool streaminfo = true; // Set to true if we want to look for streams before playback

	if(m_pInput->GetContent().length() > 0)
	{
		std::string content = m_pInput->GetContent();

		// Check if we can get a hint from content
		if(content.compare("video/x-vobsub") == 0)
			iformat = /*m_dllAvFormat.*/av_find_input_format("mpeg");
		else if(content.compare("video/x-dvd-mpeg") == 0 )
			iformat = /*m_dllAvFormat.*/av_find_input_format("mpeg");
		else if(content.compare("video/x-mpegts") == 0 )
			iformat = /*m_dllAvFormat.*/av_find_input_format("mpegts");
	}

	// Try to abort after 30 seconds
	m_timeout = GetTickCount() + 30000;

	if(m_pInput->IsStreamType(DVDSTREAM_TYPE_FFMPEG))
	{
		// Special stream type that makes avformat handle file opening
		// allows internal ffmpeg protocols to be used
		if(/*m_dllAvFormat.*/av_open_input_file(&m_pFormatContext, strFile.c_str(), iformat, FFMPEG_FILE_BUFFER_SIZE, NULL) < 0)
		{
			CLog::Log(LOGDEBUG, "Error, could not open file %s", strFile.c_str());
			Dispose();
			return false;
		}
	}
	else
	{
		unsigned char* buffer = (unsigned char*)/*m_dllAvUtil.*/av_malloc(FFMPEG_FILE_BUFFER_SIZE);
		m_ioContext = /*m_dllAvFormat.*/av_alloc_put_byte(buffer, FFMPEG_FILE_BUFFER_SIZE, 0, m_pInput, dvd_file_read, NULL, dvd_file_seek);
		m_ioContext->max_packet_size = m_pInput->GetBlockSize();
		
		if(m_ioContext->max_packet_size)
			m_ioContext->max_packet_size *= FFMPEG_FILE_BUFFER_SIZE / m_ioContext->max_packet_size;

		if(m_pInput->Seek(0, SEEK_POSSIBLE) == 0)
			m_ioContext->is_streamed = 1;

		if(iformat == NULL)
		{
			// Let ffmpeg decide which demuxer we have to open
			AVProbeData pd;
			BYTE probe_buffer[FFMPEG_FILE_BUFFER_SIZE + AVPROBE_PADDING_SIZE];

			// Init probe data
			pd.buf = probe_buffer;
			pd.filename = strFile.c_str();

			// Read data using avformat's buffers
			pd.buf_size = /*m_dllAvFormat.*/get_buffer(m_ioContext, pd.buf, m_ioContext->max_packet_size ? m_ioContext->max_packet_size : m_ioContext->buffer_size);
			if(pd.buf_size <= 0)
			{
				CLog::Log(LOGERROR, "%s - error reading from input stream, %s", __FUNCTION__, strFile.c_str());
				return false;
			}
			memset(pd.buf+pd.buf_size, 0, AVPROBE_PADDING_SIZE);

			// Restore position again
			/*m_dllAvFormat.*/url_fseek(m_ioContext , 0, SEEK_SET);

			iformat = /*m_dllAvFormat.*/av_probe_input_format(&pd, 1);

			if(!iformat)
			{
				std::string content = m_pInput->GetContent();

				// Check if we can get a hint from content
				if(content.compare("audio/aacp") == 0)
					iformat = /*m_dllAvFormat.*/av_find_input_format("aac");
				else if(content.compare("audio/aac") == 0)
					iformat = /*m_dllAvFormat.*/av_find_input_format("aac");
				else if(content.compare("video/flv") == 0)
					iformat = /*m_dllAvFormat.*/av_find_input_format("flv");
				else if(content.compare("video/x-flv") == 0)
					iformat = /*m_dllAvFormat.*/av_find_input_format("flv");
			}

			if (!iformat)
			{
				// av_probe_input_format failed, re-probe the ffmpeg/ffplay method.
				// av_open_input_file uses av_probe_input_format2 for probing format,
				// starting at 2048, up to max buffer size of 1048576. We just probe to
				// the buffer size allocated above so as to avoid seeks on content that
				// might not be seekable.
				int max_buf_size = pd.buf_size;
				for(int probe_size=min(2048, pd.buf_size); probe_size <= max_buf_size && !iformat; probe_size<<=1)
				{
					CLog::Log(LOGDEBUG, "%s - probing failed, re-probing with probe size [%d]", __FUNCTION__, probe_size);
					int score= probe_size < max_buf_size ? AVPROBE_SCORE_MAX/4 : 0;
					pd.buf_size = probe_size;
					iformat = /*m_dllAvFormat.*/av_probe_input_format2(&pd, 1, &score);
				}
			}
			if(!iformat)
			{
				CLog::Log(LOGERROR, "%s - error probing input format, %s", __FUNCTION__, strFile.c_str());
				return false;
			}
			else
			{
				if(iformat->name)
					CLog::Log(LOGDEBUG, "%s - probing detected format [%s]", __FUNCTION__, iformat->name);
				else
					CLog::Log(LOGDEBUG, "%s - probing detected unnamed format", __FUNCTION__);
			}
		}

		// Open the demuxer
		if(/*m_dllAvFormat.*/av_open_input_stream(&m_pFormatContext, m_ioContext, strFile.c_str(), iformat, NULL) < 0)
		{
			CLog::Log(LOGERROR, "%s - Error, could not open file %s", __FUNCTION__, strFile.c_str());
			Dispose();
			return false;
		}
	}

	// We need to know if this is matroska or avi later
	m_bMatroska = strncmp(m_pFormatContext->iformat->name, "matroska", 8) == 0;	// for "matroska.webm"
	m_bAVI = strcmp(m_pFormatContext->iformat->name, "avi") == 0;

	// In combination with libdvdnav seek, av_find_stream_info wont work
	// so we do this for files only
	if(streaminfo)
	{
		// To speed up dvd switches, only analyse very short
		if(m_pInput->IsStreamType(DVDSTREAM_TYPE_DVD))
			m_pFormatContext->max_analyze_duration = 500000;

		CLog::Log(LOGDEBUG, "%s - av_find_stream_info starting", __FUNCTION__);
		int iErr = /*m_dllAvFormat.*/av_find_stream_info(m_pFormatContext);
		if(iErr < 0)
		{
			CLog::Log(LOGWARNING,"could not find codec parameters for %s", strFile.c_str());
			if(m_pInput->IsStreamType(DVDSTREAM_TYPE_DVD) || (m_pFormatContext->nb_streams == 1 && m_pFormatContext->streams[0]->codec->codec_id == CODEC_ID_AC3))
			{
				// special case, our codecs can still handle it.
			}
			else
			{
				Dispose();
				return false;
			}
		}
		CLog::Log(LOGDEBUG, "%s - av_find_stream_info finished", __FUNCTION__);
	}

	// Reset any timeout
	m_timeout = 0;

	// If format can be nonblocking, let's use that
	m_pFormatContext->flags |= AVFMT_FLAG_NONBLOCK | AVFMT_FLAG_GENPTS;

	// Print some extra information
	/*m_dllAvFormat.*/dump_format(m_pFormatContext, 0, strFile.c_str(), 0);

	UpdateCurrentPTS();

	// Add the ffmpeg streams to our own stream array
	if(m_pFormatContext->nb_programs)
	{
		m_program = UINT_MAX;
		// Look for first non empty stream and discard nonselected programs
		for(unsigned int i = 0; i < m_pFormatContext->nb_programs; i++)
		{
			if(m_program == UINT_MAX && m_pFormatContext->programs[i]->nb_stream_indexes > 0)
				m_program = i;

			if(i != m_program)
				m_pFormatContext->programs[i]->discard = AVDISCARD_ALL;
		}
		if(m_program == UINT_MAX)
			m_program = 0;

		// Add streams from selected program
		for(unsigned int i = 0; i < m_pFormatContext->programs[m_program]->nb_stream_indexes; i++)
			AddStream(m_pFormatContext->programs[m_program]->stream_index[i]);
	}
	else
	{
		for(unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
			AddStream(i);
	}

	return true;
}

void CDVDDemuxFFmpeg::AddStream(int iId)
{
	AVStream* pStream = m_pFormatContext->streams[iId];
	if(pStream)
	{
		CDemuxStream* old = m_streams[iId];

		switch (pStream->codec->codec_type)
		{
			case CODEC_TYPE_AUDIO:
			{
				CDemuxStreamAudioFFmpeg* st = new CDemuxStreamAudioFFmpeg(this, pStream);
				m_streams[iId] = st;
				st->iChannels = pStream->codec->channels;
				st->iSampleRate = pStream->codec->sample_rate;
				st->iBlockAlign = pStream->codec->block_align;
				st->iBitRate = pStream->codec->bit_rate;
				st->iBitsPerSample = pStream->codec->bits_per_coded_sample;
	
				if(/*m_dllAvFormat.*/av_metadata_get(pStream->metadata, "title", NULL, 0))
					st->m_description = /*m_dllAvFormat.*/av_metadata_get(pStream->metadata, "title", NULL, 0)->value;

				break;
			}
			case CODEC_TYPE_VIDEO:
			{
				CDemuxStreamVideoFFmpeg* st = new CDemuxStreamVideoFFmpeg(this, pStream);
				m_streams[iId] = st;

				if(strcmp(m_pFormatContext->iformat->name, "flv") == 0)
					st->bVFR = true;
				else
					st->bVFR = false;

				// Never trust pts in avi files with h264.
				if(m_bAVI && pStream->codec->codec_id == CODEC_ID_H264)
					st->bPTSInvalid = true;

				//Average fps is more accurate for mkv files
				if(m_bMatroska && pStream->avg_frame_rate.den && pStream->avg_frame_rate.num)
				{
					st->iFpsRate = pStream->avg_frame_rate.num;
					st->iFpsScale = pStream->avg_frame_rate.den;
				}
				else if(pStream->r_frame_rate.den && pStream->r_frame_rate.num)
				{
					st->iFpsRate = pStream->r_frame_rate.num;
					st->iFpsScale = pStream->r_frame_rate.den;
				}
				else
				{
					st->iFpsRate  = 0;
					st->iFpsScale = 0;
				}
				st->iWidth = pStream->codec->width;
				st->iHeight = pStream->codec->height;

				if (pStream->sample_aspect_ratio.num == 0)
					st->fAspect = 0.0;
				else
					st->fAspect = (float)av_q2d(pStream->sample_aspect_ratio) * pStream->codec->width / pStream->codec->height;

				if(m_pInput->IsStreamType(DVDSTREAM_TYPE_DVD))
				{
					if(pStream->codec->codec_id == CODEC_ID_PROBE)
					{
						// Fix MPEG-1/MPEG-2 video stream probe returning CODEC_ID_PROBE for still frames.
						// ffmpeg issue 1871, regression from ffmpeg r22831.
						if((pStream->id & 0xF0) == 0xE0)
						{
							pStream->codec->codec_id = CODEC_ID_MPEG2VIDEO;
							pStream->codec->codec_tag = MKTAG('M','P','2','V');
							CLog::Log(LOGERROR, "%s - CODEC_ID_PROBE detected, forcing CODEC_ID_MPEG2VIDEO", __FUNCTION__);
						}
					}
				}
				break;
			}
			case CODEC_TYPE_DATA:
			{
				m_streams[iId] = new CDemuxStream();
				m_streams[iId]->type = STREAM_DATA;
				break;
			}
			case CODEC_TYPE_SUBTITLE:
			{
				{
					CDemuxStreamSubtitleFFmpeg* st = new CDemuxStreamSubtitleFFmpeg(this, pStream);
					m_streams[iId] = st;
				
					if(pStream->codec)
						st->identifier = pStream->codec->sub_id;
	    
					if(/*m_dllAvFormat.*/av_metadata_get(pStream->metadata, "title", NULL, 0))
						st->m_description = /*m_dllAvFormat.*/av_metadata_get(pStream->metadata, "title", NULL, 0)->value;
	
					break;
				}
			}
			case CODEC_TYPE_ATTACHMENT:
			{
				// mkv attachments. Only bothering with fonts for now.
				if(pStream->codec->codec_id == CODEC_ID_TTF)
				{
/*					std::string fileName = "special://temp/fonts/";
					DIRECTORY::CDirectory::Create(fileName);
					fileName += pStream->filename;
					XFILE::CFile file;

					if(pStream->codec->extradata && file.OpenForWrite(fileName))
					{
						file.Write(pStream->codec->extradata, pStream->codec->extradata_size);
						file.Close();
					}
*/				}
				m_streams[iId] = new CDemuxStream();
				m_streams[iId]->type = STREAM_NONE;
				break;
			}
		default:
		{
			m_streams[iId] = new CDemuxStream();
			m_streams[iId]->type = STREAM_NONE;
			break;
		}
	}

	// Delete old stream after new is created
	// since dvdplayer uses the pointer to know
	// if something changed in the demuxer
	if(old)
	{
		if( old->ExtraData ) delete[] (BYTE*)(old->ExtraData);
			delete old;
	}

	// Generic stuff
	if(pStream->duration != (int64_t)AV_NOPTS_VALUE) m_streams[iId]->iDuration = (int)((pStream->duration / AV_TIME_BASE) & 0xFFFFFFFF);

	m_streams[iId]->codec = pStream->codec->codec_id;
	m_streams[iId]->codec_fourcc = pStream->codec->codec_tag;
	m_streams[iId]->iId = iId;
	m_streams[iId]->source = STREAM_SOURCE_DEMUX;
	m_streams[iId]->pPrivate = pStream;
	m_streams[iId]->flags = (CDemuxStream::EFlags)pStream->disposition;

	strcpy( m_streams[iId]->language, pStream->language );

	if(pStream->codec->extradata && pStream->codec->extradata_size > 0)
	{
		m_streams[iId]->ExtraSize = pStream->codec->extradata_size;
		m_streams[iId]->ExtraData = new BYTE[pStream->codec->extradata_size];
		memcpy(m_streams[iId]->ExtraData, pStream->codec->extradata, pStream->codec->extradata_size);
	}

	if(m_pInput->IsStreamType(DVDSTREAM_TYPE_DVD))
    {
		// This stuff is really only valid for dvd's.
		// this is so that the physicalid matches the
		// id's reported from libdvdnav
		switch(m_streams[iId]->codec)
		{
			case CODEC_ID_AC3:
				m_streams[iId]->iPhysicalId = pStream->id - 128;
				break;
			case CODEC_ID_DTS:
				m_streams[iId]->iPhysicalId = pStream->id - 136;
				break;
			case CODEC_ID_MP2:
				m_streams[iId]->iPhysicalId = pStream->id - 448;
				break;
			case CODEC_ID_PCM_S16BE:
				m_streams[iId]->iPhysicalId = pStream->id - 160;
				break;
			case CODEC_ID_DVD_SUBTITLE:
				m_streams[iId]->iPhysicalId = pStream->id - 0x20;
				break;
			default:
				m_streams[iId]->iPhysicalId = pStream->id & 0x1f;
				break;
			}
		}
		else
			m_streams[iId]->iPhysicalId = pStream->id;
	}
}

double CDVDDemuxFFmpeg::ConvertTimestamp(int64_t pts, int den, int num)
{
	if(pts == (int64_t)AV_NOPTS_VALUE)
		return DVD_NOPTS_VALUE;

	// Do calculations in floats as they can easily overflow otherwise
	// we don't care for having a completly exact timestamp anyway
	double timestamp = (double)pts * num  / den;
	double starttime = 0.0f;

	// For dvd's we need the original time
//	if(m_pInput->IsStreamType(DVDSTREAM_TYPE_DVD))
//		starttime = static_cast<CDVDInputStreamNavigator*>(m_pInput)->GetTimeStampCorrection() / DVD_TIME_BASE;
/*	else*/ if(m_pFormatContext->start_time != (int64_t)AV_NOPTS_VALUE)
		starttime = (double)m_pFormatContext->start_time / AV_TIME_BASE;

	if(timestamp > starttime)
		timestamp -= starttime;
	else if( timestamp + 0.1f > starttime )
		timestamp = 0;

	return timestamp*DVD_TIME_BASE;
}

void CDVDDemuxFFmpeg::UpdateCurrentPTS()
{
	m_iCurrentPts = DVD_NOPTS_VALUE;
	for(unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
	{
		AVStream *stream = m_pFormatContext->streams[i];
		if(stream && stream->cur_dts != (int64_t)AV_NOPTS_VALUE)
		{
			double ts = ConvertTimestamp(stream->cur_dts, stream->time_base.den, stream->time_base.num);
			if(m_iCurrentPts == DVD_NOPTS_VALUE || m_iCurrentPts > ts )
				m_iCurrentPts = ts;
		}
	}
}

int CDVDDemuxFFmpeg::GetStreamLength()
{
	if(!m_pFormatContext)
		return 0;

	// Apperently ffmpeg messes up sometimes, so check for negative value too
	if(m_pFormatContext->duration == (int64_t)AV_NOPTS_VALUE || m_pFormatContext->duration < 0LL)
	{
		// no duration is available for us
		// try to calculate it
		int iLength = 0;
		if(m_iCurrentPts != DVD_NOPTS_VALUE && m_pFormatContext->file_size > 0 && m_pFormatContext->pb && m_pFormatContext->pb->pos > 0)
		{
			iLength = (int)(((m_iCurrentPts * m_pFormatContext->file_size) / m_pFormatContext->pb->pos) / 1000) & 0xFFFFFFFF;
		}
		return iLength;
	}

	return (int)(m_pFormatContext->duration / (AV_TIME_BASE / 1000));
}

void CDVDDemuxFFmpeg::SetSpeed(int iSpeed)
{
	g_demuxer = this;

	if(!m_pFormatContext)
		return;

	if(m_speed != DVD_PLAYSPEED_PAUSE && iSpeed == DVD_PLAYSPEED_PAUSE)
	{
		m_pInput->Pause((double)m_iCurrentPts);
		/*m_dllAvFormat.*/av_read_pause(m_pFormatContext);
	}
	else if(m_speed == DVD_PLAYSPEED_PAUSE && iSpeed != DVD_PLAYSPEED_PAUSE)
	{
		m_pInput->Pause((double)m_iCurrentPts);
		/*m_dllAvFormat.*/av_read_play(m_pFormatContext);
	}

	m_speed = iSpeed;

	AVDiscard discard = AVDISCARD_NONE;

	if(m_speed > 4*DVD_PLAYSPEED_NORMAL)
		discard = AVDISCARD_NONKEY;
	else if(m_speed > 2*DVD_PLAYSPEED_NORMAL)
		discard = AVDISCARD_BIDIR;
	else if(m_speed < DVD_PLAYSPEED_PAUSE)
		discard = AVDISCARD_NONKEY;

	for(unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
	{
		if(m_pFormatContext->streams[i])
		{
			if(m_pFormatContext->streams[i]->discard != AVDISCARD_ALL)
				m_pFormatContext->streams[i]->discard = discard;
		}
	}
}

std::string CDVDDemuxFFmpeg::GetFileName()
{
	if(m_pInput && m_pInput)
		return m_pInput->GetFileName();
	else
		return "";
}

DemuxPacket* CDVDDemuxFFmpeg::Read()
{
	g_demuxer = this;

	AVPacket pkt;
	DemuxPacket* pPacket = NULL;

	// On some cases where the received packet is invalid we will need to return an empty packet (0 length) 
	// otherwise the main loop (in CDVDPlayer) would consider this the end of stream and stop.
	bool bReturnEmpty = false;
	Lock();

	if(m_pFormatContext)
	{
		// Assume we are not eof
		if(m_pFormatContext->pb)
			m_pFormatContext->pb->eof_reached = 0;

		// Keep track if ffmpeg doesn't always set these
		pkt.size = 0;
		pkt.data = NULL;
		pkt.stream_index = MAX_STREAMS;

		// Timeout reads after 100ms
		m_timeout = GetTickCount() + 20000;
		int result = 0;
		try
		{
			result = /*m_dllAvFormat.*/av_read_frame(m_pFormatContext, &pkt);
		}
		catch(const win32_exception &e)
		{
			e.writelog(__FUNCTION__);
			result = AVERROR(EFAULT);
		}
		m_timeout = 0;

		if(result == AVERROR(EINTR) || result == AVERROR(EAGAIN))
		{
			// Timeout, probably no real error, return empty packet
			bReturnEmpty = true;
		}
		else if (result < 0)
		{
			Flush();
		}
		else if (pkt.size < 0 || pkt.stream_index >= MAX_STREAMS)
		{
			// XXX, in some cases ffmpeg returns a negative packet size
			if(m_pFormatContext->pb && !m_pFormatContext->pb->eof_reached)
			{
				CLog::Log(LOGERROR, "CDVDDemuxFFmpeg::Read() no valid packet");
				bReturnEmpty = true;
				Flush();
			}
			else
				CLog::Log(LOGERROR, "CDVDDemuxFFmpeg::Read() returned invalid packet and eof reached");

			/*m_dllAvCodec.*/av_free_packet(&pkt);
		}
		else
		{
			AVStream *stream = m_pFormatContext->streams[pkt.stream_index];

			if(m_pFormatContext->nb_programs)
			{
				// Check so packet belongs to selected program
				for(unsigned int i = 0; i < m_pFormatContext->programs[m_program]->nb_stream_indexes; i++)
				{
					if(pkt.stream_index == (int)m_pFormatContext->programs[m_program]->stream_index[i])
					{
						pPacket = CDVDDemuxUtils::AllocateDemuxPacket(pkt.size);
						break;
					}
				}

				if(!pPacket)
					bReturnEmpty = true;
			}
			else
				pPacket = CDVDDemuxUtils::AllocateDemuxPacket(pkt.size);

			if(pPacket)
			{
				// lavf sometimes bugs out and gives 0 dts/pts instead of no dts/pts
				// since this could only happens on initial frame under normal
				// circomstances, let's assume it is wrong all the time
				if(pkt.dts == 0)
					pkt.dts = AV_NOPTS_VALUE;
				
				if(pkt.pts == 0)
					pkt.pts = AV_NOPTS_VALUE;

				if(m_bMatroska && stream->codec && stream->codec->codec_type == CODEC_TYPE_VIDEO)
				{
					// matroska can store different timestamps
					// for different formats, for native stored
					// stuff it is pts, but for ms compatibility
					// tracks, it is really dts. sadly ffmpeg
					// sets these two timestamps equal all the
					// time, so we select it here instead
					if(stream->codec->codec_tag == 0)
						pkt.dts = AV_NOPTS_VALUE;
					else
						pkt.pts = AV_NOPTS_VALUE;
				}

				// We need to get duration slightly different for matroska embedded text subtitels
				if(m_bMatroska && stream->codec->codec_id == CODEC_ID_TEXT && pkt.convergence_duration != 0)
					pkt.duration = (int)pkt.convergence_duration;

				if(m_bAVI && stream->codec && stream->codec->codec_type == CODEC_TYPE_VIDEO)
				{
					// AVI's always have borked pts, specially if m_pFormatContext->flags includes
					// AVFMT_FLAG_GENPTS so always use dts
					pkt.pts = AV_NOPTS_VALUE;
				}

				// Copy contents into our own packet
				pPacket->iSize = pkt.size;

				// Maybe we can avoid a memcpy here by detecting where pkt.destruct is pointing too?
				if(pkt.data)
					memcpy(pPacket->pData, pkt.data, pPacket->iSize);

				pPacket->pts = ConvertTimestamp(pkt.pts, stream->time_base.den, stream->time_base.num);
				pPacket->dts = ConvertTimestamp(pkt.dts, stream->time_base.den, stream->time_base.num);
				pPacket->duration =  DVD_SEC_TO_TIME((double)pkt.duration * stream->time_base.num / stream->time_base.den);

				// Used to guess streamlength
				if(pPacket->dts != DVD_NOPTS_VALUE && (pPacket->dts > m_iCurrentPts || m_iCurrentPts == DVD_NOPTS_VALUE))
					m_iCurrentPts = pPacket->dts;

				// Check if stream has passed full duration, needed for live streams
				if(pkt.dts != (int64_t)AV_NOPTS_VALUE)
				{
					int64_t duration;
					duration = pkt.dts;

					if(stream->start_time != (int64_t)AV_NOPTS_VALUE)
						duration -= stream->start_time;

					if(duration > stream->duration)
					{
						stream->duration = duration;
						duration = /*m_dllAvUtil.*/av_rescale_rnd(stream->duration, stream->time_base.num * AV_TIME_BASE, stream->time_base.den, AV_ROUND_NEAR_INF);
						
						if((m_pFormatContext->duration == (int64_t)AV_NOPTS_VALUE && m_pFormatContext->file_size > 0)
							|| (m_pFormatContext->duration != (int64_t)AV_NOPTS_VALUE && duration > m_pFormatContext->duration))
						m_pFormatContext->duration = duration;
					}
				}

				// Check if stream seem to have grown since start
				if(m_pFormatContext->file_size > 0 && m_pFormatContext->pb)
				{
					if(m_pFormatContext->pb->pos > m_pFormatContext->file_size)
						m_pFormatContext->file_size = m_pFormatContext->pb->pos;
				}

				pPacket->iStreamId = pkt.stream_index; // XXX just for now
			}
			/*m_dllAvCodec.*/av_free_packet(&pkt);
		}
	}
	Unlock();

	if(bReturnEmpty && !pPacket)
		pPacket = CDVDDemuxUtils::AllocateDemuxPacket(0);

  if(!pPacket) return NULL;

	// Check streams, can we make this a bit more simple?
	if(pPacket && pPacket->iStreamId >= 0 && pPacket->iStreamId <= MAX_STREAMS)
	{
		if(!m_streams[pPacket->iStreamId] ||
			m_streams[pPacket->iStreamId]->pPrivate != m_pFormatContext->streams[pPacket->iStreamId] ||
			m_streams[pPacket->iStreamId]->codec != m_pFormatContext->streams[pPacket->iStreamId]->codec->codec_id)
		{
			// Content has changed, or stream did not yet exist
			AddStream(pPacket->iStreamId);
		}
		// We already check for a valid m_streams[pPacket->iStreamId] above
		else if (m_streams[pPacket->iStreamId]->type == STREAM_AUDIO)
		{
			if(((CDemuxStreamAudio*)m_streams[pPacket->iStreamId])->iChannels != m_pFormatContext->streams[pPacket->iStreamId]->codec->channels ||
			((CDemuxStreamAudio*)m_streams[pPacket->iStreamId])->iSampleRate != m_pFormatContext->streams[pPacket->iStreamId]->codec->sample_rate)
			{
				// content has changed
				AddStream(pPacket->iStreamId);
			}
		}
		else if (m_streams[pPacket->iStreamId]->type == STREAM_VIDEO)
		{
			if(((CDemuxStreamVideo*)m_streams[pPacket->iStreamId])->iWidth != m_pFormatContext->streams[pPacket->iStreamId]->codec->width ||
				((CDemuxStreamVideo*)m_streams[pPacket->iStreamId])->iHeight != m_pFormatContext->streams[pPacket->iStreamId]->codec->height)
			{
				// content has changed
				AddStream(pPacket->iStreamId);
			}
		}
	}
	return pPacket;
}

bool CDVDDemuxFFmpeg::SeekTime(int time, bool backwords, double *startpts)
{
	g_demuxer = this;

	if(time < 0)
		time = 0;

	CDVDInputStream::ISeekTime* ist = dynamic_cast<CDVDInputStream::ISeekTime*>(m_pInput);
	if(ist)
	{
		if(!ist->SeekTime(time))
			return false;

		if(startpts)
			*startpts = DVD_NOPTS_VALUE;

		Flush();

		// Also empty the internal ffmpeg buffer
		m_ioContext->buf_ptr = m_ioContext->buf_end;

		return true;
	}

	if(!m_pInput->Seek(0, SEEK_POSSIBLE) && !m_pInput->IsStreamType(DVDSTREAM_TYPE_FFMPEG))
	{
		CLog::Log(LOGDEBUG, "%s - input stream reports it is not seekable", __FUNCTION__);
		return false;
	}

	__int64 seek_pts = (__int64)time * (AV_TIME_BASE / 1000);
	if(m_pFormatContext->start_time != (int64_t)AV_NOPTS_VALUE)
		seek_pts += m_pFormatContext->start_time;

	Lock();

	int ret = /*m_dllAvFormat.*/avformat_seek_file(m_pFormatContext, -1, _I64_MIN, seek_pts, _I64_MAX, backwords ? AVSEEK_FLAG_BACKWARD : 0);

	if(ret >= 0)
		UpdateCurrentPTS();

	Unlock();

	if(m_iCurrentPts == DVD_NOPTS_VALUE)
		CLog::Log(LOGDEBUG, "%s - unknown position after seek", __FUNCTION__);
	else
		CLog::Log(LOGDEBUG, "%s - seek ended up on time %d", __FUNCTION__, (int)(m_iCurrentPts / DVD_TIME_BASE * 1000));

	// In this case the start time is requested time
	if(startpts)
		*startpts = DVD_MSEC_TO_TIME(time);

	// Demuxer will return failure, if you seek to eof
	if(m_pInput->IsEOF() && ret <= 0)
		return true;

	return (ret >= 0);
}

CDemuxStream* CDVDDemuxFFmpeg::GetStream(int iStreamId)
{
	if(iStreamId < 0 || iStreamId >= MAX_STREAMS) return NULL;
	return m_streams[iStreamId];
}

bool CDVDDemuxFFmpeg::Aborted()
{
	if(!m_timeout)
		return false;

	if(GetTickCount() > m_timeout)
		return true;

	return false;
}

int CDVDDemuxFFmpeg::GetNrOfStreams()
{
	int i = 0;
	while(i < MAX_STREAMS && m_streams[i]) i++;
		return i;
}

int CDVDDemuxFFmpeg::GetChapterCount()
{
//	if(m_pInput && m_pInput->IsStreamType(DVDSTREAM_TYPE_DVD))
//		return ((CDVDInputStreamNavigator*)m_pInput)->GetChapterCount();

	if(m_pFormatContext == NULL)
		return 0;

	return m_pFormatContext->nb_chapters;
}

void CDVDDemuxFFmpeg::GetChapterName(std::string& strChapterName)
{
	if(m_pInput && m_pInput->IsStreamType(DVDSTREAM_TYPE_DVD))
		return;
	else 
	{
		int chapterIdx = GetChapter();
		if(chapterIdx > 0 && m_pFormatContext->chapters[chapterIdx-1]->title)
			strChapterName = m_pFormatContext->chapters[chapterIdx-1]->title;
	}
}

int CDVDDemuxFFmpeg::GetChapter()
{
//	if(m_pInput && m_pInput->IsStreamType(DVDSTREAM_TYPE_DVD))
//		return ((CDVDInputStreamNavigator*)m_pInput)->GetChapter();
    
	if(m_pFormatContext == NULL || m_iCurrentPts == DVD_NOPTS_VALUE)
		return 0;

	for(unsigned i = 0; i < m_pFormatContext->nb_chapters; i++)
	{
		AVChapter *chapter = m_pFormatContext->chapters[i];

		if(m_iCurrentPts >= ConvertTimestamp(chapter->start, chapter->time_base.den, chapter->time_base.num)
			&& m_iCurrentPts <  ConvertTimestamp(chapter->end,   chapter->time_base.den, chapter->time_base.num))
			return i + 1;
	}
	return 0;
}