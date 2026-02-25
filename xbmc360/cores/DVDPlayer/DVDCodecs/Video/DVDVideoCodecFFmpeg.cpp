#include "DVDVideoCodecFFmpeg.h"
#include "..\..\DVDDemuxers\DVDDemux.h"
#include "..\..\DVDStreamInfo.h"
#include "..\..\DVDClock.h"
#include "..\..\DVDCodecs/DVDCodecs.h"
#include "utils\Log.h"
#include "AdvancedSettings.h"

// FFMPEG
extern "C" 
{
#include "libswscale\swscale.h"
#include "libavutil\opt.h"
}

#define RINT(x) ((x) >= 0 ? ((int)((x) + 0.5)) : ((int)((x) - 0.5)))

union pts_union
{
	double  pts_d;
	int64_t pts_i;
};

static int64_t pts_dtoi(double pts)
{
	pts_union u;
	u.pts_d = pts;
	return u.pts_i;
}

static double pts_itod(int64_t pts)
{
	pts_union u;
	u.pts_i = pts;
	return u.pts_d;
}

CDVDVideoCodecFFmpeg::CDVDVideoCodecFFmpeg() : CDVDVideoCodec()
{
	m_pCodecContext = NULL;
	m_pConvertFrame = NULL;
	m_pFrame = NULL;

	m_iPictureWidth = 0;
	m_iPictureHeight = 0;

	m_iScreenWidth = 0;
	m_iScreenHeight = 0;
	m_dts = DVD_NOPTS_VALUE;
}

CDVDVideoCodecFFmpeg::~CDVDVideoCodecFFmpeg()
{
	Dispose();
}

bool CDVDVideoCodecFFmpeg::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
	AVCodec* pCodec;

#if 0//ndef _HARDLINKED
	if(!m_dllAvUtil.Load() || !m_dllAvCodec.Load()) return false;
#endif

	avcodec_register_all();

	/* FFmpeg 1.2: use avcodec_alloc_context3 instead of avcodec_alloc_context */
	m_pCodecContext = avcodec_alloc_context3(NULL);

	pCodec = avcodec_find_decoder(hints.codec);
	if(pCodec == NULL)
	{
		CLog::Log(LOGDEBUG,"CDVDVideoCodecFFmpeg::Open() Unable to find codec %d", hints.codec);
		return false;
	}

	CLog::Log(LOGNOTICE,"CDVDVideoCodecFFmpeg::Open() Using codec: %s",pCodec->long_name ? pCodec->long_name : pCodec->name);
	m_pCodecContext->opaque = (void*)this;
	m_pCodecContext->debug_mv = 0;
	m_pCodecContext->debug = 0;
	m_pCodecContext->workaround_bugs = FF_BUG_AUTODETECT;
	m_pCodecContext->codec_tag = hints.codec_tag;

	if(pCodec->capabilities & CODEC_CAP_DR1)
		m_pCodecContext->flags |= CODEC_FLAG_EMU_EDGE;

	// If we don't do this, then some codecs seem to fail.
	m_pCodecContext->coded_height = hints.height;
	m_pCodecContext->coded_width = hints.width;

	if(hints.extradata && hints.extrasize > 0)
	{
		m_pCodecContext->extradata_size = hints.extrasize;
		m_pCodecContext->extradata = (uint8_t*)av_mallocz(hints.extrasize + FF_INPUT_BUFFER_PADDING_SIZE);
		memcpy(m_pCodecContext->extradata, hints.extradata, hints.extrasize);
	}

	/* FFmpeg 1.2: dsp_mask / FF_MM_FORCE removed entirely.
	 * The Xbox 360 PPC DSP functions are registered via ff_dsputil_init_ppc()
	 * and other init functions called automatically during codec init. */

	// Advanced setting override for skip loop filter
	if(g_advancedSettings.m_iSkipLoopFilter != 0)
		m_pCodecContext->skip_loop_filter = (AVDiscard)g_advancedSettings.m_iSkipLoopFilter;
	else
		m_pCodecContext->skip_loop_filter = AVDISCARD_NONREF;

	// Set any special options
	/* FFmpeg 1.2: use av_opt_set instead of av_set_string */
	for(CDVDCodecOptions::iterator it = options.begin(); it != options.end(); it++)
	{
		av_opt_set(m_pCodecContext, it->m_name.c_str(), it->m_value.c_str(), 0);
	}

	/* FFmpeg 1.2: use avcodec_open2 instead of avcodec_open */
	if(avcodec_open2(m_pCodecContext, pCodec, NULL) < 0)
	{
		CLog::Log(LOGDEBUG,"CDVDVideoCodecFFmpeg::Open() Unable to open codec");
		return false;
	}

	/* FFmpeg 1.2: avcodec_alloc_frame is deprecated, use av_frame_alloc if available.
	 * For FFmpeg 1.2 (libavcodec 54.x), avcodec_alloc_frame still exists. */
	m_pFrame = avcodec_alloc_frame();
	if(!m_pFrame)
		return false;

	if(pCodec->name)
		m_name = CStdString("ff-") + pCodec->name;
	else
		m_name = "ffmpeg";

	return true;
}

void CDVDVideoCodecFFmpeg::Dispose()
{
	if(m_pFrame) av_free(m_pFrame);
		m_pFrame = NULL;

	if(m_pConvertFrame)
	{
		avpicture_free(m_pConvertFrame);
		av_free(m_pConvertFrame);
	}

	m_pConvertFrame = NULL;

	if(m_pCodecContext)
	{
		if(m_pCodecContext->codec) avcodec_close(m_pCodecContext);
		if(m_pCodecContext->extradata)
		{
			av_free(m_pCodecContext->extradata);
			m_pCodecContext->extradata = NULL;
			m_pCodecContext->extradata_size = 0;
		}
		av_free(m_pCodecContext);
		m_pCodecContext = NULL;
	}

#if 0//ndef _HARDLINED
	m_dllAvCodec.Unload();
	m_dllAvUtil.Unload();
#endif
}

int CDVDVideoCodecFFmpeg::Decode(BYTE* pData, int iSize, double dts, double pts)
{
	int iGotPicture = 0, len = 0;

	if(!m_pCodecContext)
		return VC_ERROR;

	m_dts = dts;
	m_pCodecContext->reordered_opaque = pts_dtoi(pts);

	/* FFmpeg 1.2: use avcodec_decode_video2 with AVPacket instead of
	 * the old 5-argument avcodec_decode_video */
	AVPacket avpkt;
	av_init_packet(&avpkt);
	avpkt.data = pData;
	avpkt.size = iSize;
	avpkt.dts  = AV_NOPTS_VALUE;
	avpkt.pts  = AV_NOPTS_VALUE;

	len = avcodec_decode_video2(m_pCodecContext, m_pFrame, &iGotPicture, &avpkt);

	if(len < 0)
	{
		CLog::Log(LOGERROR, "%s - avcodec_decode_video2 returned failure", __FUNCTION__);
		return VC_ERROR;
	}

	/* FFmpeg 1.2: hurry_up field removed. Use skip_frame for frame dropping instead.
	 * The SetDropState() method already handles this via skip_frame/skip_idct. */
	if(len != iSize)
		CLog::Log(LOGWARNING, "%s - avcodec_decode_video2 didn't consume the full packet. size: %d, consumed: %d", __FUNCTION__, iSize, len);

	if(!iGotPicture)
		return VC_BUFFER;

	/* FFmpeg 1.2: AV_PIX_FMT_* replaces PIX_FMT_* (old names still work as aliases) */
	if(m_pCodecContext->pix_fmt != AV_PIX_FMT_YUV420P
	&& m_pCodecContext->pix_fmt != AV_PIX_FMT_YUVJ420P)
	{
#if 0//ndef _HARDLINKED
		if(!m_dllSwScale.IsLoaded())
		{
			if(!m_dllSwScale.Load())
				return VC_ERROR;
		}
#endif
		if(!m_pConvertFrame)
		{
			// Allocate an AVFrame structure
			m_pConvertFrame = (AVPicture*)av_mallocz(sizeof(AVPicture));

			// Due to a bug in swscale we need to allocate one extra line of data
			if(avpicture_alloc( m_pConvertFrame
			   , AV_PIX_FMT_YUV420P
			   , m_pCodecContext->width
			   , m_pCodecContext->height+1) < 0)
			{
				av_free(m_pConvertFrame);
				m_pConvertFrame = NULL;
				return VC_ERROR;
			}
		}

		// Convert the picture
		struct SwsContext *context = sws_getContext(m_pCodecContext->width, m_pCodecContext->height,
                                         m_pCodecContext->pix_fmt, m_pCodecContext->width, m_pCodecContext->height,
                                         AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		if(context == NULL)
		{
			CLog::Log(LOGERROR, "CDVDVideoCodecFFmpeg::Decode - unable to obtain sws context for w:%i, h:%i, pixfmt: %i", 
				      m_pCodecContext->width, m_pCodecContext->height, m_pCodecContext->pix_fmt);

			return VC_ERROR;
		}

		sws_scale(context
                          , m_pFrame->data
                          , m_pFrame->linesize
                          , 0
                          , m_pCodecContext->height
                          , m_pConvertFrame->data
                          , m_pConvertFrame->linesize);

		sws_freeContext(context);
	}
	else
	{
		// No need to convert, just free any existing convert buffers
		if(m_pConvertFrame)
		{
			avpicture_free(m_pConvertFrame);
			av_free(m_pConvertFrame);
			m_pConvertFrame = NULL;
		}
	}

	int result = VC_PICTURE | VC_BUFFER;

	if(result & VC_FLUSHED)
		Reset();

	return result;
}

void CDVDVideoCodecFFmpeg::Reset()
{
	avcodec_flush_buffers(m_pCodecContext);

	if(m_pConvertFrame)
	{
		delete[] m_pConvertFrame->data[0];
		av_free(m_pConvertFrame);
		m_pConvertFrame = NULL;
	}
}

bool CDVDVideoCodecFFmpeg::GetPictureCommon(DVDVideoPicture* pDvdVideoPicture)
{
	GetVideoAspect(m_pCodecContext, pDvdVideoPicture->iDisplayWidth, pDvdVideoPicture->iDisplayHeight);

	if(m_pCodecContext->coded_width && m_pCodecContext->coded_width < m_pCodecContext->width
	   && m_pCodecContext->coded_width  > m_pCodecContext->width  - 10)
		pDvdVideoPicture->iWidth = m_pCodecContext->coded_width;
	else
		pDvdVideoPicture->iWidth = m_pCodecContext->width;

	if(m_pCodecContext->coded_height && m_pCodecContext->coded_height < m_pCodecContext->height
	   && m_pCodecContext->coded_height > m_pCodecContext->height - 10)
		pDvdVideoPicture->iHeight = m_pCodecContext->coded_height;
	else
		pDvdVideoPicture->iHeight = m_pCodecContext->height;

	pDvdVideoPicture->pts = DVD_NOPTS_VALUE;

	if(!m_pFrame)
		return false;

	pDvdVideoPicture->iRepeatPicture = 0.5 * m_pFrame->repeat_pict;
	pDvdVideoPicture->iFlags = DVP_FLAG_ALLOCATED;
	pDvdVideoPicture->iFlags |= m_pFrame->interlaced_frame ? DVP_FLAG_INTERLACED : 0;
	pDvdVideoPicture->iFlags |= m_pFrame->top_field_first ? DVP_FLAG_TOP_FIELD_FIRST: 0;

	if(m_pCodecContext->pix_fmt == AV_PIX_FMT_YUVJ420P)
		pDvdVideoPicture->color_range = 1;

	pDvdVideoPicture->qscale_table = m_pFrame->qscale_table;
	pDvdVideoPicture->qscale_stride = m_pFrame->qstride;

	switch(m_pFrame->qscale_type)
	{
		case FF_QSCALE_TYPE_MPEG1:
		pDvdVideoPicture->qscale_type = DVP_QSCALE_MPEG1;
		break;

		case FF_QSCALE_TYPE_MPEG2:
		pDvdVideoPicture->qscale_type = DVP_QSCALE_MPEG2;
		break;

		case FF_QSCALE_TYPE_H264:
		pDvdVideoPicture->qscale_type = DVP_QSCALE_H264;
		break;

		default:
		pDvdVideoPicture->qscale_type = DVP_QSCALE_UNKNOWN;
	}

	pDvdVideoPicture->dts = m_dts;
	m_dts = DVD_NOPTS_VALUE;

	if(m_pFrame->reordered_opaque)
		pDvdVideoPicture->pts = pts_itod(m_pFrame->reordered_opaque);
	else
		pDvdVideoPicture->pts = DVD_NOPTS_VALUE;

	return true;
}

bool CDVDVideoCodecFFmpeg::GetPicture(DVDVideoPicture* pDvdVideoPicture)
{
	if(!GetPictureCommon(pDvdVideoPicture))
		return false;

	if(m_pConvertFrame)
	{
		for(int i = 0; i < 4; i++)
			pDvdVideoPicture->data[i]      = m_pConvertFrame->data[i];
		for(int i = 0; i < 4; i++)
			pDvdVideoPicture->iLineSize[i] = m_pConvertFrame->linesize[i];
	}
	else
	{
		for(int i = 0; i < 4; i++)
			pDvdVideoPicture->data[i]      = m_pFrame->data[i];
		for(int i = 0; i < 4; i++)
			pDvdVideoPicture->iLineSize[i] = m_pFrame->linesize[i];
	}

	pDvdVideoPicture->iFlags |= pDvdVideoPicture->data[0] ? 0 : DVP_FLAG_DROPPED;
	pDvdVideoPicture->format = DVDVideoPicture::FMT_YUV420P;

	return true;
}

void CDVDVideoCodecFFmpeg::SetDropState(bool bDrop)
{
	if(m_pCodecContext)
	{
		/* FFmpeg 1.2: hurry_up removed. skip_frame/skip_idct are the proper way. */
		if(bDrop)
		{
			m_pCodecContext->skip_frame = AVDISCARD_NONREF;
			m_pCodecContext->skip_idct = AVDISCARD_NONREF;
		}
		else
		{
			m_pCodecContext->skip_frame = AVDISCARD_DEFAULT;
			m_pCodecContext->skip_idct = AVDISCARD_DEFAULT;
		}
	}
}

/*
 * Calculate the height and width this video should be displayed in
 */
void CDVDVideoCodecFFmpeg::GetVideoAspect(AVCodecContext* pCodecContext, unsigned int& iWidth, unsigned int& iHeight)
{
	double aspect_ratio;

	// XXX: use variable in the frame
	if(pCodecContext->sample_aspect_ratio.num == 0)
		aspect_ratio = 0;
	else
		aspect_ratio = av_q2d(pCodecContext->sample_aspect_ratio) * pCodecContext->width / pCodecContext->height;

	if(aspect_ratio <= 0.0) aspect_ratio = (float)pCodecContext->width / (float)pCodecContext->height;

	// XXX: We suppose the screen has a 1.0 pixel ratio - CDVDVideo will compensate it
	iHeight = pCodecContext->height;
	iWidth = ((int)RINT(pCodecContext->height * aspect_ratio)) & -3;
	
	if(iWidth > (unsigned int)pCodecContext->width)
	{
		iWidth = pCodecContext->width;
		iHeight = ((int)RINT(pCodecContext->width / aspect_ratio)) & -3;
	}
}