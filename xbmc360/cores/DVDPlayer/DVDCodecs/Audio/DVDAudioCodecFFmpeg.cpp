#include "Utils\Log.h"
#include "DVDAudioCodecFFmpeg.h"
#include "GUISettings.h"
#include "utils\stdafx.h"
#include <malloc.h>
#include <stdio.h>

CDVDAudioCodecFFmpeg::CDVDAudioCodecFFmpeg() : CDVDAudioCodec()
{
	m_iBufferSize2 = 0;
	m_pBuffer2     = (BYTE*)_aligned_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE, 16);
	memset(m_pBuffer2, 0, AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);

	m_iBuffered = 0;
	m_pCodecContext = NULL;
	m_pConvert = NULL;
	m_bOpenedCodec = false;
	m_pFrame1 = NULL;
	m_iSampleFormat = AV_SAMPLE_FMT_NONE;
}

CDVDAudioCodecFFmpeg::~CDVDAudioCodecFFmpeg()
{
	_aligned_free(m_pBuffer2);
	Dispose();
}

bool CDVDAudioCodecFFmpeg::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
	AVCodec* pCodec;
	m_bOpenedCodec = false;

	avcodec_register_all();

	pCodec = avcodec_find_decoder(hints.codec);
	
	if (!pCodec)
	{
		CLog::Log(LOGDEBUG,"CDVDAudioCodecFFmpeg::Open() Unable to find codec %d", hints.codec);
		return false;
	}

	m_pCodecContext = avcodec_alloc_context3(pCodec);
	m_pCodecContext->debug_mv = 0;
	m_pCodecContext->debug = 0;
	m_pCodecContext->workaround_bugs = 1;

	if (pCodec->capabilities & CODEC_CAP_TRUNCATED)
		m_pCodecContext->flags |= CODEC_FLAG_TRUNCATED;

	m_pCodecContext->channels = hints.channels;
	m_pCodecContext->sample_rate = hints.samplerate;
	m_pCodecContext->block_align = hints.blockalign;
	m_pCodecContext->bit_rate = hints.bitrate;
	m_pCodecContext->bits_per_coded_sample = hints.bitspersample;

	if(m_pCodecContext->bits_per_coded_sample == 0)
		m_pCodecContext->bits_per_coded_sample = 16;
 
	// If we need to downmix, do it in ffmpeg as codecs are smarter then we can ever be
	if(g_guiSettings.GetBool("audiooutput.downmixmultichannel"))
	{
		m_pCodecContext->request_channel_layout = AV_CH_LAYOUT_STEREO;
		m_pCodecContext->request_channels       = 2;
	}

	if( hints.extradata && hints.extrasize > 0 )
	{
		m_pCodecContext->extradata = (uint8_t*)av_mallocz(hints.extrasize + FF_INPUT_BUFFER_PADDING_SIZE);
		
		if(m_pCodecContext->extradata)
		{
			m_pCodecContext->extradata_size = hints.extrasize;
			memcpy(m_pCodecContext->extradata, hints.extradata, hints.extrasize);
		}
	}

	m_pCodecContext->request_sample_fmt = AV_SAMPLE_FMT_S16;

	if (avcodec_open2(m_pCodecContext, pCodec, NULL) < 0)
	{
		CLog::Log(LOGDEBUG,"CDVDAudioCodecFFmpeg::Open() Unable to open codec");
		Dispose();
		return false;
	}
  
	m_pFrame1 = avcodec_alloc_frame();
	m_bOpenedCodec = true;
	m_iSampleFormat = AV_SAMPLE_FMT_NONE;
	
	return true;
}

void CDVDAudioCodecFFmpeg::Dispose()
{
	if (m_pFrame1) av_free(m_pFrame1);
	m_pFrame1 = NULL;

	if (m_pConvert)
		swr_free(&m_pConvert);

	if (m_pCodecContext)
	{
		if (m_bOpenedCodec) avcodec_close(m_pCodecContext);
		m_bOpenedCodec = false;
		av_free(m_pCodecContext);
		m_pCodecContext = NULL;
	}

	m_iBufferSize1 = 0;
	m_iBufferSize2 = 0;
	m_iBuffered = 0;
}

int CDVDAudioCodecFFmpeg::Decode(BYTE* pData, int iSize)
{
	int iBytesUsed, got_frame;
	if (!m_pCodecContext) return -1;

	m_iBufferSize1 = AVCODEC_MAX_AUDIO_FRAME_SIZE ;
	m_iBufferSize2 = 0;

	AVPacket avpkt;
	av_init_packet(&avpkt);
	avpkt.data = pData;
	avpkt.size = iSize;
	iBytesUsed = avcodec_decode_audio4( m_pCodecContext
										, m_pFrame1
										, &got_frame
										, &avpkt);
	if (iBytesUsed < 0 || !got_frame)
	{
		m_iBufferSize1 = 0;
		m_iBufferSize2 = 0;
		return iBytesUsed;
	}
	m_iBufferSize1 = av_samples_get_buffer_size(NULL, m_pCodecContext->channels, m_pFrame1->nb_samples, m_pCodecContext->sample_fmt, 1);

	/* Some codecs will attempt to consume more data than what we gave */
	if (iBytesUsed > iSize)
	{
		CLog::Log(LOGWARNING, "CDVDAudioCodecFFmpeg::Decode - decoder attempted to consume more data than given");
		iBytesUsed = iSize;
	}

	if(m_iBufferSize1 == 0 && iBytesUsed >= 0)
		m_iBuffered += iBytesUsed;
	else
		m_iBuffered = 0;

	if(m_pCodecContext->sample_fmt != AV_SAMPLE_FMT_S16 && m_iBufferSize1 > 0)
	{
		if(m_pConvert && m_pCodecContext->sample_fmt != m_iSampleFormat)
			swr_free(&m_pConvert);

		if(!m_pConvert)
		{
			m_iSampleFormat = m_pCodecContext->sample_fmt;
			m_pConvert = swr_alloc_set_opts(NULL,
						av_get_default_channel_layout(m_pCodecContext->channels), AV_SAMPLE_FMT_S16, m_pCodecContext->sample_rate,
						av_get_default_channel_layout(m_pCodecContext->channels), m_pCodecContext->sample_fmt, m_pCodecContext->sample_rate,
						0, NULL);
			
			if(!m_pConvert || swr_init(m_pConvert) < 0)
			{
				CLog::Log(LOGERROR, "CDVDAudioCodecFFmpeg::Decode - Unable to convert %d to AV_SAMPLE_FMT_S16", m_pCodecContext->sample_fmt);
				m_iBufferSize1 = 0;
				m_iBufferSize2 = 0;
				return iBytesUsed;
			}
		}

		int len = m_iBufferSize1 / av_get_bytes_per_sample(m_pCodecContext->sample_fmt);
		
		if(swr_convert(m_pConvert, &m_pBuffer2, len, (const uint8_t**)m_pFrame1->data, m_pFrame1->nb_samples) < 0)
		{
			CLog::Log(LOGERROR, "CDVDAudioCodecFFmpeg::Decode - Unable to convert %d to AV_SAMPLE_FMT_S16", (int)m_pCodecContext->sample_fmt);
			m_iBufferSize1 = 0;
			m_iBufferSize2 = 0;
			return iBytesUsed;
		}

		m_iBufferSize1 = 0;
		m_iBufferSize2 = len * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
	}

	return iBytesUsed;
}

int CDVDAudioCodecFFmpeg::GetData(BYTE** dst)
{
	if(m_iBufferSize1)
	{
		*dst = m_pFrame1->data[0];
		return m_iBufferSize1;
	}

	if(m_iBufferSize2)
	{
		*dst = m_pBuffer2;
		return m_iBufferSize2;
	}
	
	return 0;
}

void CDVDAudioCodecFFmpeg::Reset()
{
	if(m_pCodecContext) avcodec_flush_buffers(m_pCodecContext);
	m_iBufferSize1 = 0;
	m_iBufferSize2 = 0;
	m_iBuffered = 0;
}

int CDVDAudioCodecFFmpeg::GetChannels()
{
	if(m_pCodecContext) return m_pCodecContext->channels;
	return 0;
}

int CDVDAudioCodecFFmpeg::GetSampleRate()
{
	if(m_pCodecContext) return m_pCodecContext->sample_rate;
	return 0;
}

int CDVDAudioCodecFFmpeg::GetBitsPerSample()
{
	return 16;
}

int CDVDAudioCodecFFmpeg::GetBitRate()
{
	if (m_pCodecContext)
		return m_pCodecContext->bit_rate;
		
	return 0;
}