#include "DVDAudioCodecFFmpeg.h"
#include "..\..\..\utils\log.h"

CDVDAudioCodecFFmpeg::CDVDAudioCodecFFmpeg() : CDVDAudioCodec()
{
	m_iBufferSize = 0;
	m_pCodecContext = NULL;
	m_bOpenedCodec = false;
}

CDVDAudioCodecFFmpeg::~CDVDAudioCodecFFmpeg()
{
	Dispose();
}

bool CDVDAudioCodecFFmpeg::Open(CodecID codecID, int iChannels, int iSampleRate)
{
	AVCodec* pCodec;
	m_bOpenedCodec = false;

	av_register_all();

	pCodec = avcodec_find_decoder(codecID);

	m_pCodecContext = avcodec_alloc_context3(pCodec);//avcodec_alloc_context();
	avcodec_get_context_defaults3(m_pCodecContext, pCodec);

	if (!pCodec)
	{
		CLog::Log(LOGERROR, "CDVDAudioCodecFFmpeg::Open() Unable to find codec");
		return false;
	}

	m_pCodecContext->debug_mv = 0;
	m_pCodecContext->debug = 0;
	m_pCodecContext->workaround_bugs = 1;

	if (pCodec->capabilities & CODEC_CAP_TRUNCATED)
		m_pCodecContext->flags |= CODEC_FLAG_TRUNCATED;

	m_pCodecContext->channels = iChannels;
	m_pCodecContext->sample_rate = iSampleRate;
	//m_pCodecContext->bits_per_sample = 24;
 
/* //FIXME Marty
  if( ExtraData && ExtraSize > 0 )
  {
    m_pCodecContext->extradata_size = ExtraSize;
    m_pCodecContext->extradata = m_dllAvCodec.av_mallocz(ExtraSize + FF_INPUT_BUFFER_PADDING_SIZE);
    memcpy(m_pCodecContext->extradata, ExtraData, ExtraSize);
  }
*/
	// set acceleration
	//m_pCodecContext->dsp_mask = FF_MM_FORCE | FF_MM_MMX | FF_MM_MMXEXT | FF_MM_SSE; //Marty

	if (avcodec_open(m_pCodecContext, pCodec) < 0)
	{
		CLog::Log(LOGERROR, "CDVDAudioCodecFFmpeg::Open() Unable to open codec");
		Dispose();
		return false;
	}

	m_bOpenedCodec = true;
	return true;
}

void CDVDAudioCodecFFmpeg::Dispose()
{
	if (m_pCodecContext)
	{
		if (m_bOpenedCodec) avcodec_close(m_pCodecContext);
		m_bOpenedCodec = false;
		av_free(m_pCodecContext);
		m_pCodecContext = NULL;
	}
	m_iBufferSize = 0;
}

int CDVDAudioCodecFFmpeg::Decode(BYTE* pData, int iSize)
{
	int iBytesUsed;
	if (!m_pCodecContext) return -1;

	//Copy into a FFMpeg AVPAcket again
	AVPacket packet;
    av_init_packet(&packet);

    packet.data=pData;
    packet.size=iSize;

	int iOutputSize = AVCODEC_MAX_AUDIO_FRAME_SIZE; //Marty

	//avcodec_decode_audio2( data->pAudioCodecCtx, (int16_t *)m_AudioBytes, &done, data->packet.data, data->packet.size	)

	iBytesUsed = avcodec_decode_audio2(m_pCodecContext, (int16_t *)m_buffer, &iOutputSize/*m_iBufferSize*/, packet.data, packet.size );

	m_iBufferSize = iOutputSize;//Marty

	return iBytesUsed;
}

int CDVDAudioCodecFFmpeg::GetData(BYTE** dst)
{
	*dst = m_buffer;
	return m_iBufferSize;
}

void CDVDAudioCodecFFmpeg::Reset()
{
	if (m_pCodecContext) avcodec_flush_buffers(m_pCodecContext);
}

int CDVDAudioCodecFFmpeg::GetChannels()
{
	if (m_pCodecContext) return m_pCodecContext->channels;
	return 0;
}

int CDVDAudioCodecFFmpeg::GetSampleRate()
{
	if (m_pCodecContext) return m_pCodecContext->sample_rate;
	return 0;
}

int CDVDAudioCodecFFmpeg::GetBitsPerSample()
{
	if (m_pCodecContext) return 16;
	return 0;
}
