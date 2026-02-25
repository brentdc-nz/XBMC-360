#include "DVDAudioCodecFFmpeg.h"
#include "Utils\Log.h"
#include "utils\stdafx.h"
#include <malloc.h>
#include <stdio.h>

/* Maximum buffer size for decoded audio. This replaces AVCODEC_MAX_AUDIO_FRAME_SIZE
 * which was removed in FFmpeg 1.x. 192000 is a safe upper bound for
 * 48kHz * 2ch * 16bit * some overhead. */
#define AUDIO_DECODE_BUFFER_SIZE 192000

CDVDAudioCodecFFmpeg::CDVDAudioCodecFFmpeg() : CDVDAudioCodec()
{
	m_iBufferSize1 = 0;
	m_pBuffer1     = (BYTE*)_aligned_malloc(AUDIO_DECODE_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE, 16);
	memset(m_pBuffer1, 0, AUDIO_DECODE_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);

	m_iBufferSize2 = 0;
	m_pBuffer2     = (BYTE*)_aligned_malloc(AUDIO_DECODE_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE, 16);
	memset(m_pBuffer2, 0, AUDIO_DECODE_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);

	m_iBuffered = 0;
	m_iOutputSize = 0;
	m_pCodecContext = NULL;
	m_pFrame = NULL;
	m_pConvert = NULL;
	m_bOpenedCodec = false;
	m_iLastSampleFormat = AV_SAMPLE_FMT_NONE;
	m_iLastChannels = 0;
	m_iLastSampleRate = 0;
}

CDVDAudioCodecFFmpeg::~CDVDAudioCodecFFmpeg()
{
	_aligned_free(m_pBuffer1);
	_aligned_free(m_pBuffer2);
	Dispose();
}

bool CDVDAudioCodecFFmpeg::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
	AVCodec* pCodec;
	m_bOpenedCodec = false;

#if 0//ndef _HARDLINKED
	if(!m_dllAvUtil.Load() || !m_dllAvCodec.Load()) return false;
#endif

	avcodec_register_all();

	/* FFmpeg 1.2: avcodec_alloc_context3 replaces avcodec_alloc_context + avcodec_get_context_defaults */
	m_pCodecContext = avcodec_alloc_context3(NULL);

	pCodec = avcodec_find_decoder(hints.codec);
	if(!pCodec)
	{
		CLog::Log(LOGDEBUG,"CDVDAudioCodecFFmpeg::Open() Unable to find codec %d", hints.codec);
		return false;
	}

	m_pCodecContext->debug_mv = 0;
	m_pCodecContext->debug = 0;
	m_pCodecContext->workaround_bugs = 1;

	if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
		m_pCodecContext->flags |= CODEC_FLAG_TRUNCATED;

	m_pCodecContext->channels = hints.channels;
	m_pCodecContext->sample_rate = hints.samplerate;
	m_pCodecContext->block_align = hints.blockalign;
	m_pCodecContext->bit_rate = hints.bitrate;
	m_pCodecContext->bits_per_coded_sample = hints.bitspersample;

	if(m_pCodecContext->bits_per_coded_sample == 0)
		m_pCodecContext->bits_per_coded_sample = 16;

	if(hints.extradata && hints.extrasize > 0)
	{
		m_pCodecContext->extradata_size = hints.extrasize;
		m_pCodecContext->extradata = (uint8_t*)av_mallocz(hints.extrasize + FF_INPUT_BUFFER_PADDING_SIZE);
		memcpy(m_pCodecContext->extradata, hints.extradata, hints.extrasize);
	}

	/* FFmpeg 1.2: dsp_mask / FF_MM_FORCE removed entirely.
	 * Xbox 360 PPC DSP is registered automatically. */

	/* FFmpeg 1.2: use avcodec_open2 */
	if(avcodec_open2(m_pCodecContext, pCodec, NULL) < 0)
	{
		CLog::Log(LOGDEBUG,"CDVDAudioCodecFFmpeg::Open() Unable to open codec");
		Dispose();
		return false;
	}

	/* Allocate the AVFrame for decoded audio output */
	m_pFrame = avcodec_alloc_frame();
	if(!m_pFrame)
	{
		CLog::Log(LOGERROR,"CDVDAudioCodecFFmpeg::Open() Unable to allocate AVFrame");
		Dispose();
		return false;
	}

	m_bOpenedCodec = true;
	m_iSampleFormat = AV_SAMPLE_FMT_NONE;

	return true;
}

void CDVDAudioCodecFFmpeg::Dispose()
{
	if(m_pConvert)
	{
		swr_free(&m_pConvert);
		m_pConvert = NULL;
	}

	if(m_pFrame)
	{
		av_free(m_pFrame);
		m_pFrame = NULL;
	}

	if(m_pCodecContext)
	{
		if(m_bOpenedCodec) avcodec_close(m_pCodecContext);
		m_bOpenedCodec = false;
		av_free(m_pCodecContext);
		m_pCodecContext = NULL;
	}

#if 0//ndef _HARDLINKED
	m_dllAvCodec.Unload();
	m_dllAvUtil.Unload();
#endif

	m_iBufferSize1 = 0;
	m_iBufferSize2 = 0;
	m_iBuffered = 0;
	m_iOutputSize = 0;
}

int CDVDAudioCodecFFmpeg::Decode(BYTE* pData, int iSize)
{
	int iBytesUsed;
	int iGotFrame = 0;
	if(!m_pCodecContext) return -1;

	m_iBufferSize1 = 0;
	m_iBufferSize2 = 0;
	m_iOutputSize = 0;

	/* FFmpeg 1.2: use avcodec_decode_audio4 with AVPacket input and AVFrame output
	 * instead of avcodec_decode_audio2 with raw buffer. */
	AVPacket avpkt;
	av_init_packet(&avpkt);
	avpkt.data = pData;
	avpkt.size = iSize;

	avcodec_get_frame_defaults(m_pFrame);

	iBytesUsed = avcodec_decode_audio4(m_pCodecContext, m_pFrame, &iGotFrame, &avpkt);

	// Some codecs will attempt to consume more data than what we gave
	if(iBytesUsed > iSize)
	{
		CLog::Log(LOGWARNING, "CDVDAudioCodecFFmpeg::Decode - decoder attempted to consume more data than given");
		iBytesUsed = iSize;
	}

	if(iGotFrame)
	{
		/* Calculate output size from AVFrame */
		int data_size = av_samples_get_buffer_size(
			NULL,
			m_pCodecContext->channels,
			m_pFrame->nb_samples,
			m_pCodecContext->sample_fmt,
			1);

		if(data_size > 0 && data_size <= AUDIO_DECODE_BUFFER_SIZE)
		{
			/* For planar formats, we need to interleave the channels.
			 * For packed formats (S16, etc.), data is already interleaved in data[0]. */
			if(av_sample_fmt_is_planar(m_pCodecContext->sample_fmt))
			{
				/* Interleave planar data into m_pBuffer1 */
				int bps = av_get_bytes_per_sample(m_pCodecContext->sample_fmt);
				int ch;
				int nb_samples = m_pFrame->nb_samples;
				int channels = m_pCodecContext->channels;
				BYTE *dst = m_pBuffer1;

				for(int s = 0; s < nb_samples; s++)
				{
					for(ch = 0; ch < channels; ch++)
					{
						memcpy(dst, m_pFrame->extended_data[ch] + s * bps, bps);
						dst += bps;
					}
				}
				m_iBufferSize1 = nb_samples * channels * bps;
			}
			else
			{
				/* Packed format: copy directly from AVFrame */
				memcpy(m_pBuffer1, m_pFrame->data[0], data_size);
				m_iBufferSize1 = data_size;
			}
			m_iOutputSize = m_iBufferSize1;
		}
	}

	if(m_iBufferSize1 == 0 && iBytesUsed >= 0)
		m_iBuffered += iBytesUsed;
	else
		m_iBuffered = 0;

	/* FFmpeg 1.2: Use libswresample to convert non-S16 sample formats to S16 interleaved.
	 * This handles float, double, planar formats, etc. - same approach as xbmc4xbox. */
	if(m_iBufferSize1 > 0 && (m_pCodecContext->sample_fmt != AV_SAMPLE_FMT_S16 ||
	   m_iLastSampleFormat != m_pCodecContext->sample_fmt ||
	   m_iLastChannels != m_pCodecContext->channels ||
	   m_iLastSampleRate != m_pCodecContext->sample_rate))
	{
		/* (Re)create the SwrContext if the source format/channels/rate changed */
		if(m_pCodecContext->sample_fmt != m_iLastSampleFormat ||
		   m_pCodecContext->channels != m_iLastChannels ||
		   m_pCodecContext->sample_rate != m_iLastSampleRate)
		{
			if(m_pConvert)
			{
				swr_free(&m_pConvert);
				m_pConvert = NULL;
			}

			int64_t ch_layout = m_pCodecContext->channel_layout;
			if(!ch_layout)
				ch_layout = av_get_default_channel_layout(m_pCodecContext->channels);

			m_pConvert = swr_alloc_set_opts(NULL,
				ch_layout, AV_SAMPLE_FMT_S16, m_pCodecContext->sample_rate,
				ch_layout, m_pCodecContext->sample_fmt, m_pCodecContext->sample_rate,
				0, NULL);

			if(!m_pConvert || swr_init(m_pConvert) < 0)
			{
				CLog::Log(LOGERROR, "CDVDAudioCodecFFmpeg::Decode - Unable to initialise swresample context for format %d",
				          m_pCodecContext->sample_fmt);
				if(m_pConvert)
				{
					swr_free(&m_pConvert);
					m_pConvert = NULL;
				}
				m_iBufferSize1 = 0;
				m_iBufferSize2 = 0;
			}
			else
			{
				CLog::Log(LOGDEBUG, "CDVDAudioCodecFFmpeg::Decode - Created swresample context: fmt %d -> S16, %d channels, %d Hz",
				          m_pCodecContext->sample_fmt, m_pCodecContext->channels, m_pCodecContext->sample_rate);
			}

			m_iLastSampleFormat = m_pCodecContext->sample_fmt;
			m_iLastChannels = m_pCodecContext->channels;
			m_iLastSampleRate = m_pCodecContext->sample_rate;
		}

		/* Perform the conversion if we need it (non-S16 format) */
		if(m_pConvert && m_pCodecContext->sample_fmt != AV_SAMPLE_FMT_S16 && m_iBufferSize1 > 0)
		{
			int nb_samples = m_pFrame->nb_samples;
			int channels = m_pCodecContext->channels;

			/* Calculate maximum output size and ensure it fits in buffer */
			int out_size = nb_samples * channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
			if(out_size > AUDIO_DECODE_BUFFER_SIZE)
			{
				CLog::Log(LOGWARNING, "CDVDAudioCodecFFmpeg::Decode - converted output exceeds buffer size");
				m_iBufferSize1 = 0;
				m_iBufferSize2 = 0;
			}
			else
			{
				uint8_t *dst_data = m_pBuffer2;
				const uint8_t **src_data = (const uint8_t**)m_pFrame->extended_data;

				int ret = swr_convert(m_pConvert, &dst_data, nb_samples, src_data, nb_samples);
				if(ret < 0)
				{
					CLog::Log(LOGERROR, "CDVDAudioCodecFFmpeg::Decode - swr_convert failed");
					m_iBufferSize1 = 0;
					m_iBufferSize2 = 0;
				}
				else
				{
					/* Swap: conversion output is in m_pBuffer2, copy it to m_pBuffer1 */
					int converted_size = ret * channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
					memcpy(m_pBuffer1, m_pBuffer2, converted_size);
					m_iBufferSize1 = converted_size;
					m_iBufferSize2 = 0;
					m_iOutputSize = converted_size;
				}
			}
		}
	}

	return iBytesUsed;
}

int CDVDAudioCodecFFmpeg::GetData(BYTE** dst)
{
	if(m_iBufferSize1)
	{
		*dst = m_pBuffer1;
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
	m_iOutputSize = 0;
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
